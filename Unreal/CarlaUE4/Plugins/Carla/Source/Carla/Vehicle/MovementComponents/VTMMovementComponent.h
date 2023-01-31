// Copyright (c) 2022 Volvo GTT

#pragma once

#include "BaseCarlaMovementComponent.h"
#include "VTM_Interface.h"
#include "VTMMovementComponent.generated.h"


UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class UVTMMovementComponent : public UBaseCarlaMovementComponent
{
	GENERATED_BODY()

public:

	static void CreateVTMMovementComponent(ACarlaWheeledVehicle* Vehicle);

	virtual void BeginPlay() override;

	void ProcessControl(FVehicleControl& Control) override;

	void TickComponent(float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	// FVector GetVelocity() const override;

	int32 GetVehicleCurrentGear() const override;

	float GetVehicleForwardSpeed() const override;

	UPROPERTY(Category = "VTM", EditAnywhere, BlueprintReadWrite)
		float Throttle = 100.0f;


	UPROPERTY(Category = "VTM", VisibleAnywhere)
		UVTM_Interface* plantModel;
};

