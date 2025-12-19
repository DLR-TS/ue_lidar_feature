// Fill out your copyright notice in the Description page of Project Settings.


#include "LidarSensorManager.h"

// Sets default values
ALidarSensorManager::ALidarSensorManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ALidarSensorManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ALidarSensorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALidarSensorManager::Configure(TMap<FString, FInnerConfig> Configurations, AGEMSTARRoot* Root) {
	GemstarRoot = Root;

	for (const TPair<FName, FInnerConfig>& Pair : Configurations)
	{
		FSensorConfigData config;
		config.ID = Pair.Key;
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
		config.Lidar.NumberOfRaysHorizontal = width;
		config.Lidar.NumberOfRaysVertical = height;
		config.Lidar.FieldOfViewHorizontal = FoVHorizontal * PI / 180;
		config.Lidar.FieldOfViewVertical = FoVVertical * PI / 180;
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
			UE_LOG(LogTemp, Log, TEXT("Lidar: Spawn failed for %s"), *SpawnedActor->GetName());
			return;
		}

		SpawnedActor->setSensorConfiguration(sensorConfiguration);

		UE_LOG(LogTemp, Log, TEXT("Lidar: Spawned %s"), *SpawnedActor->GetName());
		if (!sensorConfiguration.Mount.IsEmpty()) {
			IGEMSTARManager::Execute_MountOn(GemstarRoot, sensorConfiguration.Mount, SpawnedActor);
		}
		else {
			ILocalTransformUpdater::Execute_UpdateTransformAfterAttachment(SpawnedActor);
		}
		AddTickPrerequisiteActor(SpawnedActor);
		SpawnedActors.Add(SpawnedActor);
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
		SpawnedActor->setSensorConfiguration(sensorConfiguration);
		SpawnedActor->SetActorRelativeTransform(sensorConfiguration.Offset);

		if (SpawnedActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawned sensor: %s"), *SpawnedActor->GetName());
			SpawnedActors.Add(SpawnedActor);
		}
	}
}

bool ALidarSensorManager::RegisterDataflow_Implementation(AActor* consumer, FString& producer, GEMSTARInterfaceType interfaceName) {
	AActor* producerActor = nullptr;
	for (auto& lidarActor : SpawnedActors) {
		if (lidarActor->GetName() == producer) {
			producerActor = lidarActor;
		}
	}
	if (producerActor == nullptr) {
		return false;
	}
	dynamic_cast<ASimpleLidar*>(producerActor)->AddDataTarget(consumer);
	consumer->AddTickPrerequisiteActor(producerActor);
	return true;
}
