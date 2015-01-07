// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Superspace.h"
#include "SuperspaceGameMode.h"
#include "SuperspacePawn.h"

ASuperspaceGameMode::ASuperspaceGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// set default pawn class to our character class
	DefaultPawnClass = ASuperspacePawn::StaticClass();
}

