#undef UNICODE
#define _WIN32_WINNT 0x501
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

//#include Windows.h, este deve ser precedido com a macro #define WIN32_LEAN_AND_MEAN. Por raz?es hist?ricas, o cabe?alho WINDOWS.H incluindo o arquivo de cabe?alho Winsock.h
//para o Windows Sockets 1.1. As declara??es constantes do arquivo de cabe?alho Winsock.h ir?o entrar em conflito com as declara??es no arquivo de cabe?alho Winsock2.h exigido pelo Windows Sockets 2.0.

#include <windows.h>
#include <winsock2.h> // Cont?m a maioria das funcionalidades WinSock
#include <ws2tcpip.h> //Cont?m as defini??es intorduzidas na Winsock 2
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <map>
#include <process.h>

#pragma comment (lib, "Ws2_32.lib") // Indica para o compilador que a biblioteca Ws2_32.lib deve ser linkada nessa aplica??o

#define DEFAULT_BUFLEN 1454
#define DOORECEIVE "15000"
#define DOORSEND "15010"
#define TYPE 4
#define GROUP 30
#define LENGTH 4
#define IP 15
#define DATA 1400
#define VERSION 1
#define ALL 1454

using namespace std;

enum flag {//Enum para as tipos de comandos

	INC,
	TRAN,
	EXIT
};

typedef struct {//estrutura para armazenar os campos do protocolo

	string Version;
	string Type;
	string Group;
	int Length;
	string Door;
	string Ip;
	string Data;
}Protocol;

typedef struct {//estrutura usada para salvar Ip do cliente e 

	string buffer;
	string ip;
	HANDLE event;//campo para ser usado pelo Timeout receber os eventos
	SOCKET clientsocket;//campo para salvar o socket retornado pelo accept
	HANDLE* mthread;//campo para salvar o ID da thread que recebe as mensagens
	HANDLE* Time_thread;// campo para salvar o ID do timeout 
}Buffer;

map <string, string> clientGroup;//map para relacionar IP com GRUPO
map <string, SOCKET> clientSocket;//map para relacionar IP com SOCKET

void careMsg(void *);//administra e organiza os clientes antes de enviar cada mensagem
int sendMsg(char *, SOCKET);//envia as mensagens
int createSocketSend(Buffer bufferclient);//cria o socket
void killClient(string ip);//exclui o cliente do servidor
int strch(char* txt, int init, int end, char* newtxt);//substiu um trecho de uma string
void talk(void*);//funcao que recebe as mensagens do usuario
void TimeOut(void* param);//timeout
bool whatSocket(Buffer client);//verifica se ja existe um socket aberto para o cliente, caso nao haja ele cria um novo
flag getGroup(Protocol prot_client);//pega o grupo
int sendGroup(Buffer client_buffer, string group);//envia para todos do grupo
Protocol initProtocol(char*);//quebra as mensagen do protocolo
void MessageDisplay(string message);//mensagem da tela do servidor

HANDLE hsemaphore = CreateSemaphore(NULL, 1, 1, NULL);//semaforo para excluir um cliente
HANDLE hsemaphoreDisplay = CreateSemaphore(NULL, 1, 1, NULL);//semaforo para mostra as mensagens na tela do servidor

int __cdecl main(void)
{
	WSADATA wsaData; // WSADATA ? o tipo utilizado para inicializar o WinSock
	SOCKADDR_IN client_info = { 0 };

	SOCKET receiveSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	Buffer recvBuffer;

	char recvbuf[ALL] = {};
	struct addrinfo hints;
	struct addrinfo *addreceive;
	HANDLE Thread, EventPulse;

	int recvbuflen = ALL, iResult = 0, addrsize = sizeof(client_info);

	// Inicializa o Winsock indicando a vers?o do Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // MAKEWORD(2,2) indica com qual vers?o o Winsock ser? inicializado
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	//Criando o Winsock no servidor

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;  // Especifica que o endere?amento IP ? do IPV4
	hints.ai_socktype = SOCK_STREAM; // Espec?fica o socket stream
	hints.ai_protocol = IPPROTO_TCP; // Especifica qual o protocolo da camada de transporte
	hints.ai_flags = AI_PASSIVE;

	/*================================= DOOR ======================================*/
	iResult = getaddrinfo(NULL, DOORECEIVE, &hints, &addreceive);//PORTA QUE RECEBE AS MENSAGENS
	if (iResult != 0) {
		printf("getaddrinfo falha. C?digo do erro: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	/*================================ SOCKETS  =====================================*/
	receiveSocket = socket(addreceive->ai_family, addreceive->ai_socktype, addreceive->ai_protocol);//SOCKET PARA RECEBEER AS MENSAGENS
	if (receiveSocket == INVALID_SOCKET) {
		printf("falha na cria??o do socket. C?digo de erro: %ld\n", WSAGetLastError());
		freeaddrinfo(addreceive);
		WSACleanup();
		return 1;
	}
	/*===============================================================================*/

	/*=================================== BIND ======================================*/
	iResult = bind(receiveSocket, addreceive->ai_addr, (int)addreceive->ai_addrlen);//CONECTA A PORTA AO SOCKET DE RECEIVE
	if (iResult == SOCKET_ERROR) {
		printf("bind falhou. C?digo do erro: %d\n", WSAGetLastError());
		freeaddrinfo(addreceive);
		closesocket(receiveSocket);
		WSACleanup();
		return 1;
	}
	/*===============================================================================*/
	freeaddrinfo(addreceive);

	iResult = listen(receiveSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("Falha no listen. C?digo de erro: %d\n", WSAGetLastError());
		closesocket(receiveSocket);
		WSACleanup();
		return 1;
	}

	do {

		MessageDisplay(" >>>>>>>>>>>>> Listen <<<<<<<<<<<<<<< ");

		ClientSocket = accept(receiveSocket, (struct sockaddr*)&client_info, &addrsize);

		if (ClientSocket == INVALID_SOCKET) {

			printf("Falha accept. Codigo de erro: %d\n", WSAGetLastError());
			closesocket(receiveSocket);

			WSACleanup();
			return 1;
		}

		recvBuffer.ip = inet_ntoa(client_info.sin_addr);//pega o ip
		recvBuffer.clientsocket = ClientSocket;//copia a socket do retorno do accept para realizar o receive na thread
		recvBuffer.mthread = &Thread;//copia o ID da thread por referencia para que possa mata-la quando receber um exit

		Thread = (HANDLE)_beginthread(talk, 0, &recvBuffer);

	} while (true);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ClientSocket);
	WSACleanup();
	system("PAUSE");
	return 0;
}

void TimeOut(void* param) {

	Buffer mParam = *(Buffer*)param;
	while (WaitForSingleObject(mParam.event, 120000) == WAIT_OBJECT_0);//milisegundos

	closesocket(mParam.clientsocket);//mata a socket do receive.
	_endthread();
}

void talk(void* param) {//funcao que recebbe as mensagens

	Buffer Clientparam = *(Buffer*)param;

	int iResult = 0;
	char recvbuf[ALL] = {};
	HANDLE Thread;
	HANDLE Time_thread;

	Clientparam.event = CreateEvent(NULL, false, false, NULL);
	Clientparam.Time_thread = &Time_thread;

	Time_thread = (HANDLE)_beginthread(TimeOut, 0, &Clientparam);

	while ( iResult = recv(Clientparam.clientsocket, recvbuf, ALL, 0) ) {

		PulseEvent(Clientparam.event);//atualiza o timeout para esperar novamente 2 min.
		Clientparam.buffer = recvbuf;

		if (iResult > 0)
			Thread = (HANDLE)_beginthread(careMsg, 0, &Clientparam);
		else break;
	}

	if (iResult <= 0) {//caso tenha dado erro com a conexao

		MessageDisplay(" >>>>>>>>>>>>>  foi perdida a conexao com " + Clientparam.ip + ".");
		killClient(Clientparam.ip);
		closesocket(Clientparam.clientsocket);
	}

	CloseHandle(Thread);
	TerminateThread(Time_thread, EXIT_SUCCESS);//mata a thread do timeout
	TerminateThread(Thread, EXIT_SUCCESS);//mata a thread que controla e administra o cliente.
	CloseHandle(Time_thread);

	_endthread();
}

void killClient(string ip = "127.0.0.1") {//exlui o cliente

	if (WaitForSingleObject(hsemaphore, INFINITE) == WAIT_OBJECT_0) {

		if (clientSocket[ip] != INVALID_SOCKET)
			closesocket(clientSocket[ip]);

		map<string, SOCKET>::iterator it = clientSocket.find(ip);
		if (it != clientSocket.end())
			clientSocket.erase(it);

		map<string, string> ::iterator it2 = clientGroup.find(ip);
		if (it2 != clientGroup.end())
			clientGroup.erase(it2);

		if (!ReleaseSemaphore(hsemaphore, 1, NULL)) 
			cout << "Error no Release do Semaforo!" << endl;
	}
}

flag getGroup(Protocol prot_client) {//pega o grupo do cliente

	if (clientGroup[prot_client.Ip] == prot_client.Group && prot_client.Type == "TRAN")
		return TRAN;
	else if (prot_client.Type == "INC ")
		return INC;
	else if (prot_client.Type == "EXIT")
		return EXIT;

	return EXIT;
}

void MessageDisplay(string message) {// mostra as mensagen na tela do serivdor

	if (WaitForSingleObject(hsemaphoreDisplay, INFINITE) == WAIT_OBJECT_0) {

		system("cls");
		cout << message.data() << endl;

		if (!ReleaseSemaphore(hsemaphoreDisplay, 1, NULL))
			cout << "Erro no Release da funcao MessageDisplay" << endl;
	}
	else cout << "Erro no WaitForSingleObject da funcao MessageDisplay" << endl;
}

int sendGroup(Buffer client_buffer, string group) {

	string ip = client_buffer.ip;

	for (std::map<string, string>::iterator it = clientGroup.begin(); it != clientGroup.end(); ++it) {

		if (it->second == group && it->first != ip) {

			client_buffer.ip = it->first;

			if (!whatSocket(client_buffer))
				cout << "Erro ae enviar a msg para o Ip: " << it->first.c_str() << endl;
		}
	}

	return 0;
}

bool whatSocket(Buffer client) {

	if (clientSocket[client.ip] == INVALID_SOCKET) {//verifica se existe uma socket ativa para o cliente

		if (createSocketSend(client) <= 0)//verifica se ocorreu erro ao criar uma socket para enviar
			return false;
		else return true;
	}
	else {

		if (sendMsg((char*) client.buffer.c_str(), clientSocket[client.ip]) > 0) // envia as mensagens
			return true;
		else return false;
	}
	return true;
}

int strch(char* txt, int init, int end, char* newtxt) {

	int cont = 0, length = strlen(newtxt);
	for (int i = init; i < (init + end); i++) {
		if (cont < length) {
			txt[i] = newtxt[cont];
			cont++;
		}
		else txt[i] = '\0';
	}
	return 1;
}

Protocol initProtocol(char* recvbuf) {

	Protocol protBuffer;
	string length;

	int i;
	protBuffer.Version = recvbuf[0];

	for (i = VERSION; i < VERSION + GROUP; i++)//GET THE GROUP
		protBuffer.Group += recvbuf[i];

	for (i = (VERSION + GROUP); i < (VERSION + TYPE + GROUP); i++)//GET THE TYPE
		protBuffer.Type += recvbuf[i];

	for (i = (VERSION + GROUP + IP + TYPE); i < (VERSION + TYPE + GROUP + IP + LENGTH); i++)//GET THE LENGTH
		length += recvbuf[i];

	protBuffer.Length = (int)atoi(length.c_str());

	for (i = (VERSION + GROUP + IP + TYPE + LENGTH); i < (VERSION + TYPE + GROUP + IP + LENGTH + protBuffer.Length); i++)//GET THE DATA
		protBuffer.Data += recvbuf[i];

	return protBuffer;
}

int createSocketSend(Buffer bufferclient) {

	struct addrinfo hints, *addSend = NULL;
	int iResult = 0;
	SOCKET sendSocket;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;  // Especifica que o endere?amento IP ? do IPV4
	hints.ai_socktype = SOCK_STREAM; // Espec?fica o socket stream
	hints.ai_protocol = IPPROTO_TCP; // Especifica qual o protocolo da camada de transporte
	hints.ai_flags = AI_NUMERICHOST;

	iResult = getaddrinfo((char*)bufferclient.ip.c_str(), DOORSEND, &hints, &addSend);
	if (iResult != 0) {
		printf("getaddrinfo falha. C?digo do erro: %d\n", iResult);
		WSACleanup();
		return 0;
	}

	for (; addSend != NULL; addSend = addSend->ai_next) {

		sendSocket = socket(addSend->ai_family, addSend->ai_socktype, addSend->ai_protocol);//SOCKET PARA ENVIAR AS MENSAGENS
		if (sendSocket == INVALID_SOCKET) {
			printf("falha na cria??o do socket. C?digo de erro: %ld\n", WSAGetLastError());
			freeaddrinfo(addSend);
			return 0;
		}

		iResult = connect(sendSocket, addSend->ai_addr, (int)addSend->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(sendSocket);
			sendSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	if (sendSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		return 0;
	}

	iResult = sendMsg((char*)bufferclient.buffer.c_str(), sendSocket);

	freeaddrinfo(addSend);
	//closesocket(sendSocket);
	clientSocket[bufferclient.ip] = sendSocket;

	ZeroMemory(&hints, sizeof(hints));

	return iResult;
}

void careMsg(void *param) {

	Buffer bufferClient = *(Buffer*)param;

	Protocol prot;
	int erro = 0;
	flag result;
	char size[4];

	prot = initProtocol((char*)bufferClient.buffer.c_str());

	prot.Ip = bufferClient.ip;
	strch( (char*)prot.Ip.c_str(), 0, 15, (char*)bufferClient.ip.c_str() );
	result = getGroup(prot);

	switch (result) {

	case EXIT:

		MessageDisplay(">>>>>> Cliente " + prot.Ip + " saiu");
		killClient(prot.Ip);//exclui o cliente do servidor
		closesocket(bufferClient.clientsocket);//fecha a socket do receive

		TerminateThread(*bufferClient.mthread, EXIT_SUCCESS);//mata a thread que recebe as mensagens
		//TerminateThread(*bufferClient.Time_thread, EXIT_SUCCESS);//mata a thread do timeout

		return _endthread();
		break;

	case TRAN:

		bufferClient.buffer = prot.Version;
		bufferClient.buffer += prot.Group;
		bufferClient.buffer += prot.Type;

		while (true) {

			if (prot.Ip.size() < 15)
				prot.Ip += ' ';
			else break;
		}
		bufferClient.buffer += prot.Ip;
		itoa(prot.Data.size(), size, 10);

		while (true) {

			if ( (int)strlen( size ) < 4)
				strcat(size, " ");
			else break;
		}
		bufferClient.buffer += size;
		bufferClient.buffer += prot.Data;

		if (sendGroup(bufferClient, prot.Group) > 0)//envia para o grupo
			cout << "Error ao enviar a mensagem para o grupo " << prot.Group.c_str() << endl;
	
		break;

	case INC:

		clientGroup[prot.Ip] = prot.Group;
		clientSocket[prot.Ip] = INVALID_SOCKET;
		break;
	}

	cout << "________________________________________________" << endl;
	cout << "Ip: " << prot.Ip.data() << endl;
	cout << "Mensagem: " << prot.Data.data() << " para o grupo: " << prot.Ip.data() << endl;
	cout << "________________________________________________" << endl << endl;

	_endthread();
}

int sendMsg(char *recvbuf, SOCKET client) {//envia mensagem

	int iSendResult;

	iSendResult = send(client, recvbuf, ALL, 0);
	if (iSendResult <= 0){
		printf("send failed with error: %d\n", WSAGetLastError());
	closesocket(client);
	WSACleanup();
	return 0;
	}

	return iSendResult;
}
