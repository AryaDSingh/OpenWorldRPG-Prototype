// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"

#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

//camera
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "GroomComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

//equip
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"

//animation montage
#include "Animation/AnimMontage.h"

#include "Components/BoxComponent.h"

// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// Camera controls rotation
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;

	ActionState = EActionState::EAS_Unoccupied;
	CharacterState = ECharacterState::ECS_Unequipped;

	//camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");


}


//animation montage
void ASlashCharacter::Attack()
{
	//UE_LOG(LogTemp, Warning, TEXT("Attack pressed"));
	
	if (!CanAttack()) return;

	// Strafing attack (Attack3)
	if (IsStrafing())
	{
		PlayAttackMontage(FName("Attack3"));
		ActionState = EActionState::EAS_Attacking;
		return;
	}

	// Normal attacks (Attack1 / Attack2)
	PlayAttackMontage();
	ActionState = EActionState::EAS_Attacking;
}

void ASlashCharacter::Kick()

{
	if (CanAttack())
	{
		PlayAttackMontage(FName("Kick"));
		ActionState = EActionState::EAS_Attacking;

	}
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::Arm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;

	if (CharacterState == ECharacterState::ECS_EquippedOneHandWeapon)
	{
		SetCombatMovement(true);
	}
	else if (CharacterState == ECharacterState::ECS_Unequipped)
	{
		SetCombatMovement(false);
	}
}


void ASlashCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		const int32 Selection = FMath::RandRange(0, 1);
		FName SectionName = FName();

		switch (Selection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;

		case 1:
			SectionName = FName("Attack2");
			break;

		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}

}

void ASlashCharacter::PlayAttackMontage(FName SectionName)
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !AttackMontage) return;

	AnimInstance->Montage_Play(AttackMontage);
	AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
}


void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage) 
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}


void ASlashCharacter::SetCombatMovement(bool bEnableCombat)
{
	if (bEnableCombat)
	{
		// Dark Souls like combat mode
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		// Exploration mode
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

}

bool ASlashCharacter::IsStrafing() const
{
	return FMath::Abs(LastMovementInput.X) > 0.2f;
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}




// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	//set exploration camera at start
	SetCombatMovement(CharacterState != ECharacterState::ECS_Unequipped);
	//enhanced movement
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
}
//enhanced movement binding
void ASlashCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2d>();

	LastMovementInput = MovementVector;

	////does not matter what character is looking at
	//const FVector Forward = GetActorForwardVector();
	//AddMovementInput(Forward, MovementVector.Y);
	//const FVector Right = GetActorRightVector();
	//AddMovementInput(Right, MovementVector.Y);

	/*if (ActionState != EActionState::EAS_Unoccupied) return; */

	if (ActionState != EActionState::EAS_Unoccupied)
	{
		return;
	}
	
	
	//Diretional movement
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDireciton = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDireciton, MovementVector.Y);
	const FVector RightDireciton = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDireciton, MovementVector.X);

}	

//look
void ASlashCharacter::Look(const FInputActionValue & Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2d>(); 

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);

}

// Called every frame
void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//enhanced input compontent

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
	}

	//jump
	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &ASlashCharacter::EKeyPressed);
	
	//attack

	PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &ASlashCharacter::Attack);
	//kick

	PlayerInputComponent->BindAction(
		FName("Kick"),
		IE_Pressed,
		this,
		&ASlashCharacter::Kick
	);
}


void ASlashCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon -> GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void ASlashCharacter::DisableWeaponCollision()
{
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"));
		CharacterState = ECharacterState::ECS_EquippedOneHandWeapon;

		//combat mode
		SetCombatMovement(true);
		OverlappingItem = nullptr;
		EquippedWeapon = OverlappingWeapon;
	}
	else
	{
		if (CanDisarm())
		{
			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
		else if (CanArm())
		{
			PlayEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::ECS_EquippedOneHandWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}

}
