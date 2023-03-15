// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Carla/Vehicle/CarlaWheeledVehicleNW.h"
#include "VTM.generated.h"

/**
 * 
 */
UCLASS()
class CARLA_API AVTM : public ACarlaWheeledVehicleNW
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties

	UPROPERTY(EditAnywhere, Category = "TireMesh")
		UStaticMesh* TireLMesh;

	UPROPERTY(EditAnywhere, Category = "TireMesh")
		UStaticMesh* TireRMesh;

	UPROPERTY(EditAnywhere, Category = "TireMesh")
		UStaticMesh* Tire2LMesh;

	UPROPERTY(EditAnywhere, Category = "TireMesh")
		UStaticMesh* Tire2RMesh;


	UPROPERTY(EditAnywhere, Category = "VehicleMesh")
		UStaticMesh* Cab_Mesh;

	UPROPERTY(EditAnywhere, Category = "VehicleMesh")
		UStaticMesh* Chassi_Mesh;

	UPROPERTY(EditAnywhere, Category = "VehicleMesh")
		UStaticMesh* Trailer_Mesh;

	UPROPERTY(EditAnywhere, Category = "VehicleMesh")
		UStaticMesh* Dolly_Mesh;

	UPROPERTY(EditAnywhere, Category = "VehicleMesh")
		UStaticMesh* Axle_Mesh;

	AVTM(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostActorCreated() override;

public:
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	void GetParts(TArray<USceneComponent*> &units_ref, TArray<UStaticMeshComponent*> &axles_ref, TArray<UStaticMeshComponent*> &tires_ref);
	//virtual void StepSimulator(float DeltaTime);
	FVector GetOffset();

	void DisableUE4VehiclePhysic();
	//virtual void UpdateVTMEnvironment();

	enum UnitTypes
	{
		cab = 1,
		tractor = 2,
		trailer = 3,
		dolly = 4
	};

private:

	virtual int Str2UT(FString input);
	//virtual FQuat RotToQuat(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33);

	//virtual void UpdatePosition();

	//virtual void UpdateRoad();
	//virtual void UpdateWind();

};
