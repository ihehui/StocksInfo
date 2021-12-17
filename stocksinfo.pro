
QT       += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = stocksinfo
TEMPLATE = app
#QTPLUGIN += qsqlite qsqlmysql

SOURCES += main.cpp\
    qcustomplot.cpp \
    tradesummaryinfoview.cpp \
    common.cpp \
    datamanager.cpp \
    mainwindow.cpp \
    downloadmanager.cpp \
    stock.cpp \
    stockstablemodel.cpp \
    stockstableview.cpp \
    database/databaseconnecter.cpp \
    database/databaseconnecterdialog.cpp \
    database/databaseutility.cpp \
    database/dataoutputdialog.cpp \
    database/dataprint.cpp \
    candlesticksview.cpp \
    qcpcandlechart.cpp \
    qcpcandleticker.cpp

HEADERS  += qcustomplot.h \
    tradesummaryinfoview.h \
    common.h \
    datamanager.h \
    mainwindow.h \
    downloadmanager.h \
    stock.h \
    stockstablemodel.h \
    stockstableview.h \
    database/databaseconnecter.h \
    database/databaseconnecterdialog.h \
    database/databaseutility.h \
    database/dataoutputdialog.h \
    database/dataprint.h \
    candlesticksview.h \
    qcpcandlechart.h \
    qcpcandleticker.h

FORMS += \
    tradesummaryinfoview.ui \
    mainwindow.ui \
    database/databaseconnecterdialog.ui \
    database/dataoutputdialog.ui

RESOURCES += \
    resource.qrc



