#include "privatechat.h"
#include "ui_privatechat.h"
#include"tcpclient.h"
#include<QDebug>

privateChat::privateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::privateChat)
{
    ui->setupUi(this);
}

privateChat &privateChat::getInstance()
{
    static privateChat instance;
    return instance;
}

privateChat::~privateChat()
{
    delete ui;
}

void privateChat::setName(const QString &perName)
{
    m_name = tcpClient::getInstance().getName();
    m_perName = perName;
}

void privateChat::showPrivateMsg(struct PDU *pPDU)
{
    if(this->isHidden()) this->show();

    setName(pPDU->caData);
    ui->msg_tb->append(QString("From %1: %2")
                       .arg(pPDU->caData)
                       .arg((char*)pPDU->Msg)
                       );
}

void privateChat::on_send_pb_clicked()
{
    QString strMsg = ui->input_le->text();
    ui->input_le->clear();
    if(strMsg.isEmpty()) return;

    uint uintMsgLen = strMsg.size();      //实际消息的大小
    struct PDU *pPDU = mkPDU(uintMsgLen+1);

    pPDU->uintPDUType = ENUM_PDU_TYPE_PRIVATE_CHAT_REQUST;
    strncpy(pPDU->caData, m_name.toStdString().c_str(), 32);
    strncpy(pPDU->caData+32, m_perName.toStdString().c_str(), 32);
    memcpy((char*)pPDU->Msg, strMsg.toStdString().c_str(), pPDU->uintMsgLen);

    tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    free(pPDU);
    pPDU = NULL;
}
