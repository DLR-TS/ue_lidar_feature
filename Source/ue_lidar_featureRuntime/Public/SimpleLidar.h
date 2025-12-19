// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "SensorConfigurationData.h"
#include "SensorDataDispatcher.h"
#include "LocalTransformUpdater.h"
#include "Utils/ToOSI.h"
#include "osi_sensorview.pb.h"
#include "osi_sensorviewconfiguration.pb.h"
#include "SimpleLidar.generated.h"

constexpr float SPEED_OF_LIGHT = 299792458.0f; // in m/s

USTRUCT()
struct FHitResultAndIntesity
{
	GENERATED_BODY()
public:
	FHitResult HitResult;
	float Intesity = 0.f;
};

USTRUCT()
struct FSimpleLidarConfiguration
{
	GENERATED_BODY()

public:
	//packed config
	osi3::SensorViewConfiguration* SensorViewConfig = nullptr;
	//prepared configuration
	TArray<float> AzimuthAngles;
	TArray<float> ElevationAngles;
	uint32_t Number_of_rays_horizontal = 0;
	uint32_t Number_of_rays_vertical = 0;
	//raw results
	TArray<FHitResultAndIntesity> RecordedHits;
	//packed results
	osi3::SensorView* SensorView = nullptr;
};

UCLASS()
class ASimpleLidar : public AActor,
	public ISensorDataDispatcher,
	public ILocalTransformUpdater
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASimpleLidar();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bDebugRayTraces = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bDebugReflections = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool bDebugNormals = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	bool SupportMultithread = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	float DistanceHalfFalloff = 2000.0;

	UFUNCTION(BlueprintCallable, Category = "GEMSTAR")
	void setSensorConfiguration(FSensorConfigData sensorConfiguration);

	void AddDataTarget(AActor* dataTarget);
	void UpdateTransformAfterAttachment_Implementation() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void GetScanData();
	void Tick(float time) override;

private:
	UPROPERTY()
	AActor* Parent = nullptr;

	FSensorConfigData sensorConfiguration;
	FSimpleLidarConfiguration Sensor;
	FCollisionQueryParams TraceParams;

	void CreateTestConfig();
	void ShowSensorDebugInformation();
	void GenerateOSIMessage();
	void SendData();
	float ComputeLidarSignalStrength(
		const FHitResult& Hit
	);
};
