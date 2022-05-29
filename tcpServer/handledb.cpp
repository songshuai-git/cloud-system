#include "handledb.h"
#include<QMessageBox>
#include<QDebug>
#include<QSqlError>
#include"protocol.h"
#include<vector>
#include <algorithm>

using namespace std;

handleDB::handleDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    init();
}

handleDB::~handleDB()
{
    m_db.close();
}

bool handleDB::handleRgst(const char *name, const char *pswd)
{
    if(name==NULL || pswd==NULL) return false;

    QSqlQuery query;
    QString cmd = QString("insert into UsrInfo(name,pswd) \
                           values(\'%1\',\'%2\')")
                  .arg(name).arg(pswd);
    qDebug() << cmd;

    if(!query.exec(cmd)) return false;

    return true;
}

bool handleDB::handleLogin(const char *name, const char *pswd)
{
    if(name==NULL || pswd==NULL) return false;

    QSqlQuery query;
    QString cmd = QString("select * from usrInfo where "
                          "name=\'%1\' and pswd = \'%2\' "
                          "and online = 0")
                  .arg(name).arg(pswd);
    qDebug() << cmd;

    if(!query.exec(cmd)) return false;
    if(!query.next()) return false;

    cmd = QString("update usrInfo set online = 1 "
                  "where name='%1'").arg(name);
    qDebug() << cmd;
    if(!query.exec(cmd)) {qDebug() << "error:" << cmd; return false;}

    return true;
}

bool handleDB::handleDisconnect(const char *name)
{
    if(name==NULL) return false;

    QSqlQuery query;
    QString cmd = QString("update usrInfo set online=0 "
                          "where name=\'%1\'")
                  .arg(name);

    if(!query.exec(cmd)) {qDebug() << "error:" << cmd; return false;}

    return true;
}

QStringList handleDB::handleAllOnline()
{
    QSqlQuery query;
    QString cmd = QString("select name from usrInfo "
                          "where online=1");
    if(!query.exec(cmd)) {qDebug() << "error:" << cmd; return QStringList();}

    QStringList nameList;
    nameList.clear();
    while(query.next()){
        nameList.append(query.value(0).toString());
    }
    return nameList;
}

int handleDB::handleSearchUsr(const char *name)
{
    if(name == NULL) return -2;

    QSqlQuery query;
    QString cmd = QString("select online from usrInfo "
                          "where name=\'%1\'").arg(name);
    qDebug() << "sql exec:" << cmd;
    query.exec(cmd);

    if(!query.next()) return -1;
    qDebug() << "online:" << query.value(0).toInt();
    return query.value(0).toInt();
}

int handleDB::handleAddFri(const char *name, const char *perName)
{
    if(name == NULL || perName == NULL) return -2;

    QSqlQuery query;
    QString cmd = QString("select * from friendId where "
                          "(id=(select id from usrInfo where name = \'%1\') and "
                          "friendId=(select id from usrInfo where name = \'%2\')) "
                          "or "
                          "(id=(select id from usrInfo where name = \'%3\') and "
                          "friendId=(select id from usrInfo where name = \'%4\'))")
                          .arg(name).arg(perName)
                          .arg(perName).arg(name);
    qDebug() << "sql exec:" << cmd;
    query.exec(cmd);

    if(query.next()){
        return ENUM_PERNAME_ALREADY_FRI;
    }else{
        int ret = handleSearchUsr(perName);
        if(ret == -2){
            return -2;
        }else if(ret == -1){
            return ENUM_PERNAME_NOT_EXIST;
        }else if(ret == 0){
            return ENUM_PERNAME_NOT_ONLINE;
        }else if(ret == 1){
            return ENUM_PERNAME_IS_ONLINE;
        }
    }

    return -2;
}

bool handleDB::handleAddFriAgree(const char *name, const char *perName)
{
    if(name == NULL || perName == NULL) return false;

    QSqlQuery query;
    QString cmd = QString("insert into friendId(id,friendId) values("
                          "(select id from usrInfo where name=\'%1\'),"
                          "(select id from usrInfo where name=\'%2\')"
                          ")"
                          )
                          .arg(name).arg(perName);
    qDebug() << "sql exec:" << cmd;
    if( !query.exec(cmd) ) return false;

    return true;

}

QStringList handleDB::handleFlushFri(const char *name)
{
   if(name == NULL) return QStringList();

    QSqlQuery query;
    QString cmd = QString( "select * from friendId where "
                           "id=(select id from usrInfo where name=\'%1\') or "
                           "friendId=(select id from usrInfo where name=\'%2\')")
                           .arg(name).arg(name);
    qDebug() << "sql exec:" << cmd;
    if( !query.exec(cmd) ) return QStringList();

    vector<int> vec;
    vector<int>::iterator iter;
    while(query.next()){
        vec.push_back(query.value(0).toInt());
        vec.push_back(query.value(1).toInt());
    }

    //查重
    sort(vec.begin(), vec.end());
    iter = unique(vec.begin(),vec.end());
    if(iter != vec.end()){
        vec.erase(iter,vec.end());
    }

//    for(iter=vec.begin(); iter!=vec.end(); ++iter)
//        qDebug()<<*iter;

    QStringList friendList;
    friendList.clear();

    for(iter=vec.begin(); iter!=vec.end(); ++iter){
        cmd = QString("select name from usrInfo where id=%1 and online=1")
                      .arg(*iter);

        qDebug() << "sql exec:" << cmd;
        if( !query.exec(cmd) ) return QStringList();

        if(query.next()){
            friendList.append(query.value(0).toString());
        }
    }

    return friendList;
}

bool handleDB::handleDelFri(const char *name, const char *perName)
{
    //delete from friendId where (id=(select id from usrInfo where name = 'simon') and friendId=(select id from usrInfo where name='nina')) or (id=(select id from usrInfo where name = 'nina') and friendId=(select id from usrInfo where name='simon'))
    if(name == NULL || perName == NULL) return false;

    QSqlQuery query;
    QString cmd = QString("delete from friendId where "
                          "(id=(select id from usrInfo where name=\'%1\') and "
                          "friendId=(select id from usrInfo where name=\'%2\')) or "
                          "(id=(select id from usrInfo where name=\'%3\') and "
                          "friendId=(select id from usrInfo where name=\'%4\'))"
                          )
                          .arg(name).arg(perName)
                          .arg(perName).arg(name);
    qDebug() << "sql exec:" << cmd;
    if( !query.exec(cmd) ) return false;

    return true;
}

handleDB &handleDB::getInstance()
{
    static handleDB instance;
    return instance;
}

void handleDB::init()
{
    m_db.setDatabaseName("F:\\p2021\\iStudy\\cloud system\\tcpServer\\cloud.db");
    if(!m_db.open()){
        QMessageBox::critical(NULL, "打开数据库", "打开数据库失败");
        return;
    }

    //qDebug() << "打开数据库成功";
    QSqlQuery sqlQuery;
    if(!sqlQuery.exec("select * from usrInfo"))
        qDebug() << sqlQuery.lastError();

    while(sqlQuery.next()){
        QString strStr = QString("%1,%2,%3,%4")
                         .arg(sqlQuery.value(0).toString())
                         .arg(sqlQuery.value(1).toString())
                         .arg(sqlQuery.value(2).toString())
                         .arg(sqlQuery.value(3).toString());
        qDebug() << strStr;
    }
}


