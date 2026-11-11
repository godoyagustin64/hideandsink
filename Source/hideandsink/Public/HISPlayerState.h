#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HISPlayerState.generated.h"

UENUM(BlueprintType)
enum class EHISPlayerRole : uint8
{
	None     UMETA(DisplayName = "Sin rol"),
	Hunter   UMETA(DisplayName = "Cazador"),
	Octopus  UMETA(DisplayName = "Pulpo")
};

UCLASS()
class HIDEANDSINK_API AHISPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHISPlayerState();


	UFUNCTION(BlueprintPure, Category = "Hide And Sink|Player")
	EHISPlayerRole GetPlayerRole() const;


	UFUNCTION(BlueprintPure, Category = "Hide And Sink|Score")
	int32 GetScorePoints() const;


	UFUNCTION(BlueprintPure, Category = "Hide And Sink|Player")
	bool IsAlive() const;


	void SetPlayerRole(EHISPlayerRole NewRole);
	void AddScorePoints(int32 PointsToAdd);
	void SetAlive(bool bNewAlive);
	void ResetForNewRound();

protected:
    
	UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Hide And Sink|Player")
	EHISPlayerRole PlayerRole = EHISPlayerRole::None;

	UPROPERTY(ReplicatedUsing = OnRep_ScorePoints, BlueprintReadOnly, Category = "Hide And Sink|Score")
	int32 ScorePoints = 0;

	UPROPERTY(ReplicatedUsing = OnRep_IsAlive, BlueprintReadOnly, Category = "Hide And Sink|Player")
	bool bIsAlive = true;

	UFUNCTION()
	void OnRep_PlayerRole();

	UFUNCTION()
	void OnRep_ScorePoints();

	UFUNCTION()
	void OnRep_IsAlive();


	UFUNCTION(BlueprintImplementableEvent, Category = "Hide And Sink|Player")
	void OnPlayerDataUpdated();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};