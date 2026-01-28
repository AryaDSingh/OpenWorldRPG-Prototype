// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class OPENWORLDRPG_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	//need to make sure name matches Healthbar, same as HealthBar in blueprints
	class UProgressBar* HealthBar;
};
