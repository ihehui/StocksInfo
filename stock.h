#ifndef STOCK_H
#define STOCK_H

#include <QObject>

#include "common.h"


class QCPFinancialData;


class TradeExtraData
{
public:
  TradeExtraData();
  TradeExtraData(uint time, double preClose, double volume_Hand, double turnover, double turnoverRate);

  uint time; //time_t
  double preClose;
  double volume_Hand;
  double turnover;
  double turnoverRate;

};

class RealTimeData
{
public:
  RealTimeData();

  QString code;
  QString time;

  double ask1, ask2, ask3, ask4, ask5;
  double askVol1, askVol2, askVol3, askVol4, askVol5;
  double bid1, bid2, bid3, bid4, bid5;
  double bidVol1, bidVol2, bidVol3, bidVol4, bidVol5;
  double open, high, low, price, change, changePercent;
  double volume_Hand, turnover;

};


class Stock : public QObject
{
    Q_OBJECT
public:
    explicit Stock(const QString &code, const QString &name ="", QObject *parent = 0);
    ~Stock();

    QString code() const;
    QString name() const;
    PeriodType periodType() const;
    QMap<double, QCPFinancialData> * ohlcDataMap();
    QMap<double, TradeExtraData> * tradeExtraDataMap();
    QVector<double> * futuresDeliveryDates(); //TODO:optimize
    //RealTimeData * realTimeData();

public slots:
    void clear();

    //JSON API:http://api.money.126.net/data/feed/1000001,money.api
    void setRealTimeData(const QString &jsonString);

signals:

public:
    QMutex mutex;

    QString m_code; //证券代码
    QString m_name; //证券名称

    PeriodType m_periodType; //数据周期类型
    QMap<double, QCPFinancialData> *m_ohlcData; //基本交易数据
    QMap<double, TradeExtraData> *m_tradeExtraData; //交易数据
    QVector<double> *m_futuresDeliveryDates; //index. Futures delivery,期指交割日，忽略放假顺延

//    RealTimeData *m_realTimeData; //实时数据

};

#endif // STOCK_H
