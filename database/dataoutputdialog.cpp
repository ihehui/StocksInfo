#include <QtGui>
#include <QtSql>
#include <QStandardItemModel>
#include <QApplication>
#include <QDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QWidget>
#include <QRegExp>

#include "dataoutputdialog.h"
#include "ui_dataoutputdialog.h"

//#include "../../../shared/gui/dataprint.h"
#include "HHSharedGUI/hdataprint.h"


namespace HEHUI{

DataOutputDialog::DataOutputDialog(QTableView *tableView, IOType ioType, QWidget *parent)
    :QDialog(parent), tableView(tableView), ioType(ioType),
      ui(new Ui::DataOutputDialogUI)
{
    ui->setupUi(this);

    initUI();

}


DataOutputDialog::~DataOutputDialog() {
    delete ui;
}

void DataOutputDialog::languageChange()
{
    ui->retranslateUi(this);
}


void DataOutputDialog::initUI(){

    if(ioType == EXPORT){
        setWindowTitle(tr("Export"));

        //初始化所支持的文件格式列表
        //Init supported FileFormates list
        supportedFileFormates.append(qMakePair(QString("Text"), FILE_FORMATE_TEXT));
        supportedFileFormates.append(qMakePair(QString("XML(XHTML)"), FILE_FORMATE_XML));
        supportedFileFormates.append(qMakePair(QString("SQL Script"), FILE_FORMATE_SQLSCRIPT));

        //填充文件格式ComboBox
        //Setup FileFormates combobox
        for(int i=0; i<supportedFileFormates.size(); i++){
            QPair<QString, FileFormate> pair = supportedFileFormates.at(i);
            ui->formatComboBox->addItem(pair.first, pair.second);
        }
        on_formatComboBox_activated(ui->formatComboBox->currentIndex());


        //初始化所支持的文件分隔符列表
        //Init supported FileSeparators list
        supportedFileSeparators.append(qMakePair(QString(tr("Tab('	')")), FILE_SEPARATOR_TAB));
        supportedFileSeparators.append(qMakePair(QString(tr("Comma(',')")), FILE_SEPARATOR_COMMA));
        supportedFileSeparators.append(qMakePair(QString(tr("Vertical('|')")), FILE_SEPARATOR_VRTICAL));
        supportedFileSeparators.append(qMakePair(QString(tr("Other")), FILE_SEPARATOR_OTHER));

        //填充文件分隔符ComboBox
        //Setup FileSeparators combobox
        for(int i=0; i<supportedFileSeparators.size(); i++){
            QPair<QString, FileSeparator> pair = supportedFileSeparators.at(i);
            ui->separatorComboBox->addItem(pair.first, pair.second);
        }
        on_separatorComboBox_activated(ui->separatorComboBox->currentIndex());

        ui->progressBar->hide();

        ui->printDataSettingsFrame->hide();

    }else if(ioType == PRINT){
        setWindowTitle(tr("Print"));
        ui->exportDataSettingsFrame->hide();
        ui->progressBar->hide();
    }

}


bool DataOutputDialog::exportResult(const QString &fileName) {
    QList <QStringList> dataList;
    QStringList headerDataList;



    //QTextStream out(&file);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QAbstractItemModel *model = getTableView()->model();
    int columnCount = model->columnCount();
    int rowCount = model->rowCount();
    for(int i=0; i<columnCount; i++){
        headerDataList << model->headerData(i,Qt::Horizontal).toString();
    }

    if(ui->rowscomboBox->currentIndex() == 0){
        ui->progressBar->show();
        ui->progressBar->setRange(0, rowCount);

        for (int i = 0; i < rowCount; ++i) {
            QModelIndex index = model->index(i, 0);
            int row = index.row();
            QStringList list;
            for(int j = 0; j < columnCount; j++){
                QModelIndex idx =  index.sibling(row,j);
                list << idx.data().toString();
            }

            dataList<<list;

            ui->progressBar->setValue(i);
            qApp->processEvents();

        }



    }else{

        QModelIndexList selectedIndexes = getTableView()->selectionModel()->selectedIndexes();
        int selectedIndexesCount = selectedIndexes.count();

        ui->progressBar->show();
        ui->progressBar->setRange(0, selectedIndexesCount);

        for (int j = 0; j < selectedIndexesCount; ++j) {
            QModelIndex index = selectedIndexes.at(j);
            if (selectedIndexes.at(j).column() != 0){
                continue;
            }
            int row = index.row();
            QStringList list;
            for(int i = 0; i < columnCount; i++){
                QModelIndex idx =  index.sibling(row,i);
                list << idx.data().toString();
            }

            dataList<<list;

            ui->progressBar->setValue(j);
            qApp->processEvents();

        }

    }

    ui->progressBar->reset();
    ui->progressBar->hide();
    QApplication::restoreOverrideCursor();

    if(ioType == PRINT){
        QTemporaryFile tempFile;
        if(tempFile.open()){
            if(generateXHTML(&tempFile, headerDataList, dataList)){
                //qDebug()<<"Temporary File Name:"<<tempFile.fileName();
                DataPrint dataPrint(tempFile.fileName(), true, this);
            }
        }
        tempFile.close();
        return true;
    }


    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::critical(this, tr("Fatal Error"), tr("Cannot write file %1:<br>%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
        return false;
    }

    switch(curFileFormate){
    case FILE_FORMATE_TEXT:
        return exportResultAsText(&file, headerDataList, dataList);
        break;
    case FILE_FORMATE_XML:
        return exportResultAsXML(&file, headerDataList, dataList);
        break;
    case FILE_FORMATE_SQLSCRIPT:
        return exportResultAsSQLScript(&file, headerDataList, dataList);
        break;

    }

    file.flush();
    return false;

}

bool DataOutputDialog::exportResultAsText(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList){


    QTextStream out(file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    out.setGenerateByteOrderMark(true);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->progressBar->show();
    ui->progressBar->setRange(0, dataList.size());

    out << headerDataList.join(QString("%1").arg(curFileSeparatorString))<<"\n";

    for (int i = 0; i < dataList.size(); i++) {
        QStringList list = dataList.at(i);

        out << list.join(QString("%1").arg(curFileSeparatorString))<<"\n";

        ui->progressBar->setValue(i);
        qApp->processEvents();

    }

    QApplication::restoreOverrideCursor();



    return true;

}

bool DataOutputDialog::exportResultAsXML(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList){

    return generateXHTML(file, headerDataList, dataList);

}

bool DataOutputDialog::generateXHTML(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList){


    QXmlStreamWriter stream(file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument(); //<?xml version="1.0" encoding="UTF-8"?>
    stream.writeDTD("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">");

    stream.writeStartElement("html");
    stream.writeAttribute("xmlns", "http://www.w3.org/1999/xhtml");




    stream.writeStartElement("head");

    stream.writeStartElement("meta");
    stream.writeAttribute("http-equiv", "Content-Type");
    stream.writeAttribute("content", "text/html; charset=utf-8");
    stream.writeEndElement();

    stream.writeStartElement("title");
    stream.writeCharacters("Data");
    stream.writeEndElement();

    stream.writeEndElement();//-------head



    stream.writeStartElement("body");

    //stream.writeStartElement("div");
    //stream.writeAttribute("align", "center");
    //stream.writeCharacters("$TITLE");
    //stream.writeEndElement();//-------div

    stream.writeStartElement("table");
    stream.writeAttribute("width", "100%");
    stream.writeAttribute("border", "1");
    stream.writeAttribute("cellspacing", "0");
    stream.writeAttribute("cellpadding", "0");
    stream.writeAttribute("bordercolor", "#00FF00FF");


    stream.writeStartElement("tr");//Table Header 表头
    for(int k=0; k<headerDataList.size(); k++){
        stream.writeStartElement("td");
        stream.writeCharacters(headerDataList.at(k));
        stream.writeEndElement();//-------td
        QApplication::processEvents();
    }
    stream.writeEndElement();//-------tr Table Header 表头

    for(int i = 0; i< dataList.size(); i++){
        stream.writeStartElement("tr");
        QStringList list = dataList.at(i);
        for(int j=0; j<list.size(); j++){
            stream.writeStartElement("td");
            stream.writeCharacters(list.at(j));
            stream.writeEndElement();//-------td
            QApplication::processEvents();
        }
        stream.writeEndElement();//-------tr
    }

    stream.writeEndElement();//-------table

    stream.writeEndElement();//-------body




    stream.writeEndElement();//-------html
    stream.writeEndDocument();

    file->flush();
    return true;

}


bool DataOutputDialog::exportResultAsSQLScript(QFile *file, QStringList &headerDataList, QList <QStringList> &dataList){
    //TODO:改进

    QTextStream out(file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    out.setGenerateByteOrderMark(true);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->progressBar->show();
    ui->progressBar->setRange(0, dataList.size());

    out << "INSERT INTO `data` ("<<headerDataList.join(",")<<") VALUES "<<"\n";

    int dataListSize = dataList.size();
    for (int i = 0; i < dataListSize; i++) {
        QStringList list = dataList.at(i);

        if(i < dataListSize-1){
            out << "(" <<list.join(",")<<"), "<<"\n";
        }else{
            out << "(" <<list.join(",")<<"); "<<"\n";
        }


        ui->progressBar->setValue(i);
        qApp->processEvents();

    }



    QApplication::restoreOverrideCursor();



    return true;




}


bool DataOutputDialog::getFilePath() {

    //QString defaultDataFileSavePath = QApplication::applicationDirPath()+QDir::separator() + "data" + QDateTime::currentDateTime().toString(Qt::ISODate);

    //QString fileName = QFileDialog::getSaveFileName(this, tr("Data Save Path:"), dataFileSavePath, tr("Text (*.txt);;CSV (*.csv);;SQL Script (*.sql);;All(*.*)"));

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Data File Path:"), "", tr("%1All(*.*)").arg(fileSuffix));

    if (!fileName.isEmpty()) {
        ui->filePathComboBox->insertItem(0, fileName);
        ui->filePathComboBox->setCurrentIndex(0);
    } else {
        return false;
    }

    return true;

}




void DataOutputDialog::on_separatorComboBox_activated(int index) {
    uint separatorType = ui->separatorComboBox->itemData(index).toUInt();
    switch(separatorType){
    case FILE_SEPARATOR_VRTICAL:
        curFileSeparatorString = "|";
        break;
    case FILE_SEPARATOR_COMMA:
        curFileSeparatorString = ",";
        break;
    case FILE_SEPARATOR_TAB:
        curFileSeparatorString = "\t";
        break;
    case FILE_SEPARATOR_OTHER:
        bool ok;
        QString newSeparator = QInputDialog::getText(this, tr("New Separator"),
                                                     tr("New Separator:"), QLineEdit::Normal,
                                                     QDir::home().dirName(), &ok);
        if (ok && !newSeparator.isEmpty()) {
            int separatorIndex =ui->separatorComboBox->findText(newSeparator,
                                                                Qt::MatchFixedString | Qt::MatchCaseSensitive);
            if(separatorIndex < 0){
                ui->separatorComboBox->insertItem(0, newSeparator, FILE_SEPARATOR_OTHER);
                ui->separatorComboBox->setCurrentIndex(0);
                curFileSeparatorString = newSeparator;
            }else{
                ui->separatorComboBox->setCurrentIndex(0);
                curFileSeparatorString = ui->separatorComboBox->itemText(separatorIndex);
            }

        }

        break;

    }



}

void DataOutputDialog::on_formatComboBox_activated(int index) {

    uint formatType = ui->formatComboBox->itemData(index).toUInt();

    switch(formatType){
    case FILE_FORMATE_TEXT:
        ui->separatorLabel->setEnabled(true);
        ui->separatorComboBox->setEnabled(true);
        ui->notesLineEdit->setEnabled(false);
        curFileFormate = FILE_FORMATE_TEXT;
        fileSuffix = "Text (*.txt);;";
        break;
    case FILE_FORMATE_XML:
        ui->separatorLabel->setEnabled(false);
        ui->separatorComboBox->setEnabled(false);
        ui->notesLineEdit->setEnabled(true);
        curFileFormate = FILE_FORMATE_XML;
        fileSuffix = "XML (*.xml);;";
        break;
    case FILE_FORMATE_SQLSCRIPT:
        ui->separatorLabel->setEnabled(false);
        ui->separatorComboBox->setEnabled(false);
        ui->notesLineEdit->setEnabled(true);
        curFileFormate = FILE_FORMATE_SQLSCRIPT;
        fileSuffix = "SQL Script (*.sql);;";
        break;

    }




}

void DataOutputDialog::on_browseButton_clicked() {
    getFilePath();

}

void DataOutputDialog::on_executeButton_clicked() {
    ui->executeButton->setEnabled(false);

    if(ioType == EXPORT){

        if(ui->filePathComboBox->currentText().isEmpty()){
            if(!getFilePath()){
                ui->executeButton->setEnabled(true);
                return;
            }
        }

        if (QFileInfo(ui->filePathComboBox->currentText()).exists()) {
            QMessageBox::StandardButton ret;
            ret = QMessageBox::question(this, tr("File Already Exists"), tr(
                                            "<b>File already exists!<br>"
                                            "Overwrite?</b>"), QMessageBox::Yes | QMessageBox::No
                                        | QMessageBox::Cancel, QMessageBox::No);

            if (ret == QMessageBox::No) {
                if(!getFilePath()){return;}
            }else if(ret == QMessageBox::Cancel){
                ui->executeButton->setEnabled(true);
                return;
            }

        }

    }

    if(exportResult(ui->filePathComboBox->currentText())){
        accept();
    }



}



















} //namespace HEHUI


