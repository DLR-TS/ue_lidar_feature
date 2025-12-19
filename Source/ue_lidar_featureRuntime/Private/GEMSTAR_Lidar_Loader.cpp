// Fill out your copyright notice in the Description page of Project Settings.

#include "GEMSTAR_Lidar_Loader.h"

// Sets default values for this component's properties
UGEMSTAR_Lidar_Loader::UGEMSTAR_Lidar_Loader()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UGEMSTAR_Lidar_Loader::BeginPlay()
{
	Super::BeginPlay();
	AActor* OwnerActor = GetOwner();
	AGEMSTARRoot* GemstarRoot = Cast<AGEMSTARRoot>(OwnerActor);
	if (GemstarRoot)
	{
		if (GemstarRoot->UseLidar)
		{
			FTransform SpawnTransform;

			FActorSpawnParameters SpawnParams;
			ALidarSensorManager* SpawnedLidarActor = GetWorld()->SpawnActor<ALidarSensorManager>(ALidarSensorManager::StaticClass(), SpawnTransform, SpawnParams);

			TMap<FString, FInnerConfig> config = GemstarRoot->RegisterManagerAndGetConfiguration(SpawnedLidarActor);
			SpawnedLidarActor->Configure(config, GemstarRoot);
			UE_LOG(LogTemp, Log, TEXT("Registered and configured Lidar"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error: Cast of GEMSTARRoot was not successful"));
	}
}

// Called every frame
void UGEMSTAR_Lidar_Loader::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
