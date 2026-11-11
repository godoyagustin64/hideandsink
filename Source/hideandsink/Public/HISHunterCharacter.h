#pragma once

#include "CoreMinimal.h"
#include "CombatCharacter.h"
#include "HISHunterCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class AHISHarpoonProjectile;
class USoundBase;

UCLASS()
class HIDEANDSINK_API AHISHunterCharacter : public ACombatCharacter
{
	GENERATED_BODY()

public:
	AHISHunterCharacter();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	/*
	 * Solamente debe ser llamada por el GameMode
	 * desde el servidor.
	 */
	void SetHunterReleased(bool bNewReleased);

	UFUNCTION(BlueprintPure, Category = "Hide And Sink|Hunter")
	bool IsHunterReleased() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_HunterReleased,BlueprintReadOnly,Category = "Hide And Sink|Hunter")
	bool bHunterReleased = false;

	UFUNCTION()
	void OnRep_HunterReleased();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Harpoon")
	TObjectPtr<UInputAction> HarpoonFireAction;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Harpoon",meta = (ClampMin = "100.0"))
	float HarpoonRange = 5000.0f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Harpoon",meta = (ClampMin = "0.1"))
	float HarpoonCooldown = 0.75f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Debug")
	bool bDrawHarpoonDebug = true;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide and Sink|Harpoon")
	TSubclassOf<AHISHarpoonProjectile> HarpoonProjectileClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Hide and Sink|Harpoon")
	float HarpoonProjectileSpeed = 5000.0f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide and Sink|Harpoon")
	TObjectPtr<USoundBase> HarpoonFireSound;

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent
	) override;

	void FireHarpoon();
	
	void SpawnHarpoonProjectile(const FVector& SpawnLocation,const FVector& TargetLocation);

	UFUNCTION(Server, Reliable)
	void ServerFireHarpoon();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHarpoonEffects(
		FVector ShotLocation
	);
	
private:
	void ApplyHunterReleasedState();
	
	float LastHarpoonFireTime = -100.0f;

	void PerformHarpoonTrace();
};