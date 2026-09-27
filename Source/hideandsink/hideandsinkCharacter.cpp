// Copyright Epic Games, Inc. All Rights Reserved.

#include "hideandsinkCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "hideandsink.h"
#include "TransformableProp.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"




AhideandsinkCharacter::AhideandsinkCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
	DisguiseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisguiseMesh"));
	DisguiseMesh->SetupAttachment(RootComponent);
	DisguiseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisguiseMesh->SetVisibility(false);
}

void AhideandsinkCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AhideandsinkCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AhideandsinkCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AhideandsinkCharacter::Look);
		
		// Transform
		EnhancedInputComponent->BindAction(TransformAction, ETriggerEvent::Started, this, &AhideandsinkCharacter::TryTransform);
	}
	else
	{
		UE_LOG(Loghideandsink, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AhideandsinkCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AhideandsinkCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AhideandsinkCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AhideandsinkCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AhideandsinkCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AhideandsinkCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}


ATransformableProp* AhideandsinkCharacter::FindTransformableProp() const
{
	if (!FollowCamera || !GetWorld())
	{
		return nullptr;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + FollowCamera->GetForwardVector() * 1000.0f;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, QueryParams))
	{
		return nullptr;
	}

	ATransformableProp* HitProp = Cast<ATransformableProp>(Hit.GetActor());
	if (!HitProp)
	{
		return nullptr;
	}

	if (FVector::Dist(GetActorLocation(), Hit.ImpactPoint) > TransformRange)
	{
		return nullptr;
	}

	return HitProp;
}

void AhideandsinkCharacter::TryTransform()
{
	ATransformableProp* Prop = FindTransformableProp();
	if (!Prop || !DisguiseMesh)
	{
		return;
	}

	UStaticMeshComponent* SourceMesh = Prop->GetPropMesh();
	if (!SourceMesh || !SourceMesh->GetStaticMesh())
	{
		return;
	}

	// Copiamos la apariencia; el prop original sigue en el mapa.
	DisguiseMesh->SetStaticMesh(SourceMesh->GetStaticMesh());
	DisguiseMesh->SetRelativeScale3D(SourceMesh->GetComponentScale());

	for (int32 Index = 0; Index < SourceMesh->GetNumMaterials(); ++Index)
	{
		DisguiseMesh->SetMaterial(Index, SourceMesh->GetMaterial(Index));
	}

	// Ubicamos la malla cerca del piso, tomando como referencia la cápsula.
	DisguiseMesh->SetRelativeLocation(
		FVector(0.0f, 0.0f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	);

	DisguiseMesh->SetVisibility(true);
	GetMesh()->SetVisibility(false);
}


void AhideandsinkCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// El resaltado es una ayuda visual para el jugador que controla este pulpo.
	if (IsLocallyControlled())
	{
		UpdateTargetHighlight();
	}
}

void AhideandsinkCharacter::UpdateTargetHighlight()
{
	ATransformableProp* NewTarget = FindTransformableProp();

	if (HighlightedProp.Get() == NewTarget)
	{
		return;
	}

	if (ATransformableProp* PreviousProp = HighlightedProp.Get())
	{
		if (UStaticMeshComponent* PropMesh = PreviousProp->GetPropMesh())
		{
			PropMesh->SetRenderCustomDepth(false);
		}
	}

	HighlightedProp = NewTarget;

	if (NewTarget)
	{
		if (UStaticMeshComponent* PropMesh = NewTarget->GetPropMesh())
		{
			PropMesh->SetRenderCustomDepth(true);
		}
	}
}

void AhideandsinkCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	APlayerController* LocalPC = Cast<APlayerController>(GetController());
	if (!LocalPC || !LocalPC->IsLocalController() ||
		!CrosshairWidgetClass || CrosshairWidget)
	{
		return;
	}

	CrosshairWidget = CreateWidget<UUserWidget>(
		LocalPC,
		CrosshairWidgetClass
	);

	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport();
	}
}
