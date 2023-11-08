// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TCPGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TCPINUNREAL_API UTCPGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UTCPGameInstance();

	virtual void StartGameInstance() override;

};
