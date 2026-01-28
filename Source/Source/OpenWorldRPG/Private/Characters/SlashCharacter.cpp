// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/SlashCharacter.h"

// =======================
// Components
// =======================
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AttributeComponent.h"

// =======================
// Enhanced Input
// =======================
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// =======================
// Camera
// =======================
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// =======================
// Groom
// =======================
#include "GroomComponent.h"

// =======================
// Movement
// =======================
#include "GameFramework/CharacterMovementComponent.h"

// =======================
// Items
// =======================
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"

// =======================
// Animation
// =======================
#include "Animation/AnimMontage.h"

// =======================
// HUD
// =======================
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

/* =====================================================
 * Constructor
 * ===================================================== */

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	ActionState = EActionState::EAS_Unoccupied;
	CharacterState = ECharacterState::ECS_Unequipped;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility,
		ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_WorldDynamic,
		ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// Groom
	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");
}

/* =====================================================
 * Lifecycle
 * ===================================================== */

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("EngageableTarget"));

	InitializeSlashOverlay();
	SetCombatMovement(CharacterState != ECharacterState::ECS_Unequipped);

	if (APlayerController* PlayerController =
		Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
}

void ASlashCharacter::Tick(float DeltaTime)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		SlashOverlay->SetStaminaBarPercent(
			Attributes->GetStaminaPercent());
	}
}

/* =====================================================
 * Input Setup
 * ===================================================== */

void ASlashCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent =
		CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(
			MovementAction,
			ETriggerEvent::Triggered,
			this,
			&ASlashCharacter::Move);

		EnhancedInputComponent->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&ASlashCharacter::Look);
	}

	PlayerInputComponent->BindAction(
		FName("Jump"),
		IE_Pressed,
		this,
		&ACharacter::Jump);

	PlayerInputComponent->BindAction(
		FName("Equip"),
		IE_Pressed,
		this,
		&ASlashCharacter::EKeyPressed);

	PlayerInputComponent->BindAction(
		FName("Attack"),
		IE_Pressed,
		this,
		&ASlashCharacter::Attack);

	PlayerInputComponent->BindAction(
		FName("Dodge"),
		IE_Pressed,
		this,
		&ASlashCharacter::Dodge);

	PlayerInputComponent->BindAction(
		FName("Kick"),
		IE_Pressed,
		this,
		&ASlashCharacter::Kick);
}

/* =====================================================
 * Damage / Hit Reactions
 * ===================================================== */

void ASlashCharacter::GetHit_Implementation(
	const FVector& ImpactPoint,
	AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

float ASlashCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

/* =====================================================
 * Pickup Interface
 * ===================================================== */

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

/* =====================================================
 * Blueprint Callable
 * ===================================================== */

void ASlashCharacter::DisableWeaponCollision()
{
}

/* =====================================================
 * Combat & Animation
 * ===================================================== */

void ASlashCharacter::Attack()
{
	Super::Attack();

	if (!CanAttack()) return;

	PlayAttackMontage();
	ActionState = EActionState::EAS_Attacking;
}

void ASlashCharacter::Kick()
{
	if (CanAttack())
	{
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughStamina()) return;

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;

	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		SlashOverlay->SetStaminaBarPercent(
			Attributes->GetStaminaPercent());
	}
}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes &&
		Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool ASlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

/* =====================================================
 * Equipment
 * ===================================================== */

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(
		GetMesh(),
		FName("RightHandSocket"),
		this,
		this);

	CharacterState = ECharacterState::ECS_EquippedOneHandWeapon;
	SetCombatMovement(true);

	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

bool ASlashCharacter::CanDisarm()
{
	return EquippedWeapon &&
		ActionState == EActionState::EAS_Unoccupied &&
		CharacterState ==
		ECharacterState::ECS_EquippedOneHandWeapon;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedOneHandWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (!EquippedWeapon) return;

	EquippedWeapon->AttachMeshToSocket(
		GetMesh(),
		FName("SpineSocket"));

	CharacterState = ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (!EquippedWeapon) return;

	EquippedWeapon->AttachMeshToSocket(
		GetMesh(),
		FName("RightHandSocket"));

	CharacterState = ECharacterState::ECS_EquippedOneHandWeapon;
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;

	if (CharacterState ==
		ECharacterState::ECS_EquippedOneHandWeapon)
	{
		SetCombatMovement(true);
	}
	else
	{
		SetCombatMovement(false);
	}
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::PlayEquipMontage(
	const FName& SectionName)
{
	UAnimInstance* AnimInstance =
		GetMesh()->GetAnimInstance();

	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(
			SectionName,
			EquipMontage);
	}
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	SetCombatMovement(false);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

/* =====================================================
 * Movement Helpers
 * ===================================================== */

void ASlashCharacter::SetCombatMovement(bool bEnableCombat)
{
	if (bEnableCombat)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

bool ASlashCharacter::IsStrafing() const
{
	return FMath::Abs(LastMovementInput.X) > 0.2f;
}

/* =====================================================
 * Input
 * ===================================================== */

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon =
		Cast<AWeapon>(OverlappingItem);

	if (OverlappingWeapon)
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
		}
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::Move(
	const FInputActionValue& Value)
{
	const FVector2D MovementVector =
		Value.Get<FVector2D>();

	LastMovementInput = MovementVector;

	if (ActionState != EActionState::EAS_Unoccupied)
	{
		return;
	}

	const FRotator Rotation =
		Controller->GetControlRotation();

	const FRotator YawRotation(
		0.f,
		Rotation.Yaw,
		0.f);

	const FVector Forward =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::X);

	const FVector Right =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MovementVector.Y);
	AddMovementInput(Right, MovementVector.X);
}

void ASlashCharacter::Look(
	const FInputActionValue& Value)
{
	const FVector2D LookAxisVector =
		Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASlashCharacter::Jump()
{
	if (!IsAlive()) return;

	Super::Jump();
}

/* =====================================================
 * HUD
 * ===================================================== */

void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		ASlashHUD* SlashHUD =
			Cast<ASlashHUD>(PlayerController->GetHUD());

		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();

			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(
					Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(
			Attributes->GetHealthPercent());
	}
}
