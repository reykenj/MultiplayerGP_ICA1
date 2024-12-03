#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>
#include <string>

/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE     1024
#define PORT_NUMBER 7890
#define IP_ADDRESS  "127.0.0.1"





int packet_add_data(char Buffer[], const char DataName[], const int Value)
{
	sprintf_s(Buffer, _msize(Buffer), "%s %s=%d", Buffer, DataName, Value);
	return strlen(Buffer);
}

int packet_add_data(char Buffer[], const char DataName[], const char Value[])
{
	sprintf_s(Buffer, _msize(Buffer), "%s %s=\"%s\"", Buffer, DataName, Value);
	return strlen(Buffer);
}

int packet_parser_get_data(const char Packet[], const char DataName[], std::string& DataString)
{
	const char* Str;
	if ((Str = strstr(Packet, DataName)) == NULL)
	{
		return 0;
	}
	const char* Pos = Str;
	int Len = strlen(Pos);
	for (int i = 0; i < Len; ++i)
	{
		if (Pos[i] == '=')
		{
			for (int j = i + 1; j < Len; ++j)
			{
				if (Pos[j] == ' ' || Pos[j] == '\n' || Pos[j] == '\0')
				{
					break;
				}
				DataString.push_back(Pos[j]);
			}
			return atoi(DataString.c_str());
		}
	}

	return 0;
}

int packet_parser_data(const char Packet[], const char DataName[])
{
	std::string DataString;
	int ReturnLength;

	return packet_parser_get_data(Packet, DataName, DataString);
}

int packet_parser_data(const char Packet[], const char DataName[], char Buffer[], int& BufferSize)
{
	std::string DataString;
	int ReturnLength;
	ReturnLength = packet_parser_get_data(Packet, DataName, DataString);
	if (ReturnLength > BufferSize) {
		ReturnLength = BufferSize;
	}
	strcpy_s(Buffer, BufferSize, DataString.c_str());
	BufferSize = ReturnLength;
	return ReturnLength;
}

int packet_encode(char Packet[], const int MaxBufferSize, const char PacketID[], const char PacketData[])
{
	int PacketLength = strlen(PacketID) + strlen(PacketData) + 7;
	sprintf_s(Packet, _msize(Packet), "<%s %03d %s>", PacketID, PacketLength, PacketData);
	return PacketLength;
}

int packet_decode(const char Packet[], char PacketID[], int& PacketIDLength, char PacketData[], int& PacketDataLength) {
	if (Packet == NULL || PacketID == NULL || PacketData == NULL) {
		return -1; // Error: invalid input
	}
	int PacketLength = strlen(Packet);
	int Pos = 1;
	int j = 0;
	while (Pos < PacketLength && Packet[Pos] != ' ') {
		PacketID[j++] = Packet[Pos++];
	}

	if (Pos >= PacketLength || Packet[Pos] != ' ') {
		return -1;
	}
	PacketID[j] = '\0';
	PacketIDLength = j;
	++Pos;
	strcpy(PacketData, &Packet[Pos]);
	PacketDataLength = strlen(PacketData);
	return strlen(Packet);
}



int main(int argc, char** argv)
{
	int          Port = PORT_NUMBER;
	char         IPAddress[16] = IP_ADDRESS;
	WSADATA      WsaData;
	SOCKET       ConnectSocket;
	SOCKADDR_IN  ServerAddr;

	int          ClientLen = sizeof(SOCKADDR_IN);

	fd_set       ReadFds, TempFds;
	TIMEVAL      Timeout; // struct timeval timeout;

	char         Message[BUFSIZE];
	int          MessageLen;
	int          Return;


	//// PACKET TESTING 
	//char PacketBuffer[BUFSIZE] = { 0 }; // Initialize the buffer
	//packet_add_data(PacketBuffer, "ATK", 40);
	//packet_add_data(PacketBuffer, "DEF", 50);

	//// Print the resulting packet
	//printf("Packet: %s\n", PacketBuffer);
	////


	printf("Destination IP Address [%s], Port number [%d]\n", IPAddress, Port);

	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		printf("WSAStartup() error!");
		return 1;
	}

	ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == ConnectSocket)
	{
		printf("socket() error");
		return 1;
	}

	///----------------------
	/// The sockaddr_in structure specifies the address family,
	/// IP address, and port of the server to be connected to.
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = inet_addr(IPAddress);

	///----------------------
	/// Connect to server.
	Return = connect(ConnectSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (Return == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		printf("Unable to connect to server: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	FD_ZERO(&ReadFds);
	FD_SET(ConnectSocket, &ReadFds);


	printf("enter messages : ");
	memset(Message, '\0', BUFSIZE);
	MessageLen = 0;
	while (1)
	{
		if (_kbhit())
		{ // To check keyboard input.
			Message[MessageLen] = _getch();
			if (('\n' == Message[MessageLen]) || ('\r' == Message[MessageLen]))
			{ // Send the message to server.
				putchar('\n');
				MessageLen++;
				Message[MessageLen] = '\0';

				Return = send(ConnectSocket, Message, MessageLen, 0);
				if (Return == SOCKET_ERROR)
				{
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				printf("Bytes Sent: %ld\n", Return);

				MessageLen = 0;
				memset(Message, '\0', BUFSIZE);
			}
			else
			{
				putchar(Message[MessageLen]);
				MessageLen++;
			}
		}
		else
		{
			TempFds = ReadFds;
			Timeout.tv_sec = 0;
			Timeout.tv_usec = 1000;

			if (SOCKET_ERROR == (Return = select(0, &TempFds, 0, 0, &Timeout)))
			{ // Select() function returned error.
				closesocket(ConnectSocket);
				printf("select() error\n");
				return 1;
			}
			else if (0 > Return)
			{
				printf("Select returned error!\n");
			}
			else if (0 < Return)
			{
				memset(Message, '\0', BUFSIZE);
				printf("Select Processed... Something to read\n");
				Return = recv(ConnectSocket, Message, BUFSIZE, 0);
				if (0 > Return)
				{ // recv() function returned error.
					closesocket(ConnectSocket);
					printf("Exceptional error :Socket Handle [%d]\n", ConnectSocket);
					return 1;
				}
				else if (0 == Return)
				{ // Connection closed message has arrived.
					closesocket(ConnectSocket);
					printf("Connection closed :Socket Handle [%d]\n", ConnectSocket);
					return 0;
				}
				else
				{ // Message received.
					printf("Bytes received   : %d\n", Return);
					printf("Message received : %s\n", Message);
					printf("enter messages : ");
				}
			}
		}
	}

	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}