#include "../include/Database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

#include <optional>

static void connectToDatabase()
{
    QSqlDatabase m_R2FSqlDatabase;
    m_R2FSqlDatabase = QSqlDatabase::addDatabase( "QSQLITE" );
    m_R2FSqlDatabase.setHostName( "127.0.0.1" );
    m_R2FSqlDatabase.setDatabaseName( "Users_Database_on_Server" );
    if ( !m_R2FSqlDatabase.open() )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: QSQLITE database is not open";
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "QSQLITE database is open";
}

static void createTable()
{
    QSqlQuery query;

    if ( !query.exec( CREATE_TABLE ) )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: table is not created";
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Table is created";
}

Database::Database()
    : QObject()
{
    connectToDatabase();
    createTable();
}
int Database::getCount()
{
    int number_of_users = 0;

    QSqlQuery query;

    if( !query.exec( GET_COUNT ) )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: getting the number of users";
    }

    if ( query.next() )
    {
        number_of_users = query.value( 0 ).toInt();
        qDebug() << __FILE__ << __LINE__ << "Number of users received: " << number_of_users;
    }
    return number_of_users;
}

bool Database::checkIfUserWithLoginExists( const QString &login )
{
    QSqlQuery query;
    QString str = CHECK_USER.arg( login );
    bool isLoginInDatabase = false;

    if( !query.exec( str ) )
    {
        qDebug() << __FILE__ << __LINE__ << "There are problems with the database or communication with it";
        isLoginInDatabase = false;
    }
    else
    {
        if ( query.next() )
        {
            int count = query.value( 0).toInt();
            if ( count > 0)
            {
                qDebug() << __FILE__ << __LINE__ << "Login exists in the database";
                isLoginInDatabase = true;
            }
            else
            {
                qDebug() << __FILE__ << __LINE__ << "Login does not exist in the database";
                isLoginInDatabase = false;
            }
        }
        else
        {
            qDebug() << __FILE__ << __LINE__ << "Error while fetching result";
            isLoginInDatabase = false;
        }
    }
    return isLoginInDatabase;
}

Identification_request Database::registration( const QString &login, const QString& password )
{
    QSqlQuery query;
    Identification_request registration_result;

    if ( checkIfUserWithLoginExists( login ) )
    {
        registration_result.is_request_granted = false;
        registration_result.message = tr( "User with this name is already registered" );
        qDebug() << __FILE__ << __LINE__ << "User with this name is already registered";
    }
    else if( query.exec( INSERT_USER.arg( QString::number( getCount() + 1 ) ).arg( login ).arg( password ).arg( false ) ) )
    {
        registration_result.is_request_granted = true;
        registration_result.message = tr( "User registered successfully" );
        qDebug() << __FILE__ << __LINE__ << "User registered successfully";
    }
    else
    {
        registration_result.is_request_granted = false;
        registration_result.message = tr( "User registeration error: technical errors on server" );
        qDebug() << __FILE__ << __LINE__ << "User registeration error: technical errors on server";
        qDebug() << __FILE__ << __LINE__ << "Error QSqlQuery: " + query.lastError().text();
    }

    return registration_result;
}

Identification_request Database::authorization(const QString &login, const QString &password )
{
    QSqlQuery query;
    Identification_request authorization_result;

    if( !query.exec( GET_PASSWORD.arg( login ) ) )
    {
        authorization_result.is_request_granted = false;
        authorization_result.message = tr( "User authorization error" );
        return authorization_result;
    }
    query.next();
    if ( !query.isValid() )
    {
        authorization_result.message = tr( "There are no clients with this login, the user is not authorized" );
        authorization_result.is_request_granted = false;
        return authorization_result;
    }
    if ( query.value( 0 ).toString() == password )
    {
        setActivityStatus( login, true);
        authorization_result.message = tr( "User is successfully authorized" );
        authorization_result.is_request_granted = true;
    }
    else
    {
        authorization_result.message = tr( "Passwords did not match, user is not logged in" );
        authorization_result.is_request_granted = false;
    }
    return authorization_result;
}

bool Database::setActivityStatus( const QString& user, bool status )
{
    QSqlQuery query;
    query.prepare("UPDATE users SET activitystatus = :activitystatus WHERE login = :user ");
    query.bindValue( ":user", user );
    query.bindValue( ":activitystatus", status );
    if ( !query.exec() )
    {
        qDebug() << __FILE__ << __LINE__ << "Error QSqlQuery: " + query.lastError().text();
        return false;
    }
    return true;
}

bool Database::setActivityStatusAllUsersToFalse()
{
    QSqlQuery query;
    query.prepare( "UPDATE users SET activitystatus = :activitystatus" );
    query.bindValue( ":activitystatus", false );
    if ( !query.exec() )
    {
        qDebug() << __FILE__ << __LINE__ << "Error QSqlQuery: error set activity status all users to false because the server has shut down";
        return false;
    }
    return true;
}

std::optional< bool> Database::getActivityStatus( const QString& user )
{
    QSqlQuery query;
    query.prepare("SELECT users.login, users.activitystatus FROM users "
                  "WHERE users.login = :name");
    query.bindValue(":name", user);
    query.exec();

    while (query.next())
    {
        bool status = query.value(1).toBool();
        return status;
    }

    if (query.lastError().isValid())
    {
       qDebug() << __FILE__ << __LINE__ << "SQL Error:" << query.lastError().text();
    }
    else
    {
        qDebug() << __FILE__ << __LINE__ << "Error: this user is not in the database";
    }

    return std::nullopt;

}

std::optional< QJsonObject > Database::getActivityStatusAllUsers()
{
    QSqlQuery query;
    query.exec( GET_ACTIVITY_AND_LOGINS );
    QJsonObject contact_list;
    int i{};
    while (query.next())
    {
        QJsonObject login_and_status_one_user;
        login_and_status_one_user[ "login" ] = query.value(0).toString();
        login_and_status_one_user[ "activityStatus" ] = query.value(1).toString();
        contact_list[QString::number( i )] = login_and_status_one_user;
        i++;
    }
    if ( contact_list.isEmpty() )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: geting logins and activity status from database";
        return std::nullopt;
    }
    return contact_list;
}
