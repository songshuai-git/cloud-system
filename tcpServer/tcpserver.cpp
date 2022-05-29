#include "tcpserver.h"
#include "ui_tcpserver.h"

#include<QFile>
#include<QMessageBox>
#include<QDebug>
#include<QHostAddress>
#include"mytcpserver.h"

tcpServer::tcpServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcpServer)
{
    ui->setupUi(this);

    loadConfig();

    myTcpServer &instance =  myTcpServer::get_instance();
    instance.listen(QHostAddress(m_strIp), m_usPort);
}

tcpServer::~tcpServer()
{
    delete ui;
}

void tcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly)){
        QByteArray bytStr = file.readAll();
        file.close();
        QString strStr = bytStr.data();
        //QString strStr = bytStr.toStdString().c_str();

        strStr.replace("\r\n", " ");
        QStringList lisStr = strStr.split(" ");

        m_strIp = lisStr.at(0);
        m_usPort = lisStr.at(1).toUShort();
    }else{
        QMessageBox::critical(this, "open file", "open failed");
    }

    return;
}
