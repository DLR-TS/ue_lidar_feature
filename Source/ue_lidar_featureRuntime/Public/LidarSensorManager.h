// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GEMSTARRoot.h"
#include "SensorConfigurationData.h"
#include "SimpleLidar.h"
#include "Engine/World.h"
#include "Misc/DefaultValueHelper.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LidarSensorManager.generated.h"

UCLASS()
class ALidarSensorManager : public AActor,
	public ISensorConfigurationData,
	public IGEMSTARManager
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALidarSensorManager();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Configure(TMap<FString, FInnerConfig> Configurations, AGEMSTARRoot* Root);

protected:
	virtual void SpawnSensor_Implementation(FSensorConfigData& sensorConfiguration) override;
	virtual bool RegisterDataflow_Implementation(AActor* consumer, FString& producer, GEMSTARInterfaceType interfaceName) override;

private:
	UPROPERTY()
	TArray<AActor*> SpawnedActors;
	AGEMSTARRoot* GemstarRoot;
	TArray<FSensorConfigData> SensorConfiguration;

	void SpawnSensor(FSensorConfigData& sensorConfiguration);
};
