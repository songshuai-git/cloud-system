#ifndef HANDLEDB_H
#define HANDLEDB_H

#include <QObject>
#include<QSqlDatabase>
#include<QSqlQuery>

class handleDB : public QObject
{
    Q_OBJECT
public:
    static handleDB &getInstance();
    void init();

    ~handleDB();

    bool handleRgst(const char *name, const char *pswd);
    bool handleLogin(const char *name, const char *pswd);
    bool handleDisconnect(const char *name);
    QStringList handleAllOnline();
    int handleSearchUsr(const char *name);
    int handleAddFri(const char *name,const char *perName);
    bool handleAddFriAgree(const char *name,const char *perName);
    QStringList handleFlushFri(const char *name);
    bool handleDelFri(const char *name,const char *perName);
signals:

public slots:

private:
    explicit handleDB(QObject *parent = nullptr);

    QSqlDatabase m_db;
};

#endif // HANDLEDB_H
