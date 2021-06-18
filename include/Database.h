#pragma once
#include "stdafx.h"
#include <QMap>

const QString CREATE_TABLE  = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY NOT NULL, "
                                "login VARCHAR(200) NOT NULL, "
                                "password TEXT NOT NULL,"
                                "activitystatus BOOLEAN);";
const QString GET_COUNT               = "SELECT COUNT(*) FROM users;";
const QString INSERT_USER             = "INSERT INTO users VALUES(%1, '%2', '%3', '%4');";
const QString GET_PASSWORD            = "SELECT password FROM users WHERE login = '%1';";
const QString CHECK_USER              = "SELECT EXISTS (SELECT login FROM users WHERE login = '%1');";
const QString GET_ALL_USERS_LOGIN     = "SELECT login FROM users";
const QString GET_ALL_USERS_STATUS    = "SELECT activitystatus FROM users";
const QString GET_ALL_USERS_ID        = "SELECT id FROM users";
const QString GET_ACTIVITY_AND_LOGINS = "SELECT login, activitystatus FROM users";

class Database
{
public:    
    explicit Database();
    bool registration( const QString &login, const QString& pass, QString& result_in_text );
    bool authorization( const QString &login, const QString &pass, QString& result_in_text );
    QJsonObject getJSoncontact();
    bool setActivityStatus( const QString& user, bool status );
    std::optional< bool >  getActivityStatus( const QString& user );
    std::optional< QJsonObject >  getActivityStatusAllUsers();

private:
    bool checkUser( const QString &login );
    int getCount();
};
