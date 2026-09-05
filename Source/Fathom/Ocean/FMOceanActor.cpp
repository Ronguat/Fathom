#include "Ocean/FMOceanActor.h"

#include "Components/DynamicMeshComponent.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "UDynamicMesh.h"

AFMOceanActor::AFMOceanActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	Mesh = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetCastShadow(false);
	RootComponent = Mesh;
}

void AFMOceanActor::Build(float Size, int32 Steps, UMaterialInterface* Material)
{
	UDynamicMesh* Target = Mesh->GetDynamicMesh();
	Target->Reset();
	FGeometryScriptPrimitiveOptions Options;
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendRectangleXY(Target, Options, FTransform::Identity, Size, Size, Steps, Steps);
	Mesh->NotifyMeshUpdated();
	Mesh->SetMaterial(0, Material);
	Mesh->SetBoundsScale(2.0f);
}
