// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Enemy.h"

// =======================
// Core / Engine
// =======================
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

// =======================
// AI
// =======================
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

// =======================
// Components
// =======================
#include "Components/SkeletalMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"

// =======================
// Items / Rewards
// =======================
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"

// =======================
// Debug
// =======================
#include "DrawDebugHelpers.h"

// Forward declarations
class UAnimMontage;
class AWeapon;

/* =====================================================
 * Constructor
 * ===================================================== */

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyState = EEnemyState::EES_Patrolling;

	// Mesh collision setup
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Health bar UI
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	// Movement settings
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Pawn sensing (vision-based AI awareness)
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 1000.f;
	PawnSensing->SetPeripheralVisionAngle(40.f);
}

/* =====================================================
 * <Actor> Overrides (Public)
 * ===================================================== */

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	// Decide between patrol logic and combat logic
	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}

	// Optional debug visualization
	if (bShowCombatRadius)
	{
		ShowCombatRadius();
	}
}

float AEnemy::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	HandleDamage(DamageAmount);

	// Set combat target to the instigator
	CombatTarget = EventInstigator->GetPawn();

	if (IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}
	else if (IsOutsideAttackRadius())
	{
		ChaseTarget();
	}

	return DamageAmount;
}

void AEnemy::Destroyed()
{
	// Clean up equipped weapon
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

/* =====================================================
 * IHitInterface
 * ===================================================== */

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	if (!IsDead())
	{
		ShowHealthBar();
	}

	// Reset AI state on hit
	ClearPatrolTimer();
	ClearAttackTimer();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	StopAttackMontage();

	if (IsInsideAttackRadius())
	{
		if (!IsDead())
		{
			StartAttackTimer();
		}
	}
}

/* =====================================================
 * <Actor> Overrides (Protected)
 * ===================================================== */

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Bind pawn sensing callback
	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	InitializeEnemy();
	Tags.Add(FName("Enemy"));
}

/* =====================================================
 * <ABaseCharacter> Overrides
 * ===================================================== */

void AEnemy::Die_Implementation()
{
	Super::Die_Implementation();

	EnemyState = EEnemyState::EES_Dead;

	ClearPatrolTimer();
	ClearAttackTimer();
	HideHealthBar();

	DisableCapsule();
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(DeathLifeSpan);
	SpawnSoul();
}

void AEnemy::Attack()
{
	Super::Attack();

	if (CombatTarget == nullptr) return;

	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

bool AEnemy::CanAttack()
{
	const bool bCanAttack =
		IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsEngaged() &&
		!IsDead();

	return bCanAttack;
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (Attributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

/* =====================================================
 * AI Behaviour (Private)
 * ===================================================== */

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());

	if (PatrolTargets.Num() > 0)
	{
		PatrolTarget = PatrolTargets[0];
		MoveToTarget(PatrolTarget);
	}

	HideHealthBar();
	SpawnDefaultWeapon();
}

void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();

		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(
			PatrolTimer,
			this,
			&AEnemy::PatrolTimerFinished,
			WaitTime);
	}
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();

		if (!IsEngaged())
		{
			StartPatrolling();
		}
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();

		if (!IsEngaged())
		{
			ChaseTarget();
		}
	}
	else if (CanAttack())
	{
		StartAttackTimer();
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;

	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(
		AttackTimer,
		this,
		&AEnemy::Attack,
		AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();

	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();

	if (World && SoulClass && Attributes)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 25.f);
		ASoul* SpawnedSoul =
			World->SpawnActor<ASoul>(SoulClass, SpawnLocation, GetActorRotation());

		if (SpawnedSoul)
		{
			SpawnedSoul->SetSouls(Attributes->GetSouls());
		}
	}
}

/* =====================================================
 * Navigation Helpers
 * ===================================================== */

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

	FNavPathSharedPtr NavPath;
	EnemyController->MoveTo(MoveRequest, &NavPath);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;

	const double DistanceToTarget =
		(Target->GetActorLocation() - GetActorLocation()).Size();

	return DistanceToTarget <= Radius;
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;

	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}

	const int32 NumTargets = ValidTargets.Num();
	if (NumTargets > 0)
	{
		const int32 Selection = FMath::RandRange(0, NumTargets - 1);
		return ValidTargets[Selection];
	}

	return nullptr;
}

/* =====================================================
 * Pawn Sensing
 * ===================================================== */

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		SeenPawn->ActorHasTag(FName("EngageableTarget"));

	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

/* =====================================================
 * Debug
 * ===================================================== */

void AEnemy::ShowCombatRadius()
{
	if (!bShowCombatRadius) return;

	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		AttackRadius,
		16,
		FColor::Red,
		false,
		-1.f,
		0,
		2.f
	);

	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		AcceptanceRadius,
		16,
		FColor::Blue,
		false,
		-1.f,
		0,
		2.f
	);
}
