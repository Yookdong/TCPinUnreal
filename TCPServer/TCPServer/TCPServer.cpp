#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>

#pragma comment(lib, "ws2_32")

using namespace std;

fd_set Reads;
fd_set CopyReads;

unsigned WINAPI ServerThread(void* arg);

int main()
{
	//----- Add DB Server -----

	//----- End DB Server -----

	WSADATA WsaData;

	int Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Result != 0)
	{
		cout << "Fail." << endl;
		cout << "Error On StartUp : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "Fail." << endl;
		cout << "ListenSocket Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ListenSockAddr.sin_port = htons(7777);

	// DB 관련 라이브러리에 같은 함수명이 존재 해 namespace 표시 해 줌
	Result = _WINSOCK2API_::bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));
	if (Result == SOCKET_ERROR)
	{
		cout << "Fail." << endl;
		cout << "Bind Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	Result = listen(ListenSocket, SOMAXCONN);
	if (Result == SOCKET_ERROR)
	{
		cout << "Fail." << endl;
		cout << "Listen Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	struct timeval Timeout;
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 500;

	FD_ZERO(&Reads);
	FD_SET(ListenSocket, &Reads);

	while (true)
	{
		CopyReads = Reads;

		int ChangeSocketCount = select(0, &CopyReads, 0, 0, &Timeout);
		if (ChangeSocketCount > 0)
		{
			for (int i = 0; i < (int)Reads.fd_count; ++i)
			{
				if (FD_ISSET(Reads.fd_array[i], &CopyReads))
				{
					if (Reads.fd_array[i] == ListenSocket)
					{
						SOCKADDR_IN ClientSocketAddr;
						memset(&ClientSocketAddr, 0, sizeof(ClientSocketAddr));
						int ClientSockAddrLength = sizeof(ClientSocketAddr);

						SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);
						if (ClientSocket == INVALID_SOCKET)
						{
							cout << "Accept Error : " << GetLastError() << endl;
							continue;
						}

						FD_SET(ClientSocket, &Reads);
						CopyReads = Reads;
						char IP[1024] = { 0, };
						inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
						printf("[%d] Connected : %s\n", (unsigned short)ClientSocket, IP);

						// create thread
						_beginthreadex(nullptr, 0, ServerThread, (void*)&ClientSocket, 0, nullptr);

						break;
					}
				}
			}
		}
		else
		{
			// when no changes on socket count while timeout
		}
	}

	closesocket(ListenSocket);
	WSACleanup();

	system("pause");

	return 0;
}

unsigned __stdcall ServerThread(void* arg)
{
	SOCKET client = *(SOCKET*)arg;

	// Send
	char message[1024] = "Server Message";

	int sendByte = send(client, message, (int)(strlen(message)), 0);
	if (sendByte <= 0)
	{
		cout << "Send Error : " << GetLastError() << endl;
		return false;
	}

	cout << "Send Message : " << message << endl;

	// Recv
	char buffer[1024] = { 0, };

	int recvByte = recv(client, buffer, 1024, 0);
	if (recvByte <= 0)
	{
		SOCKADDR_IN clientSocketAddr;
		int clientSockAddrLength = sizeof(clientSocketAddr);
		getpeername(client, (SOCKADDR*)&clientSocketAddr, &clientSockAddrLength);

		closesocket(client);
		FD_CLR(client, &Reads);
		CopyReads = Reads;

		char IP[1024] = { 0, };
		inet_ntop(AF_INET, &clientSocketAddr.sin_addr.s_addr, IP, 1024);
		cout << "disconnected : " << IP << endl;
	}

	cout << "Recv Message : " << buffer << endl;

	return 0;
}
