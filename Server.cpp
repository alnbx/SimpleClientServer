#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
// Don't forget to include "Ws2_32.lib" in the library list.
#include <winsock2.h>
#include <string.h>
#include <time.h>

#define __STDC_WANT_LIB_EXT1__ 1
#define TIME_PORT							27015
#define GETTIME								1
#define GETTIMEWITHOUTDATE					2
#define GETTIMESINCEEPOCH					3
#define GETCLIENTTOSERVERDELAYESTIMATION	4
#define GETTIMEWITHOUTDATEORSECONDS			6
#define GETYEAR								7
#define GETMONTHANDDAY						8
#define GETSECONDSSINEBEGININGOFMONTH		9
#define GETDAYANDYEAR						10
#define GETDAYLIGHTSAVINGS					11
#define OTHER								12


int userRequest(char *request)
{
	if      (0 == strcmp(request, "GetTime")) { return GETTIME; }
	else if (0 == strcmp(request, "GetTimeWithoutDate")) { return GETTIMEWITHOUTDATE;	}
	else if (0 == strcmp(request, "GetTimeSinceEpoch")) { return GETTIMESINCEEPOCH; }
	else if (0 == strcmp(request, "GetClientToServerDelayEstimation")) { return GETCLIENTTOSERVERDELAYESTIMATION; }
	else if (0 == strcmp(request, "GetTimeWithoutDateOrSeconds")) { return GETTIMEWITHOUTDATEORSECONDS; }
	else if (0 == strcmp(request, "GetYear")) { return GETYEAR; }
	else if (0 == strcmp(request, "GetMonthAndDay")) { return GETMONTHANDDAY; }
	else if (0 == strcmp(request, "GetSecondsSinceBeginingOfMonth")) { return GETSECONDSSINEBEGININGOFMONTH; }
	else if (0 == strcmp(request, "GetDayOfYear")) { return GETDAYANDYEAR; }
	else if (0 == strcmp(request, "GetDaylightSavings")) { return GETDAYLIGHTSAVINGS; }
	else return OTHER;
}

int main(int argc, char *argv[])
{
	// Initialize Winsock (Windows Sockets).
	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))  { cout << "Time Server: Error at WSAStartup()\n"; }

	// Server side:
	// Create and bind a socket to an internet address.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called m_socket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							datagram sockets (SOCK_DGRAM), 
	//							and the UDP/IP protocol (IPPROTO_UDP).
	SOCKET m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The "if" statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == m_socket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	// For a server to communicate on a network, it must first bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigdned long (4 bytes) data type).
	// INADDR_ANY means to listen on all interfaces.
	// inet_addr (Internet address) is used to convert a string (char *) into unsigned int.
	// inet_ntoa (Internet address) is the reverse function (converts unsigned int to char *)
	// The IP address 127.0.0.1 is the host itself, it's actually a loop-back.
	serverService.sin_addr.s_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
												// IP Port. The htons (host to network - short) function converts an
												// unsigned short from host to TCP/IP network byte order (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.
	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(m_socket, (SOCKADDR *)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		return 2;
	}

	// Waits for incoming requests from clients.
	// Send and receive data.
	sockaddr client_addr;
	int client_addr_len = sizeof(client_addr);
	int bytesSent = 0;
	int clientRequest = 0;
	int bytesRecv = 0;
	char sendBuff[255];
	char recvBuff[255];
	time_t timer = 0;
	struct tm *tm = NULL;
	long sec = 0;

	// Get client's requests and answer them.
	// The recvfrom function receives a datagram and stores the source address.
	// The buffer for data to be received and its available size are 
	// returned by recvfrom. The fourth argument is an idicator 
	// specifying the way in which the call is made (0 for default).
	// The two last arguments are optional and will hold the details of the client for further communication. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
	//cout << "Time Server: Wait for clients' requests.\n";

	while (true)
	{
		bytesRecv = recvfrom(m_socket, recvBuff, 255, 0, &client_addr, &client_addr_len);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Server: Error at recvfrom(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return 3;
		}

		recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
		//OLD ONE!
		//cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
		timer = time(NULL);
		tm = gmtime(&timer);

		switch (userRequest(recvBuff))
		{
			case GETTIME:
				strftime(sendBuff, sizeof(char) * sizeof(sendBuff) - 1, "%Y %m %d %H:%M:%S", tm);
				break;
			case GETTIMEWITHOUTDATE:
				strftime(sendBuff, sizeof(sendBuff), "%H:%M:%S", tm);
				break;
			case GETTIMESINCEEPOCH:
				//86400 seconds in a day
				sec = (((((tm->tm_year + 1900 - 1970) * 365 + tm->tm_yday - 1) * 86400) + (tm->tm_hour + 1) * 3600) + tm->tm_min * 60) + tm->tm_sec;
				_ltoa(sec, sendBuff, 10);
				break;
			case GETCLIENTTOSERVERDELAYESTIMATION:
				sec = (long)GetTickCount();
				_ltoa(sec, sendBuff, 10);
				break;
			case GETTIMEWITHOUTDATEORSECONDS:
				strftime(sendBuff, sizeof(char) * sizeof(sendBuff) - 1, "%H:%M", tm);
				break;
			case GETYEAR:
				sec = (long)(tm->tm_year + 1900);
				_ltoa(sec, sendBuff, 10);
				break;
			case GETMONTHANDDAY:
				strftime(sendBuff, sizeof(char) * sizeof(sendBuff) - 1, "%m %d", tm);
				break;
			case GETSECONDSSINEBEGININGOFMONTH:
				//86400 seconds in a day
				sec = (tm->tm_mday * 86400) + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
				_ltoa(sec, sendBuff, 10);
				break;
			case GETDAYANDYEAR:
				strftime(sendBuff, sizeof(char) * sizeof(sendBuff) - 1, "%Y %d", tm);
				break;
			case GETDAYLIGHTSAVINGS:
				_itoa(tm->tm_isdst, sendBuff, 10);
				break;
			case OTHER:
			default:
				exit(9);
		}
		// Answer client's request by the current time.

		// Parse the current time to printable string. - OLD ONE
		//strcpy_s(sendBuff, sizeof(char)*strlen(sendBuff),ctime(&timer));
		//sendBuff[strlen(sendBuff) - 1] = '\0'; //to remove the new-line from the created string

											   // Sends the answer to the client, using the client address gathered
											   // by recvfrom. 
		bytesSent = sendto(m_socket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr *)&client_addr, client_addr_len);
		if (SOCKET_ERROR == bytesSent)
		{
			cout << "Time Server: Error at sendto(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return 4;
		}

		cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(m_socket);
	WSACleanup();
	return 0;
}