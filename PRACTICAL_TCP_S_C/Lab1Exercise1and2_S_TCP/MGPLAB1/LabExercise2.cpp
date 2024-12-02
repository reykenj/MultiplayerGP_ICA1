//------------------------------
// Lab02.Exercise2
// Simple TCP/IP Echo Server
// Filename : simple_tcp_echo_server.cpp
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <stdio.h>
#include <string>
/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 9876
int main(void)
{
	WSADATA wsaData; // Variable to initialize Winsock
	SOCKET ServerSocket = INVALID_SOCKET; // Server socket for accept client
	SOCKET ConnectedSocket = INVALID_SOCKET; // Connected socket from client
	sockaddr_in ServerAddress; // Server address
	sockaddr_in ClientAddress; // Client address
	char MessageBuffer[DEFAULT_BUFLEN]; // Message buffer to recv from socket
	int ClientAddressLen; // Length for client sockaddr_in.
	int Result = 0; // Return value for function result
	///----------------------
	/// 1. Initiate use of the Winsock Library by a process
	Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != Result)
	{
		printf("Error at WSAStartup()\n");
		return 1;
	}
	else
	{
		printf("WSAStartup success.\n");
	}
	///----------------------
	/// 2. Create a new socket for application
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == ServerSocket)
	{
		printf("socket function failed with error: %u\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else
	{
		printf("socket creation success.\n");
	}
	///----------------------
	/// 3-1. The sockaddr_in structure specifies the address family,
	/// IP address, and port for the socket that is being bound.
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(DEFAULT_PORT);
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	///----------------------
	/// 3-2. Bind the socket with server address & port number
	if (SOCKET_ERROR == bind(ServerSocket, (SOCKADDR*)&ServerAddress,
		sizeof(ServerAddress)))
	{
		printf("bind failed with error %u\n", WSAGetLastError());
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("bind returned success\n");
	}
	///----------------------
	/// 4. Places a socket in a state in which it is listening for an incoming
	/// connection.
	/// 
	if (SOCKET_ERROR == listen(ServerSocket, SOMAXCONN))
	{
		printf("listen function failed with error: %d\n", WSAGetLastError());
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}

	// REY MAKE EVERYTHING HERE MORE EFFICIEBNT LATER
	printf("Waiting for client to connect...\n");
	///----------------------
	/// 5. Permits an incoming connection attempt on a socket
	ClientAddressLen = sizeof(ClientAddress);
	ConnectedSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddress,
		&ClientAddressLen);
	if (INVALID_SOCKET == ConnectedSocket)
	{
		printf("accept failed with error: %ld\n", WSAGetLastError());
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Client connected. IP Address : %d.%d.%d.%d, Port Number :%d\n",
			ClientAddress.sin_addr.S_un.S_un_b.s_b1,
			ClientAddress.sin_addr.S_un.S_un_b.s_b2,
			ClientAddress.sin_addr.S_un.S_un_b.s_b3,
			ClientAddress.sin_addr.S_un.S_un_b.s_b4,
			ntohs(ClientAddress.sin_port));
	}
	//strcpy_s(MessageBuffer, "Welcome");
	//send(ConnectedSocket, MessageBuffer, strlen(MessageBuffer), 0);
	///----------------------
	/// 6. Send & receive the data on a connected socket
	while (1)
	{
		memset(MessageBuffer, '\0', DEFAULT_BUFLEN);

		int MessageBufferLen = 50; // changes the length of message sent from client to server
		Result = recv(ConnectedSocket, MessageBuffer, DEFAULT_BUFLEN, 0);
		if (!strcmp(MessageBuffer, "")) {
			Result = 0;
		}
		if (0 < Result)
		{
			printf("Bytes received : %d\n", Result);

			printf("REWEWRW : %d\n", Result);
			printf("Buffer received : %s\n", MessageBuffer);
		}
		else if (0 == Result)
		{
			printf("Connection closed\n");
			/// 7. Closes an existing socket
			closesocket(ConnectedSocket);
			ConnectedSocket = INVALID_SOCKET;
			ClientAddressLen = sizeof(ClientAddress);
			ConnectedSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddress,
				&ClientAddressLen);
			if (INVALID_SOCKET == ConnectedSocket)
			{
				printf("accept failed with error: %ld\n", WSAGetLastError());
				closesocket(ServerSocket);
				WSACleanup();
				return 1;
			}
			else
			{
				printf("Client connected. IP Address : %d.%d.%d.%d, Port Number :%d\n",
					ClientAddress.sin_addr.S_un.S_un_b.s_b1,
					ClientAddress.sin_addr.S_un.S_un_b.s_b2,
					ClientAddress.sin_addr.S_un.S_un_b.s_b3,
					ClientAddress.sin_addr.S_un.S_un_b.s_b4,
					ntohs(ClientAddress.sin_port));
			}
			printf("testing: \n");
			memset(MessageBuffer, '\0', DEFAULT_BUFLEN);
			Result = recv(ConnectedSocket, MessageBuffer, DEFAULT_BUFLEN, 0);
		}
		else
		{
			printf("Recv failed: %d\n", WSAGetLastError());
			break;
		}

		//printf("Buffer receivedREYKENJ : %s\n", MessageBuffer);
		//if (!strcmp(MessageBuffer, "CLOSE MSG")) {
		//	printf("Buffer should work");
		//}
		//memset(MessageBuffer, '\0', DEFAULT_BUFLEN);
		if (!strcmp(MessageBuffer, "Hello World")) {

			memset(MessageBuffer, '\0', DEFAULT_BUFLEN);
			strcpy_s(MessageBuffer, "RTN PRINT OK");
			// fix later cuz there is a problem where if the user speaks, it would return a part of these messages and also repeatinbg a part of what they said as well
		}
		else if (!strcmp(MessageBuffer, "CLOSE MSG")) {

			memset(MessageBuffer, '\0', DEFAULT_BUFLEN);
			strcpy_s(MessageBuffer, "CLOSE OK");
			//Result = 0;
			// fix later cuz there is a problem where if the user speaks, it would return a part of these messages and also repeatinbg a part of what they said as well
		}
		else {
			memset(MessageBuffer, '\0', DEFAULT_BUFLEN);
			strcpy_s(MessageBuffer, "RTN OK");
		}
		printf("Buffer receivedREYKENJ : %s\n", MessageBuffer);
		Result = send(ConnectedSocket, MessageBuffer, strlen(MessageBuffer), 0);
		///// Echo same message to client
		//Result = send(ConnectedSocket, MessageBuffer, Result, 0);
		if (SOCKET_ERROR == Result)
		{
			printf("Send failed: %d\n", WSAGetLastError());
			break;
		}
		printf("Bytes sent : %d\n", Result);
	}
	///----------------------
	/// 7. Closes an existing socket
	closesocket(ConnectedSocket);
	closesocket(ServerSocket);
	///----------------------
	/// 8. Terminate use of the Winsock Library
	WSACleanup();
	return 0;
}