#include "HISHunterCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "HISGameState.h"
#include "HISPlayerState.h"
#include "hideandsinkGameMode.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"

AHISHunterCharacter::AHISHunterCharacter()
{
	bReplicates = true;
}

void AHISHunterCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyHunterReleasedState();
}

void AHISHunterCharacter::SetHunterReleased(
	bool bNewReleased
)
{
	if (!HasAuthority())
	{
		return;
	}

	bHunterReleased = bNewReleased;

	/*
	 * Ejecutamos la actualización manualmente en el servidor.
	 * En los clientes se ejecutará mediante OnRep.
	 */
	ApplyHunterReleasedState();

	ForceNetUpdate();
}

bool AHISHunterCharacter::IsHunterReleased() const
{
	return bHunterReleased;
}

void AHISHunterCharacter::OnRep_HunterReleased()
{
	ApplyHunterReleasedState();
}

void AHISHunterCharacter::ApplyHunterReleasedState()
{
	UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement();

	if (!MovementComponent)
	{
		return;
	}

	if (bHunterReleased)
	{
		MovementComponent->SetMovementMode(MOVE_Walking);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Cazador liberado")
		);
	}
	else
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Cazador bloqueado")
		);
	}
}

void AHISHunterCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(
		AHISHunterCharacter,
		bHunterReleased
	);
}

void AHISHunterCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Hunter: configurando inputs")
    );

    UEnhancedInputComponent* EnhancedInput =
        Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!EnhancedInput)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Hunter: InputComponent no es EnhancedInput")
        );

        return;
    }

    if (!HarpoonFireAction)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Hunter: HarpoonFireAction no está asignada")
        );

        return;
    }

    EnhancedInput->BindAction(
        HarpoonFireAction,
        ETriggerEvent::Started,
        this,
        &AHISHunterCharacter::FireHarpoon
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Hunter: IA_HarpoonFire enlazada correctamente")
    );
}

void AHISHunterCharacter::FireHarpoon()
{
    UE_LOG(
    LogTemp,
    Warning,
    TEXT("Hunter: click de arpón recibido")
);
    if (!IsLocallyControlled())
    {
        return;
    }

    if (!bHunterReleased)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("El cazador todavía está bloqueado")
        );

        return;
    }

    ServerFireHarpoon();
}

void AHISHunterCharacter::ServerFireHarpoon_Implementation()
{
    if (!bHunterReleased)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    const AHISGameState* HISGameState =
        World->GetGameState<AHISGameState>();

    if (
        !HISGameState ||
        HISGameState->GetRoundPhase() !=
            EHISRoundPhase::Hunting
    )
    {
        return;
    }

    const float CurrentTime = World->GetTimeSeconds();

    if (
        CurrentTime - LastHarpoonFireTime <
        HarpoonCooldown
    )
    {
        return;
    }

    LastHarpoonFireTime = CurrentTime;

    PerformHarpoonTrace();
}

void AHISHunterCharacter::PerformHarpoonTrace()
{
    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    AController* HunterController = GetController();

    if (!HunterController)
    {
        return;
    }

    FVector TraceStart;
    FRotator ViewRotation;

    HunterController->GetPlayerViewPoint(
        TraceStart,
        ViewRotation
    );

    const FVector TraceEnd =
        TraceStart +
        ViewRotation.Vector() * HarpoonRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;

    FHitResult HitResult;

    const bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    const FVector DebugEnd =
        bHit ? HitResult.ImpactPoint : TraceEnd;

    if (bDrawHarpoonDebug)
    {
        DrawDebugLine(
            World,
            TraceStart,
            DebugEnd,
            bHit ? FColor::Green : FColor::Red,
            false,
            2.0f,
            0,
            2.0f
        );

        if (bHit)
        {
            DrawDebugSphere(
                World,
                HitResult.ImpactPoint,
                12.0f,
                12,
                FColor::Yellow,
                false,
                2.0f
            );
        }
        if (bHit)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Arpón golpeó Actor: %s | Componente: %s"),
                *GetNameSafe(HitResult.GetActor()),
                *GetNameSafe(HitResult.GetComponent())
            );
        }
        else
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("El arpón no golpeó nada")
            );
        }
    }

    if (!bHit)
    {
        return;
    }

    APawn* HitPawn = Cast<APawn>(
        HitResult.GetActor()
    );

    if (!HitPawn)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("El arpón golpeó: %s"),
            *GetNameSafe(HitResult.GetActor())
        );

        return;
    }

    AHISPlayerState* HunterPlayerState =
        GetPlayerState<AHISPlayerState>();

    AHISPlayerState* TargetPlayerState =
        HitPawn->GetPlayerState<AHISPlayerState>();

    if (!HunterPlayerState || !TargetPlayerState)
    {
        return;
    }

    AhideandsinkGameMode* HISGameMode =
        World->GetAuthGameMode<AhideandsinkGameMode>();

    if (!HISGameMode)
    {
        return;
    }

    HISGameMode->RegisterOctopusCaught(
        HunterPlayerState,
        TargetPlayerState
    );
}