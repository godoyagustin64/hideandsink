#include "HISGameState.h"
#include "Net/UnrealNetwork.h"

AHISGameState::AHISGameState()
{
    bReplicates = true;
}

void AHISGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AHISGameState, RoundPhase);
    DOREPLIFETIME(AHISGameState, RemainingTime);
    DOREPLIFETIME(AHISGameState, AliveOctopusCount);
    DOREPLIFETIME(AHISGameState, RoundWinner);
    DOREPLIFETIME(AHISGameState, CurrentRound);
    DOREPLIFETIME(AHISGameState, TotalRounds);
}

EHISRoundPhase AHISGameState::GetRoundPhase() const
{
    return RoundPhase;
}

int32 AHISGameState::GetRemainingTime() const
{
    return RemainingTime;
}

int32 AHISGameState::GetAliveOctopusCount() const
{
    return AliveOctopusCount;
}

EHISRoundWinner AHISGameState::GetRoundWinner() const
{
    return RoundWinner;
}

int32 AHISGameState::GetCurrentRound() const
{
    return CurrentRound;
}

int32 AHISGameState::GetTotalRounds() const
{
    return TotalRounds;
}

void AHISGameState::SetRoundPhase(EHISRoundPhase NewPhase)
{
    if (!HasAuthority())
    {
        return;
    }

    RoundPhase = NewPhase;
    OnRep_RoundPhase();
}

void AHISGameState::SetRemainingTime(int32 NewRemainingTime)
{
    if (!HasAuthority())
    {
        return;
    }

    RemainingTime = FMath::Max(0, NewRemainingTime);
    OnRep_RemainingTime();
}

void AHISGameState::SetAliveOctopusCount(int32 NewCount)
{
    if (!HasAuthority())
    {
        return;
    }

    AliveOctopusCount = FMath::Max(0, NewCount);
    OnRep_AliveOctopusCount();
}

void AHISGameState::SetRoundWinner(EHISRoundWinner NewWinner)
{
    if (!HasAuthority())
    {
        return;
    }

    RoundWinner = NewWinner;
    OnRep_RoundWinner();
}

void AHISGameState::SetCurrentRound(
    int32 NewCurrentRound
)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentRound = FMath::Max(0, NewCurrentRound);

    ForceNetUpdate();
}

void AHISGameState::SetTotalRounds(
    int32 NewTotalRounds
)
{
    if (!HasAuthority())
    {
        return;
    }

    TotalRounds = FMath::Max(1, NewTotalRounds);

    ForceNetUpdate();
}

void AHISGameState::OnRep_RoundPhase()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Nueva fase de la ronda: %d"),
        static_cast<int32>(RoundPhase)
    );

    OnRoundPhaseUpdated(RoundPhase);
}

void AHISGameState::OnRep_RemainingTime()
{
    UE_LOG(
        LogTemp,
        Log,
        TEXT("Tiempo restante: %d"),
        RemainingTime
    );

    OnRemainingTimeUpdated(RemainingTime);
}

void AHISGameState::OnRep_AliveOctopusCount()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Pulpos vivos: %d"),
        AliveOctopusCount
    );

    OnAliveOctopusCountUpdated(AliveOctopusCount);
}

void AHISGameState::OnRep_RoundWinner()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Ganador actualizado: %d"),
        static_cast<int32>(RoundWinner)
    );

    OnRoundWinnerUpdated(RoundWinner);
}






