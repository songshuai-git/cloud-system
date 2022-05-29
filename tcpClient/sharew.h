#ifndef SHAREW_H
#define SHAREW_H

#include <QWidget>
#include<QPushButton>
#include<QButtonGroup>
#include<QScrollArea>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QListWidget>

class shareW : public QWidget
{
    Q_OBJECT
public:
    static shareW &getInstance();
    void test();
    void updateFirList(QListWidget* pFirList);
signals:

public slots:
    void on_selectAll(bool checked);
    void on_cancelAll(bool checked);
    void on_confirm_pb(bool checked);
    void on_pCancel_pb(bool checked);
private:
    explicit shareW(QWidget *parent = nullptr);

    QPushButton *m_pSelectAll_pb;
    QPushButton *m_pCacelAll_pb;

    QScrollArea *m_pScrollArea;
    QWidget *m_pFriend_w;
    QButtonGroup *m_pBtnGroup;

    QPushButton *m_pConfirm_pb;
    QPushButton *m_pCancel_pb;

    QVBoxLayout *m_pFriList_layout;
};

#endif // SHAREW_H
