#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include<QListWidget>
#include<QPushButton>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include"protocol.h"
#include<QTimer>
#include<QFile>

class book : public QWidget
{
    Q_OBJECT
public:
    explicit book(QWidget *parent = nullptr);
    void showFlushDir(struct PDU *pPDU);
    inline QString getCurPath()const{ return m_strCurPath; }
    inline bool getDownloadStatus()const{ return m_downloadStatus; }
    inline void setDownloadStatus(bool status){ m_downloadStatus = status; }
    inline QString getSavePath()const{ return m_strSavePath; }
    inline void setTotalSize(qint64 size){ m_totalSize = size; }
    inline qint64 getTotalSize()const{ return m_totalSize; }
    inline QFile &getFile(){ return m_file; }
    inline void setReceivedSize(qint64 size){ m_receivedSize = size; }
    inline qint64 *getReceivedSize(){ return &m_receivedSize; }
    inline QString getFileName()const{ return m_strFileName; }
signals:

public slots:
    void on_click_createDir_pb(bool checked);
    void on_click_flushDir_pb(bool checked);
    void on_click_delDir_pb(bool checked);
    void on_click_reNameDir_pb(bool checked);
    void on_enterDir(const QModelIndex &index);
    void on_click_goBack_pb(bool checked);
    void on_click_uploadFile_pb(bool checked);
    void on_uploadFileData();
    void on_click_delFile_pb(bool checked);
    void on_click_downloadFile_pb(bool checked);
    void on_click_shareFile_pb(bool checked);
    void on_click_moveFile_pb(bool checked);
    void on_click_selectDeskDir_pb(bool checked);
private:
    QListWidget *m_book_lw;

    QPushButton *m_goBack_pb;
    QPushButton *m_createDir_pb;
    QPushButton *m_delDir_pb;
    QPushButton *m_reNameDir_pb;
    QPushButton *m_flushDir_pb;

    QPushButton *m_uploadFile_pb;
    QPushButton *m_downloadFile_pb;
    QPushButton *m_shareFile_pb;
    QPushButton *m_delFile_pb;
    QPushButton *m_moveFile_pb;
    QPushButton *m_selectDeskDir_pb;

    QString m_strCurPath;
    QString m_strOpenPath;
    QString m_strSavePath;
    QTimer *m_timer;
    bool m_downloadStatus;
    qint64 m_totalSize;
    qint64 m_receivedSize;
    QFile m_file;

    QString m_strFileName;

    QString m_strMoveFileName;
    QString m_strSrcPath;
};

#endif // BOOK_H
