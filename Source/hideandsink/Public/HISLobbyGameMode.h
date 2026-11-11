#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HISLobbyGameMode.generated.h"

UCLASS()
class HIDEANDSINK_API AHISLobbyGameMode
	: public AGameModeBase
{
	GENERATED_BODY()

public:
	AHISLobbyGameMode();

	UFUNCTION(BlueprintCallable, Category = "Hide And Sink|Lobby")
	void StartGame();

protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Lobby")
	int32 MinimumPlayersToStart = 2;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Hide And Sink|Lobby")
	FString GameplayMapPath = TEXT("/Game/Maps/HuntLevel1");
};