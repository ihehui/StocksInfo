/*
 ****************************************************************************
 * databaseconnecter.h
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


#ifndef DATABASECONNECTER_H_
#define DATABASECONNECTER_H_

#include <QObject>
#include <QtSql>

#include "../../core/global_core.h"
#include "../guilib.h"

class GUI_LIB_API DatabaseConnecter: public QObject {

    Q_OBJECT

public:
    DatabaseConnecter(QObject *parent = 0);
    virtual ~DatabaseConnecter();

    bool isDatabaseOpened(const QString &connectionName, const QString &driver,
                          const QString &host, int port, const QString &user,
                          const QString &passwd, const QString &databaseName,
                          HEHUI::DatabaseType databaseType);

    bool connectToNewDatabase(QString *connectionName = NULL);


    QString dbConnectionName() const;
    QString dbDriver() const;
    QString dbServerHost() const;
    quint16 dbServerPort() const;
    QString dbUser() const;
    QString dbPasswd() const;
    QString dbName() const;
    HEHUI::DatabaseType dbType() const;
    bool settingsModified() const;
    bool saveSettings() const;


private:
    QSqlDatabase getDatabase(const QString &connectionName,
                             const QString &driver, const QString &host, int port,
                             const QString &user, const QString &passwd,
                             const QString &databaseName, HEHUI::DatabaseType databaseType);

    QSqlDatabase getDatabase2(const QString &connectionName,
                             const QString &driver, const QString &host, int port,
                             const QString &user, const QString &passwd,
                             const QString &databaseName, HEHUI::DatabaseType databaseType);



signals:
    //void signalNewDatabaseConnected(const QString &connectionName);

private slots:

private:
    QWidget *parentWidget;

    QString m_connectionName;
    QString m_driver;
    QString m_host;
    quint16 m_port;
    QString m_user;
    QString m_passwd;
    QString m_databaseName;
    HEHUI::DatabaseType m_databaseType;

    bool m_settingsModified;
    bool m_saveSettings;



};

#endif /* DATABASECONNECTER_H_ */
