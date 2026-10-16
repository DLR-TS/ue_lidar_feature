/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */

#include "Effects/LidarWeatherEffectModel.h" 

#include "LidarLog.h"

constexpr double SpeedOfLight = 299792458.5; //in m/s 
constexpr double FOG_SLOPE_LOWR_EFLECTIVITY = 0.30286536; 
constexpr double FOG_SLOPE_HIGH_REFLECTIVITY = 0.34634364; 
constexpr double RAIN_SLOPE_LOW_REFLECTIVITY = 0.15688675; 
constexpr double RAIN_SLOPE_HIGH_REFLECTIVITY = 0.22530445;
constexpr double SNOW_SLOPE_LOW_REFLECTIVITY = 0.32685039;
constexpr double SNOW_SLOPE_HIGH_REFLECTIVITY = 0.32685039;

double LidarWeatherEffectModel::GetVanishingSlope(const FName& WeatherType, double Reflectivity, double ReflectivityThreshold) const {

	const bool bLowReflectivity = Reflectivity < ReflectivityThreshold;

	if (WeatherType == GemstarWeatherTypes::Fog) {
		return bLowReflectivity ? FOG_SLOPE_LOWR_EFLECTIVITY : FOG_SLOPE_HIGH_REFLECTIVITY;
	}

	if (WeatherType == GemstarWeatherTypes::Rain) {
		return bLowReflectivity ? RAIN_SLOPE_LOW_REFLECTIVITY : RAIN_SLOPE_HIGH_REFLECTIVITY;
	}

	if (WeatherType == GemstarWeatherTypes::Snow) {
		return bLowReflectivity ? SNOW_SLOPE_LOW_REFLECTIVITY : SNOW_SLOPE_HIGH_REFLECTIVITY;
	}

	return 0.0;
}

double LidarWeatherEffectModel::CalculateVanishingDistance(const FName& WeatherType, double Reflectivity, double ReflectivityThreshold, const FWeatherData& WeatherData) const {

	const double LidarCoefficient = GetSensorWeatherParameterAsDouble(WeatherData, GEMSTARSensorWeatherParameter::LidarCoefficient, 1.0);
	const double VanishingSlope = GetVanishingSlope(WeatherType, Reflectivity, ReflectivityThreshold);

	// calculate Meteorological Optical Range in meters
	const double MOR = GetVisualRange(WeatherType, WeatherData);

	return VanishingSlope * MOR * LidarCoefficient;
}

double LidarWeatherEffectModel::GetVisualRange(const FName& WeatherType, const FWeatherData& WeatherData) const
{
	if (WeatherType == GemstarWeatherTypes::Fog) {
		return WeatherData.WeatherState.MOR;
	}

	if (WeatherType == GemstarWeatherTypes::Rain || WeatherType == GemstarWeatherTypes::Snow) {
		// ROADVIEW regression to convert precipitation intensity to equivalent MOR value
		const double equivalentMOR = -0.8308 * WeatherData.WeatherState.PrecipitationIntensity + 159.16;
		return FMath::Max(0.0, equivalentMOR);
	}

	return 0.0;
}

double LidarWeatherEffectModel::CalculateDistanceFromTimeOfFlight(double TimeOfFlight) const {

	return 0.5 * TimeOfFlight * SpeedOfLight;
}

bool LidarWeatherEffectModel::ShouldReflectionDisappear(double Distanc, double VanishingDistance) const {
	return Distanc > VanishingDistance;
}

double LidarWeatherEffectModel::GetSensorWeatherParameterAsDouble(const FWeatherData& WeatherData, const FName& Parameterkey, double DefaultValue) const {

	FString ParameterMapKey = BuildLidarWeatherParameterkey(Parameterkey);

	const FString* Value = WeatherData.SensorWeatherParameters.Find(ParameterMapKey);

	if (!Value) {
		return DefaultValue;
	}

	double ParsedValue = DefaultValue;
	if(!FDefaultValueHelper::ParseDouble(*Value, ParsedValue)){
		return DefaultValue;
	}
	return ParsedValue;
}

bool LidarWeatherEffectModel::IsWeatherTypeAvailable(const FName& WeatherType, const FWeatherData& WeatherData) const
{
	if (WeatherType == GemstarWeatherTypes::Fog) {
		return WeatherData.WeatherState.bHasMOR &&
			WeatherData.WeatherState.MOR > 0.0;
	}

	if (WeatherType == GemstarWeatherTypes::Rain) {
		return WeatherData.WeatherState.bHasPrecipitation
			&& WeatherData.WeatherState.PrecipitationType
			== GemstarPrecipitationType::Rain &&
			WeatherData.WeatherState.PrecipitationIntensity > 0.0;
	}

	if (WeatherType == GemstarWeatherTypes::Snow) {
		return WeatherData.WeatherState.bHasPrecipitation
			&& WeatherData.WeatherState.PrecipitationType
			== GemstarPrecipitationType::Snow &&
			WeatherData.WeatherState.PrecipitationIntensity > 0.0;
	}

	return false;
}

bool LidarWeatherEffectModel::DetermineActiveWeatherType(const FWeatherData& WeatherData, FName& OutActiveWeatherType) const
{
	const FName RequestedWeatherType = WeatherData.WeatherState.ActiveWeatherType;

	// Clear weather
	if (RequestedWeatherType == GemstarWeatherTypes::Clear) {
		OutActiveWeatherType = GemstarWeatherTypes::Clear;
		return false;
	}

	// Explicit adverse weather selection
	if (RequestedWeatherType == GemstarWeatherTypes::Fog || 
		RequestedWeatherType == GemstarWeatherTypes::Rain || 
		RequestedWeatherType == GemstarWeatherTypes::Snow) {
		if (!IsWeatherTypeAvailable(RequestedWeatherType, WeatherData)) {
			OutActiveWeatherType = GemstarWeatherTypes::Clear;
			return false;
		 }

		OutActiveWeatherType = RequestedWeatherType;
		return true;
	}
	const bool bFogAvailable = IsWeatherTypeAvailable(GemstarWeatherTypes::Fog, WeatherData);
	const bool bRainAvailable = IsWeatherTypeAvailable(GemstarWeatherTypes::Rain, WeatherData);
	const bool bSnowAvailable = IsWeatherTypeAvailable(GemstarWeatherTypes::Snow, WeatherData);

	if (bFogAvailable) {
		OutActiveWeatherType = GemstarWeatherTypes::Fog;
	}
	else if (bRainAvailable) {
		OutActiveWeatherType = GemstarWeatherTypes::Rain;
	}
	else if (bSnowAvailable) {
		OutActiveWeatherType = GemstarWeatherTypes::Snow;
	}
	else {
		OutActiveWeatherType = GemstarWeatherTypes::Clear;
		return false;
	}

	return true;

}

FString LidarWeatherEffectModel::BuildLidarWeatherParameterkey(const FName& Parameterkey) const {

	// Build the search key for stored sensor parameter
	return FString(TEXT("|")) + FString::FromInt(static_cast<int32>(EGEMSTARSensorType::Lidar)) + TEXT("|") + Parameterkey.ToString();
}

void LidarWeatherEffectModel::ApplyLidarWeatherEffect(osi3::SensorView& SensorView, const FWeatherData& WeatherData) const {

	if (!WeatherData.WeatherState.bWeatherEffectEnabled) {
		return;
	}

	ApplyVanishingDistanceDegradation(SensorView, WeatherData);

}

void LidarWeatherEffectModel::ApplyVanishingDistanceDegradation(osi3::SensorView& SensorView, const FWeatherData& WeatherData) const {

	FName ActiveWeatherType = GemstarWeatherTypes::Clear;

	if (!DetermineActiveWeatherType(WeatherData, ActiveWeatherType)) {
		return;
	}
	UE_LOG(Lidar, Log, TEXT("LidarWeatherEffectModel: The Weather-dependent LiDAR degradation support only one condition at a time. The applied weather effect is: '%s'."), *ActiveWeatherType.ToString());

	const double ReflectivityThreshold = GetSensorWeatherParameterAsDouble(WeatherData, GEMSTARSensorWeatherParameter::ReflectivityThreshold, 10.0);

	int32 TotalReflections = 0;
	int32 RemovedReflections = 0;
	constexpr uint64 InvalidObjectId = TNumericLimits<uint64>::Max();

	double MaxDis = 0.0;
	double MinDisVanishing = TNumericLimits<double>::Max();
	double MaxDisVanishing = 0.0;

	for (int32 LidarSensorIndex = 0; LidarSensorIndex < SensorView.lidar_sensor_view_size(); ++LidarSensorIndex) {
		osi3::LidarSensorView* LidarView = SensorView.mutable_lidar_sensor_view(LidarSensorIndex);

		if (!LidarView) {
			continue;
		}

		for (int32 ReflectionIndex = 0; ReflectionIndex < LidarView->reflection_size(); ++ReflectionIndex) {

			osi3::LidarSensorView_Reflection* Reflection = LidarView->mutable_reflection(ReflectionIndex);
			if (!Reflection) {
				continue;
			}
			++TotalReflections;

			const double SignalStrength = Reflection->signal_strength();
			if (SignalStrength <= 0.0) {
				continue;
			}

			const double Distance = CalculateDistanceFromTimeOfFlight(Reflection->time_of_flight());
			const double Reflectivity = SignalStrength * 100.0;

			const double LidarPointsVanishingDistance = CalculateVanishingDistance(ActiveWeatherType, Reflectivity, ReflectivityThreshold, WeatherData);
			if (LidarPointsVanishingDistance <= 0.0) {
				continue;
			}

			MaxDis = FMath::Max(MaxDis, Distance);
			MinDisVanishing = FMath::Min(MinDisVanishing, LidarPointsVanishingDistance);
			MaxDisVanishing = FMath::Max(MaxDisVanishing, LidarPointsVanishingDistance);


			if (ShouldReflectionDisappear(Distance, LidarPointsVanishingDistance)) {
				Reflection->set_signal_strength(0.0);
				Reflection->mutable_object_id()->set_value(InvalidObjectId);

				++RemovedReflections;
			}
		}
	}

	if (MinDisVanishing == TNumericLimits<double>::Max()) {

		MinDisVanishing =0.0;
	}
}

