#include "whats_happening.h"

_Whats_Happening::_Whats_Happening(QWidget *parent)
	: QMainWindow(parent),
	Whats(new Ui::Whats_HappeningClass)
{
	timer = new QTimer(this);
	W_message = new QTimer(this);
	this->setFixedSize(780, 550);
	Whats->setupUi(this);
	cliente = new Cliente(parent, &flag_c, &Referencia);
	Whats->Box_Send->setPlaceholderText("Message");
	Whats->Box_IP->setText("127.0.0.1");
	Whats->B_Send->setEnabled(true);
	Whats->B_Group->setEnabled(false);

	Whats->Change_Group->setEnabled(false);
	Whats->Change_IP->setEnabled(false);

	Whats->B_Group->setEnabled(false);

	connect(Whats->Box_IP, SIGNAL(returnPressed()), Whats->B_Group, SIGNAL(clicked()));
	connect(Whats->Box_Send, SIGNAL(returnPressed()), Whats->B_Send, SIGNAL(clicked()));
	connect(Whats->Box_Group, SIGNAL(returnPressed()), Whats->B_Send, SIGNAL(clicked()));
}

void _Whats_Happening::on_B_Send_clicked()
{
	string ip, group, send, comand;

	if (Whats->Box_Group->text().isEmpty() || Whats->Box_IP->text().isEmpty() || Whats->Box_Send->text().isEmpty())
	{
		QMessageBox::warning(this, "Warning", "The fields can not be empty...");
		return;
	}

	Whats->Change_Group->setEnabled(true);
	Whats->Change_IP->setEnabled(true);

	Whats->Box_Group->setEnabled(false);
	Whats->Box_IP->setEnabled(false);

	connect(timer, SIGNAL(timeout()), this, SLOT(My_time()));
	timer->start(1);
	Group.clear();
	IP.clear();
	Mensagem.clear();
	strcomand.clear();
	IP.append(Whats->Box_IP->text());
	Group.append(Whats->Box_Group->text());
	Mensagem.append(Whats->Box_Send->text());

	strip = IP.toLocal8Bit().constData();
	strgroup = Group.toLocal8Bit().constData();
	strsend.clear();

	if (flag) {

		strcomand.append("INC");
		cliente->Main_client(strip, strgroup, strsend, strcomand, flag);
		flag = false;
	}

	strcomand.clear();
	strsend = Mensagem.toLocal8Bit().constData();
	strcomand.append("TRAN");
	cliente->Main_client(strip, strgroup, strsend, strcomand, flag);
	strcomand.clear();
	Whats->Box_Send->clear();
}

void _Whats_Happening::My_time() {

	int Len;
	char  Data[ALL] = {};
	QString IP_Recv;
	QString Group_T;
	QString Message;
	char length[LENGTH] = {};

	strcpy(Data, Referencia.c_str());

	for (int i = VERSION; i < VERSION + GROUP; i++) {//GET THE IP
		Group_T.append(Data[i]);
	}

	for (int i = VERSION + COMAND + GROUP; i < VERSION + COMAND + GROUP + DEF_IP; i++) {//GET THE IP
		if (Data[i] != ' ')
			IP_Recv.append(Data[i]);
		else
			break;
	}

	for (int i = VERSION + GROUP + DEF_IP + COMAND; i < VERSION + COMAND + GROUP + DEF_IP + LENGTH; i++)//GET THE LENGTH
		length[i - (VERSION + GROUP + DEF_IP + COMAND)] = Data[i];

	Len = (int)atoi(length);

	for (int i = VERSION + GROUP + DEF_IP + COMAND + LENGTH; i < VERSION + COMAND + GROUP + DEF_IP + LENGTH + Len; i++)//GET THE DATA
		Message.append(Data[i]);

	if (!Message.isEmpty()) {
		if (IP_Recv.isEmpty()) {

			this->Whats->Text_vew_Edit->appendPlainText("Group: " + Group_T);
			this->Whats->Text_vew_Edit->appendPlainText("Send: " + Message + '\n');
		}
		else {

			this->Whats->Text_vew_Edit->appendPlainText("Group: " + Group_T);
			this->Whats->Text_vew_Edit->appendPlainText("Received IP: [" + IP_Recv + "]: " + Message + '\n');
		}
	}

	IP_Recv.clear();
	Group_T.clear();
	Referencia.clear();
	Message.clear();
	Data[0] = '\0';
}

void _Whats_Happening::My_Progress() {


}

_Whats_Happening::~_Whats_Happening()
{
	delete Whats;
	free(cliente);
	delete 	timer;
	delete W_message;
}

void _Whats_Happening::on_Change_Group_clicked()
{
	Comand.append("EXIT");
	strip.clear();
	strsend.clear();

	strcomand = Comand.toLocal8Bit().constData();

	cliente->Main_client(strip, strgroup, strsend, strcomand, false);

	on_Change_IP_clicked();
	Whats->Box_Group->clear();
	Whats->Box_IP->setText(IP);
	Whats->Box_Group->setEnabled(true);
	Whats->Box_IP->setEnabled(false);
}

void _Whats_Happening::on_Change_IP_clicked()
{
	flag = true;
	Whats->Text_vew_Edit->clear();
	Whats->Box_IP->setEnabled(true);
	Whats->Box_IP->clear();

	flag_c = false;
	//cliente->recv_Socket(&t_param);
	cliente = NULL;
	cliente = new Cliente(this, &flag_c, &Referencia);
}

void _Whats_Happening::Organiza(QString str, bool flag_send) {

	if (flag_send) {
		Whats->Text_vew_Edit->appendPlainText(str);
	}
}
