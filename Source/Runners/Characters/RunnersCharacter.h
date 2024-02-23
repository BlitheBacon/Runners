// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "RunnersCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

const enum EInterpolationOptions
{
	Default,
	EaseIn,
	EaseOut,
	EaseInOut
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ARunnersCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Camera Boom Attributes */
	UPROPERTY(Category = "Camera|CameraBoom", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=80, ClampMax=200))
	float BaseCameraBoomDistance;

	UPROPERTY(Category = "Camera|CameraBoom", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=150, ClampMax=300))
	float SprintingCameraBoomDistance;

	UPROPERTY(Category= "Camera|CameraBoom", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=1, ClampMax=10, Delta=.1f, UIMin=0, UIMax=10, Units="seconds"))
	float PushInTransitionSeconds;

	UPROPERTY(Category= "Camera|CameraBoom", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=1, ClampMax=10, Delta=.1f, UIMin=0, UIMax=10, Units="seconds"))
	float PullOutTransitionSeconds;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Move Input Actions */
	// Move
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// Sprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	// Walk
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;
	
	/** Movement Attributes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Sprinting", meta = (AllowPrivateAccess = "true"))
	float SprintMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (AllowPrivateAccess = "true"))
	float WalkMultiplier;	
	
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0, UIMin=0, ForceUnits="cm/s"))
	float BaseWalkSpeed;
	
public:
	// Constructors
	ARunnersCharacter();

	// Overrides
	virtual void Tick(float DeltaSeconds) override;

	// Methods
	/** Applies an interpolation between two distances over a set number of seconds. Call in tick function.
	 *
	 * Initial argument determines which form of interpolation is applied:
	 * 1: EaseIn,
	 * 2: EaseOut,
	 * 3: EaseInOut
	 */
	void UpdateCameraBoomDistanceDuringSprint(const EInterpolationOptions Option, float &CurrentDistance, const float TargetDistance, const float DeltaSeconds, const float TransitionDuration, const int8 Exponent);

protected:
	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for crouching input */
	void Crouch(const FInputActionValue& Value);

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	void Walk(const FInputActionValue& Value);
	void StopWalk(const FInputActionValue& Value);

protected:
	/** Movement flags */
	bool bIsSprinting;
	bool bIsWalking;

	/** Camera Attributes */
	float fCurrentCameraPosition;

	// Camera during sprint
	float fTransitionAlpha;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

