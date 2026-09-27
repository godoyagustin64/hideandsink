// Fill out your copyright notice in the Description page of Project Settings.


#include "TransformableProp.h"


// Sets default values
ATransformableProp::ATransformableProp()
{
	PrimaryActorTick.bCanEverTick = false;
	PropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropMesh"));
	RootComponent = PropMesh;
	PropMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

