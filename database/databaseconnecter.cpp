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


#include <QApplication>
#include <QMessageBox>
//#include <QProgressDialog>

#include "databaseconnecter.h"
#include "databaseconnecterdialog.h"
#include "../../core/database/databaseutility.h"

DatabaseConnecter::DatabaseConnecter(QObject *parent) :
    QObject(parent) {

    qDebug() << "----DatabaseConnecter::DatabaseConnecter(QObject *parent)";

    if (parent && parent->isWidgetType()) {
        this->parentWidget = qobject_cast<QWidget *> (parent);
        qDebug() << "~~ DatabaseConnecter: parent is Widget Type";
    } else {
        this->parentWidget = 0;
        qDebug() << "~~ DatabaseConnecter: parent is not Widget Type";
    }




    m_connectionName = "";
    m_driver = "QMYSQL";
    m_host = "";
    m_port = 3306;
    m_user = "root";
    m_passwd = "";
    m_databaseName = "";
    m_databaseType = HEHUI::MYSQL ;


    m_settingsModified = false;
    m_saveSettings = false;




}

DatabaseConnecter::~DatabaseConnecter() {

}

bool DatabaseConnecter::isDatabaseOpened(const QString &connectionName,
                                         const QString &driver, const QString &host, int port,
                                         const QString &user, const QString &passwd,
                                         const QString &databaseName, HEHUI::DatabaseType databaseType)

{


    qDebug() << "----DatabaseConnecter::isDatabaseOpened(...)";
    Q_ASSERT_X(!connectionName.isEmpty(), "DatabaseConnecter::isDatabaseOpened(...)", "'connectionName' is empty!");
    Q_ASSERT_X(!driver.isEmpty(), "DatabaseConnecter::isDatabaseOpened(...)", "'driver' is empty!");
    Q_ASSERT_X(!databaseName.isEmpty(), "DatabaseConnecter::isDatabaseOpened(...)", "'databaseName' is empty!");

    QSqlDatabase db;
    db = QSqlDatabase::database(connectionName);

    if (!db.isValid()) {
        db = getDatabase(connectionName, driver, host, port, user, passwd, databaseName, databaseType);
    }

    if(db.isValid() && db.isOpen()){
        return true;
    }else{
        return false;
    }

}

QSqlDatabase DatabaseConnecter::getDatabase(const QString &connectionName,
                                            const QString &driver, const QString &host, int port,
                                            const QString &user, const QString &passwd,
                                            const QString &databaseName, HEHUI::DatabaseType databaseType)

{
    qDebug()<<"----DatabaseConnecter::getDatabase(...)";

    qApp->processEvents();
    //QApplication::setOverrideCursor(Qt::WaitCursor);

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isValid()) {

        QSqlError err;
        DatabaseUtility databaseUtility;

        //TODO
        //可能会引起界面冻结
        //Maybe freeze the GUI

//        QString db_connectionName = connectionName;
//        QString db_driver = driver;
//        QString db_host = host;
//        quint16 db_port = port;
//        QString db_user = user;
//        QString db_passwd = passwd;
//        QString db_databaseName = databaseName;
//        HEHUI::DatabaseType db_databaseType = databaseType;

        m_connectionName = connectionName;
        m_driver = driver;
        m_host = host;
        m_port = port;
        m_user = user;
        m_passwd = passwd;
        m_databaseName = databaseName;
        m_databaseType = databaseType;


        do {
            err = databaseUtility.openDatabase(m_connectionName,
                                               m_driver,
                                               m_host,
                                               m_port,
                                               m_user,
                                               m_passwd,
                                               m_databaseName,
                                               m_databaseType
                                               );

            if (err.type() != QSqlError::NoError) {
                QApplication::restoreOverrideCursor();
                QMessageBox::critical(parentWidget, tr("Fatal Error"), tr("An error occurred when opening the database!<br> %1").arg(err.text()));
                qCritical() << QString("ERROR! An error occurred when opening the database: %1").arg(err.text());

                DatabaseConnecterDialog dbConnecterDlg(
                            m_connectionName,
                            m_host,
                            m_port,
                            "",
                            "",
                            m_databaseName,
                            m_databaseType,
                            parentWidget
                            );
                dbConnecterDlg.showSaveSettingsOption(true);
                QStringList parameters = dbConnecterDlg.getParameters();
                if (parameters.size() <= 0) {
                    return QSqlDatabase();
                }

                m_connectionName = parameters.at(0);
                m_driver = parameters.at(1);
                m_host = parameters.at(2);
                m_port = parameters.at(3).toUShort();
                m_user = parameters.at(4);
                m_passwd = parameters.at(5);
                m_databaseName = parameters.at(6);
                m_databaseType = (HEHUI::DatabaseType) parameters.at(7).toUInt();

                m_settingsModified = true;
                m_saveSettings = dbConnecterDlg.saveSettings();

            }
        } while (err.type() != QSqlError::NoError);

        db = QSqlDatabase::database(connectionName);
        //emit signalNewDatabaseConnected(connectionName);

    }

    QApplication::restoreOverrideCursor();

    return db;

}

QSqlDatabase DatabaseConnecter::getDatabase2(const QString &connectionName,
                                            const QString &driver, const QString &host, int port,
                                            const QString &user, const QString &passwd,
                                            const QString &databaseName, HEHUI::DatabaseType databaseType)

{
    qDebug()<<"----DatabaseConnecter::getDatabase(...)";

    qApp->processEvents();
    //QApplication::setOverrideCursor(Qt::WaitCursor);

    QSqlDatabase db;
    db = QSqlDatabase::database(connectionName);

    if (!db.isValid()) {

        QSqlError err;
        DatabaseUtility databaseUtility;

        //TODO:
        //        QProgressDialog progressDialog(parentWidget);
        //        progressDialog.setRange(0,0);
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &progressDialog, SLOT(close()));
        //        progressDialog.show();
        //        //progressDialog.raise();
        //        //progressDialog.exec();

        //        QEventLoop eventLoop;
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &eventLoop, SLOT(quit()));
        //        eventLoop.exec();
        //        qCritical() << QString("eventLoop.exec();");
        //
        //        QTimer timer;
        //        timer.setSingleShot(false);
        //        timer.setInterval(100);
        //        connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &eventLoop, SLOT(quit()));
        //
        //        connect(&databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &timer, SLOT(stop()));
        //        //eventLoop.exec();
        //        //timer.setInterval(500);
        //        timer.start();

        //TODO
        //可能会引起界面冻结
        //Maybe freeze the GUI
        err = databaseUtility.openDatabase(connectionName, driver, host, port,
                                           user, passwd, databaseName, databaseType);

        if (err.type() != QSqlError::NoError) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(parentWidget, tr("Fatal Error"), tr("An error occurred when opening the database!<br> %1").arg(err.text()));
            qCritical() << QString("ERROR! An error occurred when opening the database: %1").arg(err.text());

            DatabaseConnecterDialog dbConnecterDlg(connectionName, host, port, "", "", databaseName, databaseType, parentWidget);
            QStringList parameters = dbConnecterDlg.getParameters();
            if (parameters.size() <= 0) {
                qCritical() << QString("ERROR! Can not connect to database server!");
                return QSqlDatabase();
            }

            getDatabase(parameters.at(0), parameters.at(1), parameters.at(2),
                        parameters.at(3).toInt(), parameters.at(4),
                        parameters.at(5), parameters.at(6),
                        (HEHUI::DatabaseType) parameters.at(7).toUInt());

        }

        db = QSqlDatabase::database(connectionName);
        //emit signalNewDatabaseConnected(connectionName);

    }

    QApplication::restoreOverrideCursor();

    return db;

}

bool DatabaseConnecter::connectToNewDatabase(QString *connectionName) {
    qDebug() << "----DatabaseConnecter::connectToNewDatabase()";

    DatabaseConnecterDialog dbConnecterDlg(parentWidget);

    QStringList parameters = dbConnecterDlg.getParameters();
    if (parameters.size() <= 0) {
        QMessageBox::critical(parentWidget, tr("Fatal Error"), tr(
                                  "Can not connect to database server!"));
        qCritical() << QString("XX Fatal Error!") << QString(
                           "Can not connect to database server!");
        return false;
    }

    getDatabase(parameters.at(0), parameters.at(1), parameters.at(2),
                parameters.at(3).toInt(), parameters.at(4), parameters.at(5),
                parameters.at(6), (HEHUI::DatabaseType) parameters.at(7).toUInt());

    QSqlDatabase db;
    db = QSqlDatabase::database(parameters.at(0));

    if(connectionName){
        *connectionName = parameters.at(0);
    }

    return db.isValid();
}

QString DatabaseConnecter::dbConnectionName() const{
    return m_connectionName;
}

QString DatabaseConnecter::dbDriver() const{
    return m_driver;
}
QString DatabaseConnecter::dbServerHost() const{
    return m_host;
}

quint16 DatabaseConnecter::dbServerPort() const{
    return m_port;
}

QString DatabaseConnecter::dbUser() const{
    return m_user;
}

QString DatabaseConnecter::dbPasswd() const{
    return m_passwd;
}

QString DatabaseConnecter::dbName() const{
    return m_databaseName;
}

HEHUI::DatabaseType DatabaseConnecter::dbType() const{
    return m_databaseType;
}

bool DatabaseConnecter::settingsModified() const{
    return m_settingsModified;
}

bool DatabaseConnecter::saveSettings() const{
    return m_saveSettings;
}
