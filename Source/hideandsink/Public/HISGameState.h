#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HISGameState.generated.h"



// FASES DE RONDA

UENUM(BlueprintType)
enum class EHISRoundPhase : uint8
{
    Waiting  UMETA(DisplayName = "Esperando"),
    Hiding   UMETA(DisplayName = "Escondiéndose"),
    Hunting  UMETA(DisplayName = "Cacería"),
    Ended    UMETA(DisplayName = "Partida terminada")
};


// ROLES

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
    
//GETTERS

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    EHISRoundPhase GetRoundPhase() const;

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    int32 GetRemainingTime() const;

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    int32 GetAliveOctopusCount() const;

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    EHISRoundWinner GetRoundWinner() const;
    
    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    int32 GetCurrentRound() const;

    UFUNCTION(BlueprintPure, Category = "Hide And Sink|Round")
    int32 GetTotalRounds() const;

//SETTERS    
    
    void SetRoundPhase(EHISRoundPhase NewPhase);
    void SetRemainingTime(int32 NewRemainingTime);
    void SetAliveOctopusCount(int32 NewCount);
    void SetRoundWinner(EHISRoundWinner NewWinner);
    void SetCurrentRound(int32 NewCurrentRound);
    void SetTotalRounds(int32 NewTotalRounds);    
    
    




protected:
    
    UPROPERTY(ReplicatedUsing = OnRep_RoundPhase, BlueprintReadOnly, Category = "Hide And Sink|Round")
    EHISRoundPhase RoundPhase = EHISRoundPhase::Waiting;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 RemainingTime = 0;

    UPROPERTY(ReplicatedUsing = OnRep_AliveOctopusCount,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 AliveOctopusCount = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundWinner,BlueprintReadOnly,Category = "Hide And Sink|Round")
    EHISRoundWinner RoundWinner = EHISRoundWinner::None;
    
    UPROPERTY(Replicated,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 CurrentRound = 0;

    UPROPERTY(Replicated,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 TotalRounds = 0;

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