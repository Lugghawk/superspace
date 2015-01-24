// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "SuperspacePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SUPERSPACE_API ASuperspacePlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	void DoLog();
};
