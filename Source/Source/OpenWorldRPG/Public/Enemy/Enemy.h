// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// =======================
// Core
// =======================
#include "CoreMinimal.h"

// =======================
// Interfaces / Base
// =======================
#include "Interfaces/HitInterface.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"

#include "Enemy.generated.h"

// =======================
// Forward Declarations
// =======================
class UHealthBarComponent;
class UPawnSensingComponent;
class AAIController;
class AWeapon;
class ASoul;

/**
 * Enemy character class
 * Handles AI behavior such as patrolling, chasing, and attacking,
 * as well as combat reactions and rewards.
 */
UCLASS()
class OPENWORLDRPG_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:

	/* =====================================================
	 * Constructor
	 * ===================================================== */

	 // Sets default values for this character's properties
	AEnemy();

	/* =====================================================
	 * <Actor> Overrides (Public)
	 * ===================================================== */

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;
	virtual void Destroyed() override;

	/* =====================================================
	 * IHitInterface
	 * ===================================================== */

	virtual void GetHit_Implementation(
		const FVector& ImpactPoint,
		AActor* Hitter) override;

protected:

	/* =====================================================
	 * <Actor> Overrides (Protected)
	 * ===================================================== */

	virtual void BeginPlay() override;

	/* =====================================================
	 * <ABaseCharacter> Overrides
	 * ===================================================== */

	virtual void Die_Implementation() override;
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;

	/* =====================================================
	 * State
	 * ===================================================== */

	 // Current high-level enemy AI state
	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

private:

	/* =====================================================
	 * AI Behaviour
	 * ===================================================== */

	void InitializeEnemy();
	void CheckPatrolTarget();
	void CheckCombatTarget();
	void PatrolTimerFinished();

	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();

	void StartPatrolling();
	void ChaseTarget();

	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();

	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();

	void ClearPatrolTimer();
	void StartAttackTimer();
	void ClearAttackTimer();

	void SpawnDefaultWeapon();
	void SpawnSoul();

	/* =====================================================
	 * Navigation Helpers
	 * ===================================================== */

	void MoveToTarget(AActor* Target);
	bool InTargetRange(AActor* Target, double Radius);
	AActor* ChoosePatrolTarget();

	/* =====================================================
	 * Pawn Sensing
	 * ===================================================== */

	 // Callback bound to UPawnSensingComponent::OnSeePawn
	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

	/* =====================================================
	 * Components
	 * ===================================================== */

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	/* =====================================================
	 * Combat / Equipment
	 * ===================================================== */

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<ASoul> SoulClass;

	/* =====================================================
	 * Combat Ranges
	 * ===================================================== */

	UPROPERTY(EditAnywhere, Category = Combat)
	double CombatRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AcceptanceRadius = 75.f;

	/* =====================================================
	 * AI Navigation
	 * ===================================================== */

	 // AI controller controlling this enemy
	UPROPERTY()
	AAIController* EnemyController;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	// All possible patrol targets
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	// Max distance considered "at patrol target"
	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	// Patrol timing
	FTimerHandle PatrolTimer;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 10.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 125.f;

	/* =====================================================
	 * Combat Timing
	 * ===================================================== */

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 8.f;

	/* =====================================================
	 * Debug
	 * ===================================================== */

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bShowCombatRadius = false;

	void ShowCombatRadius();
};
