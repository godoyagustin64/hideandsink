#include "HISLobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"

AHISLobbyGameMode::AHISLobbyGameMode()
{
	/*
	 * En el Lobby no necesitamos crear los personajes
	 * jugables todavía.
	 */
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = true;

	/*
	 * Al viajar al mapa jugable se crearán nuevamente
	 * los Controllers y aparecerá el HUD de la partida.
	 */
	bUseSeamlessTravel = false;
}

void AHISLobbyGameMode::StartGame()
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 ConnectedPlayers =
		GameState
		? GameState->PlayerArray.Num()
		: 0;

	if (ConnectedPlayers < MinimumPlayersToStart)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"No hay suficientes jugadores. "
				"Conectados: %d | Necesarios: %d"
			),
			ConnectedPlayers,
			MinimumPlayersToStart
		);

		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const FString TravelURL =
		GameplayMapPath + TEXT("?listen");

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Iniciando partida con %d jugadores: %s"
		),
		ConnectedPlayers,
		*TravelURL
	);

	World->ServerTravel(
		TravelURL,
		false
	);
}