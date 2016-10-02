#ifndef __WHATS_HAPPENING_H
#define __WHATS_HAPPENING_H

#include <QtWidgets/QPushButton>
#include <QtWidgets/qmessagebox.h>
#include <QCoreApplication>
#include <QAbstractButton>
#include <QtWidgets/QMainWindow>
#include <qdebug.h>
#include <QTimer>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ui_whats_happening.h"
#include "Cliente.h"

class _Whats_Happening : public QMainWindow
{
	Q_OBJECT

public:

	typedef struct {
		HANDLE Event;
		bool *flag_c;
		string* Referencia;
		QWidget *parent;
		SOCKET *ClientSocket;

	}Buffer;

	Buffer t_param;
	QString IP;
	QString Mensagem;
	QString Group;
	QString Comand;
	QString Group_Ant;
	string Referencia;
	int Time = 120;

	bool flag = true;
	bool flag_c = true;

	HANDLE Event = CreateEvent(NULL, FALSE, FALSE, NULL);;
	string strip, strgroup, strsend, strcomand;

	_Whats_Happening(QWidget *parent = 0);
	~_Whats_Happening();

	private slots:

	void on_B_Send_clicked();

	void My_time();

	void My_Progress();

	void Organiza(QString str, bool flag_send);

	void on_Change_Group_clicked();

	void on_Change_IP_clicked();

private:
	Ui::Whats_HappeningClass *Whats;
	Cliente *cliente;
	QTimer *timer;
	QTimer *W_message;

};

#endif // __WHATS_HAPPENING_H
