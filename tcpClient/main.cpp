#include "tcpclient.h"
#include <QApplication>

//#include"online.h"
//#include"book.h"
//#include"sharew.h"
//#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    tcpClient::getInstance().show();
//    online w;
//    w.show();
//    book w;
//    w.show();
//    shareW::getInstance().show();

    return a.exec();
}
