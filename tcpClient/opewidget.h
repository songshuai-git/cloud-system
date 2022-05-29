#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include<QListWidget>
#include"friend.h"
#include"book.h"
#include<QStackedWidget>

class opeWidget : public QWidget
{
    Q_OBJECT
public:
    static opeWidget &getInstance();
    Friend *getFriend()const;
    inline book *getBook()const{ return m_book; }
signals:
public slots:
private:
    explicit opeWidget(QWidget *parent = nullptr);

    QListWidget* m_lw;
    Friend* m_friend;
    book* m_book;
    QStackedWidget *m_stackW;
};

#endif // OPEWIDGET_H
