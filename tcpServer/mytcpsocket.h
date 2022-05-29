#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include<QTcpSocket>
#include<QFile>
#include<QTimer>

class myTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit myTcpSocket(QObject *parent = 0);
    QString getName()const;
    void dirCopy(QString srcDir, QString deskDir);
private:
    QString m_strName;

    bool m_uploadStatus;
    QFile m_file;
    qint64 m_totalSize;
    qint64 m_receivedSize;

    QTimer *m_timer;
    QString m_strPath;
signals:
    void sigDisconnect(myTcpSocket*);
public slots:
    void on_revMsg();
    void on_disconnect();

    void on_uploadFileData();
};

#endif // MYTCPSOCKET_H
