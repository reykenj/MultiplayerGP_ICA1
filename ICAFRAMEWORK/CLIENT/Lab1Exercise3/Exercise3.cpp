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
	size_t currentLength = strlen(Buffer);
	size_t maxLength = BUFSIZE;

	// Ensure no overflow occurs
	if (currentLength + strlen(DataName) + 20 >= maxLength) // +20 for " =VALUE" and null terminator
	{
		return -1; // Error: Not enough space
	}

	sprintf_s(Buffer + currentLength, maxLength - currentLength, " %s=%d", DataName, Value);
	//	sprintf_s(Buffer, _msize(Buffer), "%s %s=%d", Buffer, DataName, Value);
	return strlen(Buffer);
}

int packet_add_data(char Buffer[], const char DataName[], const char Value[])
{
	size_t currentLength = strlen(Buffer);
	size_t maxLength = BUFSIZE;

	// Ensure no overflow occurs
	if (currentLength + strlen(DataName) + strlen(Value) + 10 >= maxLength) // +10 for ` ="VALUE"` and null terminator
	{
		return -1; // Error: Not enough space
	}

	sprintf_s(Buffer + currentLength, maxLength - currentLength, " %s=%s", DataName, Value);
	return strlen(Buffer);
}

int packet_parser_get_data(const char Packet[], const char DataName[], std::string& DataString)
{
	const char* Str = strstr(Packet, DataName);
	if (Str == NULL)
	{
		return 0; // DataName not found
	}

	const char* Pos = Str + strlen(DataName);
	while (*Pos == ' ' || *Pos == '=') Pos++; // Skip spaces and '='

	DataString.clear();
	while (*Pos != ' ' && *Pos != '\n' && *Pos != '\0')
	{
		DataString.push_back(*Pos++);
	}

	return DataString.length(); // Return the length of the parsed string
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
	int ReturnLength = packet_parser_get_data(Packet, DataName, DataString);

	if (ReturnLength > 0)
	{
		if (ReturnLength >= BufferSize)
		{
			ReturnLength = BufferSize - 1; // Ensure space for null terminator
		}

		strncpy(Buffer, DataString.c_str(), ReturnLength);
		Buffer[ReturnLength] = '\0'; // Null-terminate the buffer
		BufferSize = ReturnLength;
	}
	else
	{
		Buffer[0] = '\0'; // Clear the buffer if no data was found
		BufferSize = 0;
	}

	return ReturnLength;
}

int packet_encode(char Packet[], const int MaxBufferSize, const char PacketID[], const char PacketData[])
{
	// Reset buffer to avoid leftover data
	memset(Packet, 0, MaxBufferSize);

	// Ensure no overflow occurs
	int PacketLength = strlen(PacketID) + strlen(PacketData) + 7; // "<ID XXX DATA>"
	if (PacketLength >= MaxBufferSize)
	{
		return -1; // Error: Not enough space
	}

	sprintf_s(Packet, MaxBufferSize, "<%s %s>", PacketID, PacketData);
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
	PacketData[PacketDataLength - 1] = '\0';
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
	char PacketBuffer[BUFSIZE] = { 0 }; // Initialize the buffer
	char Packet[BUFSIZE];

	char SINGLE[BUFSIZE];


	char PacketID[BUFSIZE];
	char PacketData[BUFSIZE];

	int buffsize = BUFSIZE;

	int PacketIDLength = 0;
	int PacketDataLength = 0;

	int SessionID = NULL;
	int Height = 40;
	char Single[BUFSIZE] = "No";
	int PacketLength;

	// Add data to the packet
	packet_add_data(PacketBuffer, "HEIGHT", Height);
	packet_add_data(PacketBuffer, "SINGLE", Single);

	// Encode the packet
	//int PacketLength = packet_encode(Packet, BUFSIZE, "CHRSTA", PacketBuffer);

	//// Test the string parser
	//packet_parser_data(Packet, "SINGLE", SINGLE, buffsize);

	// Print the parsed value
	/*printf("Testing Parser: SINGLE = %s\n", SINGLE);*/

	// Print the final packet
	/*printf("Packet: %s\n", Packet);*/

	//packet_decode(Packet, PacketID, PacketIDLength, PacketData, PacketDataLength);
	//printf("Packet ID: [%d]\n", packet_decode(Packet, PacketID, PacketIDLength, PacketData, PacketDataLength));
	//printf("Packet ID: [%s]\n", PacketID);
	//printf("Packet ID: [%s]\n", PacketData);
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
	printf("%d \n", ConnectSocket);

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
				if (!strncmp(Message, "/send packet", 12)) {
					printf("\n sending packet");
					strcpy(Message, Packet);
					MessageLen = strlen(Message);
				}

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

					char* cleaned = strtok(Message, "\r\n");
					if (SessionID == NULL) {
						printf("Number Extracted : %d\n", sscanf(cleaned, "<SessionID: %d", &SessionID));
						printf("SessionID Extracted : %d\n", SessionID);
						packet_add_data(PacketBuffer, "SESSIONID", SessionID);
						packet_encode(Packet, BUFSIZE, "CHRSTA", PacketBuffer);
					}

					printf("enter messages : ");
				}
			}
		}
	}

	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}