// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/*==============================
	Core Includes
==============================*/
#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

/*==============================
	Forward Declarations
==============================*/
class USoundBase;
class UBoxComponent;
class USceneComponent;

/**
 * Weapon item used for melee combat
 */
UCLASS()
class OPENWORLDRPG_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	/*==============================
		Constructor
	==============================*/
	AWeapon();

	/*==============================
		Weapon Interface
	==============================*/

	// Equip weapon to a socket on a parent component
	void Equip(
		USceneComponent* InParent,
		FName InSocketName,
		AActor* NewOwner,
		APawn* NewInstigator
	);

	// Disable ember / idle effects when equipped
	void DeactivateEmbers();

	// Disable pickup collision sphere
	void DisableSphereCollision();

	// Play equip sound
	void PlayEquipSound();

	// Attach weapon mesh to a socket
	void AttachMeshToSocket(
		USceneComponent* InParent,
		const FName& InSocketName
	);

	/*==============================
		Weapon Collision
	==============================*/

	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }

	// Actors already hit during current swing
	TArray<AActor*> IgnoreActors;

protected:
	/*==============================
		Lifecycle
	==============================*/

	virtual void BeginPlay() override;

	/*==============================
		Collision Handling
	==============================*/

	// Called when weapon box overlaps another actor
	UFUNCTION()
	void OnBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// Prevent friendly-fire between same actor types
	bool ActorIsSameType(AActor* OtherActor);

	// Calls GetHit on hit actor via interface
	void ExecuteGetHit(FHitResult& BoxHit);

	// Blueprint hook for spawning hit fields (Niagara, decals, etc.)
	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

private:
	/*==============================
		Tracing
	==============================*/

	// Performs box trace for hit detection
	void BoxTrace(FHitResult& BoxHit);

	/*==============================
		Weapon Properties
	==============================*/

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* EquipSound;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;

	/*==============================
		Components
	==============================*/

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UBoxComponent* WeaponBox;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd;
};
