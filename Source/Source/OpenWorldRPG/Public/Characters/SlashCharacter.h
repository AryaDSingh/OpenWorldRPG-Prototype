// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/* ==============================
 * Core Includes
 * ============================== */
#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "InputActionValue.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

 /* ==============================
  * Forward Declarations
  * ============================== */

  // UI
class USlashOverlay;

// Animation
class UAnimMontage;

// Groom
class UGroomComponent;

// Camera
class UCameraComponent;
class USpringArmComponent;

// Enhanced Input
class UInputMappingContext;
class UInputAction;

// Items / Equipment
class AItem;
class ASoul;
class AWeapon;
class ATreasure;

/**
 * Player-controlled melee character.
 * Handles input, combat flow, equipment, movement modes, and HUD updates.
 */
UCLASS()
class OPENWORLDRPG_API ASlashCharacter
	: public ABaseCharacter
	, public IPickupInterface
{
	GENERATED_BODY()

public:

	/* =====================================================
	 * Constructor & Overrides
	 * ===================================================== */

	ASlashCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(
		class UInputComponent* PlayerInputComponent) override;

	virtual void GetHit_Implementation(
		const FVector& ImpactPoint,
		AActor* Hitter) override;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	/* =====================================================
	 * Pickup Interface
	 * ===================================================== */

	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;

	/* =====================================================
	 * Inline Getters
	 * ===================================================== */

	FORCEINLINE ECharacterState GetCharacterState() const
	{
		return CharacterState;
	}

	FORCEINLINE EActionState GetActionState() const
	{
		return ActionState;
	}

	/* =====================================================
	 * Blueprint Callable
	 * ===================================================== */

	UFUNCTION(BlueprintCallable)
	void DisableWeaponCollision();

protected:

	/* =====================================================
	 * Combat & Animation
	 * ===================================================== */

	virtual void Attack() override;
	void Kick();
	void Dodge();

	bool HasEnoughStamina();
	bool IsOccupied();

	virtual void AttackEnd() override;
	virtual bool CanAttack() override;
	virtual void DodgeEnd() override;

	// Equipment handling
	void EquipWeapon(AWeapon* Weapon);
	bool CanDisarm();
	bool CanArm();
	void Disarm();
	void Arm();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	void PlayEquipMontage(const FName& SectionName);

	virtual void Die_Implementation() override;

	/* =====================================================
	 * Movement & Combat State
	 * ===================================================== */

	void SetCombatMovement(bool bEnableCombat);
	bool IsStrafing() const;

	/* =====================================================
	 * Lifecycle
	 * ===================================================== */

	virtual void BeginPlay() override;

	/* =====================================================
	 * Input Handling
	 * ===================================================== */

	void EKeyPressed();

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	virtual void Jump() override;

	/* =====================================================
	 * Camera
	 * ===================================================== */

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	/* =====================================================
	 * Groom
	 * ===================================================== */

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;

private:

	/* =====================================================
	 * Animation Data
	 * ===================================================== */

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	/* =====================================================
	 * State Tracking
	 * ===================================================== */

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	/* =====================================================
	 * Interaction & Movement
	 * ===================================================== */

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	FVector2D LastMovementInput;

	/* =====================================================
	 * HUD
	 * ===================================================== */

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	void InitializeSlashOverlay();
	void SetHUDHealth();
};
