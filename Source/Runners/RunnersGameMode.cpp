// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnersGameMode.h"
#include "Characters/RunnersCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARunnersGameMode::ARunnersGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
