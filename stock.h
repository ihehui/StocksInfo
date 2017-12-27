#ifndef STOCK_H
#define STOCK_H

#include <QObject>
#include <QMutex>
#include "common.h"


class QCPFinancialData;

class Category
{
public:
  Category();
  Category(quint32 id, const QString &name, const QString &systemCode, const QStringList &stocks = QStringList());
  ~Category();

  quint32 id;
  QString name;
  QString systemCode;
  QStringList stocks; //Stock code list
};



class TradeExtraData
{
public:
  TradeExtraData();
  TradeExtraData(uint time, double preClose, double volume, double turnover, double exchangeRatio);

  uint time; //time_t
  double preClose;
  double volume;
  double turnover;
  double exchangeRatio;

};

class RealTimeQuoteData
{
public:
  RealTimeQuoteData();

  QString code;
  QString time;

  double ask1, ask2, ask3, ask4, ask5;
  double askVol1, askVol2, askVol3, askVol4, askVol5;
  double bid1, bid2, bid3, bid4, bid5;
  double bidVol1, bidVol2, bidVol3, bidVol4, bidVol5;

};

class RealTimeStatisticsData
{
public:
  RealTimeStatisticsData();

  QString time;
  double open, high, low, price, change, changePercent, yestClose;
  double volume, turnover, exchangeRatio;
  double tradableMarketCap, marketCap, pe, earnings;
  double volChangeRatio, orderChangeRatio;
  double fiveMinsChange;
};

class Stock : public QObject
{
    Q_OBJECT
public:
    explicit Stock(const QString &code, const QString &name ="", QObject *parent = 0);
    ~Stock();

    QString code() const;
    QString name() const;
    void setName(const QString &name);
    RealTimeStatisticsData * realTimeStatisticsData();

    PeriodType periodType() const;
    QMap<double, QCPFinancialData> * ohlcDataMap();
    QMap<double, TradeExtraData> * tradeExtraDataMap();
    QVector<double> * futuresDeliveryDates(); //TODO:optimize

public slots:
    void clear();


signals:

private:
    QMutex mutex;

    QString m_code; //证券代码
    QString m_name; //证券名称
    RealTimeStatisticsData *m_realTimeStatisticsData;

    PeriodType m_periodType; //数据周期类型
    QMap<double, QCPFinancialData> *m_ohlcData; //基本交易数据
    QMap<double, TradeExtraData> *m_tradeExtraData; //交易数据
    QVector<double> *m_futuresDeliveryDates; //index. Futures delivery,期指交割日，忽略放假顺延

};

#endif // STOCK_H
