// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Math/UnrealMathUtility.h"
#include <random>
#include "Weather/Wind/WindGeneration.h"
// Currently this implementation will only support a global windspeed, who knows, might change.
// It also assumes that the wind doesn't "change", it is only in one direction with noise.


float nextGustTiming    = 0.0f;
float nextGustLength = 0.0f;
float nextGustIntensity = 0.0f;
float nextRampLength = 1.0f; // Temp Constant ramp length
float nextRampStrength = 0.0f;

float windCycleTime = 0.0f;
FVector2D windVec = FVector2D(0, 0);

//Predefined values, refer to DOI: 10.1109/EPQU.2011.6128927
float l = 20;
int n = 50;
float H = 2; // Height of aeral point for our truck
float delta_f = 0.2;

TArray<float> freqs;
// Sets default values for this component's properties
UWindGeneration::UWindGeneration()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWindGeneration::BeginPlay()
{
	Super::BeginPlay();
	freqs.Init(0, n);
	for (int i = 0; i < n; i++)
	{
		freqs[i] = i * delta_f;
	}

	// ...
	
}


// Called every frame
void UWindGeneration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	windCycleTime += DeltaTime;
	if (windCycleTime > 120)
	{
		float coin_toss = FMath::FRandRange(0.0f, 1.0f);
		if (gust_chance > coin_toss)
		{
			nextGustTiming = FMath::FRandRange(0, 120);
			nextGustLength = FMath::FRandRange(0, 10);

			nextGustIntensity = (maxwind - avgwind) * (FMath::FRandRange(0, 1));
		}
		else {
			nextGustTiming = 0;
			nextGustLength = 0;

			nextGustIntensity = 0;
			nextRampStrength = 0;
		}
		windCycleTime = 0;
	}
	float Vg = 0; // Gust Velocity
	float Vr = 0; // Ramp Velocity
	float Tsg = nextGustTiming; float Teg = nextGustTiming + nextGustLength;
	float Tsr = Tsg - nextRampLength ; float Ter = Teg + nextRampLength;
	if (windCycleTime > Tsg && windCycleTime < Teg)
	{
		Vg = nextGustIntensity * (1 - cos(2 * PI * (windCycleTime - Tsg) / (Teg - Tsg)));
		// Todo implement gust.
	}
	static float Vb = Vb - DeltaTime*(0.01*(Vb-avgwind) + UWindGeneration::randn(0,1)); // This might change over time in the future
	static float Vdir= Vdir - DeltaTime * (0.01 * (Vb - winddir) + UWindGeneration::randn(0, 0.01)); // This might change over time in the future

	// Height based noise? 
	//for (int i = 0; i < n; i++)
	//{
	//	//float sw = sqrt(Swn(freqs[i])*deltaf*c
	//}

	// ...
	windVec = FVector2D(Vb * cos(Vdir), Vb * sin(Vdir));
}

float UWindGeneration::Swn(float frequency)
{
	//roughness_length
	return 0.0f;
}

FVector2D UWindGeneration::getWind()
{
	return windVec;
}

float UWindGeneration::randn(float mean, float sig)
{
	std::random_device rd{};
	std::mt19937 gen{ rd() };
	std::normal_distribution<float> d{ mean, sig };

	return d(gen);
}