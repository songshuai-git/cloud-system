#include "friend.h"
#include"protocol.h"
#include"tcpclient.h"
#include<QInputDialog>
#include"privatechat.h"

Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_showMsg_te = new QTextEdit;
    m_inputMsg_le = new QLineEdit;

    m_friList_lw = new QListWidget;

    m_sendMsg_pb = new QPushButton("信息发送");

    m_flushFri_pb = new QPushButton("刷新好友列表");
    m_delFri_pb = new QPushButton("删除好友");
    m_showOnline_pb = new QPushButton("显示在线用户");
    m_searchUsr_pb = new QPushButton("查找用户");
    m_privateChat_pb = new QPushButton("私聊");

    m_usrOnline = new online;
    //rightHLayout = new QHBoxLayout(this); //关联的方式 1:指定父对象 2:setLayout()
    QHBoxLayout *bottomHLayout = new QHBoxLayout;
    bottomHLayout->addWidget(m_inputMsg_le);
    bottomHLayout->addWidget(m_sendMsg_pb);

    QVBoxLayout *rightVLayout = new QVBoxLayout;
    rightVLayout->addWidget(m_flushFri_pb);
    rightVLayout->addWidget(m_delFri_pb);
    rightVLayout->addWidget(m_showOnline_pb);
    rightVLayout->addWidget(m_searchUsr_pb);
    rightVLayout->addWidget(m_privateChat_pb);

    QHBoxLayout *topHLayout = new QHBoxLayout;
    topHLayout->addWidget(m_showMsg_te);
    topHLayout->addWidget(m_friList_lw);
    topHLayout->addLayout(rightVLayout);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainVLayout->addLayout(topHLayout);
    mainVLayout->addLayout(bottomHLayout);
    mainVLayout->addWidget(m_usrOnline);
    m_usrOnline->hide();

    setLayout(mainVLayout);

    QObject::connect(m_showOnline_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_showOnline_pb(bool))
                     );
    QObject::connect(m_searchUsr_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_searchUsr_pb(bool))
                     );
    QObject::connect(m_flushFri_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_flushFri_pb(bool))
                     );
    QObject::connect(m_delFri_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_delFri_pb(bool))
                     );
    QObject::connect(m_privateChat_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_privateChat_pb(bool))
                     );
    QObject::connect(m_sendMsg_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_sendMsg_pb(bool))
                     );
}

void Friend::on_click_showOnline_pb(bool checked)
{
    (void)checked;

    if(m_usrOnline->isHidden()) m_usrOnline->show();
    else m_usrOnline->hide();

    struct PDU *pPDU = mkPDU();
    pPDU->uintPDUType = ENUM_PDU_TYPE_ALL_ONLINE_REQUST;

    tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    free(pPDU);
    pPDU = NULL;
}

void Friend::on_click_searchUsr_pb(bool)
{
    m_name = QInputDialog::getText(this, "查找用户", "请输入查找的用户名:");
    if(m_name.isEmpty()) return;

    struct PDU *pPDU = mkPDU();
    pPDU->uintPDUType = ENUM_PDU_TYPE_SEARCH_USR_REQUST;
    strcpy(pPDU->caData, m_name.toStdString().c_str());

    tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    free(pPDU);
    pPDU = NULL;
}

void Friend::on_click_flushFri_pb(bool checked)
{
    (void)checked;

    QString name = tcpClient::getInstance().getName();

    struct PDU *pPDU = mkPDU();
    pPDU->uintPDUType = ENUM_PDU_TYPE_FLUSH_FRI_REQUST;
    strcpy(pPDU->caData, name.toStdString().c_str());

    tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    free(pPDU);
    pPDU = NULL;
}

void Friend::on_click_delFri_pb(bool checked)
{
    (void)checked;

    QString name = tcpClient::getInstance().getName();

    QListWidgetItem *pItem = m_friList_lw->currentItem();
    if(pItem == NULL){ qDebug() << "pItem:" << pItem; return; }
    QString strPerName = pItem->text();

    struct PDU *pPDU = mkPDU();
    pPDU->uintPDUType = ENUM_PDU_TYPE_DEL_FRI_REQUST;
    strncpy(pPDU->caData, name.toStdString().c_str(), 32);
    strncpy(pPDU->caData+32, strPerName.toStdString().c_str(), 32);

    tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    free(pPDU);
    pPDU = NULL;
}

void Friend::on_click_privateChat_pb(bool)
{
    privateChat &privateChatW = privateChat::getInstance();
    if(privateChatW.isHidden()) privateChatW.show();

    QListWidgetItem *pItem = m_friList_lw->currentItem();
    if(pItem == NULL){ qDebug() << "pItem:" << pItem; return; }

    QString perName = pItem->text();
    privateChatW.setName(perName);
}

void Friend::on_click_sendMsg_pb(bool checked)
{
    (void)checked;
    QString name = tcpClient::getInstance().getName();

    if(m_inputMsg_le->text().isEmpty()) return;

    QString msg = m_inputMsg_le->text();

    uint msgLen = msg.size() + 1;
    struct PDU *pPDU = mkPDU(msgLen);
    pPDU->uintPDUType = ENUM_PDU_TYPE_GROUP_CHAT_REQUST;
    strncpy(pPDU->caData, name.toStdString().c_str(), 32);
    memcpy(pPDU->Msg, msg.toStdString().c_str(), pPDU->uintMsgLen);

    tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    free(pPDU);
    pPDU = NULL;
}

void Friend::showAllUserOnline(struct PDU *pPDU){
    m_usrOnline->showAllUserOnline(pPDU);
}

void Friend::showAllFirOnline(struct PDU *pPDU)
{
    m_friList_lw->clear();

    char firName[32] = {0};
    int count = pPDU->uintMsgLen/32;

    for(int i=0; i<count; i++){
        strncpy(firName, (char*)pPDU->Msg+i*32, 32);
        m_friList_lw->addItem(firName);
    }

    free(pPDU);
    pPDU = NULL;
}

void Friend::showGroupChatMsg(PDU *pPDU)
{
    m_showMsg_te->append(QString("From %1: %2")
                         .arg(pPDU->caData)
                         .arg((char*)pPDU->Msg));

    free(pPDU);
    pPDU = NULL;
}
