// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Superspace.h"
#include "SuperspaceGameMode.h"
#include "SuperspacePlayerController.h"
#include "SuperspacePawn.h"
#include "SuperspaceGameState.h"


ASuperspaceGameMode::ASuperspaceGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// set default pawn class to our character class
	DefaultPawnClass = ASuperspacePawn::StaticClass();

	GameStateClass = ASuperspaceGameState::StaticClass();
	static ConstructorHelpers::FObjectFinder<UBlueprint> PlayerControllerBP(TEXT("Blueprint'/Game/Blueprint/Controller/SSPlayerController.SSPlayerController'"));
	if (PlayerControllerBP.Object != NULL)	PlayerControllerClass = (UClass*)PlayerControllerBP.Object->GeneratedClass;
}

/*
Will hold checks to determine if a pawn can be killed.
*/
bool ASuperspaceGameMode::KillPawn_Validate(APawn* Dealer, APawn* PawnToKill){
	return true;
}

void ASuperspaceGameMode::KillPawn_Implementation(APawn* Dealer, APawn* PawnToKill){

	ASuperspacePlayerController* Controller = (ASuperspacePlayerController*)PawnToKill->GetController();
	PawnToKill->Destroy();
	
	//Spawn a new Pawn for the controller.
	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		ASuperspacePawn* NewPawn = World->SpawnActor<ASuperspacePawn>(SpawnLocation, FRotator(0.f,0.f,0.f));
		Controller->Possess(NewPawn);
	}
	Dealer->GetController()->PlayerState->Score++;

}