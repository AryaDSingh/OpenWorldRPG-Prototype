// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/SlashCharacter.h"
#include "NiagaraComponent.h"


//Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	RootComponent = ItemMesh;

	//collisions
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	
	Sphere->SetupAttachment(GetRootComponent());

	//niagara

	EmbersEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	EmbersEffect->SetupAttachment(GetRootComponent());
}


// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	// you have this string in the BP_Item 
	UE_LOG(LogTemp, Warning, TEXT("Begin Play called!"));

	if (GEngine) 
	{
		//.f specifies it is a float value
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, FString("Item OnScreen Message!"));
	}


	if (Sphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sphere Exists!"));
	}


	//callback bind to delegate

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FString OtherActorName = OtherActor->GetName();
	/*if (GEngine) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, OtherActorName);
	}*/

	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter) 
	{
		SlashCharacter->SetOverlappingItem(this);
	}



}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const FString OtherActorName = FString ("Ending Overlap with: ") + OtherActor->GetName();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, OtherActorName);
	}

	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter)
	{
		SlashCharacter->SetOverlappingItem(nullptr);
	}



}

float AItem::TransformedSin() const
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("DeltaTime: %f"), DeltaTime);

	RunningTime += DeltaTime;
    
	const float z = TransformedSin(); // period = 2*pi/K
	if (ItemState == EItemState::EIS_Hovering)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
	}

	//if (GEngine)
	//{
	//	//.f specifies it is a float value
	//	FString Message = FString::Printf(TEXT("DeltaTime: %f"), DeltaTime);
	//	GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, Message);

	//}

}


