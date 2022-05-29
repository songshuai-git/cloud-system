#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QString>
#include<QTcpSocket>

namespace Ui {
class tcpClient;
}

class tcpClient : public QWidget
{
    Q_OBJECT

public:
    static tcpClient &getInstance();
    ~tcpClient();

    void loadConfig();
    QTcpSocket &getTcpSocket();
    inline QString getName()const{ return m_strName; }
    inline QString getCurPath()const{ return m_strCurPath; }
    inline void setCurPath(QString strCurPath){ m_strCurPath = strCurPath; }
private:
    explicit tcpClient(QWidget *parent = 0);

    Ui::tcpClient *ui;

    QString m_strIp;
    quint16 m_usPort;

    QTcpSocket m_tcpSocket;

    QString m_strName;
    QString m_strCurPath;
    //QString m_strCurPath;
private slots:
    void on_showConnect();
    void on_recvMsg();
//    void on_sendButton_clicked();
    void on_login_pb_clicked();
    void on_regist_pb_clicked();
    void on_cancel_pb_clicked();

};

#endif // TCPCLIENT_H
