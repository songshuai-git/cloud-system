#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>

#include"protocol.h"

namespace Ui {
class online;
}

class online : public QWidget
{
    Q_OBJECT

public:
    explicit online(QWidget *parent = 0);
    ~online();

    void showAllUserOnline(struct PDU *pPDU);
private slots:
    void on_addFri_pb_clicked();

private:
    Ui::online *ui;
};

#endif // ONLINE_H
