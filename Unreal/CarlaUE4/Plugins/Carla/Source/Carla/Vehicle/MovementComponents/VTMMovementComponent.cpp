
// Copyright (c) 2022 Volvo GTT

#include "VTMMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/Custom/VTM.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

#ifdef WITH_CUSTOM_SIM
#include "VTM_Interface.h"
UVTM_Interface* plantModel;
TArray<USceneComponent*> MainUnits2 = TArray<USceneComponent*>();;
TArray<UStaticMeshComponent*> Axles2 = TArray<UStaticMeshComponent*>();
TArray<UStaticMeshComponent*> Tires2 = TArray<UStaticMeshComponent*>();
#endif

ACarlaWheeledVehicle* RootVehicle;

void UVTMMovementComponent::CreateVTMMovementComponent(ACarlaWheeledVehicle* Vehicle)
{

#ifdef WITH_CUSTOM_SIM
    RootVehicle = Vehicle;
    UVTMMovementComponent* VTMMovementComponent = NewObject<UVTMMovementComponent>(Vehicle);
    Vehicle->SetCarlaMovementComponent(VTMMovementComponent);
    VTMMovementComponent->RegisterComponent();
#else
    UE_LOG(LogCarla, Warning, TEXT("Error: Custom Simulator is not enabled"));
#endif

}

void UVTMMovementComponent::BeginPlay()
{
#ifdef WITH_CUSTOM_SIM
    Super::BeginPlay();

    DisableUE4VehiclePhysics();

    FVector init_pos = CarlaVehicle->GetActorLocation();
    FQuat init_rot = CarlaVehicle->GetActorQuat();

    GLog->Log(init_pos.ToString());

    plantModel = NewObject<UVTM_Interface>();
    // GLog->Log(FString::SanitizeFloat(plantModel));
    // GLog->Log(*plantModel);
    plantModel->Initialize(init_pos, init_rot);

    ((AVTM*)CarlaVehicle)->GetParts(MainUnits2, Axles2, Tires2);
    //Offset = ((AVTM*)CarlaVehicle)->GetOffset();

   // MainUnits2[0]->SetWorldLocation(FVector(0, 0, 1000));
    GLog->Log(FString::FromInt(MainUnits2.Num()));
    //this->SetComponentTickEnabled(true);
    // IVTM_Interface* TheInterface = Cast<>;
#endif
}

void UVTMMovementComponent::ProcessControl(FVehicleControl& Control)
{
#ifdef WITH_CUSTOM_SIM
    plantModel->InputThrottle(Control.Throttle*2000, Control.Steer/2);
#endif
}

void UVTMMovementComponent::TickComponent(float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{

#ifdef WITH_CUSTOM_SIM
    TRACE_CPUPROFILER_EVENT_SCOPE(UVTMMovementComponent::TickComponent);
    FTransform trans;
    if (DeltaTime > 0.001)
    {
        uint64_t NumberSubSteps =
            FGenericPlatformMath::FloorToInt(DeltaTime / 0.001);
        if (NumberSubSteps > 1000)
        {
            NumberSubSteps = 1000;
        }
        for (uint64_t i = 0; i < NumberSubSteps; ++i)
        {
            // Should be able to just move the tires here to get the new position for 
            plantModel->StepVTM(MainUnits2, Axles2, Tires2);
            UpdateRoad();
            UpdateWind();
        }
    }
    trans = plantModel->UpdateVehicleMesh(MainUnits2, Axles2, Tires2);
    CarlaVehicle->SetActorTransform(trans);
#endif
}

void UVTMMovementComponent::UpdateRoad()
{
	TArray<float> z;
	TArray<float> dxdz;
	TArray<float> dydz;
    TArray<float> mu;

	int n_tires = Tires2.Num();
	z.Init(0, n_tires);
    dxdz.Init(0, n_tires * 2);
    dydz.Init(0, n_tires * 2);
    mu.Init(0, n_tires);
    UWorld* World = GetWorld();

	for (int i = 0; i < n_tires; i++)
	{
		const FName TraceTag("MyTraceTag");

		//FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, RootVehicle);
        FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("LineOfSight_Trace")), false, RootVehicle);

		RV_TraceParams.bTraceComplex = true;
		RV_TraceParams.bReturnPhysicalMaterial = false;
		RV_TraceParams.TraceTag = TraceTag;

		double tracelength = 1100;
		FVector start = Tires2[i]->GetComponentLocation() + FVector(0,0,100);
        // TODO: This is not correct. The actual connection is not straight down.
		FVector end = start - RootVehicle->GetActorUpVector() * (tracelength);
		FHitResult GroundHit(ForceInit);
        World->LineTraceSingleByChannel(GroundHit, start, end, ECollisionChannel::ECC_WorldStatic, RV_TraceParams, FCollisionResponseParams::DefaultResponseParam);
		//Tires2[i]->LineTraceComponent(GroundHit, start, end, RV_TraceParams);
		if (GroundHit.bBlockingHit)
		{
			double heading = Tires2[i]->GetAttachParent()->GetComponentRotation().Euler().Z;
			FVector GroundAngle = GroundHit.ImpactNormal.RotateAngleAxis(heading, RootVehicle->GetActorUpVector());

            z[i] = GroundHit.ImpactPoint.Z;

            dxdz[i] = (-GroundAngle.X) / GroundAngle.Z;
            dxdz[i] = (-GroundAngle.Y) / GroundAngle.Z;

            //mu[i] = GroundHit.PhysMaterial.Friction;
            mu[i] = 1;

		}
		else
		{
            z[i] = -10000;

            dxdz[i] = 0;
            dxdz[i] = 0;

            //mu[i] = GroundHit.PhysMaterial.Friction;
            mu[i] = 1;
		}
	}
    plantModel->SetRoad(z, dydz, dxdz, mu);
}

void UVTMMovementComponent::UpdateWind()
{
	float wind_vehicle_x = 0; float wind_vehicle_y = 0;

	plantModel->SetWind(wind_vehicle_x, wind_vehicle_y);
}
// FVector GetVelocity() const override;

int32 UVTMMovementComponent::GetVehicleCurrentGear() const
{
    return CarlaVehicle->GetVehicleMovementComponent()->GetCurrentGear();
}

float UVTMMovementComponent::GetVehicleForwardSpeed() const
{
    return CarlaVehicle->GetVehicleMovementComponent()->GetForwardSpeed();
}
