#pragma once
#include "stdafx.h"
#include <QMap>
#include <optional>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

struct Identification_request
{
    bool is_request_granted{ false };
    QString message;
};

const QString CREATE_TABLE  = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY NOT NULL, "
                                "login VARCHAR(200) NOT NULL, "
                                "password TEXT NOT NULL,"
                                "activitystatus BOOLEAN);";
const QString GET_COUNT               = "SELECT COUNT(*) FROM users;";
const QString INSERT_USER             = "INSERT INTO users VALUES(%1, '%2', '%3', '%4');";
const QString GET_PASSWORD            = "SELECT password FROM users WHERE login = '%1';";
const QString CHECK_USER              = "SELECT EXISTS (SELECT login FROM users WHERE login = '%1');";
const QString GET_ACTIVITY_AND_LOGINS = "SELECT login, activitystatus FROM users";

class Database
{
public:
    explicit Database();
    Identification_request registration( const QString &login, const QString& password );
    Identification_request authorization( const QString &login, const QString &password );

    bool setActivityStatus( const QString& user, bool status );
    std::optional< bool >  getActivityStatus( const QString& user );

    std::optional< QJsonObject >  getActivityStatusAllUsers();
    bool setActivityStatusAllUsersToFalse();

private:
    bool checkIfUserWithLoginExists( const QString &login );
    int getCount();
};
