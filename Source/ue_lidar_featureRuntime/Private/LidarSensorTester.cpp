/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "LidarSensorTester.h"

#include "EngineUtils.h"
#include "LidarSensorManager.h"

// Sets default values
ALidarSensorTester::ALidarSensorTester()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		CubeMesh = CubeAsset.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MaterialAsset.Succeeded())
	{
		CubeMaterial = MaterialAsset.Object;
	}
}

// Called when the game starts or when spawned
void ALidarSensorTester::BeginPlay()
{
	Super::BeginPlay();

	if (CubeMesh)
	{
		FActorSpawnParameters SpawnParams;
		CubeActor = GetWorld()->SpawnActor<AStaticMeshActor>(GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

		if (CubeActor)
		{
			CubeActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			CubeActor->GetStaticMeshComponent()->SetMaterial(0, CubeMaterial);
			CubeActor->SetMobility(EComponentMobility::Movable);
			//CubeActor->SetActorLocation(CubeActor->GetActorLocation() + FVector(0, 0, 50));

		}
	}
}

// Called every frame
void ALidarSensorTester::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CubeActor->SetActorLocation(CubeActor->GetActorLocation() + FVector(1, 0, 0));
	CubeActor->SetActorRotation(CubeActor->GetActorRotation() + FRotator(0, 0, 0));

	if (bSensorSpawned) {
		return;
	}
	AActor* Spawner = FindSensorSpawner();

	if (Spawner == nullptr) {

		if (!bReportedWaiting) {
			bReportedWaiting = true;
			UE_LOG(Lidar, Log, TEXT("LidarSensorTester: no lidar manager yet, retrying every tick"));
		}
		return;
	}
	FSensorConfigData data;
	data.ID = FName(TEXT("lidar:cube"));
	data.Type = SensorType::LIDAR;
	data.Vehicle = CubeActor;
	data.Offset = FTransform(FRotator(0, 0, 0), FVector(0, 0, 100), FVector(1, 1, 1));
	data.Lidar.FieldOfViewHorizontal = PI / 4;
	data.Lidar.FieldOfViewVertical = PI / 4;
	data.Lidar.NumberOfRaysHorizontal = 20;
	data.Lidar.NumberOfRaysVertical = 20;

	if (TestActorToInform == nullptr)
	{
		data.DataTargets.Add({ this, 0 });
	}
	else {
		data.DataTargets.Add({ TestActorToInform, 0 });
	}

	ISensorConfigurationData::Execute_SpawnSensor(Spawner, data);
	bSensorSpawned = true;
	UE_LOG(Lidar, Log, TEXT("LidarSensorTester: lidar:cube spawned on %s"), *CubeActor->GetName());
}

AActor* ALidarSensorTester::FindSensorSpawner() const {
	
	if (ActorsToInform.Num()) {
		return ActorsToInform[0];
	}
	TActorIterator<ALidarSensorManager> Manager(GetWorld());
	return Manager ? *Manager : nullptr;
}

void ALidarSensorTester::RegisterSensorSpawner_Implementation(AActor* spawner)
{
	if (spawner && spawner->Implements<USensorConfigurationData>()) {
		ActorsToInform.AddUnique(spawner);
		UE_LOG(Lidar, Log, TEXT("Tried to register an actor with USensorConfigurationData."));
	}
	else {
		UE_LOG(Lidar, Log, TEXT("Tried to register an actor without USensorConfigurationData."));
	}
}

void ALidarSensorTester::UnregisterSensorSpawner_Implementation(AActor* spawner)
{
	if (spawner && spawner->Implements<USensorConfigurationData>()) {
		ActorsToInform.Remove(spawner);
	}
}

void ALidarSensorTester::SensorViewDataUpdate_Implementation(FSensorView& sensorViewData, int TargetIndex, UTextureRenderTarget2D* cameraRenderTarget)
{
	check(sensorViewData.sensorview->lidar_sensor_view_size() == 1);
	UE_LOG(Lidar, Log, TEXT("OSI Lidar data size in GameThread: %d"),
		sensorViewData.sensorview->lidar_sensor_view(0).reflection_size()
	);
}