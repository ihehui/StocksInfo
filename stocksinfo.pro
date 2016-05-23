
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = stocksinfo
TEMPLATE = app

SOURCES += main.cpp\
    qcustomplot.cpp \
    candlesticksview.cpp \
    tradesummaryinfoview.cpp \
    common.cpp \
    datamanager.cpp \
    mainwindow.cpp \
    downloadmanager.cpp \
    stock.cpp \
    stockstablemodel.cpp

HEADERS  += qcustomplot.h \
    candlesticksview.h \
    tradesummaryinfoview.h \
    common.h \
    datamanager.h \
    mainwindow.h \
    downloadmanager.h \
    stock.h \
    stockstablemodel.h

FORMS += \
    tradesummaryinfoview.ui \
    mainwindow.ui



