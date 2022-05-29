#include "mytcpserver.h"
#include <QDebug>

myTcpServer &myTcpServer::get_instance()
{
    static myTcpServer instance;
    return instance;
}

void myTcpServer::forwardToPerson(struct PDU *pPDU, QString name)
{
    qDebug() << "forwardToPerson:" << name;
    QList<myTcpSocket*>::iterator it = m_myTcpSocket.begin();
    for(; it != m_myTcpSocket.end(); it++){
        if( (*it)->getName() == name ){
            (*it)->write((char*)pPDU, pPDU->uintPDULen);
            break;
        }
    }
}

void myTcpServer::incomingConnection(qintptr socketDescriptor)
{
    //qDebug() << "new client connected";
    myTcpSocket *pTcpSocket = new myTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);

    m_myTcpSocket.append(pTcpSocket);

    QObject::connect(pTcpSocket, SIGNAL(sigDisconnect(myTcpSocket*)),
                     this, SLOT(on_sigDisconnect(myTcpSocket*)));
}

myTcpServer::myTcpServer(QObject *parent)
    :QTcpServer(parent)
{

}

void myTcpServer::on_sigDisconnect(myTcpSocket *mysocket)
{
    QList<myTcpSocket*>::iterator it = m_myTcpSocket.begin();

    for(; it != m_myTcpSocket.end(); it++){
        if(*it == mysocket){
            //delete *it;   //???double free
            *it = NULL;
            m_myTcpSocket.erase(it);
            break;
        }
    }

    for(int i=0; i<m_myTcpSocket.size(); i++){
        qDebug() << m_myTcpSocket.at(i)->getName();
    }
}
