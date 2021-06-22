#include "include/Database.h"

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
{
    connectToDatabase();
    createTable();
}

int Database::getCount()
{
    QSqlQuery query;
    if( !query.exec( GET_COUNT ) )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: getting the number of users";
        return 0;
    }
    qDebug() << __FILE__ << __LINE__ << "Number of users received";

    int numbers{};
    if ( query.next() )
    {
        numbers = query.value( 0 ).toInt();
        return numbers;
    }
    return 0;
}

bool Database::checkUser( const QString &login )
{
    QSqlQuery query;

    QString str = CHECK_USER.arg( login );
    if( !query.exec( str ) )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: user verification error";
        return false;
    }
    qDebug() << __FILE__ << __LINE__ << "User verified successfully";
    query.next();
    return query.value( 0 ).toInt();
}

bool Database::registration( const QString &login, const QString& pass, QString& result_in_text )
{

    QSqlQuery query;
    if ( checkUser( login ) )
    {
        result_in_text = QIODevice::tr( "User with this name is already registered" );
        return false;
    }

    if( !query.exec( INSERT_USER.arg( QString::number( getCount() + 1 ) ).arg( login ).arg( pass ).arg( false ) ) )
    {
        result_in_text = QIODevice::tr( "User adding error" );
        return false;
    }

    result_in_text = QIODevice::tr( "User added successfully" );
    return true;
}

bool Database::authorization(const QString &login, const QString &pass, QString& result_in_text)
{
    QSqlQuery query;

    if( !query.exec( GET_PASSWORD.arg( login ) ) )
    {
        result_in_text = QIODevice::tr( "User authorization error" );
        return false;
    }
    query.next();
    if ( !query.isValid() )
    {
        result_in_text = QIODevice::tr( "There are no clients with this login, the user is not authorized" );
        return false;
    }
    if ( query.value( 0 ).toString() == pass )
    {
        setActivityStatus( login, true);
        result_in_text = QIODevice::tr( "User is successfully authorized" );
        return true;
    }
    result_in_text = QIODevice::tr( "Passwords did not match, user is not logged in" );
    return false;
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
    QSqlQuery queryLogin;
    queryLogin.exec( GET_ALL_USERS_LOGIN );
    QSqlQuery queryStatus;
    queryStatus.exec( GET_ALL_USERS_STATUS );

    QSqlQuery query;
    query.prepare("SELECT users.login, users.activitystatus FROM users "
                  "WHERE users.login = :name");
    query.bindValue(":name", user);
    query.exec();
    bool status{false};
    while (query.next())
    {
        status = query.value(0).toBool();//0 or 1
    }

    if (query.lastError().isValid())
    {
       return "There are problems with the database or communication with it";
    }

    qDebug() << __FILE__ << __LINE__ << "Error: get activity status from database";
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
