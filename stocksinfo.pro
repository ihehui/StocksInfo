
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = stocksinfo
TEMPLATE = app

SOURCES += main.cpp\
    qcustomplot.cpp \
    candlesticksview.cpp \
    tradesummaryinfoview.cpp \
    tradeinfo.cpp \
    common.cpp \
    indicatorview.cpp \
    datamanager.cpp \
    mainwindow.cpp \
    downloadmanager.cpp \
    stock.cpp

HEADERS  += qcustomplot.h \
    candlesticksview.h \
    tradesummaryinfoview.h \
    tradeinfo.h \
    common.h \
    indicatorview.h \
    datamanager.h \
    mainwindow.h \
    downloadmanager.h \
    stock.h

FORMS += \
    tradesummaryinfoview.ui \
    mainwindow.ui



