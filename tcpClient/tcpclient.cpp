#include "tcpclient.h"
#include "ui_tcpclient.h"

#include<QFile>
#include<QMessageBox>
#include<QDebug>
#include<QHostAddress>
#include<QByteArray>

#include"protocol.h"
#include"opewidget.h"
#include"privatechat.h"

tcpClient::tcpClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcpClient)
{
    ui->setupUi(this);
    //resize();

    loadConfig();

    QObject::connect(&m_tcpSocket, SIGNAL(connected()),
                     this,SLOT(on_showConnect()));
    QObject::connect(&m_tcpSocket, SIGNAL(readyRead()),
                     this,SLOT(on_recvMsg()));

    m_tcpSocket.connectToHost(QHostAddress(m_strIp), m_usPort);
}

tcpClient &tcpClient::getInstance()
{
    static tcpClient instance;
    return instance;
}

tcpClient::~tcpClient()
{
    delete ui;
}

void tcpClient::loadConfig(){
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly)){
        QByteArray bytStr = file.readAll();
        file.close();
        QString strStr = bytStr.data();
        //QString strStr = bytStr.toStdString().c_str();

        strStr.replace("\r\n", " ");
        QStringList lisStr = strStr.split(" ");

        m_strIp = lisStr.at(0);
        m_usPort = lisStr.at(1).toUShort();

        //qDebug() << m_strIp <<endl << m_usPort;
    }else{
        QMessageBox::critical(this, "open file", "open failed");
    }

    return;
}

QTcpSocket &tcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

void tcpClient::on_showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void tcpClient::on_recvMsg()
{
    opeWidget &opeW = opeWidget::getInstance();
    if(opeW.getBook()->getDownloadStatus() == true){
        QByteArray buffer = m_tcpSocket.readAll();
        QFile &file = opeW.getBook()->getFile();
        qint64 totalSize = opeW.getBook()->getTotalSize();
        qint64 *receivedSize = opeW.getBook()->getReceivedSize();

        qint64 ret = file.write(buffer);
        qDebug() << "has accept:" << ret;
        *receivedSize += buffer.size();
        if(*receivedSize == totalSize){
            QMessageBox::information(this, "下载文件", DOWNLOAD_FILE_OK);

            file.close();
            opeW.getBook()->setDownloadStatus(false);
        }else if(*receivedSize > totalSize){
            QMessageBox::warning(this, "下载文件", "下载文件失败");

            file.close();
            opeW.getBook()->setDownloadStatus(false);
        }
        return;
    }
    //qint64 buffer = m_tcpSocket.bytesAvailable();
    //qDebug() << buffer;

    //struct PDU *pPDU = mkPDU()
    uint uintPDULen = 0;    //！此处必须初始化 不然这块空间有随机值 后面read后不会完全覆盖
    m_tcpSocket.read((char*)&uintPDULen, sizeof(int));  //已经读了四个字节了

    uint uintMsgLen = uintPDULen-sizeof(struct PDU);
    struct PDU *pPDU = mkPDU(uintMsgLen);

    m_tcpSocket.read((char*)pPDU+sizeof(uint), uintPDULen-sizeof(uint));
    //qDebug() << pPDU->uint8PDUType << (char*)pPDU->Msg;

    switch(pPDU->uintPDUType){
        case ENUM_PDU_TYPE_RGST_RESPOND:{
            if(strcmp(pPDU->caData, RGST_SUEESS)==0){
                QMessageBox::information(this, "注册", RGST_SUEESS);
            }else if(strcmp(pPDU->caData, RGST_FAIL)==0){
                QMessageBox::warning(this, "注册", RGST_FAIL);
            }

            break;
        }
        case ENUM_PDU_TYPE_LOGIN_RESPOND:{
            if(strcmp(pPDU->caData, LOGIN_SUCCESS)==0){
                QMessageBox::information(this, "登录", LOGIN_SUCCESS);

                opeWidget::getInstance().show();
                this->hide();

                m_strCurPath = QString("./%1").arg(m_strName);
            }else if(strcmp(pPDU->caData, LOGIN_FAIL)==0){
                QMessageBox::warning(this, "登录", LOGIN_FAIL);
            }

            break;
        }
        case ENUM_PDU_TYPE_ALL_ONLINE_RESPOND:{
            opeWidget::getInstance().getFriend()->showAllUserOnline(pPDU);
            break;
        }
        case ENUM_PDU_TYPE_SEARCH_USR_RESPOND:{
            QMessageBox::information(this, "查找用户",
                                     QString("%1:%2")
                                     .arg(opeWidget::getInstance().getFriend()->m_name)
                                     .arg(pPDU->caData)
                                    );
            break;
        }
        case ENUM_PDU_TYPE_ADD_FRIEND_RESPOND:{
            QMessageBox::information(this, "添加好友", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_ADD_FRIEND_REQUST:{
            QMessageBox::StandardButton btn = QMessageBox::NoButton;
            btn = QMessageBox::information(this, "添加好友请求",
                                           QString("%1 want to add you as friend,agree or not?")
                                           .arg(pPDU->caData),
                                           QMessageBox::Ok,
                                           QMessageBox::No);
            if(btn == QMessageBox::Ok){
                pPDU->uintPDUType = ENUM_PDU_TYPE_ADD_FRIEND_AGREE;
            }else{
                pPDU->uintPDUType = ENUM_PDU_TYPE_ADD_FRIEND_REJECT;
            }

            m_tcpSocket.write((char*)pPDU, pPDU->uintPDULen);

            break;
        }
        case ENUM_PDU_TYPE_ADD_FRIEND_AGREE:{
            QMessageBox::information(this, "添加好友",
                                     QString("%1 %2")
                                     .arg(pPDU->caData+32)
                                     .arg(ADD_FRIEND_AGREE)
                                     );
            break;
        }
        case ENUM_PDU_TYPE_ADD_FRIEND_REJECT:{
            QMessageBox::information(this, "添加好友",
                                     QString("%1 %2")
                                     .arg(pPDU->caData+32)
                                     .arg(ADD_FRIEND_REJECT)
                                     );
            break;
        }
        case ENUM_PDU_TYPE_FLUSH_FRI_RESPOND:{
            opeWidget::getInstance().getFriend()->showAllFirOnline(pPDU);
            break;
        }
        case ENUM_PDU_TYPE_DEL_FRI_RESPOND:{
            QMessageBox::information(this, "删除好友", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_DEL_FRI_REQUST:{
            QMessageBox::information(this, "提示",
                                     QString("%1解除了与你的好友关系")
                                     .arg(pPDU->caData)
                                     );
            break;
        }
        case ENUM_PDU_TYPE_PRIVATE_CHAT_REQUST:{
            qDebug() << "testing";
            privateChat::getInstance().showPrivateMsg(pPDU);
            break;
        }
        case ENUM_PDU_TYPE_GROUP_CHAT_REQUST:{
            opeWidget::getInstance().getFriend()->showGroupChatMsg(pPDU);
            break;
        }
        case ENUM_PDU_TYPE_CREATE_DIR_RESPOND:{
            QMessageBox::information(this, "创建文件夹", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_FLUSH_DIR_RESPOND:{
            opeWidget::getInstance().getBook()->showFlushDir(pPDU);
            break;
        }
        case ENUM_PDU_TYPE_DEL_DIR_RESPOND:{
            QMessageBox::information(this, "删除文件夹", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_RENAME_DIR_RESPOND:{
            QMessageBox::information(this, "重命名", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_ENTER_DIR_RESPOND:{
            if( strcmp(pPDU->caData, ENTER_DIR_OK)==0 ){
                m_strCurPath = opeWidget::getInstance().getBook()->getCurPath();
                qDebug() << "current path:" << m_strCurPath;
                opeWidget::getInstance().getBook()->showFlushDir(pPDU);
            }else{
                QMessageBox::information(this, "进入文件夹", pPDU->caData);
            }

            break;
        }
        case ENUM_PDU_TYPE_UPLOAD_FILE_RESPOND:{
            QMessageBox::information(this, "上传文件", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_DEL_FILE_RESPOND:{
            QMessageBox::information(this, "删除文件夹", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_DOWNLOAD_FILE_RESPOND:{
            if( strcmp(pPDU->caData, DOWNLOAD_FILE_IS_DIR)==0 ){
                QMessageBox::information(this, "下载文件", pPDU->caData);
            }else if( strcmp(pPDU->caData, SYSTEAM_ERROR)==0 ){
                QMessageBox::information(this, "下载文件", pPDU->caData);
            }else{
                char chFileName[32] = {0};
                qint64 totalSize;
                sscanf(pPDU->caData, "%s %lld", chFileName, &totalSize);
                //qDebug() << chFileName << totalSize;

                opeWidget &opeW = opeWidget::getInstance();
                opeW.getBook()->setDownloadStatus(true);
                opeW.getBook()->setTotalSize(totalSize);
                opeW.getBook()->setReceivedSize(0);
                QFile &file = opeW.getBook()->getFile();

                if(!file.open(QIODevice::WriteOnly)){
                    QMessageBox::warning(this, "下载文件", "下载文件失败：打开文件失败");
                }
            }
            break;
        }
        case ENUM_PDU_TYPE_SHARE_FILE_RESPOND:{
            if( strcmp(pPDU->caData, SHARE_FILE_OK)==0 )
                QMessageBox::information(this, "分享文件", pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_SHARE_FILE_NOTICE_REQUST:{
            char chSendName[32] = {'\0'};
            char *pPath = new char[pPDU->uintMsgLen];
            strncpy(chSendName, pPDU->caData, 32);
            memcpy(pPath, pPDU->Msg, pPDU->uintMsgLen);

            char *pos = strrchr(pPath, '/');
            QString fileName = ++pos;
            QString strNotice = QString("%1 share->%2 to you,do you accept?")
                                     .arg(chSendName).arg(fileName);

            int ret = QMessageBox::question(this, "分享", strNotice,
                                  QMessageBox::Yes, QMessageBox::No);
            if(ret == QMessageBox::Yes){
               pPDU->uintPDUType = ENUM_PDU_TYPE_SHARE_FILE_NOTICE_RESPOND;
               m_tcpSocket.write((char*)pPDU, pPDU->uintPDULen);
            }
            break;
        }
        case ENUM_PDU_TYPE_MOVE_FILE_RESPOND:{
            if( strcmp(pPDU->caData, MOVE_FILE_OK)==0 ){
                QMessageBox::information(this, "移动", pPDU->caData);
            }else{
                QMessageBox::critical(this, "移动", pPDU->caData);
            }
            break;
        }
        default:
            break;
    }
    free(pPDU);
    pPDU = NULL;
}

#if 0
void tcpClient::on_sendButton_clicked()
{

    QString msg = ui->lineEdit->text();
    struct PDU *pPDU = mkPDU(msg.size()+1);

    pPDU->uint8PDUType = 1;
    //memcpy(pPDU->Msg, &msg, msg.size());  //虚函数列表的指针
    memcpy(pPDU->Msg, msg.toStdString().c_str(), msg.size());
    qDebug() << (char*)pPDU->Msg;

    m_tcpSocket.write((char*)pPDU, pPDU->uintPDULen);
    //qDebug() << ret;

    free(pPDU);
    pPDU = NULL;
}
#endif

void tcpClient::on_login_pb_clicked()
{
    QString name = ui->name_le->text();
    QString pswd = ui->pswd_le->text();

    if(name.isEmpty() || pswd.isEmpty()){
        QMessageBox::critical(this, "登录", "登录失败:用户名或密码为空");
        return;
    }

    m_strName = name;

    struct PDU *pPDU = mkPDU();
    pPDU->uintPDUType = ENUM_PDU_TYPE_LOGIN_REQUST;
    strncpy(pPDU->caData, name.toStdString().c_str(), 32);
    strncpy(pPDU->caData+32, pswd.toStdString().c_str(), 32);

    m_tcpSocket.write((char*)pPDU, pPDU->uintPDULen);
    free(pPDU);
    pPDU = NULL;
}

void tcpClient::on_regist_pb_clicked()
{
    QString name = ui->name_le->text();
    QString pswd = ui->pswd_le->text();

    if(name.isEmpty() || pswd.isEmpty()){
        QMessageBox::critical(this, "注册", "注册失败:用户名或密码为空");
        return;
    }

    struct PDU *pPDU = mkPDU();
    pPDU->uintPDUType = ENUM_PDU_TYPE_RGST_REQUST;
    strncpy(pPDU->caData, name.toStdString().c_str(), 32);
    strncpy(pPDU->caData+32, pswd.toStdString().c_str(), 32);

    m_tcpSocket.write((char*)pPDU, pPDU->uintPDULen);
    free(pPDU);
    pPDU = NULL;
}

void tcpClient::on_cancel_pb_clicked()
{

}
