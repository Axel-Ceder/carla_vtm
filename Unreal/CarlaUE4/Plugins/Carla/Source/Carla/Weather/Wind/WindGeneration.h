// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WindGeneration.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UWindGeneration : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWindGeneration();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindGenerator", meta = (ClampMin = "0.0", ClampMax = "30.0", UIMin = "0.0", UIMax = "30.0"))
		float maxwind = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindGenerator", meta = (ClampMin = "0.0", ClampMax = "30.0", UIMin = "0.0", UIMax = "30.0"))
		float avgwind = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindGenerator", meta = (ClampMin = "-3.14", ClampMax = "3.14", UIMin = "-3.14", UIMax = "3.14"))
		float winddir = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindGenerator", meta = (ClampMin = "0.0", ClampMax = "1", UIMin = "0.0", UIMax = "1"))
		float winddirspread = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindGenerator", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float roughness_length = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WindGenerator", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float gust_chance = 1.0f;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	FVector2D getWind();

private:
	float Swn(float frequency);
	float randn(float mean, float sig);
};
