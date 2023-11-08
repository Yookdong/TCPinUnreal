// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameModeBase.h"
#include "TCPGameInstanceSubsystem.h"

void ALobbyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SubGI = GetGameInstance()->GetSubsystem<UTCPGameInstanceSubsystem>();

	if (SubGI)
	{
		SubGI->ConnectToTCPServer();
	}
}
