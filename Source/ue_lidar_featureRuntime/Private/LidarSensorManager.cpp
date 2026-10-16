/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "LidarSensorManager.h"

// Sets default values
ALidarSensorManager::ALidarSensorManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AcceptChannel(GemstarInterfaceType::SENSORVIEW,
		[this](FDataTarget& Target, const FString& Producer)
		{
			for (AActor* LidarActor : SpawnedActors)
			{
				if (!IsValid(LidarActor) || LidarActor->GetName() != Producer) { continue; }
				ASimpleLidar* SimpleLidar = Cast<ASimpleLidar>(LidarActor);
				if (SimpleLidar == nullptr) { return false; }
				SimpleLidar->AddDataTarget(Target);
				Target.Actor->AddTickPrerequisiteActor(LidarActor);
				return true;
			}
			return false;
		},
		[this](FDataTarget& Target, const FString&)
		{
			bool bRemoved = false;
			for (AActor* LidarActor : SpawnedActors)
			{
				if (ASimpleLidar* SimpleLidar = Cast<ASimpleLidar>(LidarActor))
				{
					SimpleLidar->RemoveDataTarget(Target);
					Target.Actor->RemoveTickPrerequisiteActor(SimpleLidar);
					bRemoved = true;
				}
			}
			return bRemoved;
		});
}

// Called when the game starts or when spawned
void ALidarSensorManager::BeginPlay()
{
	Super::BeginPlay();

}

void ALidarSensorManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GemstarSimulationSubsystem)){
		GemstarSimulationSubsystem->UnregisterManager(this);
		GemstarSimulationSubsystem = nullptr;
	}

	for (int32 i = SpawnedActors.Num() - 1; i >= 0; --i){
		AActor* Sensor = SpawnedActors[i];
		
		if (!IsValid(Sensor)) {
			continue;
		}

		RemoveTickPrerequisiteActor(Sensor);

		Sensor->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
		Sensor->Destroy();
	}

	SpawnedActors.Reset();
	SensorConfiguration.Reset();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ALidarSensorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ALidarSensorManager::Configure(TMap<FString, FInnerConfig> Configurations, UGemstarSimulationSubsystem* SimulationSubsystem) {
	GemstarSimulationSubsystem = SimulationSubsystem;

	for (const TPair<FString, FInnerConfig>& Pair : Configurations)
	{
		FSensorConfigData config;
		config.ID = FName(Pair.Key);
		if (const FString* ValueStr = Pair.Value.Entries.Find("Mount")) {
			config.Mount = *ValueStr;
		}
		double x = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("MountingPositionX"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, x);
		}
		double y = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("MountingPositionY"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, y);
		}
		double z = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("MountingPositionZ"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, z);
		}
		config.Offset.SetLocation(FVector(x, y, z));

		double roll = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("MountingOrientationRoll"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, roll);
		}
		double pitch = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("MountingOrientationPitch"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, pitch);
		}
		double yaw = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("MountingOrientationYaw"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, yaw);
		}
		config.Offset.SetRotation(FRotator(pitch, yaw, roll).Quaternion());

		double width = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("NumberOfRaysHorizontal"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, width);
		}
		double height = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("NumberOfRaysVertical"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, height);
		}
		double FoVHorizontal = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("FoVHorizontal"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, FoVHorizontal);
		}
		double FoVVertical = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("FoVVertical"))
		{
			FDefaultValueHelper::ParseDouble(*ValueStr, FoVVertical);
		}
		float distanceHalfFalloff = 0.0;
		if (const FString* ValueStr = Pair.Value.Entries.Find("DistanceHalfFalloff"))
		{
			FDefaultValueHelper::ParseFloat(*ValueStr, distanceHalfFalloff);
		}
		if (const FString* ValueStr = Pair.Value.Entries.Find("WeatherInput"))
		{
			FDataTarget dt;
			dt.Actor = this;
			ConnectInputs(ValueStr, GemstarSimulationSubsystem, dt, GemstarInterfaceType::WEATHERDATA);
		}
		config.Lidar.NumberOfRaysHorizontal = width;
		config.Lidar.NumberOfRaysVertical = height;
		config.Lidar.FieldOfViewHorizontal = FMath::DegreesToRadians(FoVHorizontal);
		config.Lidar.FieldOfViewVertical = FMath::DegreesToRadians(FoVVertical);
		// DistanceHalfFalloff in Meter
		config.Lidar.DistanceHalfFalloff = distanceHalfFalloff * 100.0f;
		SensorConfiguration.Add(config);
	}
	for (auto& config : SensorConfiguration)
	{
		SpawnSensor(config);
	}
}

void ALidarSensorManager::SpawnSensor(FSensorConfigData& sensorConfiguration) {
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters params;
		params.Name = sensorConfiguration.ID;
		ASimpleLidar* SpawnedActor = World->SpawnActor<ASimpleLidar>(ASimpleLidar::StaticClass(), FTransform(), params);
		if (SpawnedActor == nullptr) {
			UE_LOG(Lidar, Log, TEXT("Lidar: Spawn failed for %s"), *SpawnedActor->GetName());
			return;
		}

		SpawnedActor->SetSensorConfiguration(sensorConfiguration);

		UE_LOG(Lidar, Log, TEXT("Lidar: Spawned %s"), *SpawnedActor->GetName());
		if (!sensorConfiguration.Mount.IsEmpty()) {
			IGemstarManager::Execute_MountOn(GemstarSimulationSubsystem, sensorConfiguration.Mount, SpawnedActor);
		}
		else {
			ILocalTransformUpdater::Execute_UpdateTransformAfterAttachment(SpawnedActor);
		}
		AddTickPrerequisiteActor(SpawnedActor);
		SpawnedActors.Add(SpawnedActor);

		if (bHasWeatherData) {
			SpawnedActor->SetWeatherData(LatestWeatherData);
		}
	}
}

void ALidarSensorManager::SpawnSensor_Implementation(FSensorConfigData& sensorConfiguration)
{
	if (sensorConfiguration.Type != SensorType::LIDAR) {
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters params;
		if (sensorConfiguration.ID.IsNone()) {
			params.Name = "Unnamed_Lidar_Sensor";
		}
		else {
			params.Name = sensorConfiguration.ID;
		}
		params.Owner = sensorConfiguration.Vehicle;

		ASimpleLidar* SpawnedActor = World->SpawnActor<ASimpleLidar>(ASimpleLidar::StaticClass(), FTransform(), params);
		SpawnedActor->AttachToActor(sensorConfiguration.Vehicle, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		SpawnedActor->SetSensorConfiguration(sensorConfiguration);
		SpawnedActor->SetActorRelativeTransform(sensorConfiguration.Offset);

		if (SpawnedActor)
		{
			UE_LOG(Lidar, Warning, TEXT("Spawned sensor: %s"), *SpawnedActor->GetName());
			SpawnedActors.Add(SpawnedActor);
		}
	}
}

void ALidarSensorManager::WeatherDataUpdate_Implementation(FWeatherData& data, int32 targetIndex)
{
	LatestWeatherData = data;
	bHasWeatherData = true;

	for (auto& lidarActor : SpawnedActors) {
		ASimpleLidar* SimpleLidar = Cast<ASimpleLidar>(lidarActor);

		if (!SimpleLidar) {
			continue;
		}

		SimpleLidar->SetWeatherData(LatestWeatherData);
	}
}
