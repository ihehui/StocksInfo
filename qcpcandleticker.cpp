#include "qcpcandleticker.h"

QString QCPCandleTickerNum::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision)
{
    return QCPAxisTicker::getTickLabel(tick, locale, formatChar, precision);
}

QString QCPCandleTickerDateTime::getTickLabel(double tick, const QLocale &locale)
{
    return QCPAxisTickerDateTime::getTickLabel(tick, locale, 'g', 6);
}
