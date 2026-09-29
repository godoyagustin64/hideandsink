// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "hideandsinkCharacter.generated.h"

class USpringArmComponent;
class UUserWidget;
class UCameraComponent;
class UInputAction;
class ATransformableProp;
class UStaticMeshComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AhideandsinkCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transformacion",
	meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> DisguiseMesh;
	
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* TransformAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ReturnToOctopusAction;

	UPROPERTY(EditAnywhere, Category = "Transformacion", meta = (ClampMin = "1.0"))
	float TransformRange = 250.0f;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* RotatePropAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Interfaz")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CrosshairWidget;

public:

	/** Constructor */
	AhideandsinkCharacter();	
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void PawnClientRestart() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	//BORDE RESALTADO
	void UpdateTargetHighlight();
	TWeakObjectPtr<ATransformableProp> HighlightedProp;
	
	//ROTACION
	bool bRotatePropHeld = false;
	void StartRotatingProp();
	void StopRotatingProp();
	
	UPROPERTY(EditAnywhere, Category = "Transformacion")
	float DisguiseRotationSpeed = 90.0f; // Grados por segundo

	UPROPERTY(ReplicatedUsing = OnRep_DisguiseRotation)
	FRotator DisguiseRotation = FRotator::ZeroRotator;

	UFUNCTION()
	void OnRep_DisguiseRotation();

	UFUNCTION(Server, Unreliable)
	void ServerSetDisguiseRotation(FRotator NewRotation);

	UFUNCTION(Server, Reliable)
	void ServerFinishDisguiseRotation(FRotator FinalRotation);
	
	void TryReturnToOctopus();
	
	UFUNCTION(Server, Reliable)
	void ServerReturnToOctopus();
	

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	void TryTransform();
	
	ATransformableProp* FindTransformableProp() const;
	
	
	// FUNCIONES PARA REPLICAR EN SERVER 
	
	// El cliente solicita copiar este prop. El servidor decide si es válido.
	UFUNCTION(Server, Reliable)
	void ServerTryTransform(ATransformableProp* RequestedProp);

	// Se ejecuta en los clientes cuando cambia el prop copiado.
	UFUNCTION()
	void OnRep_DisguiseProp();

	// Actualiza las mallas; se llama explícitamente también en el servidor.
	void ApplyDisguise();

	UPROPERTY(ReplicatedUsing = OnRep_DisguiseProp)
	TObjectPtr<ATransformableProp> DisguiseProp;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

