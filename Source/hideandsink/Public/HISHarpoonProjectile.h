#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HISHarpoonProjectile.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class HIDEANDSINK_API AHISHarpoonProjectile : public AActor
{
	GENERATED_BODY()

public:

	AHISHarpoonProjectile();

protected:

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Harpoon"
	)
	TObjectPtr<UStaticMeshComponent> HarpoonMesh;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Harpoon"
	)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

public:

	void InitializeHarpoon(
		const FVector& Direction,
		float Speed
	);
};
