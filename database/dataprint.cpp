/*
 ****************************************************************************
 * dataprint.cpp
 *
 * Created on: 2009-8-14
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
 * Last Modified on: 2010-05-14
 * Last Modified by: 贺辉
 ***************************************************************************
 */




#include <QFile>
#include <QMessageBox>
#include <QByteArray>
#include <QTextCodec>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QFileDialog>
#include <QDebug>


#include "dataprint.h"


namespace HEHUI{

DataPrint::DataPrint(const QString &fileName, bool preview, QObject *parent)
    :QObject(parent)
{

    document = new QTextDocument(this);

    if (!loadFile(fileName)) {
        return;
    }


    if(preview){
        slotDataPrintPreview();
    }else{
        slotDataPrint();
    }



}

DataPrint::DataPrint(const QString &string, bool isRichText, bool preview, QObject *parent)
    :QObject(parent)
{
    document = new QTextDocument(this);
    if (isRichText) {
        document->setHtml(string);
    } else {
        document->setPlainText(string);
    }

    if(preview){
        slotDataPrintPreview();
    }else{
        slotDataPrint();
    }


}

DataPrint::~DataPrint() {
    document->clear();
    delete document;
    document = 0;

}

bool DataPrint::loadFile(const QString &fileName) {
    if (!QFile::exists(fileName)) {
        QMessageBox::critical(0, tr("Fatal Error"), tr("File '%1' does not exist!").arg(fileName));
        return false;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)){
        QMessageBox::critical(0, tr("Fatal Error"), tr("Can not open file '%1'!").arg(fileName));
        return false;
    }

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    document->clear();
    if (Qt::mightBeRichText(str)) {
        document->setHtml(str);
    } else {
        //str = QString::fromLocal8Bit(data);
        document->setPlainText(str);
    }
    return true;
}

void DataPrint::slotDataPrint() {
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, 0);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
        document->print(&printer);
    }
    delete dlg;
    dlg = 0;
#endif

}

void DataPrint::slotDataPrintPreview() {
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, 0);
    connect(&preview, SIGNAL(paintRequested(QPrinter *)),
            SLOT(slotPrintPreview(QPrinter *)));
    preview.exec();

#endif

}

void DataPrint::slotPrintPreview(QPrinter *printer) {
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    document->print(printer);
#endif
}

void DataPrint::slotFilePrintPdf() {
#ifndef QT_NO_PRINTER
    QString fileName = QFileDialog::getSaveFileName(0, "Export PDF", QString(), "*.pdf");
    if (!fileName.isEmpty()) {
        if (QFileInfo(fileName).suffix().isEmpty())
            fileName.append(".pdf");
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        document->print(&printer);
    }
#endif
}











} //namespace HEHUI


