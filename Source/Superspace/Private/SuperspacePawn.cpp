// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Superspace.h"
#include "SuperspacePawn.h"
#include "SuperspaceProjectile.h"
#include "SuperspaceGameMode.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"

const FName ASuperspacePawn::MoveForwardBinding("MoveForward");
const FName ASuperspacePawn::MoveRightBinding("MoveRight");
const FName ASuperspacePawn::FireBinding("Fire");

ASuperspacePawn::ASuperspacePawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/Meshes/UFO.UFO"));
	// Create the mesh component
	ShipMeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Audio/TemplateTSS_WeaponFire.TemplateTSS_WeaponFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->RelativeRotation = FRotator(-80.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("TopDownCamera"));
	CameraComponent->AttachTo(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MaxMoveSpeed = 10.0f;
	AccelerationRate = 10.f;
	RotationRate = 120.f;
	CurrentVelocity = FVector(0.f, 0.f, 0.f);
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;
	Health = 10;
	
}

void ASuperspacePawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	// set up gameplay key bindings
	InputComponent->BindAxis(MoveForwardBinding);
	InputComponent->BindAxis(MoveRightBinding);
	InputComponent->BindAction(FireBinding, IE_Pressed, this, &ASuperspacePawn::Shoot);
}

void ASuperspacePawn::Shoot(){
	FireShot(GetActorForwardVector());
}
void ASuperspacePawn::BeginPlay(){
	SpawnLocation = GetActorLocation();
}

void ASuperspacePawn::Tick(float DeltaSeconds)
{
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	if (RightValue != 0.f){
		const FRotator NewRotation = GetActorRotation() + (FRotator(0.f, RotationRate * RightValue, 0.f)  * DeltaSeconds);
		SetFacingDirection(NewRotation);
	}


	//Acceleration is always in forward vector's direction
	const FVector AccelerationDirection = GetActorForwardVector() / GetActorForwardVector().Size();

	if (ForwardValue != 0.f){
		//Accelerate in the given direction.
		CurrentVelocity += ForwardValue * AccelerationRate * DeltaSeconds * AccelerationDirection;
		SetCurrentVelocity(CurrentVelocity);
	}
	CurrentVelocity = CurrentVelocity.ClampMaxSize(MaxMoveSpeed);

	FHitResult Hit(1.f);
	RootComponent->MoveComponent(CurrentVelocity, GetActorRotation(), true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		
		const FVector Normal2D = Hit.Normal.SafeNormal2D();
		CurrentVelocity = ReflectVector(CurrentVelocity, Normal2D);
	}

	const FVector FireDirection = GetActorForwardVector();

	UE_LOG(LogTemp, Log, TEXT("Location: %s"), *GetActorLocation().ToString());

}

void ASuperspacePawn::SetFacingDirection(FRotator Rotation){
	SetActorRotation(Rotation);
	if (Role < ROLE_Authority){
		ServerSetFacingDirection(Rotation);
	}
}

bool ASuperspacePawn::ServerSetFacingDirection_Validate(FRotator Rotation){
	return true;
}

void ASuperspacePawn::ServerSetFacingDirection_Implementation(FRotator Rotation){
	SetFacingDirection(Rotation);
}

void ASuperspacePawn::SetCurrentVelocity(FVector Velocity){
	CurrentVelocity = Velocity; 
	if (Role < ROLE_Authority){
		ServerSetCurrentVelocity(Velocity);
	}
}

bool ASuperspacePawn::ServerSetCurrentVelocity_Validate(FVector Velocity){
	return true;
}

void ASuperspacePawn::ServerSetCurrentVelocity_Implementation(FVector Velocity){
	SetCurrentVelocity(Velocity);
}

FVector ASuperspacePawn::ReflectVector(FVector InitialDirection, FVector Normal){
	const float DProduct = (FVector::DotProduct(InitialDirection, Normal));
	return FVector((Normal* DProduct * -2) + InitialDirection);
}

void ASuperspacePawn::FireShot(FVector FireDirection)
{
	
	if (Role < ROLE_Authority){
		ServerFireShot(FireDirection);
	}
	else{
		MultiCastFireShot(FireDirection);
		LocalFireShot(FireDirection);
	}
		
}

void ASuperspacePawn::LocalFireShot(FVector FireDirection){

	// If we it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				ASuperspaceProjectile* projectile = World->SpawnActor<ASuperspaceProjectile>(SpawnLocation, FireRotation);
				projectile->Instigator = this;
			}

			World->GetTimerManager().SetTimer(this, &ASuperspacePawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

bool ASuperspacePawn::ServerFireShot_Validate(FVector FireDirection){
	return true;
}

void ASuperspacePawn::ServerFireShot_Implementation(FVector FireDirection){
	FireShot(FireDirection);
}

bool ASuperspacePawn::MultiCastFireShot_Validate(FVector FireDirection){
	return true;
}

void ASuperspacePawn::MultiCastFireShot_Implementation(FVector FireDirection){
	LocalFireShot(FireDirection);
}

void ASuperspacePawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicate to everyone
	DOREPLIFETIME(ASuperspacePawn, CurrentVelocity);
	DOREPLIFETIME(ASuperspacePawn, Health);
	DOREPLIFETIME(ASuperspacePawn, bCanFire);
}

void ASuperspacePawn::ShotTimerExpired()
{
	bCanFire = true;
}

bool ASuperspacePawn::DoDamage_Validate(APawn* Dealer, int32 Damage){
	return true;
}

void ASuperspacePawn::DoDamage_Implementation(APawn* Dealer, int32 Damage){
	Health -= Damage;
	if (Health < 1){
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the new pawn
			/*ASuperspacePawn* pawn = World->SpawnActor<ASuperspacePawn>(SpawnLocation, GetActorRotation());
			AController* controller = GetController();
			controller->Possess(pawn);*/

			ASuperspaceGameMode* GameMode = World->GetAuthGameMode<ASuperspaceGameMode>();
			GameMode->KillPawn(Dealer, this);

		}
	}
}

