/*
 ****************************************************************************
 * databaseutility.h
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
  * Last Modified on: 2010-05-08
  * Last Modified by: 贺辉
  ***************************************************************************
*/



#ifndef DATABASEUTILITY_H_
#define DATABASEUTILITY_H_


#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "../global_core.h"
#include "../core_lib.h"


class CORE_LIB_API DatabaseUtility :public QObject {
    Q_OBJECT

public:
    DatabaseUtility(QObject *parent = 0);
    virtual ~DatabaseUtility();


    static QStringList availableDrivers();

    QSqlError openDatabase(const QString &connectionNameString, const QString &driver,
                           const QString &host, int port, const QString &user, const QString &passwd, const QString &databaseName, HEHUI::DatabaseType databaseType);


    QSqlQuery  queryDatabase(const QString & queryString, const QString &connectionName, const QString &driver,
                             const QString &host, int port, const QString &user, const QString &passwd,
                             const QString &databaseName, HEHUI::DatabaseType databaseType) ;


    static QSqlError excuteSQLScriptFromFile(const QSqlDatabase &database, const QString &scriptFileName, const QString &fileCodec = "System",  QSqlQuery *q = 0, bool stopOnError = true);

    static QSqlDatabase getDatabase(const QString &connectionName);

    static void closeDBConnection(const QString &connectionName);
    static void closeAllDBConnections();

private:
    QSqlError openRemoteDatabase(const QString &connectionName, const QString &driver,
                                 const QString &host, int port, const QString &user, const QString &passwd, const QString &databaseName, HEHUI::DatabaseType databaseType);

    QSqlError openLocalFileDatabase(const QString &connectionName, const QString &databaseFileNamePath, const QString &driverName);



signals:
    void signalDatabaseConnectionFinished(const QString &connectionName, const QSqlError &err);

};

#endif /* DATABASEUTILITY_H_ */
