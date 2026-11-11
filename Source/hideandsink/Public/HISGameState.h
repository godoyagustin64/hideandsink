#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HISGameState.generated.h"

UENUM(BlueprintType)
enum class EHISRoundPhase : uint8
{
    Waiting  UMETA(DisplayName = "Esperando"),
    Hiding   UMETA(DisplayName = "Escondiéndose"),
    Hunting  UMETA(DisplayName = "Cacería"),
    Ended    UMETA(DisplayName = "Partida terminada")
};

UENUM(BlueprintType)
enum class EHISRoundWinner : uint8
{
    None       UMETA(DisplayName = "Nadie"),
    Hunter     UMETA(DisplayName = "Cazador"),
    Octopuses  UMETA(DisplayName = "Pulpos")
};

UCLASS()
class HIDEANDSINK_API AHISGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AHISGameState();

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    EHISRoundPhase GetRoundPhase() const;

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    int32 GetRemainingTime() const;

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    int32 GetAliveOctopusCount() const;

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    EHISRoundWinner GetRoundWinner() const;


    void SetRoundPhase(EHISRoundPhase NewPhase);
    void SetRemainingTime(int32 NewRemainingTime);
    void SetAliveOctopusCount(int32 NewCount);
    void SetRoundWinner(EHISRoundWinner NewWinner);

protected:
    UPROPERTY(ReplicatedUsing = OnRep_RoundPhase, BlueprintReadOnly, Category = "Hide And Sink|Round")
    EHISRoundPhase RoundPhase = EHISRoundPhase::Waiting;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 RemainingTime = 0;

    UPROPERTY(ReplicatedUsing = OnRep_AliveOctopusCount,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 AliveOctopusCount = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundWinner,BlueprintReadOnly,Category = "Hide And Sink|Round")
    EHISRoundWinner RoundWinner = EHISRoundWinner::None;

    UFUNCTION()
    void OnRep_RoundPhase();

    UFUNCTION()
    void OnRep_RemainingTime();

    UFUNCTION()
    void OnRep_AliveOctopusCount();

    UFUNCTION()
    void OnRep_RoundWinner();

    
    
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Hide And Sink|Round")
    void OnRoundPhaseUpdated(EHISRoundPhase NewPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hide And Sink|Round")
    void OnRemainingTimeUpdated(int32 NewRemainingTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hide And Sink|Round")
    void OnAliveOctopusCountUpdated(int32 NewCount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hide And Sink|Round")
    void OnRoundWinnerUpdated(EHISRoundWinner NewWinner);

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;
};