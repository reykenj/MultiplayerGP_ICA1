//------------------------------
// Lab02.Exercise3
// Simple TCP/IP Echo Client
// Filename : simple_tcp_echo_client.cpp
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <winsock2.h>
#include <stdio.h>
#include <iostream>
/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 9876
int main(void)
{
	///----------------------
	/// Declare and initialize variables.
	WSADATA wsaData; // Variable to initialize Winsock
	SOCKET ConnectSocket = INVALID_SOCKET; // Socket to connect to server
	sockaddr_in ServerAddress; // Socket address to connect to server
	char MessageBuffer[DEFAULT_BUFLEN]; // Buffer to recv from socket
	int BufferLen; // Length of the message buffer
	int Result; // Return value for function result
	int i;
	///----------------------
	/// 1. Initiate use of the Winsock Library by a process
	Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != Result)
	{
		printf("WSAStartup failed: %d\n", Result);
		return 1;
	}
	///----------------------
	/// 2. Create a new socket for application
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == ConnectSocket)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	/////----------------------
	///// The sockaddr_in structure specifies the address family,
	///// IP address, and port of the server to be connected to.
	//ServerAddress.sin_family = AF_INET;
	///// Connecting to local machine. "127.0.0.1" is the loopback address.
	//ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	//ServerAddress.sin_port = htons(DEFAULT_PORT);
	/////----------------------
	///// 3. Establish a connection to a specified socket
	//if (SOCKET_ERROR == connect(ConnectSocket, (SOCKADDR*)&ServerAddress,
	//	sizeof(ServerAddress)))
	//{
	//	closesocket(ConnectSocket);
	//	printf("Unable to connect to server: %ld\n", WSAGetLastError());
	//	WSACleanup();
	//	return 1;
	//}
	/// Receive until the peer closes the connection
	while (1)
	{

		// Create a new socket
		ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == ConnectSocket)
		{
			printf("Error at socket(): %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		memset(MessageBuffer, '\0', DEFAULT_BUFLEN);
		std::cout << "Destination IP address : ";
		for (i = 0; i < (DEFAULT_BUFLEN - 1); i++)
		{
			MessageBuffer[i] = getchar();
			if (MessageBuffer[i] == '\n')
			{
				MessageBuffer[i++] = '\0';
				break;
			}
		}
		BufferLen = i;
		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear leftover input

		///----------------------
		/// The sockaddr_in structure specifies the address family,
		/// IP address, and port of the server to be connected to.
		ServerAddress.sin_family = AF_INET;
		/// Connecting to local machine. "127.0.0.1" is the loopback address.
		ServerAddress.sin_addr.s_addr = inet_addr(MessageBuffer);
		ServerAddress.sin_port = htons(DEFAULT_PORT);


		///----------------------
		/// 3. Establish a connection to a specified socket
		/// 
		/// 
		if (SOCKET_ERROR == connect(ConnectSocket, (SOCKADDR*)&ServerAddress,
			sizeof(ServerAddress)))
		{
			closesocket(ConnectSocket);
			std::cout << "Unable to connect to server: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}

		printf("Connected to server.\n");


		memset(MessageBuffer, '\0', DEFAULT_BUFLEN);

		printf("Enter messages : ");
		for (i = 0; i < (DEFAULT_BUFLEN - 1); i++)
		{
			MessageBuffer[i] = getchar();
			if (MessageBuffer[i] == '\n')
			{
				MessageBuffer[i++] = '\0';
				break;
			}
		}
		BufferLen = i;
		/// 4. Send & receive the data on a connected socket
		if (SOCKET_ERROR == (Result = send(ConnectSocket, MessageBuffer, BufferLen, 0)))
		{
			printf("Send failed: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		printf("Bytes sent: %ld\n", Result);

		memset(MessageBuffer, '\0', DEFAULT_BUFLEN);
		Result = recv(ConnectSocket, MessageBuffer, DEFAULT_BUFLEN, 0);
		if (0 < Result)
		{
			printf("Bytes received : %d\n", Result);
			printf("Message received : %s\n", MessageBuffer);

			if (!strcmp(MessageBuffer, "CLOSE OK")) {
				printf("Connection closed\n");
				break;
			}
		}
		else if (0 == Result)
		{
			printf("Connection closed\n");
		}
		else
		{
			printf("Recv failed: %d\n", WSAGetLastError());
			break;
		}
		printf("Bytes received : %d\n", Result);

		//memset(MessageB/'\0', DEFAULT_BUFLEN);
		//send(ConnectSocket, MessageBuffer, BufferLen, 0);

		closesocket(ConnectSocket);
		
		ConnectSocket = INVALID_SOCKET;
		//WSACleanup();
	}
	/// 5. close & cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}