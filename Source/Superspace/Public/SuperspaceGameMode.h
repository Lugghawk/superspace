// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "SuperspacePawn.h"
#include "GameFramework/GameMode.h"
#include "SuperspaceGameMode.generated.h"


UCLASS()
class ASuperspaceGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASuperspaceGameMode(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(reliable, server, WithValidation)
	void KillPawn(APawn* Dealer, APawn* PawnToKill);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category= "Spawn")
	TArray<FVector> SpawnLocations;

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FVector GetRandomSpawnLocation();
};



