#ifndef STOCK_H
#define STOCK_H

#include <QObject>
#include "common.h"

class QCPFinancialData;
class TradeExtraData;

class Stock : public QObject
{
    Q_OBJECT
public:
    explicit Stock(QObject *parent = 0);

signals:

public:
    QString code; //证券代码
    QString name; //证券名称

    PeriodType periodType; //数据周期类型
    QMap<double, QCPFinancialData> *ohlcData; //基本交易数据
    QMap<double, TradeExtraData> *tradeExtraData; //交易数据


};

#endif // STOCK_H
