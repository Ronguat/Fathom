#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FMOceanActor.generated.h"

class UDynamicMeshComponent;
class UMaterialInterface;

/** The visible surface: a Geometry Script plane wearing the ocean material, whose vertices the GPU displaces. No collision. */
UCLASS()
class FATHOM_API AFMOceanActor : public AActor
{
	GENERATED_BODY()

public:
	AFMOceanActor();

	void Build(float Size, int32 Steps, UMaterialInterface* Material);

protected:
	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UDynamicMeshComponent> Mesh;
};
