#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HISGameState.h"
#include "hideandsinkGameMode.generated.h"

class AHISPlayerState;
class AHISHunterCharacter;
class AhideandsinkCharacter;

UCLASS(Abstract)
class HIDEANDSINK_API AhideandsinkGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AhideandsinkGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    // NUEVA RONDA
    UFUNCTION(BlueprintCallable, Category = "Hide And Sink|Round")
    void StartRound();


    UFUNCTION(BlueprintCallable, Category = "Hide And Sink|Round")
    void RegisterOctopusCaught(AHISPlayerState* HunterPlayerState, AHISPlayerState* OctopusPlayerState);
    
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    
    virtual void Logout(AController* Exiting) override;

protected:

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 HidingDuration = 30;


    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 HuntingDuration = 180;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Round")
    int32 MinimumPlayers = 2;


    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Testing")
    bool bAutoStartRound = true;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Testing")
    float AutoStartDelay = 3.0f;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Score")
    int32 PointsPerCapture = 100;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Score")
    int32 PointsForSurviving = 100;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Characters")
    TSubclassOf<APawn> HunterPawnClass;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Characters")
    TSubclassOf<APawn> OctopusPawnClass;
    
    
    ///
    
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Score")
    int32 PointsForCloseCall = 15;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Score")
    float CloseCallDistance = 500.0f;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Score")
    float CloseCallResetDistance = 800.0f;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Score")
    float CloseCallCooldown = 10.0f;
    
    
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Match",meta = (ClampMin = "1"))
    int32 RoundsPerMatch = 3;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Match",meta = (ClampMin = "1.0"))
    float RoundEndDuration = 8.0f;
    
    
    ///
    
    
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Spawn")
    FName HunterPlayerStartTag = TEXT("Hunter");

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Spawn")
    FName OctopusPlayerStartTag = TEXT("Octopus");
    
    
private:
    FTimerHandle RoundTimerHandle;
    FTimerHandle AutoStartTimerHandle;

    void AssignRandomRoles();
    void BeginHidingPhase();
    void BeginHuntingPhase();
    void HandleRoundTimerTick();
    void EndRound(EHISRoundWinner Winner);
    void RespawnPlayersForAssignedRoles();

    AHISGameState* GetHISGameState() const;
    
    AHISHunterCharacter* FindHunterCharacter() const;
    void SetHunterReleased(bool bReleased);
    
    
    TSet<TWeakObjectPtr<AHISPlayerState>>
    OctopusesNearHunter;

    TMap<TWeakObjectPtr<AHISPlayerState>, float>
        LastCloseCallAwardTimes;

    void CheckCloseCallScores();
    
    FTimerHandle RoundEndTimerHandle;

    void HandleRoundEndFinished();
    void FinishMatch();
    
    
    TSet<TWeakObjectPtr<AActor>> UsedRoundPlayerStarts;
    
};

