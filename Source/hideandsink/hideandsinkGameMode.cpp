#include "hideandsinkGameMode.h"
#include "HISGameState.h"
#include "HISPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HISHunterCharacter.h"
#include "hideandsinkCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
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

    const int32 ConnectedPlayerCount =
    HISGameState->PlayerArray.Num();

    if (ConnectedPlayerCount < MinimumPlayers)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "No hay suficientes jugadores conectados. "
                "Conectados: %d"
            ),
            ConnectedPlayerCount
        );

        return;
    }
    
    if (
    HISGameState->GetCurrentRound() >=
    RoundsPerMatch
)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Ya se jugaron todas las rondas")
        );

        return;
    }

    GetWorldTimerManager().ClearTimer(
        RoundEndTimerHandle
    );

    HISGameState->SetTotalRounds(RoundsPerMatch);

    HISGameState->SetCurrentRound(
        HISGameState->GetCurrentRound() + 1
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "COMENZANDO RONDA %d DE %d"
        ),
        HISGameState->GetCurrentRound(),
        HISGameState->GetTotalRounds()
    );

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
    {
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

    if (!HasAuthority())
    {
        return;
    }

    AHISGameState* HISGameState =
        GetHISGameState();

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

    HunterPlayerState->AddScorePoints(
        PointsPerCapture
    );

    const int32 NewAliveCount =
        HISGameState->GetAliveOctopusCount() - 1;

    HISGameState->SetAliveOctopusCount(
        NewAliveCount
    );

    APlayerController* OctopusController =
        Cast<APlayerController>(
            OctopusPlayerState->GetOwner()
        );

    if (OctopusController)
    {

        APawn* CaughtPawn =
            OctopusController->GetPawn();
        
        AhideandsinkCharacter* OctopusCharacter =
            Cast<AhideandsinkCharacter>(
                CaughtPawn
            );

        if (OctopusCharacter)
        {
            OctopusCharacter->ClientNotifyCaught();

            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "Servidor envió ClientNotifyCaught a %s"
                ),
                *OctopusPlayerState->GetPlayerName()
            );
        }
        OctopusController->UnPossess();

        if (CaughtPawn)
        {
            CaughtPawn->Destroy();
        }

        OctopusController->ChangeState(
            NAME_Spectating
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "%s pasó temporalmente a espectador"
            ),
            *OctopusPlayerState->GetPlayerName()
        );
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
        EndRound(
            EHISRoundWinner::Hunter
        );
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
    
    GetWorldTimerManager().SetTimer(
    RoundEndTimerHandle,
    this,
    &AhideandsinkGameMode::HandleRoundEndFinished,
    RoundEndDuration,
    false
);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "La siguiente ronda comenzará en %.1f segundos"
        ),
        RoundEndDuration
    );
}

void AhideandsinkGameMode::HandleRoundEndFinished()
{
    AHISGameState* HISGameState =
        GetHISGameState();

    if (!HISGameState)
    {
        return;
    }
    const int32 ConnectedPlayerCount =
    HISGameState->PlayerArray.Num();

    if (ConnectedPlayerCount < MinimumPlayers)
    {
        HISGameState->SetRoundPhase(
            EHISRoundPhase::Waiting
        );

        HISGameState->SetRoundWinner(
            EHISRoundWinner::None
        );

        HISGameState->SetRemainingTime(0);
        HISGameState->SetAliveOctopusCount(0);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "No hay suficientes jugadores. "
                "La partida queda esperando."
            )
        );

        return;
    }

    if (
        HISGameState->GetCurrentRound() <
        HISGameState->GetTotalRounds()
    )
    {
        StartRound();
        return;
    }

    FinishMatch();
}

void AhideandsinkGameMode::FinishMatch()
{
    AHISGameState* HISGameState =
        GetHISGameState();

    if (!HISGameState)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(
        RoundEndTimerHandle
    );

    AHISPlayerState* BestPlayer = nullptr;

    for (
        APlayerState* BasePlayerState :
        HISGameState->PlayerArray
    )
    {
        AHISPlayerState* HISPlayerState =
            Cast<AHISPlayerState>(
                BasePlayerState
            );

        if (!HISPlayerState)
        {
            continue;
        }

        if (
            !BestPlayer ||
            HISPlayerState->GetScorePoints() >
            BestPlayer->GetScorePoints()
        )
        {
            BestPlayer = HISPlayerState;
        }
    }

    if (BestPlayer)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "PARTIDA TERMINADA | "
                "GANADOR: %s | PUNTOS: %d"
            ),
            *BestPlayer->GetPlayerName(),
            BestPlayer->GetScorePoints()
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PARTIDA TERMINADA SIN GANADOR")
        );
    }
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
    UsedRoundPlayerStarts.Empty();
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

        PlayerController->ChangeState(NAME_Playing);
        
        APawn* PreviousPawn = PlayerController->GetPawn();

        if (PreviousPawn)
        {
            PlayerController->UnPossess();
            PreviousPawn->Destroy();
        }
        
        AHISPlayerState* HISPlayerState =
    PlayerController->GetPlayerState<
        AHISPlayerState
    >();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Respawn | Jugador: %s | Rol: %d"
            ),
            *GetNameSafe(PlayerController->PlayerState),
            HISPlayerState
                ? static_cast<int32>(
                    HISPlayerState->GetPlayerRole()
                )
                : -1
        );

        AActor* SelectedPlayerStart =
    ChoosePlayerStart(PlayerController);

        if (SelectedPlayerStart)
        {
            RestartPlayerAtPlayerStart(
                PlayerController,
                SelectedPlayerStart
            );
        }
        else
        {
            RestartPlayer(PlayerController);
        }
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
            OctopusesNearHunter.Remove(OctopusKey);
        }
    }
}


AActor*
AhideandsinkGameMode::ChoosePlayerStart_Implementation(
    AController* Player
)
{
    if (!Player)
    {
        return Super::ChoosePlayerStart_Implementation(
            Player
        );
    }

    const AHISPlayerState* HISPlayerState =
        Player->GetPlayerState<AHISPlayerState>();

    if (!HISPlayerState)
    {
        return Super::ChoosePlayerStart_Implementation(
            Player
        );
    }

    FName DesiredTag = NAME_None;

    switch (HISPlayerState->GetPlayerRole())
    {
        case EHISPlayerRole::Hunter:
            DesiredTag = HunterPlayerStartTag;
            break;

        case EHISPlayerRole::Octopus:
            DesiredTag = OctopusPlayerStartTag;
            break;

        default:
            return Super::ChoosePlayerStart_Implementation(
                Player
            );
    }

    TArray<APlayerStart*> AvailableStarts;
    TArray<APlayerStart*> AllMatchingStarts;

    for (
        TActorIterator<APlayerStart> Iterator(GetWorld());
        Iterator;
        ++Iterator
    )
    {
        APlayerStart* PlayerStart = *Iterator;

        if (
            !PlayerStart ||
            PlayerStart->PlayerStartTag != DesiredTag
        )
        {
            continue;
        }

        AllMatchingStarts.Add(PlayerStart);

        if (
            !UsedRoundPlayerStarts.Contains(PlayerStart)
        )
        {
            AvailableStarts.Add(PlayerStart);
        }
    }

    APlayerStart* SelectedStart = nullptr;

    if (!AvailableStarts.IsEmpty())
    {
        const int32 RandomIndex =
            FMath::RandRange(
                0,
                AvailableStarts.Num() - 1
            );

        SelectedStart = AvailableStarts[RandomIndex];
    }
    else if (!AllMatchingStarts.IsEmpty())
    {
        const int32 RandomIndex =
            FMath::RandRange(
                0,
                AllMatchingStarts.Num() - 1
            );

        SelectedStart = AllMatchingStarts[RandomIndex];
    }

    if (SelectedStart)
    {
        UsedRoundPlayerStarts.Add(SelectedStart);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Spawn seleccionado | Jugador: %s | "
                "Rol: %d | PlayerStart: %s"
            ),
            *GetNameSafe(Player->PlayerState),
            static_cast<int32>(
                HISPlayerState->GetPlayerRole()
            ),
            *SelectedStart->GetName()
        );

        return SelectedStart;
    }

    UE_LOG(
        LogTemp,
        Error,
        TEXT(
            "No se encontró PlayerStart con tag: %s"
        ),
        *DesiredTag.ToString()
    );

    return Super::ChoosePlayerStart_Implementation(
        Player
    );
}


void AhideandsinkGameMode::Logout(
    AController* Exiting
)
{
    AHISGameState* HISGameState =
        GetHISGameState();

    AHISPlayerState* LeavingPlayerState =
        Exiting
        ? Exiting->GetPlayerState<AHISPlayerState>()
        : nullptr;

    /*
     * Guardamos los datos antes de llamar a Super,
     * porque después el PlayerState será retirado
     * del PlayerArray.
     */
    const EHISPlayerRole LeavingRole =
        LeavingPlayerState
        ? LeavingPlayerState->GetPlayerRole()
        : EHISPlayerRole::None;

    const bool bWasAlive =
        LeavingPlayerState &&
        LeavingPlayerState->IsAlive();

    const FString LeavingPlayerName =
        LeavingPlayerState
        ? LeavingPlayerState->GetPlayerName()
        : TEXT("Jugador desconocido");

    Super::Logout(Exiting);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "Jugador desconectado: %s | Rol: %d"
        ),
        *LeavingPlayerName,
        static_cast<int32>(LeavingRole)
    );

    if (!HISGameState)
    {
        return;
    }

    const EHISRoundPhase CurrentPhase =
        HISGameState->GetRoundPhase();

    const bool bRoundInProgress =
        CurrentPhase == EHISRoundPhase::Hiding ||
        CurrentPhase == EHISRoundPhase::Hunting;

    if (!bRoundInProgress)
    {
        return;
    }

    if (LeavingRole == EHISPlayerRole::Hunter)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "El cazador se desconectó. "
                "Ganan los pulpos."
            )
        );

        EndRound(EHISRoundWinner::Octopuses);
        return;
    }

    if (
        LeavingRole == EHISPlayerRole::Octopus &&
        bWasAlive
    )
    {
        const int32 NewAliveCount =
            FMath::Max(
                0,
                HISGameState->GetAliveOctopusCount() - 1
            );

        HISGameState->SetAliveOctopusCount(
            NewAliveCount
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Un pulpo vivo se desconectó. "
                "Pulpos restantes: %d"
            ),
            NewAliveCount
        );

        if (NewAliveCount <= 0)
        {
            EndRound(EHISRoundWinner::Hunter);
        }
    }
}

void AhideandsinkGameMode::HandleSeamlessTravelPlayer(
    AController*& Controller
)
{
    Super::HandleSeamlessTravelPlayer(Controller);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "Jugador recibido mediante Seamless Travel: %s"
        ),
        *GetNameSafe(Controller)
    );

    if (!bAutoStartRound)
    {
        return;
    }

    /*
     * HandleSeamlessTravelPlayer se ejecuta una vez por jugador.
     * Programamos un único chequeo.
     */
    if (
        !GetWorldTimerManager().IsTimerActive(
            AutoStartTimerHandle
        )
    )
    {
        GetWorldTimerManager().SetTimer(
            AutoStartTimerHandle,
            this,
            &AhideandsinkGameMode::
                TryStartRoundAfterSeamlessTravel,
            AutoStartDelay,
            false
        );
    }
}

void AhideandsinkGameMode::
TryStartRoundAfterSeamlessTravel()
{
    AHISGameState* HISGameState =
        GetHISGameState();

    if (!HISGameState)
    {
        return;
    }

    const int32 ConnectedPlayerCount =
        HISGameState->PlayerArray.Num();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "Chequeo después de Seamless Travel | "
            "Jugadores: %d/%d"
        ),
        ConnectedPlayerCount,
        MinimumPlayers
    );

    if (
        ConnectedPlayerCount < MinimumPlayers
    )
    {

        GetWorldTimerManager().SetTimer(
            AutoStartTimerHandle,
            this,
            &AhideandsinkGameMode::
                TryStartRoundAfterSeamlessTravel,
            1.0f,
            false
        );

        return;
    }

    if (
        HISGameState->GetRoundPhase() !=
        EHISRoundPhase::Waiting
    )
    {
        return;
    }

    StartRound();
}