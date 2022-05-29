#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>

namespace Ui {
class tcpServer;
}

class tcpServer : public QWidget
{
    Q_OBJECT

public:
    explicit tcpServer(QWidget *parent = 0);
    ~tcpServer();

    void loadConfig();
private:
    Ui::tcpServer *ui;

    QString m_strIp;
    quint16 m_usPort;
};

#endif // TCPSERVER_H
