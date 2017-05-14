// ����������, ����� �������� ����������� � DLL-����������� 
// ��� ������ � �������
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>

using namespace std;

int client_socket;// ����� ������� ��������� ������������ ������������� � ������� �� � ����� ��������
SOCKET* Connections;// ��� ������������ ������� ������ ����
//SOCKET Listen;// ���������� ������
int listen_socket;

int client_count = 0;

void SendMessageToClient(int ID)
{
	const int max_client_buffer_size = 1024;
	char buffer[max_client_buffer_size];
	for (;; Sleep(75))
	{
		memset(buffer, 0, sizeof(buffer));// ��������
		int result = recv(Connections[ID], buffer, max_client_buffer_size, NULL);

		if (result == SOCKET_ERROR) {
			// ������ ��������� ������
			cerr << "Recv failed: " << result << "\n";
			//shutdown(Connections[ID], 2);
			closesocket(Connections[ID]);
			
		}
		else if (result == 0) {
			// ���������� ������� ��������
			cerr << "connection closed...\n";
		}
		else if (result > 0)
		{
			buffer[result] = '\0';
			printf("%s", buffer);
			//printf("\n");
			for (int i = 0; i < client_count; i++)
			{
				result = send(Connections[i], buffer, strlen(buffer), NULL);

				if (result == SOCKET_ERROR) {
					// ��������� ������ ��� �������� ������
					cerr << "send failed: " << WSAGetLastError() << "\n";
				}
			}
		}
		//closesocket(Connections[ID]);
	}
	delete []buffer;
}

int main()
{
	setlocale(LC_ALL, "rus");

	// ��������� ��������� ��� �������� ����������
	// � ���������� Windows Sockets
	WSADATA data;
	// ��������� ������ �������
	WORD version = MAKEWORD(2, 2);

	// ����� ������������� ���������� ������� ���������
	// (������������ Ws2_32.dll)
	int result = WSAStartup(version, &data);

	// ���� ��������� ������ ��������� ����������
	if (result != 0) {
		cerr << "WSAStartup failed: " << result << "\n";
		return result;
	}

	struct addrinfo* addr = NULL; // ���������, �������� ���������� �� IP-������  ���������� ������

	// ������ ��� ������������� ��������� ������
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;// ����� �� ��������� ipv4
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;// ������ ��������� ��� ������ (���� ���������� tcp/ip)
	hints.ai_protocol = IPPROTO_TCP;

	// �������������� ���������, �������� ����� ������ - addr.
	result = getaddrinfo(NULL, "7770", &hints, &addr);

	// ���� ������������� ��������� ������ ����������� � �������,
	// ������� ���������� �� ���� � �������� ���������� ��������� 
	if (result != 0) {
		cerr << "getaddrinfo failed: " << result << "\n";
		WSACleanup(); // �������� ���������� Ws2_32.dll
		return 1;
	}

	// �������� ������
	listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

	// ���� �������� ������ ����������� � �������, ������� ���������,
	// ����������� ������, ���������� ��� ��������� addr,
	// ��������� dll-���������� � ��������� ���������
	if (listen_socket == INVALID_SOCKET) {
		cerr << "Error at socket: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	// ����������� ����� � IP-������
	result = bind(listen_socket, addr->ai_addr, addr->ai_addrlen);// ��������� ����� ����� � �����
	
	// ���� ��������� ����� � ������ �� �������, �� ������� ���������
	// �� ������, ����������� ������, ���������� ��� ��������� addr.
	// � ��������� �������� �����.
	// ��������� DLL-���������� �� ������ � ��������� ���������.
	if (result == SOCKET_ERROR) {
		cerr << "bind failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	// �������������� ��������� ����� � �������� ������� ����������
	// SOMAXCONN-����� ������������ ���������� �������� ����������
	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
		cerr << "listen failed with error: " << WSAGetLastError() << "\n";
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(addr);

	printf("Start server...\n");
	char m_connect[] = "Connect...;;;5";

	Connections = (SOCKET*)calloc(64, sizeof(SOCKET));//�������������� ������ �������������

	for (;; Sleep(75))
	{
		if (client_socket = accept(listen_socket, NULL, NULL))// accept-��������� ������� 
		{
			printf("Client connect...\n");
			Connections[client_count] = client_socket;
			result = send(Connections[client_count], m_connect, strlen(m_connect), NULL);
			if (result == SOCKET_ERROR) {
				// ��������� ������ ��� �������� ������
				cerr << "send failed: " << WSAGetLastError() << "\n";
			}
			client_count++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendMessageToClient, (LPVOID)(client_count - 1), NULL, NULL);
		}
	}
	closesocket(client_socket);
	WSACleanup();

	return 0;
}