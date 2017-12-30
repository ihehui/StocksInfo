#ifndef QCPCANDLETICKER_H
#define QCPCANDLETICKER_H

#include "qcustomplot.h"

class QCPCandleTickerNum : public QCPAxisTicker
{
public:
    QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision);
};

class QCPCandleTickerDateTime : public QCPAxisTickerDateTime
{
public:
    QString getTickLabel(double tick, const QLocale &locale);
};

#endif // QCPCANDLETICKER_H
