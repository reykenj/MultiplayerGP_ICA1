#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <array>

/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFSIZE     1024
#define PORT_NUMBER 7890

int packet_parser_get_data(const char Packet[], const char DataName[], std::string& DataString)
{
	const char* Str = strstr(Packet, DataName);
	if (Str == NULL)
	{
		return 0;
	}

	const char* Pos = Str + strlen(DataName);
	while (*Pos == ' ' || *Pos == '=') Pos++;

	DataString.clear();
	while (*Pos != ' ' && *Pos != '\n' && *Pos != '\0' && *Pos != '>')
	{
		DataString.push_back(*Pos++);
	}

	return DataString.length();
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
			ReturnLength = BufferSize - 1;
		}
		//printf("Return Length: %d", ReturnLength);
		//printf("Return Length: %s", DataString);
		strncpy(Buffer, DataString.c_str(), ReturnLength);
		Buffer[ReturnLength] = '\0';
		BufferSize = ReturnLength;
	}
	else
	{
		Buffer[0] = '\0';
		BufferSize = 0;
	}

	return ReturnLength;
}

int packet_decode(const char Packet[], char PacketID[], int& PacketIDLength, char PacketData[], int& PacketDataLength) {
	if (Packet == NULL || PacketID == NULL || PacketData == NULL) {
		return -1;
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

void send_welcome_message(SOCKET ClientSocket)
{
	char WelcomeMessage[100];
	int WelcomeMessageLength;

	sprintf_s(WelcomeMessage, "<SessionID: %d. Welcome to my I/O multiplexing server!>", ClientSocket);
	WelcomeMessageLength = strlen(WelcomeMessage);

	send(ClientSocket, WelcomeMessage, WelcomeMessageLength, 0);
}



void store_packet(std::vector<std::array<char, BUFSIZE>>& packetlist, const char Message[]) {
	std::array<char, BUFSIZE> packet;
	std::memcpy(packet.data(), Message, BUFSIZE); 
	packetlist.push_back(packet);
}

void get_and_send_packet_info(std::vector<std::array<char, BUFSIZE>> packetlist, char DataName[], SOCKET ClientSocket, int TargettedSocket) {
	char* cleanedName = strtok(DataName, "\r\n");
	int NameLength = strlen(cleanedName) + 1;
	char sessionID[BUFSIZE];
	int sessionidLength;
	int bufsize = BUFSIZE;
	char SessionIDintToString[BUFSIZE];
	char Info[BUFSIZE];
	char TOTALMESSAGE[BUFSIZE];

	char NoPacketMessage[100];
	int NoPacketMessageLength;
	sprintf_s(NoPacketMessage, "<No Packet Found>");
	NoPacketMessageLength = strlen(NoPacketMessage);

	char NoDataNameMessage[100];
	int NoDataNameMessageLength;
	sprintf_s(NoDataNameMessage, "<No Data Called %s Found>", DataName);
	NoDataNameMessageLength = strlen(NoDataNameMessage);



	for (int i = 0; i < packetlist.size(); i++) {
		packet_parser_data(packetlist[i].data(), "SESSIONID", sessionID, bufsize);
		sprintf_s(SessionIDintToString, "%d", TargettedSocket);
		printf("PACKET SESSION ID %s\n", sessionID);
		printf("PACKET SESSION ID %s\n", SessionIDintToString);
		if (!strcmp(sessionID, SessionIDintToString)) {
			printf("FOUND THE PACKET\n");
			bufsize = BUFSIZE;
			//int ReturnLength;
			int ReturnLength = packet_parser_data(packetlist[i].data(), DataName, Info, bufsize);
			if (ReturnLength > 0) {
				printf("INFO SENDING: %s\n", Info);
				sprintf_s(TOTALMESSAGE, "%s = %s", DataName, Info);
				send(ClientSocket, TOTALMESSAGE, strlen(TOTALMESSAGE), 0);
			}
			else {
				send(ClientSocket, NoDataNameMessage, NoDataNameMessageLength, 0);
			}
			return;
		}
	}

	send(ClientSocket, NoPacketMessage, NoPacketMessageLength, 0);
}


void edit_packet_info(std::vector<std::array<char, BUFSIZE>>& packetlist, const char DataName[], SOCKET ClientSocket, const char NewData[]) {
	char sessionID[BUFSIZE];
	int bufsize = BUFSIZE;
	char sessionIDString[BUFSIZE];

	sprintf_s(sessionIDString, "%d", ClientSocket);

	for (auto& packet : packetlist) {
		if (packet_parser_data(packet.data(), "SESSIONID", sessionID, bufsize) > 0 &&
			strcmp(sessionID, sessionIDString) == 0) {

			printf("Editing packet for SESSIONID: %s\n", sessionID);

			char* fieldStart = strstr(packet.data(), DataName);
			if (fieldStart != nullptr) {
				char* valueStart = fieldStart + strlen(DataName);
				while (*valueStart == ' ' || *valueStart == '=') valueStart++;
				char* valueEnd = valueStart;
				while (*valueEnd && *valueEnd != ' ' && *valueEnd != '\n' && *valueEnd != '\0') {
					valueEnd++;
				}

				int oldValueLength = valueEnd - valueStart;
				int newValueLength = strlen(NewData);

				if (newValueLength <= oldValueLength) {
					memcpy(valueStart, NewData, newValueLength); 
					memset(valueStart + newValueLength, ' ', oldValueLength - newValueLength); 
				}
				else {
					printf("Error: New data is longer than existing space.\n");
					return;
				}

				printf("Field %s updated to: %s\n", DataName, NewData);
				return;
			}
			else {
				printf("Field %s not found in packet.\n", DataName);
				return;
			}
		}
	}

	printf("No matching packet found for SESSIONID: %s\n", sessionIDString);
}

void session_info_message(fd_set ReadFds, SOCKET ClientSocket)
{
	int i;
	char InfoMessage[100];
	int InfoMessageLength;

	for (i = 1; i < ReadFds.fd_count; ++i)
	{
		if (ClientSocket == ReadFds.fd_array[i])
		{
			continue;
		}

		sprintf_s(InfoMessage, "<Already connected client with session ID %d>", ReadFds.fd_array[i]);
		InfoMessageLength = strlen(InfoMessage);

		send(ClientSocket, InfoMessage, InfoMessageLength, 0);
	}
}

void send_notice_message(fd_set ReadFds, SOCKET ClientSocket)
{
	int i;
	char InfoMessage[100];
	int InfoMessageLength;

	for (i = 1; i < ReadFds.fd_count; ++i)
	{
		if (ClientSocket == ReadFds.fd_array[i])
		{
			continue;
		}

		sprintf_s(InfoMessage, "<New client has connected. Session ID is %d>", ClientSocket);
		InfoMessageLength = strlen(InfoMessage);

		send(ReadFds.fd_array[i], InfoMessage, InfoMessageLength, 0);
	}
}

void whisper_to_one(fd_set ReadFds, char Message[], int MessageLength, SOCKET ClientSocket, int TargetSocket)
{
	int MsgPos;
	int FD_Index;
	char FailMessage[] = "<Fail to send a message>";
	int FailMessageLength = strlen(FailMessage);
	char SuccessMessage[BUFSIZE];
	int SuccessMessageLength;
	char WhisperMessage[BUFSIZE];
	int WhisperMessageLength;

	for (FD_Index = 1; FD_Index < ReadFds.fd_count; ++FD_Index)
	{
		if (ReadFds.fd_array[FD_Index] == TargetSocket)
		{
			sprintf_s(WhisperMessage, "<Sender %d sent:%s>", ClientSocket, Message);
			WhisperMessageLength = strlen(WhisperMessage);
			send(ReadFds.fd_array[FD_Index], WhisperMessage, WhisperMessageLength, 0);

			sprintf_s(SuccessMessage, "<Message send to %d>", TargetSocket);
			SuccessMessageLength = strlen(SuccessMessage);
			send(ClientSocket, SuccessMessage, SuccessMessageLength, 0);

			break;
		}
	}
	if (FD_Index == ReadFds.fd_count)
	{
		send(ClientSocket, FailMessage, FailMessageLength, 0);
	}
}
void print_user_list(fd_set ReadFds, SOCKET ClientSocket) {
	int FD_Index;
	char UserListMessage[BUFSIZE];
	int UserListMessageLength;

	sprintf_s(UserListMessage, "\n <USER LIST: %d> \n", ReadFds.fd_count - 1);
	for (FD_Index = 1; FD_Index < ReadFds.fd_count; ++FD_Index)
	{
		sprintf_s(UserListMessage, "%s <User: %d> \n", UserListMessage, ReadFds.fd_array[FD_Index]);
	}
	UserListMessageLength = strlen(UserListMessage);
	send(ClientSocket, UserListMessage, UserListMessageLength, 0);
}

void print_commands(SOCKET ClientSocket) {
	char CommandListMessage[BUFSIZE];
	int CommandListMessageLength;

	sprintf_s(CommandListMessage, "\n<COMMAND LIST:> \n");
	sprintf_s(CommandListMessage, "%s <COMMAND: /w [UserNumber] [Message]> = Private messaging a specific user \n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <COMMAND: /send packet> = Send user information to server \n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <COMMAND: /getuserlist> = Get list of users in server \n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <COMMAND: /getinfo [UserNumber] [DataName]> = Getting info of person \n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <DataNames: [HEIGHT], [WEIGHT], [RELATIONSHIP]>\n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <COMMAND: /leave> = Leave Server \n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <COMMAND: /kick [UserNumber] [Reason]> = Kicks players off the server \n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <COMMAND: /setpassword [Password]> = Changes and sets a password \n", CommandListMessage);
	sprintf_s(CommandListMessage, "%s <COMMAND: /changepacket [DataName] [NewData]> = Changes packet data provided \n", CommandListMessage);

	CommandListMessageLength = strlen(CommandListMessage);
	send(ClientSocket, CommandListMessage, CommandListMessageLength, 0);
}

void send_to_all(fd_set ReadFds, char Message[], int MessageLength)
{
	int i;

	for (i = 1; i < ReadFds.fd_count; ++i)
	{
		send(ReadFds.fd_array[i], Message, MessageLength, 0);
	}
}

int main(int argc, char** argv)
{
	int          Port = PORT_NUMBER;
	WSADATA      WsaData;
	SOCKET       ServerSocket;
	SOCKADDR_IN  ServerAddr;

	unsigned int Index;
	int          ClientLen = sizeof(SOCKADDR_IN);
	SOCKET       ClientSocket;
	int       RoomMasterSessionID;
	SOCKADDR_IN  ClientAddr;

	fd_set       ReadFds, TempFds;
	TIMEVAL      Timeout; // struct timeval timeout;

	char         Message[BUFSIZE];
	int          Return;

	char Password[BUFSIZE] = { 0 };
	bool RoomMasterPresent = true;

	std::vector<std::array<char, BUFSIZE>> PacketList;

	std::vector<int> unverified_userlist;

	if (2 == argc)
	{
		Port = atoi(argv[1]);
	}
	printf("Using port number : [%d]\n", Port);

	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		printf("WSAStartup() error!\n");
		return 1;
	}

	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == ServerSocket)
	{
		printf("socket() error\n");
		return 1;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(ServerSocket, (SOCKADDR*)&ServerAddr,
		sizeof(ServerAddr)))
	{
		printf("bind() error\n");
		return 1;
	}

	if (SOCKET_ERROR == listen(ServerSocket, 5))
	{
		printf("listen() error\n");
		return 1;
	}

	FD_ZERO(&ReadFds);
	FD_SET(ServerSocket, &ReadFds);

	while (1)
	{
		TempFds = ReadFds;
		Timeout.tv_sec = 5;
		Timeout.tv_usec = 0;

		if (SOCKET_ERROR == (Return = select(0, &TempFds, 0, 0, &Timeout)))
		{ // Select() function returned error.
			printf("select() error\n");
			return 1;
		}
		if (0 == Return)
		{ // Select() function returned by timeout.
			printf("Select returned timeout.\n");
		}
		else if (0 > Return)
		{
			printf("Select returned error!\n");
		}
		else
		{
			for (Index = 0; Index < TempFds.fd_count; Index++)
			{
				if (TempFds.fd_array[Index] == ServerSocket)
				{ // New connection requested by new client.
					if (ReadFds.fd_count > 1) {
						RoomMasterPresent = true;
					}
					ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddr, &ClientLen);
					FD_SET(ClientSocket, &ReadFds);
					printf("New Client Accepted : Socket Handle [%d]\n", ClientSocket);
					if (RoomMasterPresent && ReadFds.fd_count == 2) {
						RoomMasterSessionID = ClientSocket;
						printf("FOUND ROOM MASTER: [%d]\n", RoomMasterSessionID);


						char RoomMasterMessage[100];
						int RoomMasterMessageLength;
						sprintf_s(RoomMasterMessage, "\n <You are now the RoomMaster, you have extra permissions now!> \n");
						RoomMasterMessageLength = strlen(RoomMasterMessage);
						send(ClientSocket, RoomMasterMessage, RoomMasterMessageLength, 0);
					}
					int PasswordLength = strlen(Password);
					if (PasswordLength > 0) {
						unverified_userlist.push_back(ClientSocket);

						char GivePasswordMessage[100];
						int GivePasswordMessageLength;
						sprintf_s(GivePasswordMessage, "\n <MESSAGE PASSWORD TO ENTER SERVER:> \n");
						GivePasswordMessageLength = strlen(GivePasswordMessage);

						send(ClientSocket, GivePasswordMessage, GivePasswordMessageLength, 0);
					}
					else {
						send_welcome_message(ClientSocket);
						session_info_message(ReadFds, ClientSocket);
						send_notice_message(ReadFds, ClientSocket);
					}
				}
				else
				{ // Something to read from socket.
					memset(Message, '\0', BUFSIZE);
					Return = recv(TempFds.fd_array[Index], Message, BUFSIZE, 0);
					if (0 == Return)
					{ // Connection closed message has arrived.
						closesocket(TempFds.fd_array[Index]);
						printf("Connection closed :Socket Handle [%d]\n", TempFds.fd_array[Index]);
						FD_CLR(TempFds.fd_array[Index], &ReadFds);
					}
					else if (0 > Return)
					{ // recv() function returned error.
						closesocket(TempFds.fd_array[Index]);
						printf("Exceptional error :Socket Handle [%d]\n", TempFds.fd_array[Index]);
						FD_CLR(TempFds.fd_array[Index], &ReadFds);
					}
					else
					{ // Message recevied.
						int ClientSocketNumber = NULL;
						char getmessage[BUFSIZE];
						char getmessage2[BUFSIZE];
						char* cleaned = strtok(Message, "\r\n");
						bool unverified = false;
						int UnverifiedIndex;
						//<CHRSTA  
						for (UnverifiedIndex = 0; UnverifiedIndex < unverified_userlist.size(); UnverifiedIndex++) {
							if (TempFds.fd_array[Index] == unverified_userlist[UnverifiedIndex]) {
								unverified = true;
								break;
							}
						}

						if (!unverified) {
							if (sscanf(cleaned, "/w %d %[^\n]", &ClientSocketNumber, getmessage) > 0)
							{
								whisper_to_one(ReadFds, getmessage, Return, TempFds.fd_array[Index], ClientSocketNumber);
							}
							else if (!strncmp("/leave", cleaned, 6))
							{

								char LeaveMessage[100];
								int LeaveMessageLength;

								sprintf_s(LeaveMessage, "<Leave Accepted>\n");
								LeaveMessageLength = strlen(LeaveMessage);

								send(TempFds.fd_array[Index], LeaveMessage, LeaveMessageLength, 0);


								closesocket(TempFds.fd_array[Index]);
								printf("Connection closed :Socket Handle [%d]\n", TempFds.fd_array[Index]);
								FD_CLR(TempFds.fd_array[Index], &ReadFds);
							}
							else if (!strncmp("/getuserlist", cleaned, 12))
							{
								print_user_list(ReadFds, TempFds.fd_array[Index]);
							}
							else if (!strncmp("<CHRSTA", cleaned, 7))
							{
								printf("RECIEVED PACKET\n");
								store_packet(PacketList, Message);
							}
							else if (sscanf(cleaned, "/getinfo %d %[^\n]", &ClientSocketNumber, getmessage) > 0)
							{
								get_and_send_packet_info(PacketList, getmessage, TempFds.fd_array[Index], ClientSocketNumber);
								printf("GETTING STUFF");
								//whisper_to_one(ReadFds, whispherMessage, Return, TempFds.fd_array[Index], ClientSocketNumber);
							}
							else if (sscanf(cleaned, "/kick %d %[^\n]", &ClientSocketNumber, getmessage) > 0)
							{
								char NoRoomMasterMessage[100];
								int NoRoomMasterMessageLength;
								sprintf_s(NoRoomMasterMessage, "<You must be the Room Master to use this command>\n");
								NoRoomMasterMessageLength = strlen(NoRoomMasterMessage);

								if (RoomMasterSessionID == TempFds.fd_array[Index]) {
									char LeaveMessage[100];
									int LeaveMessageLength;

									sprintf_s(LeaveMessage, "<KICKED FOR: %s>", getmessage);
									LeaveMessageLength = strlen(LeaveMessage);

									send(ClientSocketNumber, LeaveMessage, LeaveMessageLength, 0);


									closesocket(ClientSocketNumber);
									printf("Connection closed :Socket Handle [%d]\n", ClientSocketNumber);
									FD_CLR(ClientSocketNumber, &ReadFds);
								}
								else {
									send(TempFds.fd_array[Index], NoRoomMasterMessage, NoRoomMasterMessageLength, 0);
								}
							}
							else if (sscanf(cleaned, "/setpassword %[^\n]", getmessage) > 0)
							{
								char NoRoomMasterMessage[100];
								int NoRoomMasterMessageLength;
								sprintf_s(NoRoomMasterMessage, "<You must be the Room Master to use this command>\n");
								NoRoomMasterMessageLength = strlen(NoRoomMasterMessage);

								if (RoomMasterSessionID == TempFds.fd_array[Index]) {
									strcpy(Password, getmessage);
								}
								else {
									send(TempFds.fd_array[Index], NoRoomMasterMessage, NoRoomMasterMessageLength, 0);
								}
							}
							else if (sscanf(cleaned, "/changepacket %s %s", getmessage, getmessage2) > 0)
							{
								edit_packet_info(PacketList, getmessage, TempFds.fd_array[Index], getmessage2);
							}
							else if (!strncmp("/help", cleaned, 5))
							{
								print_commands(TempFds.fd_array[Index]);
								//whisper_to_one(ReadFds, whispherMessage, Return, TempFds.fd_array[Index], ClientSocketNumber);
							}
							else
							{
								send_to_all(ReadFds, Message, Return);
							}
						}
						else {

							char PasswordCorrectMessage[100];
							int PasswordCorrectMessageLength;
							sprintf_s(PasswordCorrectMessage, "<PASSWORD IS CORRECT>\n");
							PasswordCorrectMessageLength = strlen(PasswordCorrectMessage);

							char PasswordWrongMessage[100];
							int PasswordWrongMessageLength;
							sprintf_s(PasswordWrongMessage, "<PASSWORD IS INCORRECT>\n");
							PasswordWrongMessageLength = strlen(PasswordWrongMessage);

							if (!strcmp(Password, cleaned)) {
								unverified_userlist.erase(unverified_userlist.begin() + UnverifiedIndex);
								send(TempFds.fd_array[Index], PasswordCorrectMessage, PasswordCorrectMessageLength, 0);

								send_welcome_message(TempFds.fd_array[Index]);
								session_info_message(ReadFds, TempFds.fd_array[Index]);
								send_notice_message(ReadFds, TempFds.fd_array[Index]);

							}
							else {
								send(TempFds.fd_array[Index], PasswordWrongMessage, PasswordWrongMessageLength, 0);
							}
						}

						//printf("RECIEVED PACKET SIZE %d\n", PacketList.size());

					}
				}
			}
		}
	}

	WSACleanup();

	return 0;
}
