/*
 ****************************************************************************
 * databaseconnecterdialog.h
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




#ifndef DATABASECONNECTERDIALOG_H
#define DATABASECONNECTERDIALOG_H

#include <QSqlError>
#include <QDialog>
#include <QMessageBox>

#include "common.h"

namespace Ui {
class DatabaseConnecterDialogUI;
}

class DatabaseConnecterDialog : public QDialog
{
    Q_OBJECT

public:
    DatabaseConnecterDialog(QWidget *parent = 0);
    DatabaseConnecterDialog(const QString &connectionName, const QString &host = "", int port = 0, const QString &user = "root", const QString &passwd = "", const QString &databaseName = "", HEHUI::DatabaseType databaseType = HEHUI::MYSQL, QWidget *parent = 0);
    DatabaseConnecterDialog(QWidget *parent, HEHUI::DatabaseType databaseType, const QString &databaseFilePath = "");

    ~DatabaseConnecterDialog();

    QStringList getParameters();
    void getParameters(QString *dbConnectionName, QString *dbDriverName, QString *dbHostAddress, quint16 *dbHostPort, QString *dbUser, QString *dbPassword, QString *dbName, HEHUI::DatabaseType *dbType);


    bool saveSettings();
    void showSaveSettingsOption(bool show);

private:
    //初始化
    //Initialization
    void  setup();

    //TODO: 与databaseutility.h中重复
    QStringList availableDrivers() const;

    QString driverName() const;
    QString databaseName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    int port() const;

    QString connectionName() const;
    QString remoteDatabaseName() const;
    QString localDatabaseFilePath() const;


private slots:
    void on_databaseTypeComboBox_currentIndexChanged(int index);
    void on_driverCombo_currentIndexChanged(const QString & text);
    void on_okButton_clicked();
    void on_cancelButton_clicked() { reject(); }
    void on_browseButton_clicked();

protected:
    void keyPressEvent(QKeyEvent *e) ;
    void languageChange();


private:
    Ui::DatabaseConnecterDialogUI *ui;

    HEHUI::DatabaseType selectedDatabaseType;
    QList <QPair<QString, HEHUI::DatabaseType> >supportedDatabases;
    QString specifiedConnectionName;
    QStringList parameters;

};
#endif
