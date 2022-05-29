#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include"mytcpsocket.h"

#include <QList>
#include"protocol.h"

using namespace std;

class myTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static myTcpServer &get_instance();

    void forwardToPerson(struct PDU*, QString);
protected:
    virtual void incomingConnection(qintptr socketDescriptor);

private:
    explicit myTcpServer(QObject *parent = 0);
    QList<myTcpSocket*> m_myTcpSocket;  //用类继承，处理各个客户端的事情
public slots:
    void on_sigDisconnect(myTcpSocket*);
};

#endif // MYTCPSERVER_H
