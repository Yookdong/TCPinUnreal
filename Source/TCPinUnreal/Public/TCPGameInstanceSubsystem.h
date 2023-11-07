// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TCPGameInstanceSubsystem.generated.h"

UCLASS()
class TCPINUNREAL_API UTCPGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:

protected:
	FSocket* Socket;
	class FClientThread* ClientThread;
	class FRunnableThread* ClientThreadHandle;
	FTimerHandle ManageRecvHandle;

public:
	UTCPGameInstanceSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	bool Connect(const int32& PortNum, const FString& IP);
	void PrintSocketError(const FString& Text);

public:
	void ConnectToTCPServer();
	bool Recv();
	bool Send();
	bool IsConnect();
	void DestroySocket();
};


class TCPINUNREAL_API FClientThread : public FRunnable
{
private:
	class UTCPGameInstanceSubsystem* GameInstanceSubsystem;
public:
	FClientThread(class UTCPGameInstanceSubsystem* GISubsystem);

protected:
	virtual uint32 Run() override;

public:
	void StopThread();

private:
	bool bStopThread;

};