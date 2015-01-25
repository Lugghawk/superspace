// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Character.h"
#include "UnrealNetwork.h"
#include "SuperspacePawn.generated.h"

UCLASS(Blueprintable)
class ASuperspacePawn : public APawn
{
	GENERATED_BODY()

	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShipMeshComponent;

	/** The camera */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
	ASuperspacePawn(const FObjectInitializer& ObjectInitializer);

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite )
	FVector GunOffset;
	
	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float FireRate;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MaxMoveSpeed;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float AccelerationRate;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float RotationRate;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	virtual void BeginPlay() override;
	// End Actor Interface


	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	void Shoot();

	FVector ReflectVector(FVector InitialDirection, FVector Normal);

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName StrafeBinding;
	static const FName FireBinding;

	int32 PlayerNumber;


	/* Network replicated stuff */
	void SetCurrentVelocity(FVector Velocity);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetCurrentVelocity(FVector Velocity);

	void SetFacingDirection(FRotator Rotation);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetFacingDirection(FRotator Rotation);

	/* Fire a shot in the specified direction */
	void LocalFireShot(FVector FireDirection);
	void FireShot(FVector FireDirection);

	UFUNCTION(reliable, server, WithValidation)
	void ServerFireShot(FVector FireDirection);

	UFUNCTION(reliable, NetMulticast, WithValidation)
	void MultiCastFireShot(FVector FireDirection);

	UFUNCTION(reliable, server, WithValidation)
	void DoDamage(APawn* Dealer,int32 Damage);


private:

	/* Flag to control firing  */
	UPROPERTY(Replicated)
	uint32 bCanFire : 1;

	/* Current speed */
	UPROPERTY(Replicated)
	FVector CurrentVelocity;

	UPROPERTY(Replicated)
	int32 Health;

	FVector SpawnLocation;

public:
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};

