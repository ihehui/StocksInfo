#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QWidget>
#include <QTableView>
#include <QDialog>
#include <QMessageBox>
#include <QFile>

#include "../guilib.h"


namespace Ui {
class DataOutputDialogUI;
}

namespace HEHUI {

class GUI_LIB_API DataOutputDialog: public QDialog
{
    Q_OBJECT

    enum FileFormate{FILE_FORMATE_TEXT, FILE_FORMATE_XML, FILE_FORMATE_SQLSCRIPT};
    enum FileSeparator{FILE_SEPARATOR_VRTICAL, FILE_SEPARATOR_COMMA, FILE_SEPARATOR_TAB, FILE_SEPARATOR_OTHER};

public:
    enum IOType{EXPORT, IMPORT, PRINT};

    DataOutputDialog(QTableView *tableView, IOType ioType = EXPORT, QWidget *parent = 0);
    ~DataOutputDialog();


    QTableView *getTableView(){return tableView;}
    void setTableView(QTableView *tv){tableView = tv;}


protected:
    void languageChange();

private:
    void initUI();

    bool exportResult(const QString &fileName);
    bool exportResultAsText(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList);
    bool exportResultAsXML(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList);
    bool generateXHTML(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList);

    bool exportResultAsSQLScript(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList);


    bool getFilePath();

private slots:
    void on_separatorComboBox_activated ( int index);
    void on_formatComboBox_activated ( int index );
    void on_browseButton_clicked();

    void on_executeButton_clicked();
    void on_cancelButton_clicked() { reject(); }

private:
    Ui::DataOutputDialogUI *ui;

    QTableView *tableView;

    FileFormate curFileFormate;
    FileSeparator curFileSeparator;

    QList <QPair<QString, FileFormate> >supportedFileFormates;
    QList <QPair<QString, FileSeparator> >supportedFileSeparators;

    QString resultString;

    QString fileSuffix;
    QString curFileSeparatorString;

    IOType ioType;

};

} //namespace HEHUI

#endif

