// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Pawn.h"

//#include <Components/CapsuleComponent.h"

#include "InputACtionValue.h"
// bird.generated has to be last
#include "Bird.generated.h"

//forward declare
class UCapsuleComponent;
class USkeletalMeshComponent;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;



UCLASS()
class OPENWORLDRPG_API ABird : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABird();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//bird movement

	void MoveForward(float Value);

	//enhanced input

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* BirdMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;
	
	//looking around with mouse
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	


private:
	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent *Capsule;
	
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* BirdMesh;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm; 

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;




};
