/*
 ****************************************************************************
 * databaseutility.cpp
 *
 * Created on: 2009-8-5
 *     Author: 贺辉
 *    License: LGPL
 *    Comment:
 *
 *
 *    =============================  Usage  =============================
 *|
 *|
 *    ===================================================================
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ****************************************************************************
 */

/*
  ***************************************************************************
  * Last Modified on: 2010-05-07
  * Last Modified by: 贺辉
  ***************************************************************************
*/


#include <QtCore>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QFileInfo>



#include "databaseutility.h"


DatabaseUtility::DatabaseUtility(QObject *parent)
    :QObject(parent)
{

    //qDebug()<<"----DatabaseUtility::DatabaseUtility(...)";
    //qDebug()<<"Available Database Drivers:"<<QSqlDatabase::drivers().join(",");

    //	if (getValidDrivers().isEmpty()) {
    //		qCritical()<< QString("Fatal Error: No database driver found");
    //	}

}

DatabaseUtility::~DatabaseUtility() {
    // TODO Auto-generated destructor stub
}


QStringList DatabaseUtility::availableDrivers()  {
    //获取可用的数据库驱动
    //Get available database drivers
    QStringList driversList = QSqlDatabase::drivers();

    // remove compat names
    driversList.removeAll("QMYSQL3");
    driversList.removeAll("QOCI8");
    driversList.removeAll("QODBC3");
    driversList.removeAll("QPSQL7");
    driversList.removeAll("QTDS7");

    return driversList;
    //return QSqlDatabase::drivers();
}


QSqlError DatabaseUtility::openDatabase(const QString &connectionNameString, const QString &driver,
                                        const QString &host, int port, const QString &user, const QString &passwd,
                                        const QString &databaseName, HEHUI::DatabaseType databaseType)
{

    qDebug()<<"--DatabaseUtility::openDatabase(...)";

    Q_ASSERT_X(!driver.isEmpty(), "DatabaseUtility::openDatabase(...)", "'driver' is empty!");
    Q_ASSERT_X(!databaseName.isEmpty(), "DatabaseUtility::openDatabase(...)", "'databaseName' is empty!");

    //QCoreApplication::processEvents();

    QString connectionName = connectionNameString;
    if(connectionName.isEmpty()){
        switch (databaseType) {
        case HEHUI::SQLITE:
        case HEHUI::M$ACCESS:
            connectionName = databaseName;
            break;

        default:
            connectionName = user+"@"+host+":"+QString::number(port)+"/"+databaseName;
            break;
        }

    }

    QSqlError err ;

    if(QSqlDatabase::contains(connectionName)){
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if(db.isValid()){
            if(!db.isOpen()){
                if(!db.open()){
                    closeDBConnection(connectionName);
                }else{
                    return db.lastError();
                }
            }else{
                //err.setType(QSqlError::ConnectionError);
                err.setType(QSqlError::NoError);
                err.setDatabaseText(tr("Database connection('%1') has been previously added!").arg(databaseName));
                qCritical()<<QString("Database connection('%1') has been previously added!").arg(databaseName);
                return err;
            }

        }else{
            QSqlDatabase::removeDatabase(connectionName);
        }


    }

    if(databaseType == HEHUI::SQLITE || databaseType == HEHUI::M$ACCESS){
        err = openLocalFileDatabase(connectionName, databaseName, driver);
    }else{
        err = openRemoteDatabase(connectionName, driver, host, port, user, passwd, databaseName, databaseType);
    }

    emit signalDatabaseConnectionFinished(connectionName, err);

    return err;

}


QSqlError DatabaseUtility::openRemoteDatabase(const QString &connectionName, const QString &driver,
                                              const QString &host, int port, const QString &user, const QString &passwd, const QString &databaseName, HEHUI::DatabaseType databaseType)
{
    //qDebug()<<"--DatabaseUtility::DatabaseUtility::openRemoteDatabase(...)";

    Q_ASSERT_X(!driver.isEmpty(), "DatabaseUtility::openRemoteDatabase(...)", "'driver' is empty!");
    Q_ASSERT_X(!databaseName.isEmpty(), "DatabaseUtility::openRemoteDatabase(...)", "'databaseName' is empty!");
    Q_ASSERT_X(!host.isEmpty(), "DatabaseUtility::openRemoteDatabase(...)", "'host' is empty!");


    QSqlDatabase db;

    if(databaseType == HEHUI::M$SQLSERVER && OS_IS_WINDOWS ){
        db = QSqlDatabase::addDatabase("QODBC", connectionName);
        db.setDatabaseName(
                    QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;UID=%3;PWD=%4")
                    .arg(host.trimmed())
                    .arg(databaseName.trimmed())
                    .arg(user.trimmed())
                    .arg(passwd.trimmed())
                    );
        db.open();

    }else{
        db = QSqlDatabase::addDatabase(driver, connectionName);
        db.setDatabaseName(databaseName);
        db.setHostName(host);
        db.setPort(port);
        db.setUserName(user);
        db.setPassword(passwd);
        db.open();
    }

    QSqlError error = db.lastError();

    if (!db.isOpen()) {
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        //qCritical()<< QString("XX Database '%1' open failed!").arg(databaseName);
        //qCritical()<< QString("XX An error occurred when opening the database: %1").arg(error.text());
    }

    qDebug()<< QString("~~ Database connection %1 is %2").arg(connectionName).arg(db.isValid() ? "Valid" : "Invalid");

    return error;

}


QSqlError DatabaseUtility::openLocalFileDatabase(const QString &connectionName, const QString &databaseFileNamePath, const QString &driverName) {
    qDebug()<<"----DatabaseUtility::openLocalFileDatabase(...)";

    Q_ASSERT_X(!driverName.isEmpty(), "DatabaseUtility::openLocalFileDatabase(...)", "'driverName' is empty!");
    Q_ASSERT_X(!databaseFileNamePath.isEmpty(), "DatabaseUtility::openLocalFileDatabase(...)", "'databaseFileNamePath' is empty!");


    QSqlError err;

    if(driverName != "QSQLITE"){

        if (!QFileInfo(databaseFileNamePath).exists()) {
            err.setType(QSqlError::ConnectionError);
            err.setDatabaseText(QObject::tr("Database file  %1  does not exists !"
                                            ).arg(databaseFileNamePath)
                                );

            //qCritical()<< QString("XX An error occurred when opening the database: %1").arg(err.text());
            return err;

        } else if (!QFileInfo(databaseFileNamePath).permission(QFile::WriteUser
                                                               | QFile::ReadUser)) {

            err.setType(QSqlError::ConnectionError);
            err.setDatabaseText(QObject::tr("Database file  %1  can not be read or written !")
                                .arg(databaseFileNamePath)
                                );

            //qCritical()<< QString("XX An error occurred when opening the database: %1").arg(err.text());
            return err;
        }

    }



    QSqlDatabase db;

    if (driverName == "QODBC") {

        db = QSqlDatabase::addDatabase("QODBC", connectionName);
        db.setDatabaseName(
                    QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                    .arg(databaseFileNamePath));

    } else {

        db = QSqlDatabase::addDatabase(driverName, connectionName);
        db.setDatabaseName(databaseFileNamePath);

    }

    err = db.lastError();

    if (!db.open()) {
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);

        //qCritical()<< QString("XX An error occurred when opening the database: %1").arg(err.text());

    }

    return err;

}

QSqlError DatabaseUtility::excuteSQLScriptFromFile(const QSqlDatabase &database, const QString &scriptFileName, const QString &fileCodec, QSqlQuery *q, bool stopOnError){

    QSqlError error;
    error.setType(QSqlError::NoError);
    error.setDatabaseText("");
    error.setDriverText("");

    if(scriptFileName.trimmed().isEmpty()){
        error.setType(QSqlError::StatementError);
        error.setDatabaseText(tr("Invalid SQL Script File Name!"));
        return error;
    }

    QFile file(scriptFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QString msg = file.errorString();
        error.setType(QSqlError::StatementError);
        error.setDatabaseText(msg);
        qCritical()<< msg;
        return error;
    }

    if(!database.isValid()){
        error.setType(QSqlError::ConnectionError);
        error.setDatabaseText(tr("Invalid Database!"));
        return error;
    }
    if(!database.isOpen()){
        error.setType(QSqlError::ConnectionError);
        error.setDatabaseText(tr("Database Not Open!"));
        return error;
    }


    QSqlQuery query(database);
    QString statement = "";

    QTextStream in(&file);
    if(fileCodec.trimmed().isEmpty()){
        in.setCodec(QTextCodec::codecForLocale());
    }else{
        in.setCodec(fileCodec.toLocal8Bit());
    }
    in.setAutoDetectUnicode(true);

    while (!in.atEnd()) {
        QString line = in.readLine();

        if(line.startsWith("--", Qt::CaseInsensitive)){
            qDebug();
            qDebug("Single Line Comment Ignored:\n%s", qPrintable(line));
            qDebug();

            continue;
        }

        statement += (statement.isEmpty()?line:("\n" + line));

        QString temp = line.trimmed();
        temp.replace(";", "");
        if(statement.trimmed().startsWith("/*", Qt::CaseInsensitive) && temp.endsWith("*/", Qt::CaseInsensitive)){
            qDebug();
            qDebug("Multi Line Comment Ignored:\n%s", qPrintable(statement));
            qDebug();

            statement = "";
            continue;
        }

        if(line.endsWith(";") || in.atEnd()){
            if(!query.exec(statement)){
                error = query.lastError();
                qDebug();
                qCritical("Can not excute SQL statement:\n%s\n%s", qPrintable(statement), qPrintable(error.text()));
                qDebug();

                if(stopOnError){
                    if(q){
                        *q = query;
                    }
                    return error;
                }


                statement = "";
                continue;
            }else{
                qDebug();
                qDebug("SQL Statement Excuted:\n%s", qPrintable(statement));
                qDebug();

                statement = "";
            }
        }

    }

    if(q){
        *q = query;
    }
    error = query.lastError();
    return error;



}

QSqlDatabase DatabaseUtility::getDatabase(const QString &connectionName){
    qDebug()<<"----DatabaseUtility::getDatabase(const QString &connectionName)";

    return QSqlDatabase::database(connectionName);

}

QSqlQuery DatabaseUtility::queryDatabase(const QString & queryString, const QString &connectionName, const QString &driver,
                                         const QString &host, int port, const QString &user, const QString &passwd,
                                         const QString &databaseName, HEHUI::DatabaseType databaseType) {

    //qDebug()<<"----DatabaseUtility::queryDatabase(...)";

    Q_ASSERT_X(!queryString.isEmpty(), "DatabaseUtility::queryDatabase(...)", "'queryString' is empty!");
    Q_ASSERT_X(!driver.isEmpty(), "DatabaseUtility::queryDatabase(...)", "'driver' is empty!");
    Q_ASSERT_X(!databaseName.isEmpty(), "DatabaseUtility::queryDatabase(...)", "'databaseName' is empty!");

    QSqlDatabase db;
    db = QSqlDatabase::database(connectionName);

    if (!db.isValid()) {
        QSqlError err;
        err = openDatabase(connectionName, driver, host, port, user, passwd,
                           databaseName, databaseType);
        if(err.type() != QSqlError::NoError){
            return QSqlQuery();
        }

        db = QSqlDatabase::database(connectionName);
        qDebug()<<"~~Database Connection: "<<connectionName<<"Valid? "<<db.isValid();

    }

    QSqlQuery q( db);
    q.exec(queryString);
    q.first();
    //qDebug()<<"~~~~~~~~~q.size():"<<q.size();
    //qDebug()<<"~~~~~~~~~q.value(0).toString():"<<q.value(0).toString();
    //qDebug()<<"~~~~~~~~~q.lastError().text():"<<q.lastError().text();

    return q;

    //return QSqlQuery(queryString, db);

}




void DatabaseUtility::closeDBConnection(const QString &connectionName){
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if(db.isValid()){
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

}

void DatabaseUtility::closeAllDBConnections(){
    qDebug()<<"----DatabaseUtility::closeAllDBConnections()";

    QStringList connectionNames = QSqlDatabase::connectionNames();

    for (int i = 0; i < connectionNames.count(); ++i) {
        QSqlDatabase db = QSqlDatabase::database(connectionNames.at(i), false);

        if (db.isOpen()) {
            db.close ();
            qDebug()<<QString("~~ Close Database '%1'(Connection Name:%2)").arg(db.databaseName(), connectionNames.at(i));
        }
        QSqlDatabase::removeDatabase(connectionNames.at(i));
        qDebug()<<QString("~~ Remove Database Connection '%1'(Database Name:%2)").arg(connectionNames.at(i), db.databaseName());
    }

}


