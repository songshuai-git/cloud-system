#include "sharew.h"
#include<QCheckBox>
#include<QDebug>
#include"tcpclient.h"
#include"opewidget.h"

shareW &shareW::getInstance()
{
    static shareW instance;
    return instance;
}

void shareW::updateFirList(QListWidget *pFirList)
{
    if(NULL == pFirList){ qDebug() << "updateFirList():" << "invalid param"; }

    QList<QAbstractButton*> list = m_pBtnGroup->buttons();
    for(int i=0; i<list.size(); i++){
        m_pBtnGroup->removeButton(list[i]);
        m_pFriList_layout->removeWidget(list[i]);
        //list.removeOne(list[i]);
        delete list[i]; //!必须删除控件 不然控件还在而且已经被show出来了，会有残留
    }

    QCheckBox *pCheckBox = NULL;
    //qDebug() << "m_pFriList_layout count:" << m_pFriList_layout->count();
    //qDebug() << "pFirList->count():" << pFirList->count();
    for(int i=0; i<pFirList->count(); i++){
        QListWidgetItem *item = pFirList->item(i);
        //qDebug() << item->text();
        pCheckBox = new QCheckBox(item->text());
        m_pFriList_layout->addWidget(pCheckBox);
        m_pBtnGroup->addButton(pCheckBox);
    }

    m_pScrollArea->setWidget(m_pFriend_w);
    //m_pScrollArea->ensureWidgetVisible(m_pFriend_w);
    m_pScrollArea->setWidgetResizable(true);//哈哈，找到你了
}

void shareW::on_selectAll(bool checked)
{
    (void)checked;

    QList<QAbstractButton*> pSelectList = m_pBtnGroup->buttons();
    for(int i=0; i<pSelectList.size(); i++){
        if(!pSelectList[i]->isChecked())
            pSelectList[i]->setChecked(true);
    }
}

void shareW::on_cancelAll(bool checked)
{
    (void)checked;

    QList<QAbstractButton*> pSelectList = m_pBtnGroup->buttons();
    for(int i=0; i<pSelectList.size(); i++){
        if(pSelectList[i]->isChecked())
            pSelectList[i]->setChecked(false);
    }
}

void shareW::on_confirm_pb(bool checked)
{
    (void)checked;

    QString strSendName = tcpClient::getInstance().getName();
    QString strCurPath = tcpClient::getInstance().getCurPath();
    QString strFileName = opeWidget::getInstance().getBook()->getFileName();
    QString strPath = QString("%1/%2").arg(strCurPath).arg(strFileName);

    QList<QAbstractButton*> pBtnList = m_pBtnGroup->buttons();
    int num = 0;
    for(int i=0; i<pBtnList.size(); i++){
        if(pBtnList[i]->isChecked()){
            num++;
        }
    }

    uint uintMsgLen = num*32 + strPath.size() + 1;
    struct PDU *pPDU = mkPDU(uintMsgLen);

    pPDU->uintPDUType = ENUM_PDU_TYPE_SHARE_FILE_REQUST;
    sprintf(pPDU->caData, "%s %d", strSendName.toStdString().c_str(), num);
    int j = 0;
    for(int i=0; i<pBtnList.size(); i++){
        if(pBtnList[i]->isChecked()){
            memcpy((char*)pPDU->Msg+32*j,
                   (void*)pBtnList[i]->text().toStdString().c_str(),
                   32);
            j++;
        }
    }
    memcpy((char*)pPDU->Msg+32*num, strPath.toStdString().c_str(), strPath.size());

    qint64 ret = tcpClient::getInstance().getTcpSocket()
            .write((char*)pPDU, pPDU->uintPDULen);
    qDebug() << "has sent:" << ret << "bytes";
    qDebug() << "content:" << (char*)pPDU->Msg+2*32 << pPDU->caData;
    free(pPDU);
    pPDU = NULL;
}

void shareW::on_pCancel_pb(bool checked)
{
    (void)checked;
    this->hide();
}

shareW::shareW(QWidget *parent) : QWidget(parent)
{
    m_pSelectAll_pb = new QPushButton("全选");
    m_pCacelAll_pb = new QPushButton("取消");

    m_pScrollArea = new QScrollArea;
    m_pFriend_w = new QWidget;   //new QWidget(m_pScrollArea)
    m_pFriList_layout = new QVBoxLayout(m_pFriend_w);
    m_pBtnGroup = new QButtonGroup(m_pFriend_w);
    m_pBtnGroup->setExclusive(false);

    m_pConfirm_pb = new QPushButton("确定");
    m_pCancel_pb = new QPushButton("取消");

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAll_pb);
    pTopHBL->addWidget(m_pCacelAll_pb);
    pTopHBL->addStretch();

    QHBoxLayout *pBtmHBL = new QHBoxLayout;
    pBtmHBL->addWidget(m_pConfirm_pb);
    pBtmHBL->addWidget(m_pCancel_pb);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pScrollArea);
    pMainVBL->addWidget(m_pFriend_w);
    pMainVBL->addLayout(pBtmHBL);

    setLayout(pMainVBL);
    //test();

    connect(m_pSelectAll_pb, SIGNAL(clicked(bool)),
            this, SLOT(on_selectAll(bool))
            );
    connect(m_pCacelAll_pb, SIGNAL(clicked(bool)),
            this, SLOT(on_cancelAll(bool))
            );
    connect(m_pConfirm_pb, SIGNAL(clicked(bool)),
            this, SLOT(on_confirm_pb(bool))
            );
    connect(m_pCancel_pb, SIGNAL(clicked(bool)),
            this, SLOT(on_pCancel_pb(bool))
            );
}

void shareW::test()
{
    QVBoxLayout *pMainVBL = new QVBoxLayout(m_pFriend_w);
    for(int i=0; i<15; i++){
        QCheckBox *pCheckBox = new QCheckBox("nina");
        m_pBtnGroup->addButton(pCheckBox);
        pMainVBL->addWidget(pCheckBox);
    }
    m_pScrollArea->setWidget(m_pFriend_w);
}

