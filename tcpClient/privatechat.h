#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include"protocol.h"

namespace Ui {
class privateChat;
}

class privateChat : public QWidget
{
    Q_OBJECT

public:
    static privateChat &getInstance();
    ~privateChat();

    void setName(const QString &pername);
    void showPrivateMsg(struct PDU *pPDU);
private slots:
    void on_send_pb_clicked();

private:
    Ui::privateChat *ui;

    explicit privateChat(QWidget *parent = 0);

    QString m_name;
    QString m_perName;
};

#endif // PRIVATECHAT_H
