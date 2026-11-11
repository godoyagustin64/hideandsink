#include "hideandsinkGameMode.h"
#include "HISGameState.h"
#include "HISPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HISHunterCharacter.h"
#include "hideandsinkCharacter.h"
#include "TimerManager.h"

AhideandsinkGameMode::AhideandsinkGameMode()
{
    PlayerStateClass = AHISPlayerState::StaticClass();
    GameStateClass = AHISGameState::StaticClass();
}

void AhideandsinkGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Jugador conectado. Total: %d"),
        GetNumPlayers()
    );

    AHISGameState* HISGameState = GetHISGameState();

    if (!bAutoStartRound || !HISGameState)
    {
        return;
    }

    const bool bEnoughPlayers =
        GetNumPlayers() >= MinimumPlayers;

    const bool bRoundIsWaiting =
        HISGameState->GetRoundPhase() == EHISRoundPhase::Waiting;

    const bool bStartAlreadyScheduled =
        GetWorldTimerManager().IsTimerActive(
            AutoStartTimerHandle
        );

    if (
        bEnoughPlayers &&
        bRoundIsWaiting &&
        !bStartAlreadyScheduled
    )
    {
        GetWorldTimerManager().SetTimer(
            AutoStartTimerHandle,
            this,
            &AhideandsinkGameMode::StartRound,
            AutoStartDelay,
            false
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("La ronda comenzará en %.1f segundos"),
            AutoStartDelay
        );
    }
}

void AhideandsinkGameMode::StartRound()
{
    AHISGameState* HISGameState = GetHISGameState();

    if (!HISGameState)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("No se encontró AHISGameState")
        );

        return;
    }

    if (GetNumPlayers() < MinimumPlayers)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("No hay suficientes jugadores para comenzar")
        );

        return;
    }

    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    GetWorldTimerManager().ClearTimer(AutoStartTimerHandle);

    HISGameState->SetRoundWinner(EHISRoundWinner::None);

    OctopusesNearHunter.Empty();
    LastCloseCallAwardTimes.Empty();
    
    AssignRandomRoles();
    BeginHidingPhase();
}

void AhideandsinkGameMode::AssignRandomRoles()
{
    AHISGameState* HISGameState = GetHISGameState();

    if (!HISGameState)
    {
        return;
    }

    TArray<AHISPlayerState*> ValidPlayers;

    for (APlayerState* BasePlayerState : HISGameState->PlayerArray)
    {
        AHISPlayerState* HISPlayerState =
            Cast<AHISPlayerState>(BasePlayerState);

        if (HISPlayerState)
        {
            HISPlayerState->ResetForNewRound();
            ValidPlayers.Add(HISPlayerState);
        }
    }

    if (ValidPlayers.Num() < MinimumPlayers)
    {
        return;
    }

    const int32 HunterIndex =
        FMath::RandRange(0, ValidPlayers.Num() - 1);

    for (int32 Index = 0; Index < ValidPlayers.Num(); ++Index)
    {
        AHISPlayerState* CurrentPlayer = ValidPlayers[Index];

        if (Index == HunterIndex)
        {
            CurrentPlayer->SetPlayerRole(
                EHISPlayerRole::Hunter
            );

            CurrentPlayer->SetAlive(true);

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("%s fue elegido como CAZADOR"),
                *CurrentPlayer->GetPlayerName()
            );
        }
        else
        {
            CurrentPlayer->SetPlayerRole(
                EHISPlayerRole::Octopus
            );

            CurrentPlayer->SetAlive(true);

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("%s fue elegido como PULPO"),
                *CurrentPlayer->GetPlayerName()
            );
        }
    }

    HISGameState->SetAliveOctopusCount(
        ValidPlayers.Num() - 1
    );
    RespawnPlayersForAssignedRoles();
}

void AhideandsinkGameMode::BeginHidingPhase()
{
    AHISGameState* HISGameState = GetHISGameState();

    if (!HISGameState)
    {
        return;
    }

    HISGameState->SetRoundPhase(EHISRoundPhase::Hiding);
    HISGameState->SetRemainingTime(HidingDuration);

    GetWorldTimerManager().SetTimer(
        RoundTimerHandle,
        this,
        &AhideandsinkGameMode::HandleRoundTimerTick,
        1.0f,
        true
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("FASE DE ESCONDITE: %d segundos"),
        HidingDuration
    );
    SetHunterReleased(false);
}

void AhideandsinkGameMode::BeginHuntingPhase()
{
    AHISGameState* HISGameState = GetHISGameState();

    if (!HISGameState)
    {
        return;
    }

    HISGameState->SetRoundPhase(EHISRoundPhase::Hunting);
    HISGameState->SetRemainingTime(HuntingDuration);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("EL CAZADOR FUE LIBERADO: %d segundos"),
        HuntingDuration
    );
    SetHunterReleased(true);
}

void AhideandsinkGameMode::HandleRoundTimerTick()
{
    AHISGameState* HISGameState = GetHISGameState();

    if (!HISGameState)
    {+
        return;
    }
    if (
    HISGameState->GetRoundPhase() ==
    EHISRoundPhase::Hunting
)
    {
        CheckCloseCallScores();
    }

    const int32 NewRemainingTime =
        HISGameState->GetRemainingTime() - 1;

    HISGameState->SetRemainingTime(NewRemainingTime);

    if (NewRemainingTime > 0)
    {
        return;
    }

    switch (HISGameState->GetRoundPhase())
    {
        case EHISRoundPhase::Hiding:
            BeginHuntingPhase();
            break;

        case EHISRoundPhase::Hunting:
            EndRound(EHISRoundWinner::Octopuses);
            break;

        default:
            break;
    }
}

void AhideandsinkGameMode::RegisterOctopusCaught(
    AHISPlayerState* HunterPlayerState,
    AHISPlayerState* OctopusPlayerState
)
{
    AHISGameState* HISGameState = GetHISGameState();

    if (
        !HISGameState ||
        !HunterPlayerState ||
        !OctopusPlayerState
    )
    {
        return;
    }

    if (
        HISGameState->GetRoundPhase() !=
        EHISRoundPhase::Hunting
    )
    {
        return;
    }

    if (
        HunterPlayerState->GetPlayerRole() !=
        EHISPlayerRole::Hunter
    )
    {
        return;
    }

    if (
        OctopusPlayerState->GetPlayerRole() !=
        EHISPlayerRole::Octopus
    )
    {
        return;
    }

    if (!OctopusPlayerState->IsAlive())
    {
        return;
    }

    OctopusPlayerState->SetAlive(false);
    HunterPlayerState->AddScorePoints(PointsPerCapture);

    const int32 NewAliveCount =
        HISGameState->GetAliveOctopusCount() - 1;

    HISGameState->SetAliveOctopusCount(NewAliveCount);
    APlayerController* OctopusController =
    Cast<APlayerController>(
        OctopusPlayerState->GetOwner()
    );

    if (OctopusController)
    {
        APawn* CaughtPawn = OctopusController->GetPawn();

        OctopusController->StartSpectatingOnly();

        if (CaughtPawn)
        {
            CaughtPawn->Destroy();
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s atrapó a %s"),
        *HunterPlayerState->GetPlayerName(),
        *OctopusPlayerState->GetPlayerName()
    );

    if (NewAliveCount <= 0)
    {
        EndRound(EHISRoundWinner::Hunter);
    }
}

void AhideandsinkGameMode::EndRound(
    EHISRoundWinner Winner
)
{
    AHISGameState* HISGameState = GetHISGameState();

    if (!HISGameState)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    SetHunterReleased(false);


    if (Winner == EHISRoundWinner::Octopuses)
    {
        for (
            APlayerState* BasePlayerState :
            HISGameState->PlayerArray
        )
        {
            AHISPlayerState* HISPlayerState =
                Cast<AHISPlayerState>(BasePlayerState);

            if (
                HISPlayerState &&
                HISPlayerState->GetPlayerRole() ==
                    EHISPlayerRole::Octopus &&
                HISPlayerState->IsAlive()
            )
            {
                HISPlayerState->AddScorePoints(
                    PointsForSurviving
                );
            }
        }
    }

    HISGameState->SetRemainingTime(0);
    HISGameState->SetRoundWinner(Winner);
    HISGameState->SetRoundPhase(EHISRoundPhase::Ended);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("RONDA TERMINADA. GANADOR: %s"),
        Winner == EHISRoundWinner::Hunter
            ? TEXT("CAZADOR")
            : TEXT("PULPOS")
    );
}

AHISGameState* AhideandsinkGameMode::GetHISGameState() const
{
    return GetGameState<AHISGameState>();
}

UClass*
AhideandsinkGameMode::GetDefaultPawnClassForController_Implementation(
    AController* InController
)
{
    if (!InController)
    {
        return Super::GetDefaultPawnClassForController_Implementation(
            InController
        );
    }

    const AHISPlayerState* HISPlayerState =
        InController->GetPlayerState<AHISPlayerState>();

    if (!HISPlayerState)
    {
        return Super::GetDefaultPawnClassForController_Implementation(
            InController
        );
    }

    if (
        HISPlayerState->GetPlayerRole() ==
        EHISPlayerRole::Hunter &&
        HunterPawnClass
    )
    {
        return HunterPawnClass;
    }

    if (
        HISPlayerState->GetPlayerRole() ==
        EHISPlayerRole::Octopus &&
        OctopusPawnClass
    )
    {
        return OctopusPawnClass;
    }

    return Super::GetDefaultPawnClassForController_Implementation(
        InController
    );
}

void AhideandsinkGameMode::RespawnPlayersForAssignedRoles()
{
    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    for (
        FConstPlayerControllerIterator Iterator =
            World->GetPlayerControllerIterator();
        Iterator;
        ++Iterator
    )
    {
        APlayerController* PlayerController =
            Iterator->Get();

        if (!PlayerController)
        {
            continue;
        }

        APawn* PreviousPawn = PlayerController->GetPawn();

        if (PreviousPawn)
        {
            PlayerController->UnPossess();
            PreviousPawn->Destroy();
        }

        RestartPlayer(PlayerController);
    }
}

AHISHunterCharacter*
AhideandsinkGameMode::FindHunterCharacter() const
{
    UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    for (
        FConstPlayerControllerIterator Iterator =
            World->GetPlayerControllerIterator();
        Iterator;
        ++Iterator
    )
    {
        APlayerController* PlayerController =
            Iterator->Get();

        if (!PlayerController)
        {
            continue;
        }

        AHISPlayerState* HISPlayerState =
            PlayerController->GetPlayerState<AHISPlayerState>();

        if (
            HISPlayerState &&
            HISPlayerState->GetPlayerRole() ==
                EHISPlayerRole::Hunter
        )
        {
            return Cast<AHISHunterCharacter>(
                PlayerController->GetPawn()
            );
        }
    }

    return nullptr;
}

void AhideandsinkGameMode::SetHunterReleased(
    bool bReleased
)
{
    AHISHunterCharacter* HunterCharacter =
        FindHunterCharacter();

    if (!HunterCharacter)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("No se encontró al personaje cazador")
        );

        return;
    }

    HunterCharacter->SetHunterReleased(bReleased);
}

void AhideandsinkGameMode::CheckCloseCallScores()
{
    AHISGameState* HISGameState = GetHISGameState();
    AHISHunterCharacter* HunterCharacter =
        FindHunterCharacter();

    if (!HISGameState || !HunterCharacter)
    {
        return;
    }

    if (
        HISGameState->GetRoundPhase() !=
        EHISRoundPhase::Hunting
    )
    {
        return;
    }

    const float CurrentTime =
        GetWorld()->GetTimeSeconds();

    const FVector HunterLocation =
        HunterCharacter->GetActorLocation();

    const float CloseDistanceSquared =
        FMath::Square(CloseCallDistance);

    const float ResetDistanceSquared =
        FMath::Square(CloseCallResetDistance);

    for (
        APlayerState* BasePlayerState :
        HISGameState->PlayerArray
    )
    {
        AHISPlayerState* OctopusPlayerState =
            Cast<AHISPlayerState>(BasePlayerState);

        if (
            !OctopusPlayerState ||
            OctopusPlayerState->GetPlayerRole() !=
                EHISPlayerRole::Octopus ||
            !OctopusPlayerState->IsAlive()
        )
        {
            continue;
        }

        APlayerController* OctopusController =
            Cast<APlayerController>(
                OctopusPlayerState->GetOwner()
            );

        if (!OctopusController)
        {
            continue;
        }

        AhideandsinkCharacter* OctopusCharacter =
            Cast<AhideandsinkCharacter>(
                OctopusController->GetPawn()
            );

        if (!OctopusCharacter)
        {
            continue;
        }

        const float DistanceSquared =
            FVector::DistSquared(
                HunterLocation,
                OctopusCharacter->GetActorLocation()
            );

        TWeakObjectPtr<AHISPlayerState> OctopusKey =
            OctopusPlayerState;

        if (DistanceSquared <= CloseDistanceSquared)
        {
            const bool bWasAlreadyNear =
                OctopusesNearHunter.Contains(OctopusKey);

            /*
             * Lo marcamos como cercano aunque todavía no esté
             * disfrazado. Así no puede transformarse junto al
             * cazador para activar puntos artificialmente.
             */
            OctopusesNearHunter.Add(OctopusKey);

            if (bWasAlreadyNear)
            {
                continue;
            }

            if (!OctopusCharacter->IsDisguised())
            {
                continue;
            }

            const float* LastAwardTime =
                LastCloseCallAwardTimes.Find(OctopusKey);

            const bool bCooldownFinished =
                !LastAwardTime ||
                CurrentTime - *LastAwardTime >=
                    CloseCallCooldown;

            if (!bCooldownFinished)
            {
                continue;
            }

            OctopusPlayerState->AddScorePoints(
                PointsForCloseCall
            );

            LastCloseCallAwardTimes.Add(
                OctopusKey,
                CurrentTime
            );

            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "%s ganó %d puntos porque "
                    "el cazador pasó cerca"
                ),
                *OctopusPlayerState->GetPlayerName(),
                PointsForCloseCall
            );
        }
        else if (
            DistanceSquared >= ResetDistanceSquared
        )
        {
            /*
             * El cazador se alejó lo suficiente.
             * Una futura aproximación puede volver a puntuar.
             */
            OctopusesNearHunter.Remove(OctopusKey);
        }
    }
}