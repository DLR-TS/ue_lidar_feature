/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/Material.h"
#include "PhysicsPublic.h"
#include "Sensors/GemstarSensor.h"
#include "Utils/ToOSI.h"
#include "LidarLog.h"
#include "osi_sensorview.pb.h"
#include "osi_sensorviewconfiguration.pb.h"
#include "SimpleLidar.generated.h"

constexpr float SPEED_OF_LIGHT = 299705543.0f; //in m/s in air 299792458.0f in vakuum;

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
class UE_LIDAR_FEATURERUNTIME_API ASimpleLidar : public AGemstarSensor
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GEMSTAR")
	float Range = 20.0;

	virtual void SetSensorConfiguration(FSensorConfigData NewSensorConfiguration) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void GetScanData();
	void Tick(float time) override;

private:
	UPROPERTY()
	AActor* Parent = nullptr;

	FSimpleLidarConfiguration Sensor;
	FCollisionQueryParams TraceParams;

	FWeatherData LatestWeatherData;

	bool bReportedNoDataTarget = false;

	void CreateTestConfig();
	void ShowSensorDebugInformation();
	void GenerateOSIMessage();
	float ComputeLidarSignalStrength(
		const FHitResult& Hit
	);
	UMaterialInterface* GetHitMaterial(const FHitResult& Hit) const;
	float GetReflectivity(const UMaterialInterface* Material) const;
};
