// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterTypes.h"
#include "InputActionValue.h"
#include "SlashCharacter.generated.h"


class USlashOverlay;
class UAnimMontage;

//hair/eyebrows

class UGroomComponent;

//camera
class UCameraComponent;
class USpringArmComponent;

//enhanced movement
class UInputMappingContext;
class UInputAction;

//equip
class AItem;

//anim montage

class UAnimMontage;
class AWeapon;

UCLASS()
class OPENWORLDRPG_API ASlashCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASlashCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//inline-> code is expanded at the point of call during compilation, reducing the overhead associated with function calls 
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState;  }

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	void DisableWeaponCollision();

protected:
	// animation montage
	void Attack();
	//kick
	void Kick();
	//play montage functions
	void PlayAttackMontage();

	//play montage functions
	void PlayAttackMontage(FName SectionName);

	//camera mode when in combat

	void SetCombatMovement(bool bEnableCombat);

	//check if moving left or right for horiziontal 360 attack
	bool IsStrafing() const;

	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	bool CanAttack();

	bool CanDisarm();
	bool CanArm();
	UFUNCTION(BlueprintCallable)
	void Disarm();
	UFUNCTION(BlueprintCallable)
	void Arm();
	
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	void PlayEquipMontage(const FName& SectionName);
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//equip
	void EKeyPressed();

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* SlashContext;
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MovementAction;
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	//camera

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	//Hair/eyebrows
	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows; 
private:
	//animation montage
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(VisibleAnywhere)
	AWeapon* EquippedWeapon;

	// for strafe recognization
	FVector2D LastMovementInput;


};
