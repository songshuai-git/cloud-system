#include "mytcpsocket.h"
#include"protocol.h"
#include"handledb.h"
#include<QStringList>
#include"mytcpserver.h"
#include<QDir>

myTcpSocket::myTcpSocket(QObject *parent)
    :QTcpSocket(parent)
{
    m_timer = new QTimer(this);
    m_uploadStatus = false;

    QObject::connect(this, SIGNAL(readyRead()), \
                     this, SLOT(on_revMsg()));
    QObject::connect(this, SIGNAL( disconnected() ),
                     this, SLOT( on_disconnect() ) );
    QObject::connect(m_timer, SIGNAL( timeout() ),
                     this, SLOT( on_uploadFileData() ) );

}

QString myTcpSocket::getName() const
{
    return m_strName;
}

void myTcpSocket::dirCopy(QString srcDir, QString deskDir)
{
    QDir dir;
    dir.mkdir(deskDir);
    dir.setPath(srcDir);

    QFileInfoList fileInfoList = dir.entryInfoList();
    QString srcTemp = srcDir;
    QString deskTemp = deskDir;
    for(int i=0; i<fileInfoList.size(); i++){
        if(fileInfoList[i].isFile()){
            srcTemp = srcDir + '/' + fileInfoList[i].fileName();
            deskTemp = deskDir + '/' + fileInfoList[i].fileName();
            QFile::copy(srcTemp, deskTemp);
        }else if(fileInfoList[i].isDir()){
            if( "." == fileInfoList[i].fileName() ||
                ".." == fileInfoList[i].fileName() )
                continue;
            srcTemp = srcDir + '/' + fileInfoList[i].fileName();
            deskTemp = deskDir + '/' + fileInfoList[i].fileName();
            dirCopy(srcTemp, deskTemp);
        }
    }
}

void myTcpSocket::on_revMsg()
{
    if(m_uploadStatus==true){
        QByteArray byteArray =  this->readAll();
        qDebug() << byteArray.size();

        //m_file.write(byteArray);
        m_file.write(byteArray.data(), byteArray.size());
        m_receivedSize += byteArray.size();

        if(m_receivedSize == m_totalSize){
            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(resPDU->caData, UPLOAD_FILE_OK);
            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            m_uploadStatus = false;
            m_file.close();
        }else if(m_receivedSize > m_totalSize){
            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(resPDU->caData, UPLOAD_FILE_FAIL);
            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            m_uploadStatus = false;
            m_file.close();
        }
        return;
    }
    //qint64 buffer = bytesAvailable();
    //qDebug() << buffer;

    //struct PDU *pPDU = mkPDU()
    uint uintPDULen = 0;    //！此处必须初始化 不然这块空间有随机值 后面read后不会完全覆盖
    this->read((char*)&uintPDULen, sizeof(int));  //已经读了四个字节了

    uint uintMsgLen = uintPDULen-sizeof(struct PDU);
    struct PDU *pPDU = mkPDU(uintMsgLen);

    this->read((char*)pPDU+sizeof(uint), uintPDULen-sizeof(uint));
    //qDebug() << pPDU->uint8PDUType << (char*)pPDU->Msg;

    switch(pPDU->uintPDUType){
        case ENUM_PDU_TYPE_RGST_REQUST:{
            char name[32] = {0};
            char pswd[32] = {0};
            strncpy(name, pPDU->caData, 32);
            strncpy(pswd, pPDU->caData+32, 32);
            bool ret = handleDB::getInstance().handleRgst(name, pswd);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_RGST_RESPOND;
            if(ret){
                strcpy(resPDU->caData, RGST_SUEESS);

                QDir dir("./");
                dir.mkdir(name);
                qDebug() << "mkdir:" << name;
            }else{
                strcpy(resPDU->caData, RGST_FAIL);
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            break;
        }
        case ENUM_PDU_TYPE_LOGIN_REQUST:{
            char name[32] = {0};
            char pswd[32] = {0};
            strncpy(name, pPDU->caData, 32);
            strncpy(pswd, pPDU->caData+32, 32);
            bool ret = handleDB::getInstance().handleLogin(name, pswd);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_LOGIN_RESPOND;
            if(ret){
                strcpy(resPDU->caData, LOGIN_SUCCESS);

                m_strName = name;
            }else{
                strcpy(resPDU->caData, LOGIN_FAIL);
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            break;
        }
        case ENUM_PDU_TYPE_ALL_ONLINE_REQUST:{
            QStringList friList = handleDB::getInstance().handleAllOnline();

            uint uintMsgLen = friList.size()*32;
            struct PDU *resPDU = mkPDU(uintMsgLen);

            resPDU->uintPDUType = ENUM_PDU_TYPE_ALL_ONLINE_RESPOND;
            for(int i=0; i<friList.size(); i++){
                memcpy((char*)resPDU->Msg+i*32,
                       friList.at(i).toStdString().c_str(),
                       32);
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            break;
        }
        case ENUM_PDU_TYPE_SEARCH_USR_REQUST:{
            char name[32] = {0};
            strcpy(name, pPDU->caData);
            int ret = handleDB::getInstance().handleSearchUsr(name);

            struct PDU *resPDU = mkPDU(uintMsgLen);
            resPDU->uintPDUType = ENUM_PDU_TYPE_SEARCH_USR_RESPOND;

            if(ret == -2){
                qDebug() << "invalid parameter";
                break;
            }else if(ret == 0){
                strcpy(resPDU->caData, SEARCH_USR_OFFLINE);
            }else if(ret == -1){
                strcpy(resPDU->caData, SEARCH_USR_NO);
            }else if(ret == 1){
                strcpy(resPDU->caData, SEARCH_USR_ONLINE);
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            break;
        }
        case ENUM_PDU_TYPE_ADD_FRIEND_REQUST:{
            char name[32] = {0};
            char perName[32] = {0};
            strncpy(name, pPDU->caData, 32);
            strncpy(perName, pPDU->caData+32, 32);
            int  ret = handleDB::getInstance().handleAddFri(name, perName);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_ADD_FRIEND_RESPOND;

            if(ret == -2){
                strcpy(resPDU->caData, SYSTEAM_ERROR_FORMAL_PARAM);
            }else if(ret == ENUM_PERNAME_ALREADY_FRI){
                strcpy(resPDU->caData, PERNAME_ALREADY_FRI);
            }else if(ret == ENUM_PERNAME_NOT_EXIST){
                strcpy(resPDU->caData, PERNAME_NOT_EXIST);
            }else if(ret == ENUM_PERNAME_NOT_ONLINE){
                strcpy(resPDU->caData, PERNAME_NOT_ONLINE);
            }else if(ret == ENUM_PERNAME_IS_ONLINE){
                myTcpServer::get_instance().forwardToPerson(pPDU, pPDU->caData+32);
                break;
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            break;
        }
        case ENUM_PDU_TYPE_ADD_FRIEND_AGREE:{
            char name[32] = {0};
            char perName[32] = {0};
            strncpy(name, pPDU->caData, 32);
            strncpy(perName, pPDU->caData+32, 32);
            bool ret = handleDB::getInstance().handleAddFriAgree(name, perName);

            if(!ret) { qDebug() << SYSTEAM_ERROR_FORMAL_PARAM; break;}
//            if(ret){
//                strcpy(pPDU->caData, ADD_FRIEND_AGREE);
//            }else{
//                strcpy(pPDU->caData, SYSTEAM_ERROR_FORMAL_PARAM);
//            }

            myTcpServer::get_instance().forwardToPerson(pPDU, pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_ADD_FRIEND_REJECT:{
            myTcpServer::get_instance().forwardToPerson(pPDU, pPDU->caData);
            break;
        }
        case ENUM_PDU_TYPE_FLUSH_FRI_REQUST:{
            char name[32] = {0};
            strncpy(name, pPDU->caData, 32);
            QStringList friList = handleDB::getInstance().handleFlushFri(name);

//            for(int i=0; i<friList.size(); i++){
//                qDebug() << friList.at(i);
//            }
            uint uintMsgLen = friList.size()*32;
            struct PDU *resPDU = mkPDU(uintMsgLen);

            resPDU->uintPDUType = ENUM_PDU_TYPE_FLUSH_FRI_RESPOND;
            for(int i=0; i<friList.size(); i++){
                memcpy((char*)resPDU->Msg+i*32,
                       friList.at(i).toStdString().c_str(),
                       32);
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_DEL_FRI_REQUST:{
            char name[32] = {0};
            char perName[32] = {0};
            strncpy(name, pPDU->caData, 32);
            strncpy(perName, pPDU->caData+32, 32);
            bool ret = handleDB::getInstance().handleDelFri(name, perName);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_DEL_FRI_RESPOND;
            if(ret){
                myTcpServer::get_instance().forwardToPerson(pPDU, pPDU->caData+32);
                strcpy(resPDU->caData, DEL_FRIEND_SUCCESS);
            }else{
                strcpy(resPDU->caData, DEL_FRIEND_FAIL);
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            break;
        }
        case ENUM_PDU_TYPE_PRIVATE_CHAT_REQUST:{
            myTcpServer::get_instance().forwardToPerson(pPDU, pPDU->caData+32);
            break;
        }
        case ENUM_PDU_TYPE_GROUP_CHAT_REQUST:{
            char name[32] = {0};
            strncpy(name, pPDU->caData, 32);
            QStringList friList = handleDB::getInstance().handleFlushFri(name);

//            for(int i=0; i<friList.size(); i++){
//                qDebug() << friList.at(i);
//            }
            for(int i=0; i<friList.size(); i++){
                myTcpServer::get_instance().forwardToPerson(pPDU, friList.at(i));
            }
            free(pPDU);
            pPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_CREATE_DIR_REQUST:{
            QDir dir;
            QString strCurPath = QString((char*)pPDU->Msg);
            QString strNewDirName = QString(pPDU->caData+32);
            QString strNewDir = strCurPath + '/' + strNewDirName;

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_CREATE_DIR_RESPOND;
            if( !dir.exists(strCurPath) ){
                qDebug() << strCurPath
                         << "current path not exist";
                strcpy(resPDU->caData, CREATE_DIR_PATH_NOT_EXIST);
            }else if( dir.exists(strNewDir) ){
                qDebug() << strNewDir
                         << "dir name existed";
                strcpy(resPDU->caData, CREATE_DIR_DIRNAME_EXIST);
            }else{
                qDebug() << "create dir:" << strNewDir;
                dir.mkdir(strNewDir);
                strcpy(resPDU->caData, CREATE_DIR_OK);
            }

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_FLUSH_DIR_REQUST:{
            QString strCurPath = QString((char*)pPDU->Msg);
            QDir dir(strCurPath);

            //struct FileInfo
            QFileInfoList fileInfoList = dir.entryInfoList();
            
            uint uintMsgLen = fileInfoList.size() * sizeof(struct FileInfo);
            struct PDU *resPDU = mkPDU(uintMsgLen);
            
            resPDU->uintPDUType = ENUM_PDU_TYPE_FLUSH_DIR_RESPOND;
            for(int i=0; i<fileInfoList.size(); i++){
                struct FileInfo *pFileInfo = (struct FileInfo*)resPDU->Msg + i;
                strncpy(pFileInfo->caFileName,
                        fileInfoList[i].fileName().toStdString().c_str(),
                        32);
                //qDebug() << pFileInfo->caFileName;
                if(fileInfoList[i].isFile()){
                    pFileInfo->iFileType = 1;
                }else if(fileInfoList[i].isDir()){
                    pFileInfo->iFileType = 2;
                };
            }

            int ret = this->write((char*)resPDU, resPDU->uintPDULen);
            qDebug() << "has sent:" << ret << "bytes";
            qDebug() << "content:" << ((struct FileInfo*)resPDU->Msg + 2)->caFileName;
            free(resPDU);
            resPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_DEL_DIR_REQUST:{
            QString strCurPath = QString((char*)pPDU->Msg);
            QString strDirName = QString(pPDU->caData);
            QString strPath = strCurPath + "/" + strDirName;
            qDebug() << "delete:" << strPath;

            QDir dir;
            dir.setPath(strPath);

            QFileInfo *fileInfo = new QFileInfo(strPath);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_DEL_DIR_RESPOND;
            if(fileInfo->isFile()){
                strcpy(resPDU->caData, DEL_DIR_IS_FILE);
            }else if(fileInfo->isDir()){
                bool ret = dir.removeRecursively();
                if(ret) strcpy(resPDU->caData, DEL_DIR_OK);
                else strcpy(resPDU->caData, DEL_DIR_NOT_EXIST);
            }else{
                 strcpy(resPDU->caData, SYSTEAM_ERROR);
            }

            int ret = this->write((char*)resPDU, resPDU->uintPDULen);
            qDebug() << "has sent:" << ret << "bytes";
            qDebug() << "content:" << resPDU->uintPDUType;
            free(resPDU);
            resPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_RENAME_DIR_REQUST:{
            QString strCurPath = QString((char*)pPDU->Msg);
            QString strOldName = QString(pPDU->caData);
            QString strNewName = QString(pPDU->caData+32);

            QString strOldPath = strCurPath + "/" + strOldName;
            QString strNewPath = strCurPath + "/" + strNewName;
            qDebug() << "rename:" << strOldPath << strNewPath;

            QDir dir;
            bool ret = dir.rename(strOldPath, strNewPath);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_RENAME_DIR_RESPOND;
            if(ret==true){ strcpy(resPDU->caData, RENAME_DIR_OK); }
            else{ strcpy(resPDU->caData, RENAME_DIR_FAIL); }

            int bytes = this->write((char*)resPDU, resPDU->uintPDULen);
            qDebug() << "has sent:" << bytes << "bytes";
            qDebug() << "content:" << resPDU->uintPDUType;
            free(resPDU);
            resPDU = NULL;

            break;
        }
        case ENUM_PDU_TYPE_ENTER_DIR_REQUST:{
            QString strCurPath = QString((char*)pPDU->Msg);
            QString strDirName = QString(pPDU->caData);
            QString strPath = strCurPath + "/" + strDirName;
            qDebug() << "enter:" << strPath;

            QFileInfo *fileInfo = new QFileInfo(strPath);

            struct PDU *resPDU = NULL;
            if(fileInfo->isFile()){
                resPDU = mkPDU();
                resPDU->uintPDUType = ENUM_PDU_TYPE_ENTER_DIR_RESPOND;
                strcpy(resPDU->caData, ENTER_DIR_IS_FILE);
            }else if(fileInfo->isDir()){
                QDir dir(strPath);

                //struct FileInfo
                QFileInfoList fileInfoList = dir.entryInfoList();

                uint uintMsgLen = fileInfoList.size() * sizeof(struct FileInfo);
                resPDU = mkPDU(uintMsgLen);

                resPDU->uintPDUType = ENUM_PDU_TYPE_ENTER_DIR_RESPOND;
                for(int i=0; i<fileInfoList.size(); i++){
                    struct FileInfo *pFileInfo = (struct FileInfo*)resPDU->Msg + i;
                    strncpy(pFileInfo->caFileName,
                            fileInfoList[i].fileName().toStdString().c_str(),
                            32);
                    //qDebug() << pFileInfo->caFileName;
                    if(fileInfoList[i].isFile()){
                        pFileInfo->iFileType = 1;
                    }else if(fileInfoList[i].isDir()){
                        pFileInfo->iFileType = 2;
                    };
                }
                strcpy(resPDU->caData, ENTER_DIR_OK);
            }else{
                resPDU = mkPDU();
                resPDU->uintPDUType = ENUM_PDU_TYPE_ENTER_DIR_RESPOND;
                strcpy(resPDU->caData, SYSTEAM_ERROR);
            }

            int ret = this->write((char*)resPDU, resPDU->uintPDULen);
            qDebug() << "has sent:" << ret << "bytes";
            qDebug() << "content:" << resPDU->uintPDUType;
            free(resPDU);
            resPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_UPLOAD_FILE_REQUST:{
            QString strCurPath = QString((char*)pPDU->Msg);
            char chFileName[32] = {0};
            m_totalSize = 0;
            m_receivedSize = 0;

            sscanf(pPDU->caData, "%s %lld", chFileName, &m_totalSize);
            QString strPath = strCurPath + '/' + chFileName;
            qDebug() << strPath << m_totalSize;

            m_file.setFileName(strPath);
            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_UPLOAD_FILE_RESPOND;
            if( !m_file.open(QIODevice::WriteOnly) ){
                struct PDU *resPDU = mkPDU();
                resPDU->uintPDUType = ENUM_PDU_TYPE_UPLOAD_FILE_RESPOND;
                strcpy(resPDU->caData, UPLOAD_FILE_FAIL);
                this->write((char*)resPDU, resPDU->uintPDULen);
                free(resPDU);
                resPDU = NULL;
                break;
            }

            m_uploadStatus = true;
            break;
        }
        case ENUM_PDU_TYPE_DEL_FILE_REQUST:{
            QString strCurPath = QString((char*)pPDU->Msg);
            QString strDirName = QString(pPDU->caData);
            QString strPath = strCurPath + "/" + strDirName;
            qDebug() << "delete:" << strPath;

            QDir dir;
            dir.setPath(strCurPath);

            QFileInfo *fileInfo = new QFileInfo(strPath);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_DEL_FILE_RESPOND;
            if(fileInfo->isFile()){
                bool ret = dir.remove(strDirName);
                if(ret) strcpy(resPDU->caData, DEL_FILE_OK);
                else strcpy(resPDU->caData, DEL_FILE_NOT_EXIST);
            }else if(fileInfo->isDir()){
                strcpy(resPDU->caData, DEL_FILE_IS_DIR);
            }else{
                 strcpy(resPDU->caData, SYSTEAM_ERROR);
            }

            int ret = this->write((char*)resPDU, resPDU->uintPDULen);
            qDebug() << "has sent:" << ret << "bytes";
            qDebug() << "content:" << resPDU->uintPDUType;
            free(resPDU);
            resPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_DOWNLOAD_FILE_REQUST:{
            QString strCurPath = QString((char*)pPDU->Msg);
            QString strFileName = QString(pPDU->caData);
            QString strPath = strCurPath + "/" + strFileName;
            m_strPath = strPath;
            qDebug() << "download:" << strPath;

            QFileInfo *fileInfo = new QFileInfo(strPath);

            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_DOWNLOAD_FILE_RESPOND;
            if(fileInfo->isFile()){
                QFile file(strPath);
                qint64 totalSize = file.size();

                sprintf(resPDU->caData,
                        "%s %lld",
                        strFileName.toStdString().c_str(),
                        totalSize
                        );
            }else if(fileInfo->isDir()){
                strcpy(resPDU->caData, DOWNLOAD_FILE_IS_DIR);
            }else{
                strcpy(resPDU->caData, SYSTEAM_ERROR);
            }

            int ret = this->write((char*)resPDU, resPDU->uintPDULen);
            qDebug() << "has sent:" << ret << "bytes";
            qDebug() << "content:" << resPDU->uintPDUType;
            free(resPDU);
            resPDU = NULL;

            m_timer->start(1000);
            break;
        }
        case ENUM_PDU_TYPE_SHARE_FILE_REQUST:{
            char chSendName[32] = {'\0'};
            int num = 0;
            sscanf(pPDU->caData, "%s %d", chSendName, &num);
            QString strPath = QString("%1").arg((char*)pPDU->Msg + 32*num);
            //qDebug() << strPath;

            uint uintMsgLen = 0;
            uintMsgLen = strPath.size() + 1;
            struct PDU *resPDU = mkPDU(uintMsgLen);

            resPDU->uintPDUType = ENUM_PDU_TYPE_SHARE_FILE_NOTICE_REQUST;
            memcpy(resPDU->caData, chSendName, 32);
            memcpy(resPDU->Msg, strPath.toStdString().c_str(), strPath.size());

            char chReceiveName[32] = {'\0'};
            for(int i=0; i<num; i++){
                strncpy(chReceiveName, (char*)pPDU->Msg + 32*i, 32);
                myTcpServer::get_instance().forwardToPerson(resPDU, chReceiveName);
            }
            free(resPDU);
            resPDU = NULL;

            resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_SHARE_FILE_RESPOND;
            strcpy(resPDU->caData, SHARE_FILE_OK);

            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;
            break;
        }
        case ENUM_PDU_TYPE_SHARE_FILE_NOTICE_RESPOND:{

            QString strSourcePath = QString("%1").arg((char*)pPDU->Msg);

            int index = strSourcePath.lastIndexOf('/');
            QString strFileName = strSourcePath.right(strSourcePath.size()-index-1);

            QFileInfo fileInfo(strSourcePath);
            QString strDesPath = QString("./%1/%2").arg(m_strName).arg(strFileName);
            if(fileInfo.isFile()){
                if( !QFile::copy(strSourcePath, strDesPath) )
                    qDebug() << "分享文件失败！！";
            }else if(fileInfo.isDir()){
                dirCopy(strSourcePath,strDesPath);
            }
            break;
        }
        case ENUM_PDU_TYPE_MOVE_FILE_REQUST:{
            char chMoveFileName[32] = {'\0'};
            int srcPathSize = 0;
            int deskPathSize = 0;
            sscanf(pPDU->caData,
                   "%s %d %d",
                   chMoveFileName,
                   &srcPathSize, &deskPathSize
                   );

            char *pSrcPath = new char[srcPathSize+1];
            char *pDeskPath = new char[deskPathSize+1+32];
            memset(pSrcPath, 0, srcPathSize+1);
            memset(pDeskPath, 0, deskPathSize+1+32);
            memcpy(pSrcPath, (char*)pPDU->Msg, srcPathSize);
            memcpy(pDeskPath, (char*)pPDU->Msg+srcPathSize+1, deskPathSize);
            qDebug() << pSrcPath << endl << pDeskPath;

            QFileInfo fileInfo(pDeskPath);
            struct PDU *resPDU = mkPDU();
            resPDU->uintPDUType = ENUM_PDU_TYPE_MOVE_FILE_RESPOND;
            if(fileInfo.isDir()){
                strcat(pDeskPath, "/");
                strcat(pDeskPath, chMoveFileName);
                bool ret = QFile::rename(pSrcPath, pDeskPath);
                if(ret){
                    strcpy(resPDU->caData, MOVE_FILE_OK);
                }else{
                    strcpy(resPDU->caData, SYSTEAM_ERROR);
                }
            }else if(fileInfo.isFile()){
                strcpy(resPDU->caData, MOVE_FILE_FAIL);
            }
            this->write((char*)resPDU, resPDU->uintPDULen);
            free(resPDU);
            resPDU = NULL;

            break;
        }
        default:
            break;
    }
    free(pPDU);
    pPDU = NULL;
}

void myTcpSocket::on_disconnect()
{
    qDebug() << "cd on_disconnect()";
    bool ret = handleDB::getInstance()
            .handleDisconnect(m_strName.toStdString().c_str());
    (void)ret;

    emit sigDisconnect(this);
}

void myTcpSocket::on_uploadFileData()
{
    m_timer->stop();

    m_file.setFileName(m_strPath);
    if( !m_file.open(QIODevice::ReadOnly) ){
        qDebug() << "uploadFileData():" << "打开文件失败";
        return;
    }

    char buffer[4096];
    while(true){
        qint64 readBytes = m_file.read(buffer, 4096);
        if(readBytes == 0) break;
        else if(readBytes < 0) { qDebug() << "读取文件过程中失败"; break; };
        qint64 ret = this->write(buffer, readBytes);
        qDebug() << "has sent:" << ret << "bytes";
    }
    m_file.close();
}
