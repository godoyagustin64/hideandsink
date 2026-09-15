#include "MainMenuWidget.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CreateGameButton)
	{
		CreateGameButton->OnClicked.AddDynamic(
			this,
			&UMainMenuWidget::OnCreateGameClicked
		);
	}

	if (JoinGameButton)
	{
		JoinGameButton->OnClicked.AddDynamic(
			this,
			&UMainMenuWidget::OnJoinGameClicked
		);
	}

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.AddDynamic(
			this,
			&UMainMenuWidget::OnQuitGameClicked
		);
	}
}

void UMainMenuWidget::OnCreateGameClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Crear partida presionado"));
}

void UMainMenuWidget::OnJoinGameClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Unirse a partida presionado"));
}

void UMainMenuWidget::OnQuitGameClicked()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetOwningPlayer(),
		EQuitPreference::Quit,
		false
	);
}