// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SensorConfigurationData.h"
#include "SensorConfigurationDispatcher.h"
#include "Engine/StaticMeshActor.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LidarSensorTester.generated.h"

UCLASS()
class ALidarSensorTester : public AActor,
	public ISensorSpawnerDispatcher,
	public ISensorDataUpdate
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALidarSensorTester();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RegisterSensorSpawner_Implementation(AActor* spawner) override;

	void UnregisterSensorSpawner_Implementation(AActor* spawner) override;

	void SensorViewDataUpdate_Implementation(FSensorView& sensorViewData) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UStaticMesh* CubeMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* CubeMaterial;

	UPROPERTY(EditAnywhere)
	AStaticMeshActor* CubeActor;

	UPROPERTY(EditAnywhere)
	AActor* TestActorToInform;

private:
	TArray<AActor*> ActorsToInform;
	int counter = 0;
};
