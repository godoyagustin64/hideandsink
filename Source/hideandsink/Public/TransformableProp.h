// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransformableProp.generated.h"

class UStaticMeshComponent;

UCLASS()
class HIDEANDSINK_API ATransformableProp : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATransformableProp();
	UStaticMeshComponent* GetPropMesh() const { return PropMesh; }

	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PropMesh;
};
