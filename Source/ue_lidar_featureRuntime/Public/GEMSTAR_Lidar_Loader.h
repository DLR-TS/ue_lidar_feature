// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GEMSTARRoot.h"
#include "LidarSensorManager.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GEMSTAR_Lidar_Loader.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UE_LIDAR_FEATURERUNTIME_API UGEMSTAR_Lidar_Loader : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGEMSTAR_Lidar_Loader();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};
