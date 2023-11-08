// Fill out your copyright notice in the Description page of Project Settings.


#include "TCPGameInstanceSubsystem.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"

UTCPGameInstanceSubsystem::UTCPGameInstanceSubsystem()
{
}

void UTCPGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogTemp, Display, TEXT("Initialize InstanceSubsystem"));
}

void UTCPGameInstanceSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Display, TEXT("Deinitialize InstanceSubsystem"));

	if (IsConnect())
	{
		DestroySocket();
	}
}

bool UTCPGameInstanceSubsystem::Connect(const int32& PortNum, const FString& IP)
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	if (!Socket)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSocket Failed"));
		return false;
	}

	FIPv4Address IPv4Address;
	FIPv4Address::Parse(IP, IPv4Address);

	//TSharedPtr<FInternetAddr> SocketAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr(); // 참조를 바꾸지 않을 것이기에
	TSharedRef<FInternetAddr> SocketAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	SocketAddress->SetPort(PortNum);
	SocketAddress->SetIp(IPv4Address.Value);

	if (Socket->Connect(*SocketAddress))
	{
		UE_LOG(LogTemp, Warning, TEXT("Connect TCP Success!"));

		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Connect TCP Failed"));
		PrintSocketError(TEXT("Connect"));

		DestroySocket();

		return false;
	}
}

void UTCPGameInstanceSubsystem::PrintSocketError(const FString& Text)
{
	ESocketErrors SocketErrorCode = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
	const TCHAR* SocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(SocketErrorCode);

	UE_LOG(LogSockets, Error, TEXT("[%s]  SocketError : %s"), *Text, SocketError);
}

void UTCPGameInstanceSubsystem::ConnectToTCPServer()
{
	UWorld* world = GetWorld();
	if (!world)
	{
		UE_LOG(LogTemp, Error, TEXT("World Error"));
		return;
	}

	bool bConnect = Connect(9877, TEXT("127.0.0.1"));
	if (!bConnect)
	{
		UE_LOG(LogTemp, Error, TEXT("Connect Server Failed"));

		// Reconnect to login server
		FTimerHandle reconnectServerHandle;
		world->GetTimerManager().SetTimer(reconnectServerHandle, this, &UTCPGameInstanceSubsystem::ConnectToTCPServer, 5.f, false);
	}
	else
	{
		// Start Client Thread
		ClientThread = new FClientThread(this);
		ClientThreadHandle = FRunnableThread::Create(ClientThread, TEXT("ClientThread"));
	}
}

bool UTCPGameInstanceSubsystem::Recv()
{
	UE_LOG(LogTemp, Display, TEXT("0000000"));

	if (!Socket)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket is null"));
		return false;
	}

	uint32 pendingDataSize;
	if (Socket->HasPendingData(pendingDataSize))
	{
		UE_LOG(LogTemp, Display, TEXT("111111"));

		TArray<uint8> buffer;
		buffer.SetNumUninitialized(FMath::Min(pendingDataSize, 65507u));

		int32 recvByte = 0;
		bool bSuccess = Socket->Recv(buffer.GetData(), buffer.Num(), recvByte);
		if (!bSuccess)
		{
			PrintSocketError(TEXT("Recv"));
			return false;
		}
		UE_LOG(LogTemp, Display, TEXT("222222"));

		// Ensure the data is null-terminated.
		buffer.Add(0);

		FString bufferString = UTF8_TO_TCHAR(buffer.GetData());

		UE_LOG(LogTemp, Log, TEXT("Message received: %s"), *bufferString);

		return true;
	}

	//if (Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(.5f)))
	//{
	//	uint16 RecvPayloadSize;
	//	uint16 RecvPacketType;

	//	// Get Size and Type from HeaderBuffer
	//	FMemory::Memcpy(&RecvPayloadSize, temp, sizeof(uint16_t));
	//	FMemory::Memcpy(&RecvPacketType, temp + sizeof(uint16_t), sizeof(uint16_t));

	//	/* I Skip Network Byte Ordering because most of game devices use little endian */
	//	RecvPayloadSize = ntoh(RecvPayloadSize);
	//	RecvPacketType = ntoh(RecvPacketType);

	//	OutRecvPacket.PacketType = static_cast<ELoginPacket>(RecvPacketType);

	//	// Recv Payload
	//	if (RecvPayloadSize > 0)
	//	{
	//		uint8_t* PayloadBuffer = new uint8_t[RecvPayloadSize + 1];

	//		BytesRead = 0;
	//		bool bRecvPayload = Socket->Recv(PayloadBuffer, RecvPayloadSize, BytesRead);

	//		if (!bRecvPayload)
	//		{
	//			PrintSocketError(TEXT("Receive Payload"));
	//			return false;
	//		}
	//		PayloadBuffer[RecvPayloadSize] = '\0';

	//		//Utf8 to FStirng
	//		FString PayloadString;
	//		PayloadString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(PayloadBuffer)));

	//		OutRecvPacket.Payload = PayloadString;

	//		delete[] PayloadBuffer;
	//		PayloadBuffer = nullptr;
	//	}

	//	UE_LOG(LogTemp, Warning, TEXT(" [Recv] PacketType : %d, PayloadSize : %d"), RecvPacketType, RecvPayloadSize);
	//}

	return false;
}

bool UTCPGameInstanceSubsystem::Send()
{
	if (!Socket)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket is null"));
		return false;
	}

	FString message = TEXT("Unreal Send");
	TCHAR* serializedChar = message.GetCharArray().GetData();
	FTCHARToUTF8 data(serializedChar);

	int32 size = FCString::Strlen(serializedChar) + 1;

	int32 sendByte = 0;
	bool bSuccess = Socket->Send((uint8*)data.Get(), size, sendByte);
	if (!bSuccess)
	{
		PrintSocketError(TEXT("Send"));
		return false;
	}

	return true;
}

bool UTCPGameInstanceSubsystem::IsConnect()
{
	return false;
}

void UTCPGameInstanceSubsystem::DestroySocket()
{
	if (ClientThread)
	{
		ClientThread->StopThread();

		if (ClientThreadHandle)
		{
			ClientThreadHandle->WaitForCompletion();
			delete ClientThreadHandle;
			ClientThreadHandle = nullptr;
		}

		delete ClientThread;
		ClientThread = nullptr;

		UE_LOG(LogTemp, Warning, TEXT("CleanUp Thread"));
	}

	// Clean Socket
	if (Socket)
	{
		if (Socket->GetConnectionState() == SCS_Connected)
		{
			Socket->Close();
			UE_LOG(LogTemp, Warning, TEXT("Close Socket"));
		}

		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		UE_LOG(LogTemp, Warning, TEXT("Destroy Socket"));

		Socket = nullptr;
		delete Socket;
	}
}


// ABout FRunable
FClientThread::FClientThread(class UTCPGameInstanceSubsystem* GISubsystem) : GameInstanceSubsystem(GISubsystem)
{
	bStopThread = false;
}

uint32 FClientThread::Run()
{
	while (!bStopThread)
	{
		bool recvByte = GameInstanceSubsystem->Recv();
		if (!recvByte)
		{
			UE_LOG(LogTemp, Error, TEXT("Recv Error, Stop Thread"));
			break;
		}

		bool sendByte = GameInstanceSubsystem->Send();
		if (!sendByte)
		{
			UE_LOG(LogTemp, Error, TEXT("Recv Error, Stop Thread"));
			break;
		}
	}

	return 0;
}

void FClientThread::StopThread()
{
	bStopThread = true;
}
