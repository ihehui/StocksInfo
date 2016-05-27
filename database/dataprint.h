/*
 ****************************************************************************
 * dataprint.h
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




#ifndef DATAPRINT_H_
#define DATAPRINT_H_

#include <QObject>
#include <QTextDocument>


class QPrinter;


namespace HEHUI{

class DataPrint : public QObject{
    Q_OBJECT
public:
    DataPrint(const QString &fileName, bool preview, QObject *parent = 0);

    DataPrint(const QString &string, bool isRichText, bool preview, QObject *parent = 0);
    virtual ~DataPrint();

private:
    bool loadFile(const QString &fileName);


public slots:
    void slotDataPrint();
    void slotDataPrintPreview();
    void slotFilePrintPdf();

private slots:
    void slotPrintPreview(QPrinter *printer);

private:
    QTextDocument *document;


};

} //namespace HEHUI

#endif /* DATAPRINT_H_ */
