// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class TCPINUNREAL_API ALobbyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
private:
	class UTCPGameInstanceSubsystem* SubGI;

public:
	virtual void BeginPlay() override;

};
