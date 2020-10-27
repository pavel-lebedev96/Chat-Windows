#pragma comment (lib,"Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <string>
#include <iostream> 
#include <vector>
#include<thread>
using namespace std;
SOCKET clientSock;
void receive_messages()
{
	while (1)
	{
		char szResponse[256];
		string s;
		//ѕытаемс€ получить ответ от сервера
		int retVal = recv(clientSock, szResponse, 256, 0);
		if (retVal == SOCKET_ERROR)
		{
			cout << "Unable to recv" << endl;
			WSACleanup();
			Sleep(1000);
			exit(1);
		}
		for (int i = 0; i < retVal; ++i)
			s.push_back(szResponse[i]);
		cout << s;
		if (s == "server was shut down\n")
		{
			Sleep(1000);
			exit(1);
		}
	}
}
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA wsaData;
	int retVal = 0;
	WSAStartup(ver, (LPWSADATA)&wsaData);
	LPHOSTENT hostEnt;
	hostEnt = gethostbyname("localhost");
	if (!hostEnt)
	{
		cout<<"Unable to collect gethostbyname"<<endl;
		WSACleanup();
		system("pause");
		return 1;
	}
	//—оздаем сокет
	clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSock == SOCKET_ERROR)
	{
		cout << "Unable to create socket" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}
	string ip;
	cout << "ip>";
	cin >> ip;
	cin.ignore();

	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = PF_INET;
	serverInfo.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	serverInfo.sin_port = htons(2005);
	//ѕытаемс€ присоединитс€ к серверу по ip и port
	retVal = connect(clientSock, (LPSOCKADDR)&serverInfo, sizeof(serverInfo));
	if (retVal == SOCKET_ERROR)
	{
		cout << "Unable to connect" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}
	cout << "Connection made sucessfully" << endl;
	thread th(receive_messages);
	th.detach();
	while (1)
	{
		string s;
		getline(cin,s,'\n');
		//ќтсылаем данные на сервер
		retVal = send(clientSock, s.c_str(), s.size(), 0);
		if (s == "exit")
		{
			th.~thread();
			closesocket(clientSock);
			exit(1);
		}
		if (retVal == SOCKET_ERROR)
		{
			cout << "Unable to send" << endl;
			th.~thread();
			closesocket(clientSock);
			WSACleanup();
			system("pause");
			exit(1);
		}
	}
}