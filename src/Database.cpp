#include "include/Database.h"

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
        numbers = query.value(0).toInt();
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
    return query.value(0).toInt();
}

bool Database::registration( const QString &login, const QString& pass, QString& result_in_text )
{

    QSqlQuery query;
    if ( checkUser( login ) )
    {
        result_in_text = QIODevice::tr( "User with this name is already registered" );
        return false;
    }

    if( !query.exec( INSERT_USER.arg( QString::number( getCount() + 1 ) ).arg( login ).arg( pass ).arg( true ) ) )
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
    if ( query.value(0).toString() == pass )
    {
        result_in_text = QIODevice::tr( "User is successfully authorized" );
        return true;
    }
    result_in_text = QIODevice::tr( "Passwords did not match, user is not logged in" );
    return false;
}

QJsonObject Database::getJSoncontact()
{
    QSqlQuery queryLogin;
    queryLogin.exec( GET_ALL_USERS_LOGIN );
    QSqlQuery queryAStatus;
    queryAStatus.exec( GET_ALL_USERS_STATUS );
    QSqlQuery queryID;
    queryID.exec( GET_ALL_USERS_ID );
    int i{};
    QJsonObject Contactlist;
    while ( queryLogin.next(), queryAStatus.next() )
    {
        QJsonObject NumberLoginAndStatus;
        NumberLoginAndStatus[ "login" ] = queryLogin.value(0).toString();
        NumberLoginAndStatus[ "activityStatus" ] = queryAStatus.value(0).toString();
        Contactlist[QString::number( i )] = NumberLoginAndStatus;
        i++;
    }
    return Contactlist;
}

bool Database::setActivityStatus( const QString& user, bool status )
{
    QSqlQuery queryLogin;
    queryLogin.exec( GET_ALL_USERS_LOGIN );
    QSqlQuery queryStatus;
    queryStatus.exec( GET_ALL_USERS_STATUS );
    while ( queryLogin.next(), queryStatus.next() )
    {
        if ( queryLogin.value( 0 ).toString() == user )
        {
            queryStatus.value( 0 ) = status;
            return true;
        }
    }
    return false;
}

std::optional< bool> Database::getActivityStatus( const QString& user )
{
    QSqlQuery queryLogin;
    queryLogin.exec( GET_ALL_USERS_LOGIN );
    QSqlQuery queryStatus;
    queryStatus.exec( GET_ALL_USERS_STATUS );

    QMap< QString, bool> status_map;
    while ( queryLogin.next(), queryStatus.next() )
    {
        if ( queryLogin.value( 0 ).toString() == user )
            return queryStatus.value( 0 ).toBool();
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
