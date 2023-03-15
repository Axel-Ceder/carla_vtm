// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Vehicle/Custom/VTM.h"
#include <stdio.h>
#include "math.h"
//#include "Math/TransformNonVectorized.h"
#include "Math/Quat.h" 	

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Components/InputComponent.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include "Misc/FileHelper.h"
#include "Json.h"
#include "Serialization/JsonSerializer.h"
#include "Interfaces/IPluginManager.h"

bool has_crashed = false;
float m2unit = 100.0f;

USceneComponent* RootPosition;
TArray<USceneComponent*> MainUnits = TArray<USceneComponent*>();;
TArray<UStaticMeshComponent*> Axles = TArray<UStaticMeshComponent*>();
TArray<UStaticMeshComponent*> Tires = TArray<UStaticMeshComponent*>();
TArray<UStaticMeshComponent*> MainMeshes = TArray<UStaticMeshComponent*>();

TArray<int> meshType;
int32 n_tires = 0;
int32 n_axles = 0;
int32 n_units = 0;

FVector offset = FVector(0, 0, 0);

float base_radius = 58.25;

// Sets default values
AVTM::AVTM(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UWheeledVehicleMovementComponentNW>(AWheeledVehicle::VehicleMovementComponentName))
{
#ifdef WITH_CUSTOM_SIM
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	//FString testString = GetClass()->GetName().LeftChop(2);
	FString testString = "VTMLibrary";
	GLog->Log(*testString);
	n_tires = 0;
	n_axles = 0;
	n_units = 0;

	Tires.Empty();
	Axles.Empty();
	MainUnits.Empty();
	MainMeshes.Empty();
	meshType.Empty();

	// TODO: This is bad, locked in to specific jsonfile. I wonder if we can use an input when we call for the vehicle?
	FString ContentDir = IPluginManager::Get().FindPlugin("VTMForCarla")->GetBaseDir() + "/Source/ThirdParty/" + testString;
	FString jsonLoc = ContentDir + "/vtm.json";

	//RootPosition = CreateDefaultSubobject<USceneComponent>(TEXT("StartLocation"));
	//RootComponent = RootPosition;


	FString JsonString; //Json converted to FString

	FFileHelper::LoadFileToString(JsonString, *jsonLoc);

	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);

	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		TArray<TSharedPtr<FJsonValue>> unit_type = JsonParsed->GetArrayField("type");
		TArray<TSharedPtr<FJsonValue>> n_axles_per_unit = JsonParsed->GetArrayField("na");
		TArray<TSharedPtr<FJsonValue>> n_tires_per_axle = JsonParsed->GetArrayField("n");
		TArray<TSharedPtr<FJsonValue>> dist_to_each_axle = JsonParsed->GetArrayField("ll");
		TArray<TSharedPtr<FJsonValue>> dist_to_conn_point = JsonParsed->GetArrayField("lc");
		TArray<TSharedPtr<FJsonValue>> track_width = JsonParsed->GetArrayField("w");
		TArray<TSharedPtr<FJsonValue>> wheel_radius = JsonParsed->GetArrayField("r");
		TArray<TSharedPtr<FJsonValue>> sensor_f = JsonParsed->GetArrayField("sensor_f");

		float xjCab = JsonParsed->GetNumberField("x0cab");
		float zjCab = JsonParsed->GetNumberField("z0cab");

		const FString UnitString = FString(TEXT("{0}_{1}"));
		const FString AxleGroup = FString(TEXT("AxleGroup For {0}_{1}"));
		const FString UnitMeshStrin = FString(TEXT("Mesh: {0}_{1}"));
		const FString AxleString = FString(TEXT("Axle: {0}, {1}"));
		const FString TireStringL = FString(TEXT("Tire: {0}, {1}L"));
		const FString TireStringR = FString(TEXT("Tire: {0}, {1}R"));

		TArray<TSharedPtr<FJsonValue>> sensor_base = sensor_f[0]->AsArray();

		float center_point = 0;
		//TArray<TSharedPtr<FJsonValue>> sensor_f_this_unit = sensor_f[0]->AsArray();

		for (int32 unit = 0; unit < n_axles_per_unit.Num(); unit++)
		{
			TArray<TSharedPtr<FJsonValue>> n_tires_this_unit = n_tires_per_axle[unit]->AsArray();
			TArray<TSharedPtr<FJsonValue>> dist_to_axle_this_unit = dist_to_each_axle[unit]->AsArray();
			TArray<TSharedPtr<FJsonValue>> this_unit_conn_point = dist_to_conn_point[unit]->AsArray();
			TArray<TSharedPtr<FJsonValue>> track_width_this_unit = track_width[unit]->AsArray();
			TArray<TSharedPtr<FJsonValue>> wheel_radius_this_unit = wheel_radius[unit]->AsArray();
			TArray<TSharedPtr<FJsonValue>> sensor_f_this_unit = sensor_f[unit]->AsArray();

			int32 nAxlesThis = n_axles_per_unit[unit]->AsNumber();

			FString unitType = unit_type[unit]->AsString();
			GLog->Log(*unitType);

			center_point = center_point - this_unit_conn_point[0]->AsNumber() * m2unit;

			const FString UnitStringFormatted = FString::Format(*UnitString, { unitType, unit });

			if (unitType.Equals("tractor"))
			{
				const FString CabStringForm = FString::Format(*UnitString, { FString("cabU"), unit });
				USceneComponent* UUnit = CreateDefaultSubobject<USceneComponent>(FName(CabStringForm));
				UUnit->SetupAttachment(RootComponent);
				UUnit->SetRelativeLocation(FVector(center_point, 0.0f, 0.0f) + FVector(xjCab,
					0,
					zjCab) * m2unit);
				MainUnits.Add(UUnit);

				const FString MeshStringForm = FString::Format(*UnitMeshStrin, { FString("cab"), unit });

				UStaticMeshComponent* UMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(MeshStringForm));
				MainMeshes.Add(UMesh);
				meshType.Add(Str2UT("cab"));
				n_units += 1;
				UMesh->SetupAttachment(UUnit);
				UMesh->SetRelativeLocation(FVector(-xjCab,
					0,
					-zjCab)*m2unit);
			}

			USceneComponent* UUnit = CreateDefaultSubobject<USceneComponent>(FName(UnitStringFormatted));
			UUnit->SetupAttachment(RootComponent);
			UUnit->SetRelativeLocation(FVector(center_point, 0.0f, 0.0f) + FVector(sensor_f_this_unit[0]->AsNumber(),
				sensor_f_this_unit[1]->AsNumber(),
				sensor_f_this_unit[2]->AsNumber()) * m2unit);
			MainUnits.Add(UUnit);

			const FString MeshStringForm = FString::Format(*UnitMeshStrin, { unitType, unit });

			UStaticMeshComponent* UMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(MeshStringForm));
			MainMeshes.Add(UMesh);
			meshType.Add(Str2UT(unitType));
			n_units += 1;
			UMesh->SetupAttachment(UUnit);
			UMesh->SetRelativeLocation(FVector(-sensor_f_this_unit[0]->AsNumber(),
				-sensor_f_this_unit[1]->AsNumber(),
				-sensor_f_this_unit[2]->AsNumber()) * m2unit);

			const FString AxleGroupFormatted = FString::Format(*AxleGroup, { unitType, unit });

			USceneComponent* UAxleG = CreateDefaultSubobject<USceneComponent>(FName(AxleGroupFormatted));
			UAxleG->SetupAttachment(RootComponent);
			UAxleG->SetRelativeLocation(FVector(center_point, 0.0f, 0.0f));

			for (int32 axle = 0; axle < nAxlesThis; axle++)
			{

				const FString AxleStringFormatted = FString::Format(*AxleString, { unit, axle });
				float axleDist = dist_to_axle_this_unit[axle]->AsNumber() * m2unit;

				UStaticMeshComponent* UAxle = CreateDefaultSubobject<UStaticMeshComponent>(FName(AxleStringFormatted));
				UAxle->SetupAttachment(UAxleG);
				UAxle->SetRelativeLocation(FVector(axleDist, 0.0f, 47.0f));
				UAxle->AttachToComponent(UAxleG, FAttachmentTransformRules::KeepRelativeTransform);
				UAxle->SetupAttachment(UAxleG);


				const FString TireStringLFormatted = FString::Format(*TireStringL, { unit, axle });
				const FString TireStringRFormatted = FString::Format(*TireStringR, { unit, axle });

				float radius = wheel_radius_this_unit[axle]->AsNumber() * 100.0f;
				float scale = radius / base_radius;
				float wYPos = track_width_this_unit[axle]->AsNumber() * m2unit / 2;

				Tires.Add(CreateDefaultSubobject<UStaticMeshComponent>(FName(TireStringLFormatted)));
				Tires.Last()->SetupAttachment(UAxle);
				Tires.Last()->SetRelativeLocation(FVector(0.0f, -wYPos, 0.0f));
				Tires.Last()->SetRelativeScale3D(FVector(scale, scale, scale));

				Tires.Add(CreateDefaultSubobject<UStaticMeshComponent>(FName(TireStringRFormatted)));
				Tires.Last()->SetupAttachment(UAxle);
				Tires.Last()->SetRelativeLocation(FVector(0.0f, wYPos, 0.0f));
				Tires.Last()->SetRelativeScale3D(FVector(scale, scale, scale));

				n_tires += 2;
				Axles.Add(UAxle);
				n_axles += 1;
			}
			center_point = center_point + this_unit_conn_point[1]->AsNumber() * m2unit;

		}
	}
#endif
}

// Called when the game starts or when spawned
void AVTM::BeginPlay()
{
	DisableUE4VehiclePhysic();
	Super::BeginPlay();
}


void AVTM::OnConstruction(const FTransform& Transform)
{
	FBoundingBox BoundingBox = UBoundingBoxCalculator::GetVehicleBoundingBox(this);
	//AdjustVehicleBounds();
}

void AVTM::PostActorCreated()
{
	Super::PostActorCreated();


	TArray<UStaticMesh*> TireTypes = { TireLMesh, TireRMesh };
	for (int i = 0; i < n_tires; i++)
	{
		Tires[i]->SetStaticMesh(TireTypes[i % 2]);
	}
	for (int i = 0; i < n_axles; i++)
	{
		Axles[i]->SetStaticMesh(Axle_Mesh);
	}
	try
	{
		for (int i = 0; i < n_units; i++)
		{

			switch (meshType[i])
			{
			case 0:
				GLog->Log("Cab");
				MainMeshes[i]->SetStaticMesh(Cab_Mesh);
				break;
			case 2:
				GLog->Log("Trailer");
				MainMeshes[i]->SetStaticMesh(Trailer_Mesh);
				break;
			case 1:
				GLog->Log("Tractor");
				MainMeshes[i]->SetStaticMesh(Chassi_Mesh);
				break;
			case 3:
				MainMeshes[i]->SetStaticMesh(Dolly_Mesh);
				break;
			default:
				GLog->Log("dafack");
				break;
			}
		}
	}
	catch (int e)
	{
		FString tt = "GodDamnit" + FString::FromInt(e);
		GLog->Log(*tt);
	}
}

void AVTM::GetParts(TArray<USceneComponent*>& units_ref, TArray<UStaticMeshComponent*>& axles_ref, TArray<UStaticMeshComponent*>& tires_ref)
{
	units_ref = MainUnits;
	axles_ref = Axles;
	tires_ref = Tires;
}

FVector AVTM::GetOffset()
{
	return offset;
}

void AVTM::DisableUE4VehiclePhysic()
{
	GetVehicleMovementComponent()->SetComponentTickEnabled(false);
	GetVehicleMovementComponent()->Deactivate();
	//PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;
	//auto* Bone = GetBodyInstance(NAME_None);
	//if (Bone)
	//{
	//	Bone->SetInstanceSimulatePhysics(false);
	//}
}


int AVTM::Str2UT(FString input)
{
	if (input == "cab")
	{
		GLog->Log("Return Cab Enum");
		return 0;
	}
	if (input == "tractor")
	{

		GLog->Log("Return tractor Enum");
		return 1;
	}
	if (input == "semi-trailer") return 2;
	if (input == "dolly") return 3;
	return 2;
}
//
//FQuat AVTM_Actor::RotToQuat(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
//{
//	float tr = m00 + m11 + m22;
//	float qw; float qx; float qy; float qz;
//
//	if (tr > 0) {
//		float S = sqrt(tr + 1.0) * 2; // S=4*qw 
//		qw = 0.25 * S;
//		qx = (m21 - m12) / S;
//		qy = (m02 - m20) / S;
//		qz = (m10 - m01) / S;
//	}
//	else if ((m00 > m11) && (m00 > m22)) {
//		float S = sqrt(1.0 + m00 - m11 - m22) * 2; // S=4*qx 
//		qw = (m21 - m12) / S;
//		qx = 0.25 * S;
//		qy = (m01 + m10) / S;
//		qz = (m02 + m20) / S;
//	}
//	else if (m11 > m22) {
//		float S = sqrt(1.0 + m11 - m00 - m22) * 2; // S=4*qy
//		qw = (m02 - m20) / S;
//		qx = (m01 + m10) / S;
//		qy = 0.25 * S;
//		qz = (m12 + m21) / S;
//	}
//	else {
//		float S = sqrt(1.0 + m22 - m00 - m11) * 2; // S=4*qz
//		qw = (m10 - m01) / S;
//		qx = (m02 + m20) / S;
//		qy = (m12 + m21) / S;
//		qz = 0.25 * S;
//	}
//
//	return FQuat(qx, qy, qz, qw);
//}