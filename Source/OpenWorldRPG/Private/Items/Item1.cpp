// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item1.h"

// Sets default values
AItem1::AItem1()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AItem1::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItem1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

