#include "stock.h"

#include "qcustomplot.h"



Stock::Stock(QObject *parent) : QObject(parent)
{
    stockCode = "";
    stockName = "";

    ohlcData = new QMap<double, QCPFinancialData>();
    tradeExtraData = new QMap<double, TradeExtraData>();
}
