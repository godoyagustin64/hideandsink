#include "HISPlayerState.h"
#include "Net/UnrealNetwork.h"

AHISPlayerState::AHISPlayerState()
{
    bReplicates = true;
}

EHISPlayerRole AHISPlayerState::GetPlayerRole() const
{
    return PlayerRole;
}

int32 AHISPlayerState::GetScorePoints() const
{
    return ScorePoints;
}

bool AHISPlayerState::IsAlive() const
{
    return bIsAlive;
}

void AHISPlayerState::SetPlayerRole(EHISPlayerRole NewRole)
{
    if (!HasAuthority())
    {
        return;
    }

    PlayerRole = NewRole;
    OnRep_PlayerRole();
}

void AHISPlayerState::AddScorePoints(int32 PointsToAdd)
{
    if (!HasAuthority() || PointsToAdd <= 0)
    {
        return;
    }

    ScorePoints += PointsToAdd;
    OnRep_ScorePoints();
}

void AHISPlayerState::SetAlive(bool bNewAlive)
{
    if (!HasAuthority())
    {
        return;
    }

    bIsAlive = bNewAlive;
    OnRep_IsAlive();
}

void AHISPlayerState::ResetForNewRound()
{
    if (!HasAuthority())
    {
        return;
    }

    PlayerRole = EHISPlayerRole::None;
    bIsAlive = true;

    // No reseteamos ScorePoints porque será el puntaje total
    // acumulado entre rondas.

    OnRep_PlayerRole();
    OnRep_IsAlive();
}

void AHISPlayerState::OnRep_PlayerRole()
{
    const UEnum* RoleEnum = StaticEnum<EHISPlayerRole>();

    const FString RoleName = RoleEnum
        ? RoleEnum->GetNameStringByValue(
            static_cast<int64>(PlayerRole)
        )
        : TEXT("Unknown");

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Jugador %s - Rol actualizado: %s"),
        *GetPlayerName(),
        *RoleName
    );

    OnPlayerDataUpdated();
}

void AHISPlayerState::OnRep_ScorePoints()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Jugador %s - Puntaje actualizado: %d"),
        *GetPlayerName(),
        ScorePoints
    );

    OnPlayerDataUpdated();
}

void AHISPlayerState::OnRep_IsAlive()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Jugador %s - Estado: %s"),
        *GetPlayerName(),
        bIsAlive ? TEXT("Vivo") : TEXT("Eliminado")
    );

    OnPlayerDataUpdated();
}

void AHISPlayerState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AHISPlayerState, PlayerRole);
    DOREPLIFETIME(AHISPlayerState, ScorePoints);
    DOREPLIFETIME(AHISPlayerState, bIsAlive);
}