// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnersCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ARunnersCharacter

ARunnersCharacter::ARunnersCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Set crouch behavior allowance to true
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	// Set Movement Flags
	bIsSprinting = false;
	bIsWalking = false;

	// Set Movement Scalars
	WalkMultiplier = 0.5f;
	SprintMultiplier = 2.f;

	// Set Camera Attribute Defaults
	BaseCameraBoomDistance = 150.f;
	SprintingCameraBoomDistance = 250.f;
	fCurrentCameraPosition = BaseCameraBoomDistance;
	fTransitionAlpha = 0.f;
	PushInTransitionSeconds = 2.5f;
	PullOutTransitionSeconds = 2.5f;

	// Set Character movement defaults
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed = 440.f;
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = BaseCameraBoomDistance; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ARunnersCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// If sprinting, Update Boom to distance value
	if (bIsSprinting)
	{
		UpdateCameraBoomDistanceDuringSprint(EaseIn, fCurrentCameraPosition, SprintingCameraBoomDistance, DeltaSeconds, PushInTransitionSeconds, 3);
	}
	// If not sprinting and camera not in base position, adjust to base position
	else if (!bIsSprinting && fCurrentCameraPosition > BaseCameraBoomDistance)
	{
		UpdateCameraBoomDistanceDuringSprint(EaseOut, fCurrentCameraPosition, BaseCameraBoomDistance, DeltaSeconds, PullOutTransitionSeconds, 3);
	}
}

void ARunnersCharacter::UpdateCameraBoomDistanceDuringSprint(const EInterpolationOptions Option, float &CurrentDistance, const float TargetDistance, const float DeltaSeconds, const float TransitionDuration, const int8 Exponent)
{	
	// Calculates current Interpolation alpha
	fTransitionAlpha += DeltaSeconds / TransitionDuration;

	// Derives current distance based on given interpolation inputs
	switch (Option)
	{
		case EaseIn:
			CurrentDistance = FMath::InterpEaseIn(CurrentDistance, TargetDistance, fTransitionAlpha, Exponent);
		case EaseOut:
			CurrentDistance = FMath::InterpEaseOut(CurrentDistance, TargetDistance, fTransitionAlpha, Exponent);
		case EaseInOut:
			CurrentDistance = FMath::InterpEaseInOut(CurrentDistance, TargetDistance, fTransitionAlpha, Exponent);
		case Default:
			CurrentDistance = FMath::InterpEaseInOut(CurrentDistance, TargetDistance, fTransitionAlpha, Exponent);
			break;
	}

	// Sets boom position to derived 'CurrentDistance'
	CameraBoom->TargetArmLength = CurrentDistance;
}

void ARunnersCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARunnersCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARunnersCharacter::Move);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ARunnersCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARunnersCharacter::StopSprint);

		// Walking
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ARunnersCharacter::Walk);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ARunnersCharacter::StopWalk);

		// Movement Actions
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ARunnersCharacter::Crouch);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARunnersCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARunnersCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARunnersCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(RightDirection, MovementVector.X);
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}
}


void ARunnersCharacter::Sprint(const FInputActionValue& Value)
{
	bIsSprinting = true;
	fTransitionAlpha = 0;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * SprintMultiplier;
}

void ARunnersCharacter::StopSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;
	fTransitionAlpha = 0;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void ARunnersCharacter::Walk(const FInputActionValue& Value)
{
	bIsWalking = true;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * WalkMultiplier;
}

void ARunnersCharacter::StopWalk(const FInputActionValue& Value)
{
	bIsWalking = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void ARunnersCharacter::Crouch(const FInputActionValue& Value)
{
	// Basic crouch toggle behavior
	GetCharacterMovement()->bWantsToCrouch = !GetCharacterMovement()->bWantsToCrouch;

	// TODO: Smooth crouch height transitions and implement animations in derived blueprint
}


