#include "Cliente.h"


Cliente::Cliente(QWidget *parent, bool *flag_c, string* Referencia) {

	t_param.flag_c = flag_c;
	t_param.parent = parent;
	t_param.Referencia = Referencia;
	initWin_Socket();
	Thread_Recv = (HANDLE)_beginthread(recv_Socket, 0, &t_param);
}

void Cliente::Main_client(string str_IP, string str_Group, string str_Send, string str_Comand, bool flag) {

	this->str_IP.clear();
	this->str_Group.clear();
	this->str_Send.clear();
	this->str_Comand.clear();

	this->str_IP = str_IP;
	this->str_Group = str_Group;
	this->str_Send = str_Send;
	this->flag = flag;
	this->str_Comand = str_Comand;

	char Str_len[10];
	char sendbuf[DEFAULT_BUFLEN] = {};

	int recvbuflen = DEFAULT_BUFLEN;
	sendbuf[0] = '1';
	_itoa_s(str_Send.length(), Str_len, 10);

	strch(sendbuf, VERSION, VERSION + GROUP, (char *)str_Group.c_str());//group
	strch(sendbuf, VERSION + GROUP, VERSION + GROUP + COMAND, (char *)str_Comand.c_str());//comand
	strch(sendbuf, VERSION + GROUP + COMAND, VERSION + GROUP + COMAND + DEF_IP, (char*)" ");//ip
	strch(sendbuf, VERSION + GROUP + COMAND + DEF_IP, VERSION + GROUP + COMAND + DEF_IP + LENGTH, Str_len);//tamanho
	strch(sendbuf, VERSION + GROUP + COMAND + DEF_IP + LENGTH, VERSION + GROUP + COMAND + DEF_IP + LENGTH + DADOS, (char *)str_Send.c_str());//texto

																																			 // Initialize Winsock
	if (flag) {

		init_send();
		send_protocol(sendbuf);
	}
	else {

		if (send_protocol(sendbuf)) {

			t_param.Referencia->append(sendbuf);
		}
		//else
			//exit(1);
	}
}

void Cliente::initWin_Socket() {

	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {

		QMessageBox::warning(t_param.parent, "Failed", "WSAStartup failed with error!...");
		exit(1);
	}
}

void Cliente::init_send() {

	struct addrinfo *result = NULL, *ptr = NULL, hints_s;

	ZeroMemory(&hints_s, sizeof(hints_s));
	hints_s.ai_family = AF_INET;  // AF_INET para IP v4
	hints_s.ai_socktype = SOCK_STREAM;
	hints_s.ai_protocol = IPPROTO_TCP; // Define o Protocolo
	hints_s.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo((char*)str_IP.c_str(), DOOR_OUT, &hints_s, &result);
	if (iResult != 0) {

		QMessageBox::warning(t_param.parent, "Failed", "getaddrinfo failed with error...");
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {

			QMessageBox::warning(t_param.parent, "Failed", "socket failed with error...");
			WSACleanup();
			return;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {

			QMessageBox::warning(t_param.parent, "Failed", "connect failed with error...");
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	if (ConnectSocket == INVALID_SOCKET) {

		QMessageBox::warning(t_param.parent, "Failed", "Unable to connect to server!...");
		WSACleanup();
		return;
	}
}

void Cliente::recv_Socket(void *Param_t) {

	Buffer *Param = (Buffer*)Param_t;

	char *Mensage;
	SOCKADDR_IN client_info = { 0 };
	HANDLE Recv;
	Param->Event = CreateEvent(NULL, FALSE, FALSE, NULL);

	SOCKET receiveSocket = INVALID_SOCKET;
	SOCKET client_t = INVALID_SOCKET;

	char recvbuf[ALL] = {};
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	struct addrinfo *addreceive;

	if (Param->flag_c) {

		int recvbuflen = ALL, iResult = 0, addrsize = sizeof(client_info);

		//Criando o Winsock no servidor

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;  // Especifica que o endere?amento IP ? do IPV4
		hints.ai_socktype = SOCK_STREAM; // Espec?fica o socket stream
		hints.ai_protocol = IPPROTO_TCP; // Especifica qual o protocolo da camada de transporte
		hints.ai_flags = AI_PASSIVE;
		/*================================= DOOR ======================================*/

		iResult = getaddrinfo(NULL, DOOR_IN, &hints, &addreceive);//PORTA QUE RECEBE AS MENSAGENS
		if (iResult != 0 && !Param->flag_c) {

			MessageBox(NULL, (LPCWSTR)L"getaddrinfo failed...!", (LPCWSTR)L"Failed", MB_ICONHAND);
			WSACleanup();
			return;
		}

		/*================================ SOCKETS  =====================================*/
		receiveSocket = socket(addreceive->ai_family, addreceive->ai_socktype, addreceive->ai_protocol);//SOCKET PARA RECEBEER AS MENSAGENS
		if (receiveSocket == INVALID_SOCKET) {

			MessageBox(NULL, (LPCWSTR)L"falha na criação do socket!", (LPCWSTR)L"Failed", MB_ICONHAND);
			freeaddrinfo(addreceive);
			WSACleanup();
			return;
		}

		/*=================================== BIND ======================================*/

		iResult = bind(receiveSocket, addreceive->ai_addr, (int)addreceive->ai_addrlen);//CONECTA A PORTA AO SOCKET DE RECEIVE
		if (iResult == SOCKET_ERROR && !Param->flag_c) {

			MessageBox(NULL, (LPCWSTR)L"bind falhou!", (LPCWSTR)L"Failed", MB_ICONHAND);
			freeaddrinfo(addreceive);
			closesocket(receiveSocket);
			WSACleanup();
			return;
		}

		iResult = listen(receiveSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR && !Param->flag_c) {

			MessageBox(NULL, (LPCWSTR)L"Falha no listen!", (LPCWSTR)L"Failed", MB_ICONHAND);
			closesocket(receiveSocket);
			WSACleanup();
			return;
		}
		
		client_t = accept(receiveSocket, (struct sockaddr*)&client_info, &addrsize);
		if (client_t == INVALID_SOCKET && !Param->flag_c) {

			MessageBox(NULL, (LPCWSTR)L"Falha accept!", (LPCWSTR)L"Failed", MB_ICONHAND);
			closesocket(receiveSocket);
			WSACleanup();
			return;
		}
		
		while (true) {

			if (recv(client_t, recvbuf, ALL, 0) > 0) {
				Param->Referencia->append(recvbuf);
			}
			else {
				if (!Param->flag_c)
					MessageBox(NULL, (LPCWSTR)L"Erro ao receber...", (LPCWSTR)L"failed", MB_ICONHAND);
				break;
			}
		}
	}

	closesocket(client_t);
	closesocket(receiveSocket);
	freeaddrinfo(result);
	freeaddrinfo(addreceive);
	WSACleanup();
	ZeroMemory(&hints, sizeof(hints));

}

void Cliente::Rec_msg(void *Buffer_t) {

	Buffer *Param_t = (Buffer*)Buffer_t;
	char recvbuf[ALL] = {};

	while (recv(*(Param_t->ClientSocket), recvbuf, ALL, 0) > 0) {

		MessageBox(NULL, (LPCWSTR)L"Recebido...", (LPCWSTR)recvbuf, MB_ICONHAND);
		Param_t->Referencia->append(recvbuf);
	}
	MessageBox(NULL, (LPCWSTR)L"Erro ao receber...", (LPCWSTR)recvbuf, MB_ICONHAND);
}

int Cliente::strch(char* txt, int init, int end, char* newtxt) {

	int cont = 0;
	for (int i = init; i < end; i++) {
		if (cont < strlen(newtxt)) {
			txt[i] = newtxt[cont];
			cont++;
		}
		else txt[i] = ' ';
	}
	return 1;
}

bool Cliente::send_protocol(char *mensagem) {

	iResult = send(ConnectSocket, mensagem, ALL, 0);

	if (iResult == SOCKET_ERROR) {

		QMessageBox::warning(t_param.parent, "Failed", "send failed with error...");
		closesocket(ConnectSocket);
		WSACleanup();
		return false;
	}
	else
		return true;
}


Cliente::~Cliente()
{
	closesocket(ConnectSocket);
}
