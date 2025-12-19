// Fill out your copyright notice in the Description page of Project Settings.


#include "LidarSensorTester.h"

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
	if (counter++ == 0) {
		FSensorConfigData data;
		data.Type = SensorType::LIDAR;
		data.Vehicle = CubeActor;
		data.Offset = FTransform(FRotator(0, 0, 0), FVector(0, 0, 100), FVector(1, 1, 1));
		data.Lidar.FieldOfViewHorizontal = PI / 4;
		data.Lidar.FieldOfViewVertical = PI / 4;
		data.Lidar.NumberOfRaysHorizontal = 20;
		data.Lidar.NumberOfRaysVertical = 20;
		if (TestActorToInform == nullptr)
		{
			data.DataTargets.Add(this);
		}
		else {
			data.DataTargets.Add(TestActorToInform);
		}
		if (ActorsToInform.Num()) {
			ISensorConfigurationData::Execute_SpawnSensor(ActorsToInform[0], data);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No actor to inform"));
		}
	}
}

void ALidarSensorTester::RegisterSensorSpawner_Implementation(AActor* spawner)
{
	if (spawner && spawner->Implements<USensorConfigurationData>()) {
		ActorsToInform.AddUnique(spawner);
		UE_LOG(LogTemp, Log, TEXT("Tried to register an actor with USensorConfigurationData."));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Tried to register an actor without USensorConfigurationData."));
	}
}

void ALidarSensorTester::UnregisterSensorSpawner_Implementation(AActor* spawner)
{
	if (spawner && spawner->Implements<USensorConfigurationData>()) {
		ActorsToInform.Remove(spawner);
	}
}

void ALidarSensorTester::SensorViewDataUpdate_Implementation(FSensorView& sensorViewData)
{
	check(sensorViewData.sensorview->lidar_sensor_view_size() == 1);
	UE_LOG(LogTemp, Log, TEXT("OSI Lidar data size in Gamethread: %d"),
		sensorViewData.sensorview->lidar_sensor_view(0).reflection_size()
	);
}