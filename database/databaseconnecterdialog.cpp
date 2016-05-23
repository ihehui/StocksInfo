/*
 ****************************************************************************
 * databaseconnecterdialog.cpp
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




#include <QtGui>
#include <QFileDialog>
#include <QFileInfo>
#include <QtSql>
#include <QEvent>


#include "databaseconnecterdialog.h"
#include "ui_databaseconnecterdialog.h"



DatabaseConnecterDialog::DatabaseConnecterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseConnecterDialogUI)
{
    setup();
}

DatabaseConnecterDialog::DatabaseConnecterDialog(const QString &connectionName, const QString &host, int port, const QString &user, const QString &passwd, const QString &databaseName, HEHUI::DatabaseType databaseType, QWidget *parent) :
    QDialog(parent), specifiedConnectionName(connectionName),
    ui(new Ui::DatabaseConnecterDialogUI)
{

    setup();

//    for(int i=0; i<supportedDatabases.size(); i++){
//        QPair<QString, HEHUI::DatabaseType> pair = supportedDatabases.at(i);
//        if(pair.second == databaseType){ui->databaseTypeComboBox->setCurrentIndex(i);}
//        break;
//    }
    ui->databaseTypeComboBox->setCurrentIndex(ui->databaseTypeComboBox->findData(databaseType));

    ui->hostnameEdit->setText(host);
    if(port > 0){ui->portSpinBox->setValue(port);}
    ui->userNameEdit->setText(user);
    ui->passwordEdit->setText(passwd);
    ui->databaseNameEdit->setText(databaseName);

}

DatabaseConnecterDialog::DatabaseConnecterDialog(QWidget *parent, HEHUI::DatabaseType databaseType, const QString &databaseFilePath)
    :QDialog(parent),
    ui(new Ui::DatabaseConnecterDialogUI)
{
    setup();

//    for(int i=0; i<supportedDatabases.size(); i++){
//        QPair<QString, HEHUI::DatabaseType> pair = supportedDatabases.at(i);
//        if(pair.second == databaseType){ui->databaseTypeComboBox->setCurrentIndex(i);}
//    }
    ui->databaseTypeComboBox->setCurrentIndex(ui->databaseTypeComboBox->findData(databaseType));

    if(!databaseFilePath.trimmed().isEmpty()){
        ui->databaseFilePathComboBox->insertItem(0, databaseFilePath);
        ui->databaseFilePathComboBox->setCurrentIndex(0);
    }

}

void DatabaseConnecterDialog::setup(){

    ui->setupUi(this);
    ui->checkBoxSaveSettings->hide();

    //获取有可用的数据库驱动, 填充数据库驱动ComboBox
    //Get available database drivers， setup the database drivers ComboBox
    QStringList drivers = availableDrivers();
    if(drivers.isEmpty()){
        QMessageBox::critical(this, tr("Fatal Error"), tr("No available database driver!"));
        qCritical("XX Critical Error! No available database driver!");
        return;
    }
    ui->driverCombo->addItems(drivers);

    //初始化所支持的数据库列表
    //Init supported databases list
    supportedDatabases.clear();
    if(drivers.contains(QString("QMYSQL"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("MySQL"), HEHUI::MYSQL));
    }

    if(drivers.contains(QString("QSQLITE"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("SQLite"), HEHUI::SQLITE));
    }

    if(drivers.contains(QString("QPSQL"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("PostgreSQL"), HEHUI::POSTGRESQL));
    }

    if(drivers.contains(QString("QIBASE"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("Firebird"), HEHUI::FIREBIRD));
    }

    if(drivers.contains(QString("QDB2"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("IBM DB2"), HEHUI::DB2));
    }

    if(drivers.contains(QString("QOCI"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("Oracle"), HEHUI::ORACLE));
    }

    if(drivers.contains(QString("QODBC"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("M$ SQL SERVER"), HEHUI::M$SQLSERVER));
    }

    if(drivers.contains(QString("QODBC"), Qt::CaseInsensitive)){
        supportedDatabases.append(qMakePair(QString("M$ Access"), HEHUI::M$ACCESS));
    }

    if(!drivers.isEmpty()){
        supportedDatabases.append(qMakePair(QString("Other"), HEHUI::OTHER));
    }

    //填充数据库类型ComboBox
    //Setup database type combobox
    for(int i=0; i<supportedDatabases.size(); i++){
        QPair<QString, HEHUI::DatabaseType> pair = supportedDatabases.at(i);
        ui->databaseTypeComboBox->addItem(pair.first, pair.second);
    }

    setWindowFlags(Qt::Dialog);
}

DatabaseConnecterDialog::~DatabaseConnecterDialog() {

    supportedDatabases.clear();
    parameters.clear();

    delete ui;
}

void DatabaseConnecterDialog::keyPressEvent(QKeyEvent *e)
{

    int key = e->key();

    switch (key) {
    //case Qt::Key_Escape:
    //	close();
    //	break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        //焦点跳转到下一个控件
        //Set focus to next child
        focusNextChild () ;
        break;


    default:
        QWidget::keyPressEvent(e);
    }

}

void DatabaseConnecterDialog::languageChange() {
    ui->retranslateUi(this);
}


QStringList DatabaseConnecterDialog::availableDrivers() const {
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
}

QString DatabaseConnecterDialog::driverName() const {

    //如果数据库驱动ComboBox的当前索引是0(即为"自动选择"),那么根据所选的数据库类型返回对应的驱动程序名称,默认返回‘QODBC’
    //If the index of database driver combobox is 0("Auto Select"), return the driver name of the selected database type, 'QODBC' for default
    if (ui->driverCombo->currentIndex() == 0) {

        switch(selectedDatabaseType){
        case HEHUI::MYSQL:
            return QString("QMYSQL");
            break;
        case HEHUI::SQLITE:
            return QString("QSQLITE");
            break;
        case HEHUI::POSTGRESQL:
            return QString("QPSQL");
            break;
        case HEHUI::FIREBIRD:
            return QString("QIBASE");
            break;
        case HEHUI::DB2:
            return QString("QDB2");
            break;
        case HEHUI::ORACLE:
            return QString("QOCI");
            break;
        case HEHUI::M$SQLSERVER:
            return QString("QODBC");
            break;
        case HEHUI::M$ACCESS:
            return QString("QODBC");
            break;
        default:
            return QString("QODBC");
            break;
        }

    }

    return ui->driverCombo->currentText();

}

QString DatabaseConnecterDialog::databaseName() const {

    //如果所选数据库类型是SQLite或M$ACCESS，则返回数据库文件路径
    if(selectedDatabaseType == HEHUI::SQLITE || selectedDatabaseType == HEHUI::M$ACCESS){
        return localDatabaseFilePath();
    }else{
        return remoteDatabaseName();
    }


}

QString DatabaseConnecterDialog::remoteDatabaseName() const {
    return ui->databaseNameEdit->text();
}

QString DatabaseConnecterDialog::localDatabaseFilePath() const {

#ifdef Q_OS_WIN
    return QString(ui->databaseFilePathComboBox->currentText()).toLower();
#else
    return ui->databaseFilePathComboBox->currentText();
#endif

}

QString DatabaseConnecterDialog::userName() const {
    return ui->userNameEdit->text();
}

QString DatabaseConnecterDialog::password() const {
    return ui->passwordEdit->text();
}

QString DatabaseConnecterDialog::hostName() const {
    return ui->hostnameEdit->text();
}

int DatabaseConnecterDialog::port() const {
    return ui->portSpinBox->value();
}

QString DatabaseConnecterDialog::connectionName() const{

    if(selectedDatabaseType == HEHUI::SQLITE || selectedDatabaseType == HEHUI::M$ACCESS){
        return localDatabaseFilePath();
    }else if(!specifiedConnectionName.isEmpty()){
        return specifiedConnectionName;
    }else{
        return QString(userName()+"@"+hostName()+":"+QString::number(port())+"/"+remoteDatabaseName());
    }

    return QString("");
}

QStringList DatabaseConnecterDialog::getParameters(){

    if(this->exec() != QDialog::Accepted){
        return QStringList();
    }

    parameters.clear();
    parameters<<connectionName()
             <<driverName()
            <<hostName()
           <<QString::number(port())
          <<userName()
         <<password()
        <<databaseName()
       <<QString::number((uint)selectedDatabaseType)
      //<<(ui->checkBoxSaveSettings->isChecked())?"1":"0"
         ;

    return parameters;
}

void DatabaseConnecterDialog::getParameters(QString *dbConnectionName, QString *dbDriverName, QString *dbHostAddress, quint16 *dbHostPort, QString *dbUser, QString *dbPassword, QString *dbName, HEHUI::DatabaseType *dbType){
    if(dbConnectionName){
        *dbConnectionName = connectionName();
    }
    if(dbDriverName){
        *dbDriverName = driverName();
    }
    if(dbHostAddress){
        *dbHostAddress = hostName();
    }
    if(dbHostPort){
        *dbHostPort = port();
    }
    if(dbUser){
        *dbUser = userName();
    }
    if(dbPassword){
        *dbPassword = password();
    }
    if(dbName){
        *dbName = databaseName();
    }
    if(dbType){
        *dbType = selectedDatabaseType;
    }

}


bool DatabaseConnecterDialog::saveSettings(){
    return ui->checkBoxSaveSettings->isChecked();
}

void DatabaseConnecterDialog::showSaveSettingsOption(bool show){
    ui->checkBoxSaveSettings->setVisible(show);
}

void DatabaseConnecterDialog::on_browseButton_clicked() {
    QString databaseFilePath = QFileDialog::getOpenFileName(this,
                                                            tr("Database File Path"),
                                                            QDir::currentPath(),
                                                            tr("Database (*.db *.mdb);;All Files (*.*)")
                                                            );

    if (!databaseFilePath.isEmpty()) {
        int index = ui->databaseFilePathComboBox->findText(databaseFilePath, Qt::MatchFixedString);
        if(index>=0){
            ui->databaseFilePathComboBox->setCurrentIndex(index);
        }else{
            ui->databaseFilePathComboBox->insertItem(0, databaseFilePath);
            ui->databaseFilePathComboBox->setCurrentIndex(0);
        }
        ui->okButton->setFocus();
    }

}

void DatabaseConnecterDialog::on_databaseTypeComboBox_currentIndexChanged(int index){

    //根据所选的数据库类型设置ui->stackedWidget的当前激活页
    //Set the active page of the ui->stackedWidget according to the selected database type
    uint databaseType = ui->databaseTypeComboBox->itemData(index).toUInt();
    switch(databaseType){
    case HEHUI::MYSQL:
        ui->stackedWidget->setCurrentWidget(ui->networkDatabasePage);
        ui->networkDatabaseGroupbox->setTitle(tr("MySQL"));
        //ui->portSpinBox->setValue(3306);
        ui->portSpinBox->setToolTip(tr("Default Port:3306"));
        ui->userNameEdit->setText("root");
        selectedDatabaseType = HEHUI::MYSQL;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QMYSQL")));
        break;

    case HEHUI::SQLITE:
        ui->stackedWidget->setCurrentWidget(ui->localDatabaseFilePage);
        ui->localDatabaseGroupbox->setTitle(tr("SQLite"));
        selectedDatabaseType = HEHUI::SQLITE;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QSQLITE")));
        break;

    case HEHUI::POSTGRESQL:
        ui->stackedWidget->setCurrentWidget(ui->networkDatabasePage);
        ui->networkDatabaseGroupbox->setTitle(tr("PostgreSQL"));
        ui->portSpinBox->setToolTip(tr("Default Port:5432"));
        ui->userNameEdit->setText("");
        selectedDatabaseType = HEHUI::POSTGRESQL;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QPSQL")));
        break;

    case HEHUI::FIREBIRD:
        ui->stackedWidget->setCurrentWidget(ui->networkDatabasePage);
        ui->networkDatabaseGroupbox->setTitle(tr("Firebird"));
        ui->portSpinBox->setToolTip(tr("Default Port:3050"));
        ui->userNameEdit->setText("");
        selectedDatabaseType = HEHUI::FIREBIRD;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QIBASE")));
        break;

    case HEHUI::DB2:
        ui->stackedWidget->setCurrentWidget(ui->networkDatabasePage);
        ui->networkDatabaseGroupbox->setTitle(tr("IBM DB2"));
        ui->portSpinBox->setToolTip(tr("Default Port:50000"));
        ui->userNameEdit->setText("");
        selectedDatabaseType = HEHUI::DB2;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QDB2")));
        break;

    case HEHUI::ORACLE:
        ui->stackedWidget->setCurrentWidget(ui->networkDatabasePage);
        ui->networkDatabaseGroupbox->setTitle(tr("Oracle"));
        ui->portSpinBox->setToolTip(tr(""));
        ui->userNameEdit->setText("");
        selectedDatabaseType = HEHUI::ORACLE;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QOCI")));
        break;

    case HEHUI::M$SQLSERVER:
        ui->stackedWidget->setCurrentWidget(ui->networkDatabasePage);
        ui->networkDatabaseGroupbox->setTitle(tr("M$ SQL Server"));
        ui->userNameEdit->setText("sa");
        //ui->portSpinBox->setValue(1433);
        ui->portSpinBox->setToolTip(tr("Default Port:1433"));
        selectedDatabaseType = HEHUI::M$SQLSERVER;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QODBC")));
        break;

    case HEHUI::M$ACCESS:
        ui->stackedWidget->setCurrentWidget(ui->localDatabaseFilePage);
        ui->localDatabaseGroupbox->setTitle(tr("M$ Access"));
        selectedDatabaseType = HEHUI::M$ACCESS;
        ui->driverCombo->setCurrentIndex(ui->driverCombo->findText(QString("QODBC")));
        break;

    case HEHUI::OTHER:
        ui->stackedWidget->setCurrentWidget(ui->networkDatabasePage);
        ui->networkDatabaseGroupbox->setTitle(tr("Other"));
        ui->portSpinBox->setValue(0);
        ui->portSpinBox->setToolTip(tr("Database Server Port"));
        ui->userNameEdit->setText("root");
        selectedDatabaseType = HEHUI::OTHER;
        ui->driverCombo->setCurrentIndex(0);
        break;

    default:
        break;

    }

    //其它设置
    //Other settings
    on_driverCombo_currentIndexChanged(ui->driverCombo->currentText());

}

void DatabaseConnecterDialog::on_driverCombo_currentIndexChanged(const QString & text){

    //根据所选的驱动类型设置ui->stackedWidget的当前激活页的控件
    //Setup the component in the active page of the ui->stackedWidget according to the selected database driver type
    QString driverText = text;
    if(driverText == QString("QODBC")){
#ifndef Q_OS_WIN
        ui->hostnameEdit->setText(tr("localhost"));
        ui->hostnameEdit->setEnabled(false);
        ui->portSpinBox->setValue(0);
        ui->portSpinBox->setEnabled(false);
        ui->databaseNameLabel->setText(tr("DSN:"));
        return;
#else

#endif

    }


    ui->hostnameEdit->setEnabled(true);
    ui->portSpinBox->setEnabled(true);
    ui->databaseNameLabel->setText(tr("&Database:"));

}

void DatabaseConnecterDialog::on_okButton_clicked() {
    ui->okButton->setEnabled(false);

    if (selectedDatabaseType == HEHUI::MYSQL || selectedDatabaseType == HEHUI::M$SQLSERVER || selectedDatabaseType == HEHUI::OTHER) {

        if (userName().isEmpty()||remoteDatabaseName().isEmpty()||hostName().isEmpty()) {
            QMessageBox::information(this, tr("Error"), tr("<b>Please fill in every required information!</b>"));
            ui->hostnameEdit->setFocus();
            ui->okButton->setEnabled(true);
            return;

        }

    } else if (/*selectedDatabaseType == HEHUI::SQLITE ||*/ selectedDatabaseType == HEHUI::M$ACCESS) {

        if (!QFileInfo(localDatabaseFilePath()).exists()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("<b>No database file found in your specified path !<br>"
                                     "Please check out and try again !</b>"));
            ui->databaseFilePathComboBox->setFocus();
            ui->okButton->setEnabled(true);
            return;
        }

    }


    /*

    if (addConnection()) {
                accept();
    } else {
            ui->okButton->setEnabled(true);
            return;
    }

    ui->okButton->setEnabled(true);
*/


    //    parameters.clear();
    //    parameters<<connectionName()
    //            <<driverName()
    //            <<hostName()
    //            <<QString::number(port())
    //            <<userName()
    //            <<password()
    //            <<databaseName()
    //            <<QString::number((uint)selectedDatabaseType)
    //            ;

    accept();

}


/*

bool DatabaseConnecterDialog::addConnection() {

    //QEventLoop eventLoop;
    //connect(databaseUtility, SIGNAL(signalDatabaseConnectionFinished(const QString &, const QSqlError &)), &eventLoop, SLOT(quit()));
    //eventLoop.exec();

    //根据不同的数据库类型使用不同的方法建立连接
    //use specific method to create database connection according to the selected database type
    QSqlError err;

    err = databaseUtility->openDatabase(connectionName(),
            driverName(),
            hostName(),
            port(),
            userName(),
            password(),
            databaseName(),
            selectedDatabaseType
            );


    if (err.type() != QSqlError::NoError) {
            QMessageBox::critical(this, tr("Fatal Error"),
                    tr("An error occured while opening the connection:<br>%1").arg(err.text()));
            return false;
    } else {
        ui->passwordEdit->clear();
            return true;
    }





        return false;

}

*/

