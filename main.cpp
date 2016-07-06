#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")


int initialize(WSADATA& wsaData) 
{
	int result = WSAStartup(WINSOCK_VERSION, &wsaData);
	return result;
}


int createSocket(const char *ip, const char *port, SOCKET& Socket, PADDRINFOA& addrInfo)
{
	Socket = INVALID_SOCKET;
	
	ADDRINFOA hints;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	int result = getaddrinfo(ip, port, &hints, &addrInfo);
	if (result != 0) 
	{
		return result;
	}
	PADDRINFOA ptr = addrInfo;
	Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	return 0;
}


int connectSocket(SOCKET Socket, PADDRINFOA addrInfo)
{
	PADDRINFOA ptr = addrInfo;
	int result = connect(Socket, ptr->ai_addr, ptr->ai_addrlen);

	return result;
}


int main(int argc, char **argv)
{

	////////
	if (argc < 3)
	{
		printf("Need 2 args: [ip] [port]\n");
		return 0;
	}
	////////

	////////
	WSADATA WSAData;
	int result = initialize(WSAData);
	if (result != 0) 
	{
		printf("Invalid initialize WinSock. Code: %d", result);
		return result;
	}
	////////

	////////
	PADDRINFOA addrInfo;
	SOCKET Socket;
	result = createSocket(argv[1], argv[2], Socket, addrInfo);

	if (result != 0) 
	{
		printf("getaddrinfo failed: %d\n", result);
		WSACleanup();
		return result;
	}

	if (Socket == INVALID_SOCKET)
	{
		result = WSAGetLastError();
		printf("Error at socket(): %ld\n", result);
		freeaddrinfo(addrInfo);
		WSACleanup();
		return result;
	}
	////////

	////////
	result = connectSocket(Socket, addrInfo);
	if (result == SOCKET_ERROR) 
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
	}

	freeaddrinfo(addrInfo);

	if (Socket == INVALID_SOCKET) 
	{
		printf("Unable to connect to server\n");
		WSACleanup();
		return result;
	}
	////////

	////////
	char* sendbuf = "get_stream";
	int sendbuflen = (int)strlen(sendbuf);

	result = send(Socket, sendbuf, sendbuflen, 0);
	if (result == SOCKET_ERROR) 
	{
		result = WSAGetLastError();
		printf("send failed: %d\n", result);
		closesocket(Socket);
		WSACleanup();
		return result;
	}

	printf("Bytes Sent: %ld\n", result);
	printf("%s", sendbuf);
	////////

	////////
	const int recvbuflen = 14;
	char recvbuf[recvbuflen + 1];

	do 
	{
		result = recv(Socket, recvbuf, recvbuflen, 0);
		if (result > 0)
		{
			printf("Bytes received: %d\n", result);

			recvbuf[result] = 0;
			printf("%s\n", recvbuf);
		}
		else if (result == 0)
		{
			printf("Connection closed\n");
		}
		else
		{
			result = WSAGetLastError();
			printf("recv failed: %d\n", result);
			return result;
		}

	} while (result > 0);
		
	////////

	closesocket(Socket);
	WSACleanup();
		
	return 0;
}