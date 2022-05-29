#include "opewidget.h"
#include<QHBoxLayout>

opeWidget::opeWidget(QWidget *parent) : QWidget(parent)
{
    resize(700, 500);

    m_lw = new QListWidget(this); //!此处必须指定父对象
    m_lw->addItem("好友");
    m_lw->addItem("图书");

    m_friend = new Friend;
    m_book = new book;
//    QHBoxLayout *layout = new QHBoxLayout;
//    layout->addWidget(m_lw);
//    layout->addWidget(m_friend);
//    layout->addWidget(m_book);

    m_stackW = new QStackedWidget;
    m_stackW->addWidget(m_friend);
    m_stackW->addWidget(m_book);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_lw);
    layout->addWidget(m_stackW);
    setLayout(layout);

    QObject::connect(m_lw, SIGNAL(currentRowChanged(int)),
                     m_stackW, SLOT(setCurrentIndex(int))
                     );
}

opeWidget &opeWidget::getInstance()
{
    static opeWidget instance;
    return instance;
}

Friend *opeWidget::getFriend() const
{
    return m_friend;
}

