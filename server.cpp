#pragma comment (lib,"Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
SOCKET servSock;
std::mutex mtx;
std::vector<SOCKET> clients;
int users=0;
//удаление пользователя из вектора clients
void client_delete(int cur_user)
{
	mtx.lock();
	closesocket(clients[cur_user-1]);
	std::vector<SOCKET>::iterator it = clients.begin();
	advance(it, cur_user-1);
	clients.erase(it);
	mtx.unlock();
}
void chat(SOCKET clientSock, SOCKADDR_IN from)
{
	mtx.lock();
	users++;
	int cur_user = users;
	clients.push_back(clientSock);
	mtx.unlock();
	std::vector<SOCKET>::iterator it;
	int retVal;
	
	while (1)
	{
		char szReq[256];
		retVal = recv(clientSock, szReq, 256, 0);
		std::string s;
		for (int i = 0; i < retVal; ++i)
			s.push_back(szReq[i]);
		// Команда на выключение сервера
		if (s == "shut down")
		{
			s = "server was shut down\n";
			for (auto i : clients)
				send(i, s.c_str(), s.size(), 0);
			std::cout << s;
			system("pause");
			exit(1);
		} 
		if (s == "exit")
		{
			s = inet_ntoa(from.sin_addr);
			s+=" left the conversation\n";
			client_delete(cur_user);
			std::cout << s;
			for (auto i : clients)
				send(i, s.c_str(), s.size(), 0);
			break;
		}
		std::string temp;
		temp = inet_ntoa(from.sin_addr);
		temp += ": " + s;
		s = temp;
		s.push_back('\n');
		std::cout << s;
		for (auto i : clients)
			send(i, s.c_str(), s.size(), 0);
	}
}
int main(void)
{
	std::vector<std::thread>trhs;
	WORD sockVer;
	WSADATA wsaData;
	int retVal;
	std::string s;
	sockVer = MAKEWORD(2, 2);
	WSAStartup(sockVer, &wsaData);
	//Создаем сокет
	servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (servSock == INVALID_SOCKET)
	{
		std::cout << "Unable to create socket" << std::endl;
		WSACleanup();
		system("pause");
		return SOCKET_ERROR;
	}

	SOCKADDR_IN sin;
	sin.sin_family = PF_INET;
	sin.sin_port = htons(2005);
	sin.sin_addr.s_addr = INADDR_ANY;

	retVal = bind(servSock, (LPSOCKADDR)&sin, sizeof(sin));
	if (retVal == SOCKET_ERROR)
	{
		std::cout << "Unable to bind" << std::endl;
		WSACleanup();
		system("pause");
		return SOCKET_ERROR;
	}
	std::thread *client_th = nullptr;
	std::cout << "Server started at " << inet_ntoa(sin.sin_addr) << ", port " << htons(sin.sin_port) << std::endl;
	while (true)
	{
		//Пытаемся начать слушать сокет
		retVal = listen(servSock, 2005);
		if (retVal == SOCKET_ERROR)
		{
			std::cout << "Unable to listen" << std::endl;
			WSACleanup();
			system("pause");
			return SOCKET_ERROR;
		}
		//Ждем клиента
		SOCKET clientSock;
		SOCKADDR_IN from;
		int fromlen = sizeof(from);
		clientSock = accept(servSock, (struct sockaddr*)&from, &fromlen);
		if (clientSock != INVALID_SOCKET)
		{
			s = "Welcome new user \"";
			s += inet_ntoa(from.sin_addr);
			s+="\"\n";
			std::cout << s;
			for (auto i : clients)
				send(i, s.c_str(), s.size(), 0);
			client_th = new std::thread(chat, clientSock, from);
			client_th->detach();
		}
	}
	//Закрываем сокет	
	closesocket(servSock);
	WSACleanup();
	return 0;
}