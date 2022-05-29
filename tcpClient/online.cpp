#include "online.h"
#include "ui_online.h"
#include<QDebug>
#include"tcpclient.h"

online::online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::online)
{
    ui->setupUi(this);
}

online::~online()
{
    delete ui;
}

void online::showAllUserOnline(struct PDU *pPDU)
{
    ui->listWidget->clear();

    char name[32] = {0};
    int count = pPDU->uintMsgLen/32; //模块数=总大小/每个模块的大小

    for(int i=0; i<count; i++){
        strncpy(name, (char*)pPDU->Msg+i*32, 32);
        ui->listWidget->addItem(name);
    }
}

void online::on_addFri_pb_clicked()
{
    QString strName = tcpClient::getInstance().getName();

    QListWidgetItem *pItem = ui->listWidget->currentItem();
    if(pItem == NULL){ qDebug() << "pItem:" << pItem; return; }
    QString strPerName = pItem->text();

    struct PDU *pPDU = mkPDU();
    pPDU->uintPDUType = ENUM_PDU_TYPE_ADD_FRIEND_REQUST;
    strncpy(pPDU->caData, strName.toStdString().c_str(), 32);
    strncpy(pPDU->caData+32, strPerName.toStdString().c_str(), 32);

    tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);
    free(pPDU);
    pPDU = NULL;
}
