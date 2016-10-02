#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <string.h>
#include "ui_whats_happening.h"
#include <iostream>
#include <QtWidgets/QPushButton>
#include <QtWidgets/qmessagebox.h>
#include <QCoreApplication>
#include <QAbstractButton>
#include <QtWidgets/QMainWindow>
#include <qdebug.h>
#include <fstream>
#include <QTimer>
#include <tchar.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 1454
#define DOOR_IN "15010"
#define DOOR_OUT "15000"
#define VERSION 1
#define COMAND 4
#define LENGTH 4
#define GROUP 30
#define DADOS 1400
#define DEF_IP 15
#define ALL 1454

#pragma once

using namespace std;

typedef struct {
	char Version;
	char Comand[COMAND + 1];
	int Length;
	char Dado[DADOS];
}Protocol;

class Cliente
{
public:
	int iResult = 0;
	Protocol T_Protocol;
	bool flag = true;
	SOCKET ConnectSocket = INVALID_SOCKET;

	typedef struct {
		HANDLE Event;
		bool *flag_c;
		string* Referencia;
		QWidget *parent;
		SOCKET *ClientSocket;

	}Buffer;

	Buffer t_param;
	HANDLE Thread_Recv;
	string str_IP, str_Group, str_Send, str_Comand;


	Cliente(QWidget *parent, bool *flag_c, string* Referencia);

	void Main_client(string str_IP, string str_Group, string str_Send, string str_Comand, bool flag);

	void initWin_Socket();

	void init_send();

	static void recv_Socket(void *parent);

	static void prepare_message(char *);

	void Init_Protocol(char[]);

	static void Rec_msg(void *);

	bool send_protocol(char *);

	int strch(char* txt, int init, int end, char* newtxt);

private:

	~Cliente();
};

