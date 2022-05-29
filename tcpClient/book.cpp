#include "book.h"
#include<QHBoxLayout>
#include<QInputDialog>
#include<tcpclient.h>
#include"protocol.h"
#include <QListWidgetItem>
#include<QMessageBox>
#include <QFileDialog>
#include"sharew.h"
#include"opewidget.h"

book::book(QWidget *parent) : QWidget(parent)
{
    m_downloadStatus = false;
    m_timer = new QTimer(this);

    m_book_lw = new QListWidget;

    m_goBack_pb = new QPushButton("返回");
    m_createDir_pb = new QPushButton("创建文件夹");
    m_delDir_pb = new QPushButton("删除文件夹");
    m_reNameDir_pb = new QPushButton("重命名文件夹");
    m_flushDir_pb = new QPushButton("刷新文件夹");
    QVBoxLayout *middleVLayout = new QVBoxLayout;
    middleVLayout->addWidget(m_goBack_pb);
    middleVLayout->addWidget(m_createDir_pb);
    middleVLayout->addWidget(m_delDir_pb);
    middleVLayout->addWidget(m_reNameDir_pb);
    middleVLayout->addWidget(m_flushDir_pb);

    m_uploadFile_pb = new QPushButton("上传文件");
    m_downloadFile_pb = new QPushButton("下载文件");
    m_shareFile_pb = new QPushButton("分享文件");
    m_delFile_pb = new QPushButton("删除文件");
    m_moveFile_pb = new QPushButton("移动文件");
    m_selectDeskDir_pb = new QPushButton("目标目录");
    m_selectDeskDir_pb->setEnabled(false);
    QVBoxLayout *rightHLayout = new QVBoxLayout;
    rightHLayout->addWidget(m_uploadFile_pb);
    rightHLayout->addWidget(m_downloadFile_pb);
    rightHLayout->addWidget(m_shareFile_pb);
    rightHLayout->addWidget(m_delFile_pb);
    rightHLayout->addWidget(m_moveFile_pb);
    rightHLayout->addWidget(m_selectDeskDir_pb);

    QHBoxLayout *mainHLayout = new QHBoxLayout;
    mainHLayout->addWidget(m_book_lw);
    mainHLayout->addLayout(middleVLayout);
    mainHLayout->addLayout(rightHLayout);

    setLayout(mainHLayout);

    QObject::connect(m_createDir_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_createDir_pb(bool))
                     );
    QObject::connect(m_flushDir_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_flushDir_pb(bool))
                     );
    QObject::connect(m_delDir_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_delDir_pb(bool))
                     );
    QObject::connect(m_reNameDir_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_reNameDir_pb(bool))
                     );
    QObject::connect(m_book_lw, SIGNAL(doubleClicked(QModelIndex)),
                     this, SLOT(on_enterDir(QModelIndex))
                     );
    QObject::connect(m_goBack_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_goBack_pb(bool))
                     );
    QObject::connect(m_uploadFile_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_uploadFile_pb(bool))
                     );
    QObject::connect(m_timer, SIGNAL(timeout()),
                     this, SLOT(on_uploadFileData())
                     );
    QObject::connect(m_delFile_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_delFile_pb(bool))
                     );
    QObject::connect(m_downloadFile_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_downloadFile_pb(bool))
                     );
    QObject::connect(m_shareFile_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_shareFile_pb(bool))
                     );
    QObject::connect(m_moveFile_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_moveFile_pb(bool))
                     );
    QObject::connect(m_selectDeskDir_pb, SIGNAL(clicked(bool)),
                     this, SLOT(on_click_selectDeskDir_pb(bool))
                     );
}

void book::showFlushDir(struct PDU *pPDU)
{
    qDebug() << "cd:" << "showFlushDir()";
    m_book_lw->clear();

    int itemCount = pPDU->uintMsgLen/sizeof(struct FileInfo);
    for(int i=0; i<itemCount; i++){
        QListWidgetItem *item = new QListWidgetItem;

        struct FileInfo *pFileInfo = (struct FileInfo*)pPDU->Msg + i;

        item->setText(pFileInfo->caFileName);

        QIcon dirIcon(":/icon/dir.jpg");
        QIcon fileIcon(":/icon/file.jpg");
        if(pFileInfo->iFileType == 1){
            item->setIcon(fileIcon);
        }else if(pFileInfo->iFileType == 2){
            item->setIcon(dirIcon);
        }

        m_book_lw->addItem(item);
    }

    free(pPDU);
    pPDU = NULL;
}

void book::on_click_createDir_pb(bool checked)
{
    (void)checked;

    QString strDirName = QInputDialog::getText(this, "创建文件夹", "请输入文件夹名字:");
    QString strCurPath = tcpClient::getInstance().getCurPath();
    QString strName = tcpClient::getInstance().getName();

    uint uintMsgLen = strCurPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);
    pPDU->uintPDUType = ENUM_PDU_TYPE_CREATE_DIR_REQUST;
    strncpy(pPDU->caData, strName.toStdString().c_str(), 32);
    strncpy(pPDU->caData+32, strDirName.toStdString().c_str(), 32);
    memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData+32;
    free(pPDU);
    pPDU = NULL;
}

void book::on_click_flushDir_pb(bool checked)
{
    (void)checked;
    QString strCurPath = tcpClient::getInstance().getCurPath();
    QString strName = tcpClient::getInstance().getName();

    uint uintMsgLen = strCurPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);
    pPDU->uintPDUType = ENUM_PDU_TYPE_FLUSH_DIR_REQUST;
    strncpy(pPDU->caData, strName.toStdString().c_str(), 32);
    memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData+32;
    free(pPDU);
    pPDU = NULL;
}

void book::on_click_delDir_pb(bool checked)
{
    (void)checked;
    QString strCurPath = tcpClient::getInstance().getCurPath();

    QListWidgetItem *item = m_book_lw->currentItem();
    if(item == NULL) { qDebug() << "item" << item; return; };
    QString strDirName = item->text();

    uint uintMsgLen = strCurPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);
    pPDU->uintPDUType = ENUM_PDU_TYPE_DEL_DIR_REQUST;
    strncpy(pPDU->caData, strDirName.toStdString().c_str(), 32);
    memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData;
    free(pPDU);
    pPDU = NULL;
}

void book::on_click_reNameDir_pb(bool checked)
{
    (void)checked;
    QString strCurPath = tcpClient::getInstance().getCurPath();

    QListWidgetItem *item = m_book_lw->currentItem();
    if(item == NULL) { qDebug() << "item:" << item; return; };
    QString strOldName = item->text();

    QString strNewName = QInputDialog::getText(this, "重命名", "请输入新的文件名：");
    if(strNewName.isEmpty()) { qDebug() << "strNewName is empty"; return; }

    uint uintMsgLen = strCurPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);

    pPDU->uintPDUType = ENUM_PDU_TYPE_RENAME_DIR_REQUST;
    strncpy(pPDU->caData, strOldName.toStdString().c_str(), 32);
    strncpy(pPDU->caData+32, strNewName.toStdString().c_str(), 32);
    memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "rename:" << (char*)pPDU->Msg << pPDU->caData;
    free(pPDU);
    pPDU = NULL;
}

void book::on_enterDir(const QModelIndex &index)
{
   QString strCurPath = tcpClient::getInstance().getCurPath();
   QString strDirName = index.data().toString();

   m_strCurPath = strCurPath + '/' + strDirName;

   uint uintMsgLen = strCurPath.size() + 1;
   struct PDU *pPDU = mkPDU(uintMsgLen);
   pPDU->uintPDUType = ENUM_PDU_TYPE_ENTER_DIR_REQUST;
   strncpy(pPDU->caData, strDirName.toStdString().c_str(), 32);
   memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

   qint64 ret = tcpClient::getInstance().getTcpSocket()
           .write((char*)pPDU, pPDU->uintPDULen);

   qDebug() << "has sent:" << ret << "bytes";
   qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData;
   free(pPDU);
   pPDU = NULL;
}

void book::on_click_goBack_pb(bool checked)
{
    (void)checked;
    QString strCurPath = tcpClient::getInstance().getCurPath();
    QString strRootPath = "./" + tcpClient::getInstance().getName();

    if(strCurPath == strRootPath){
        QMessageBox::warning(this, "返回", "already at root dir");
        return;
    }

    int index = strCurPath.lastIndexOf('/');
    strCurPath.remove(index, strCurPath.size()-index);
    qDebug() << "go back:" << strCurPath;
    tcpClient::getInstance().setCurPath(strCurPath);

    on_click_flushDir_pb(true);
}

void book::on_click_uploadFile_pb(bool checked)
{
    (void)checked;
    QString strCurPath = tcpClient::getInstance().getCurPath();
    m_strOpenPath = QFileDialog::getOpenFileName();
    qDebug() << "upload path:" << m_strOpenPath;

    if(m_strOpenPath.isEmpty()){
        QMessageBox::warning(this, "上传", "上传的文件为空");
        return;
    }
    
    int index = m_strOpenPath.lastIndexOf('/');
    QString strFileName = m_strOpenPath.right(m_strOpenPath.size()-index-1);
    qDebug() << "upload file:" << strFileName;

    QFile file(m_strOpenPath);
    qint64 totalSize = file.size();

    uint uintMsgLen = strCurPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);
    pPDU->uintPDUType = ENUM_PDU_TYPE_UPLOAD_FILE_REQUST;
    sprintf(pPDU->caData, "%s %lld", strFileName.toStdString().c_str(), totalSize);
    memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData;
    free(pPDU);
    pPDU = NULL;

    m_timer->start(1000);
}

void book::on_uploadFileData()  //!bug:文件大小若超过1G
{
    m_timer->stop();

    QFile file(m_strOpenPath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this, "上传文件", "上传失败");
    }

    char buffer[4096] = {0};  //4*2^10=4KB
    while(true){
        qint64 readBytes = file.read(buffer, 4096);

        if(readBytes == 0) break;
        else if(readBytes < 0){ qDebug() << "读取文件过程中失败"; break; }
        qint64 ret = tcpClient::getInstance().getTcpSocket()
                .write(buffer, readBytes);

        qDebug() << "has sent:" << ret << "bytes";
    }
    file.close();
}

void book::on_click_delFile_pb(bool checked)
{
    (void)checked;
    QString strCurPath = tcpClient::getInstance().getCurPath();

    QListWidgetItem *item = m_book_lw->currentItem();
    if(item == NULL) { qDebug() << "item" << item; return; };
    QString strDirName = item->text();

    uint uintMsgLen = strCurPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);
    pPDU->uintPDUType = ENUM_PDU_TYPE_DEL_FILE_REQUST;
    strncpy(pPDU->caData, strDirName.toStdString().c_str(), 32);
    memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData;
    free(pPDU);
    pPDU = NULL;
}

void book::on_click_downloadFile_pb(bool checked)
{
    (void)checked;
    QString strCurPath = tcpClient::getInstance().getCurPath();
    m_strSavePath = QFileDialog::getSaveFileName();
    qDebug() << "download path:" << m_strSavePath;
    m_file.setFileName(m_strSavePath);

    QListWidgetItem *item = m_book_lw->currentItem();
    if(item == NULL){
        QMessageBox::information(this, "下载文件", "请选择要下载的文件");
        return;
    };
    QString strFileName = item->text();
    qDebug() << "download file:" << strFileName;

    uint uintMsgLen = strCurPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);
    pPDU->uintPDUType = ENUM_PDU_TYPE_DOWNLOAD_FILE_REQUST;
    strncpy(pPDU->caData, strFileName.toStdString().c_str(), 32);
    memcpy(pPDU->Msg, strCurPath.toStdString().c_str(), pPDU->uintMsgLen);

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData;
    free(pPDU);
    pPDU = NULL;

    
}

void book::on_click_shareFile_pb(bool checked)
{
    (void)checked;

    QListWidgetItem *item = m_book_lw->currentItem();
    if(item == NULL){
        QMessageBox::warning(this, "提示", "请选择要分享的文件");
        return;
    }
    m_strFileName = item->text();

    QListWidget* pFriList = opeWidget::getInstance().getFriend()->getFirList();
    shareW::getInstance().updateFirList(pFriList);

    if(shareW::getInstance().isHidden()) shareW::getInstance().show();
}

void book::on_click_moveFile_pb(bool checked)
{
    (void)checked;
    QListWidgetItem *item = m_book_lw->currentItem();
    if(item == NULL){
        QMessageBox::warning(this, "提示", "请选择要移动的文件");
        return;
    }
    m_strMoveFileName = item->text();
    m_strSrcPath = tcpClient::getInstance().getCurPath();
    m_selectDeskDir_pb->setEnabled(true);
}

void book::on_click_selectDeskDir_pb(bool checked)
{
    (void)checked;
    QListWidgetItem *item = m_book_lw->currentItem();
    if(item == NULL){
        QMessageBox::warning(this, "提示", "请选择目标目录");
        return;
    }
    QString strDeskDir = item->text();

    QString strCurPath = tcpClient::getInstance().getCurPath();
    QString strDeskPath = strCurPath + '/' + strDeskDir;
    QString strSrcPath = m_strSrcPath + '/' + m_strMoveFileName;
    //qDebug() << strDeskPath << endl << strSrcPath;

    m_selectDeskDir_pb->setEnabled(false);

    uint uintMsgLen = strSrcPath.size() + strDeskPath.size() + 2;
    struct PDU *pPDU = mkPDU(uintMsgLen);

    pPDU->uintPDUType = ENUM_PDU_TYPE_MOVE_FILE_REQUST;
    sprintf(pPDU->caData, "%s %d %d",
            m_strMoveFileName.toStdString().c_str(),
            strSrcPath.size(),
            strDeskPath.size()
            );
    memcpy((char*)pPDU->Msg,
           strSrcPath.toStdString().c_str(),
           strSrcPath.size()
           );
    memcpy((char*)pPDU->Msg + strSrcPath.size() + 1,
           strDeskPath.toStdString().c_str(),
           strDeskPath.size()
           );

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);

    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg << pPDU->caData;
    free(pPDU);
    pPDU = NULL;
}
