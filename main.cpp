// Необходимо, чтобы линковка происходила с DLL-библиотекой 
// Для работы с сокетам
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>

using namespace std;

int client_socket;// Сокет который принимает подключаемый пользователей и передаёт их в общую колекцию
SOCKET* Connections;// Все пользователи которые вообще есть
//SOCKET Listen;// Объявление сокета
int listen_socket;

int client_count = 0;

void SendMessageToClient(int ID)
{
	const int max_client_buffer_size = 1024;
	char buffer[max_client_buffer_size];
	for (;; Sleep(75))
	{
		memset(buffer, 0, sizeof(buffer));// зануляем
		int result = recv(Connections[ID], buffer, max_client_buffer_size, NULL);

		if (result == SOCKET_ERROR) {
			// ошибка получения данных
			cerr << "Recv failed: " << result << "\n";
			//shutdown(Connections[ID], 2);
			closesocket(Connections[ID]);
			
		}
		else if (result == 0) {
			// соединение закрыто клиентом
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
					// произошла ошибка при отправле данных
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

	// служебная структура для хранение информации
	// о реализации Windows Sockets
	WSADATA data;
	// указываем версию сокетов
	WORD version = MAKEWORD(2, 2);

	// старт использования библиотеки сокетов процессом
	// (подгружается Ws2_32.dll)
	int result = WSAStartup(version, &data);

	// Если произошла ошибка подгрузки библиотеки
	if (result != 0) {
		cerr << "WSAStartup failed: " << result << "\n";
		return result;
	}

	struct addrinfo* addr = NULL; // структура, хранящая информацию об IP-адресе  слущающего сокета

	// Шаблон для инициализации структуры адреса
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;// сокет по протоколу ipv4
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;// Задаем потоковый тип сокета (если соединение tcp/ip)
	hints.ai_protocol = IPPROTO_TCP;

	// Инициализируем структуру, хранящую адрес сокета - addr.
	result = getaddrinfo(NULL, "7770", &hints, &addr);

	// Если инициализация структуры адреса завершилась с ошибкой,
	// выведем сообщением об этом и завершим выполнение программы 
	if (result != 0) {
		cerr << "getaddrinfo failed: " << result << "\n";
		WSACleanup(); // выгрузка библиотеки Ws2_32.dll
		return 1;
	}

	// Создание сокета
	listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

	// Если создание сокета завершилось с ошибкой, выводим сообщение,
	// освобождаем память, выделенную под структуру addr,
	// выгружаем dll-библиотеку и закрываем программу
	if (listen_socket == INVALID_SOCKET) {
		cerr << "Error at socket: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	// Привязываем сокет к IP-адресу
	result = bind(listen_socket, addr->ai_addr, addr->ai_addrlen);// связываем адрес порта и сокет
	
	// Если привязать адрес к сокету не удалось, то выводим сообщение
	// об ошибке, освобождаем память, выделенную под структуру addr.
	// и закрываем открытый сокет.
	// Выгружаем DLL-библиотеку из памяти и закрываем программу.
	if (result == SOCKET_ERROR) {
		cerr << "bind failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	// Инициализируем слушающий сокет и начинаем слушать соединения
	// SOMAXCONN-число одновременно пытающихся получить соединения
	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
		cerr << "listen failed with error: " << WSAGetLastError() << "\n";
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(addr);

	printf("Start server...\n");
	char m_connect[] = "Connect...;;;5";

	Connections = (SOCKET*)calloc(64, sizeof(SOCKET));//инициализируем массив пользователей

	for (;; Sleep(75))
	{
		if (client_socket = accept(listen_socket, NULL, NULL))// accept-принимает команды 
		{
			printf("Client connect...\n");
			Connections[client_count] = client_socket;
			result = send(Connections[client_count], m_connect, strlen(m_connect), NULL);
			if (result == SOCKET_ERROR) {
				// произошла ошибка при отправле данных
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