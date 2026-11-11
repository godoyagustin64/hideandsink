#include "HISHarpoonProjectile.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHISHarpoonProjectile::AHISHarpoonProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	HarpoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		TEXT("HarpoonMesh")
	);

	SetRootComponent(HarpoonMesh);
	
	HarpoonMesh->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	HarpoonMesh->SetGenerateOverlapEvents(false);

	ProjectileMovement =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovement")
		);

	ProjectileMovement->UpdatedComponent = HarpoonMesh;

	ProjectileMovement->InitialSpeed = 5000.0f;
	ProjectileMovement->MaxSpeed = 5000.0f;

	ProjectileMovement->ProjectileGravityScale = 0.0f;

	ProjectileMovement->bRotationFollowsVelocity = true;

	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = 2.0f;
}

void AHISHarpoonProjectile::InitializeHarpoon(
	const FVector& Direction,
	float Speed
)
{
	if (!ProjectileMovement)
	{
		return;
	}

	const FVector NormalizedDirection =
		Direction.GetSafeNormal();

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;

	ProjectileMovement->Velocity =
		NormalizedDirection * Speed;

	SetActorRotation(
		NormalizedDirection.Rotation()
	);
}