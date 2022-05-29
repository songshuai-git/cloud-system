#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include<QTextEdit>
#include<QLineEdit>
#include<QPushButton>
#include<QLayout>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QListWidget>

#include"online.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    QString m_name;

    explicit Friend(QWidget *parent = nullptr);

    void showAllUserOnline(struct PDU *pPDU);
    void showAllFirOnline(struct PDU *pPDU);
    void showGroupChatMsg(struct PDU *pPDU);

    inline QListWidget *getFirList()const{ return m_friList_lw; }
signals:

public slots:
    void on_click_showOnline_pb(bool);
    void on_click_searchUsr_pb(bool);
    void on_click_flushFri_pb(bool);
    void on_click_delFri_pb(bool);
    void on_click_privateChat_pb(bool);
    void on_click_sendMsg_pb(bool);
private:
//    QHBoxLayout *m_hLayout;
//    QVBoxLayout *m_vLayout;
//    QLayout *m_layout;

    QTextEdit *m_showMsg_te;
    QLineEdit *m_inputMsg_le;

    QListWidget *m_friList_lw;

    QPushButton *m_sendMsg_pb;
    QPushButton *m_flushFri_pb;
    QPushButton *m_delFri_pb;
    QPushButton *m_showOnline_pb;
    QPushButton *m_searchUsr_pb;
    QPushButton *m_privateChat_pb;

    online *m_usrOnline;
};

#endif // FRIEND_H
