/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "GEMSTAR_Lidar_Loader.h"

#include "Subsystems/GemstarSimulationSubsystem.h"
#include "LidarSensorManager.h"
#include "LidarLog.h"

// Sets default values for this component's properties
UGEMSTAR_Lidar_Loader::UGEMSTAR_Lidar_Loader()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGEMSTAR_Lidar_Loader::BeginPlay()
{
	Super::BeginPlay();

	UGemstarSimulationSubsystem* GemstarSimulationSubsystem = UGemstarSimulationSubsystem::Get(GetWorld());
	
	if (GemstarSimulationSubsystem) {
		FTransform SpawnTransform;

		FActorSpawnParameters SpawnParams;
		ALidarSensorManager *SpawnedLidarActor = GetWorld()->SpawnActor<ALidarSensorManager>(ALidarSensorManager::StaticClass(), SpawnTransform, SpawnParams);

		TMap<FString, FInnerConfig> config = GemstarSimulationSubsystem->RegisterManagerAndGetConfiguration(
			SpawnedLidarActor, "ue_lidar_feature");
		SpawnedLidarActor->Configure(config, GemstarSimulationSubsystem);
		UE_LOG(Lidar, Log, TEXT("Registered and configured Lidar"));
	} else {
		UE_LOG(Lidar, Error, TEXT("Error: Cast of GemstarSimulationSubsystem was not successful"));
	}
}
