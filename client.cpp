#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <stdio.h>
// Don't forget to include "ws2_32.lib" in the library list.
#include <winsock2.h> 
#include <string.h>
#include <windows.h>

#define TIME_PORT	27015
#define FALSE								0
#define TRUE								1
#define GETTIME								1
#define GETTIMEWITHOUTDATE					2
#define GETTIMESINCEEPOCH					3
#define GETCLIENTTOSERVERDELAYESTIMATION	4
#define MEASURERTT							5
#define GETTIMEWITHOUTDATEORSECONDS			6
#define GETYEAR								7
#define GETMONTHANDDAY						8
#define GETSECONDSSINCEBEGININGOFMONTH		9
#define GETDAYANDYEAR						10
#define GETDAYLIGHTSAVINGS					11
#define EXIT								12

int getUserSelection()
{
	int userChoice = 0;

	printf(" 1) GETTIME - Get time and date\n");
	printf(" 2) GETTIMEWITHOUTDATE - Get time without date\n");
	printf(" 3) GETTIMESINCEEPOCH - Get time since epoch\n");
	printf(" 4) GETCLIENTTOSERVERDELAYESTIMATION - Get delay between client and server\n");
	printf(" 5) MEASURERTT - Mesure RTT time\n");
	printf(" 6) GETTIMEWITHOUTDATEORSECONDS - Get hours and minutes\n");
	printf(" 7) GETYEAR - Get year\n");
	printf(" 8) GETMONTHANDDAY - Get month and day\n");
	printf(" 9) GETSECONDSSINCEBEGININGOFMONTH - get the number of seconds since the 1st of the month\n");
	printf("10) GETDAYANDYEAR - Get the number of days since the 1st of January\n");
	printf("11) GETDAYLIGHTSAVINGS - Get summer (1) or winter (0)\n");
	printf("12) Exit\n");
	printf("Please enter your choice: \n");
	scanf("%d", &userChoice);
	fflush(stdin);

	return userChoice;
}

int SendClientToServerDelayEstimation(char *sendBuff, SOCKET connSocket, sockaddr_in *server)
{
	DWORD clicksCount = 0;
	long firstClick = 0;
	long secondClick = 0;
	int rttLoops = 100;
	int bytesSent = 0;

	for (; rttLoops > 0; rttLoops--)
	{
		bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr *)server, sizeof(*server));
		if (SOCKET_ERROR == bytesSent)
		{
			std::cout << "Time Client: Error at sendto(): " << WSAGetLastError() << std::endl;
			std::cout << "Can't estimate" << std::endl;
			closesocket(connSocket);
			WSACleanup();
			return TRUE;
		}
	}
	return FALSE;
}

long RecvClientToServerDelayEstimation(SOCKET connSocket)
{
	long firstClick = 0;
	long secondClick = 0;
	long clicksCount = 0;
	char recvBuff[255] = { 0 };
	int rttLoops = 0;
	int bytesRecv = 0;

	bytesRecv = recv(connSocket, recvBuff, 255, 0);
	if (SOCKET_ERROR == bytesRecv)
	{
		std::cout << "Time Client: Error at recv(): " << WSAGetLastError() << std::endl;
		std::cout << "Can't messure RTT" << std::endl;
		closesocket(connSocket);
		WSACleanup();
		return -1;
	}

	firstClick = atol(recvBuff);
	for (rttLoops = 99; rttLoops > 0; rttLoops--)
	{
		bytesRecv = recv(connSocket, recvBuff, 255, 0);
		if (SOCKET_ERROR == bytesRecv)
		{
			std::cout << "Time Client: Error at recv(): " << WSAGetLastError() << std::endl;
			std::cout << "Can't messure RTT" << std::endl;
			closesocket(connSocket);
			WSACleanup();
			return -1;
		}
		secondClick = atol(recvBuff);
		clicksCount += abs(firstClick - secondClick);
		firstClick = secondClick;
	}

	return clicksCount;
}

int sendToServer(SOCKET connSocket, char *sendBuff, sockaddr_in *server)
{
	int bytesSent = 0;
	int bytesRecv = 0;
	char recvBuff[255] = { 0 };

	bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr *)server, sizeof(*server));
	if (SOCKET_ERROR == bytesSent)
	{
		std::cout << "Time Client: Error at sendto(): " << WSAGetLastError() << std::endl;
		closesocket(connSocket);
		WSACleanup();
		return TRUE;
	}
	std::cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	// Gets the server's answer using simple recieve (no need to hold the server's address).
	bytesRecv = recv(connSocket, recvBuff, 255, 0);
	if (SOCKET_ERROR == bytesRecv)
	{
		std::cout << "Time Client: Error at recv(): " << WSAGetLastError() << std::endl;
		closesocket(connSocket);
		WSACleanup();
		return TRUE;
	}

	recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
	std::cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
	return FALSE;

}

void mesureRTT(SOCKET connSocket, char *sendBuff, sockaddr_in *server)
{
	int rttLoops = 100;
	int bytesSent = 0;
	int errorEccured = FALSE;
	int bytesRecv = 0;
	int clicksCount = 0;
	DWORD beforeSend = 0;
	char recvBuff[255] = { 0 };

	while (rttLoops > 0)
	{
		beforeSend = GetTickCount();
		bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr *)server, sizeof(*server));
		if (SOCKET_ERROR == bytesSent)
		{
			std::cout << "Time Client: Error at sendto(): " << WSAGetLastError() << std::endl;
			std::cout << "Can't messure RTT" << std::endl;
			errorEccured = TRUE;
			closesocket(connSocket);
			WSACleanup();
			break;
		}

		// Gets the server's answer using simple recieve (no need to hold the server's address).
		bytesRecv = recv(connSocket, recvBuff, 255, 0);
		clicksCount += GetTickCount() - beforeSend;
		if (SOCKET_ERROR == bytesRecv)
		{
			std::cout << "Time Client: Error at recv(): " << WSAGetLastError() << std::endl;
			std::cout << "Can't messure RTT" << std::endl;
			closesocket(connSocket);
			errorEccured = TRUE;
			WSACleanup();
			break;
		}
		rttLoops--;
	}

	if (FALSE == errorEccured)
	{
		double avg = (double)clicksCount / 100.0;
		std::cout << "RTT Time: " << avg << std::endl;
	}
}

void main()
{

	// Initialize Winsock (Windows Sockets).
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) { std::cout << "Time Client: Error at WSAStartup()\n"; }

	// Client side:
	// Create a socket and connect to an internet address.

	SOCKET connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connSocket)
	{
		std::cout << "Time Client: Error at socket(): " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	// For a client to communicate on a network, it must connect to a server.
	// Need to assemble the required data for connection in sockaddr structure.
	// Create a sockaddr_in object called server. 
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(TIME_PORT);

	// Send and receive data.
	int bytesSent = 0;
	int bytesRecv = 0;
	long clicksCount = 0;
	char sendBuff[255] = { 0 };
	char recvBuff[255] = { 0 };
	unsigned char needToSend = FALSE;
	unsigned char errorEccured = FALSE;

	while (true)
	{
		switch (getUserSelection())
		{
			case GETTIME:
				needToSend = TRUE;
				strcpy(sendBuff, "GetTime");
				sendBuff[strlen(sendBuff)] = '\0';
				break;
			case GETTIMEWITHOUTDATE:
				needToSend = TRUE;
				strcpy(sendBuff, "GetTimeWithoutDate");
				sendBuff[strlen(sendBuff)] = '\0';
				break;
			case GETTIMESINCEEPOCH:
				needToSend = TRUE;
				strcpy(sendBuff, "GetTimeSinceEpoch");
				sendBuff[strlen(sendBuff)] = '\0';
				break;
			case GETCLIENTTOSERVERDELAYESTIMATION:
				needToSend = FALSE;
				strcpy(sendBuff, "GetClientToServerDelayEstimation");
				sendBuff[strlen(sendBuff)] = '\0';
				errorEccured = SendClientToServerDelayEstimation(sendBuff, connSocket, &server);

				if (FALSE == errorEccured)
				{
					clicksCount = RecvClientToServerDelayEstimation(connSocket);
					if (-1 == clicksCount) { errorEccured = TRUE; }
				}

				if (FALSE == errorEccured)
				{
					double avg = (double)clicksCount / 99.0;
					std::cout << "Estimation Time: " << avg << std::endl;
				}
				break;

			case MEASURERTT:	
				needToSend = FALSE;
				errorEccured = FALSE;
				strcpy(sendBuff, "GetTime");
				sendBuff[strlen(sendBuff)] = '\0';
				mesureRTT(connSocket, sendBuff, &server);
				break;

			case GETTIMEWITHOUTDATEORSECONDS:
				needToSend = TRUE;
				strcpy(sendBuff, "GetTimeWithoutDateOrSeconds");
				sendBuff[strlen(sendBuff)] = '\0';
				break;

			case GETYEAR:		
				needToSend = TRUE;
				strcpy(sendBuff, "GetYear");
				sendBuff[strlen(sendBuff)] = '\0';
				break;

			case GETMONTHANDDAY:
				needToSend = TRUE;
				strcpy(sendBuff, "GetMonthAndDay");
				sendBuff[strlen(sendBuff)] = '\0';
				break;

			case GETSECONDSSINCEBEGININGOFMONTH:
				needToSend = TRUE;
				strcpy(sendBuff, "GetSecondsSinceBeginingOfMonth");
				sendBuff[strlen(sendBuff)] = '\0';
				break;

			case GETDAYANDYEAR:	
				needToSend = TRUE;
				strcpy(sendBuff, "GetDayOfYear");
				sendBuff[strlen(sendBuff)] = '\0';
				break;

			case GETDAYLIGHTSAVINGS:
				needToSend = TRUE;
				strcpy(sendBuff, "GetDaylightSavings");
				sendBuff[strlen(sendBuff)] = '\0';
				break;

			case EXIT:
				// Closing connections and Winsock.
				std::cout << "Time Client: Closing Connection.\n";
				closesocket(connSocket);
				return;

			default:
				printf("You know where the sun comes from... that's a bad choice! BYE!!\n");
				closesocket(connSocket);
				return;
		}


		// Asks the server what's the currnet time.
		// The send function sends data on a connected socket.
		// The buffer to be sent and its size are needed.
		// The fourth argument is an idicator specifying the way in which the call is made (0 for default).
		// The two last arguments hold the details of the server to communicate with. 
		// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
		if (TRUE == needToSend)
		{
			errorEccured = FALSE;
			errorEccured = sendToServer(connSocket, sendBuff, &server);
			if (TRUE == errorEccured) { return; }
		}

	}
}