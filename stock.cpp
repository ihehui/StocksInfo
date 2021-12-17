﻿#include "stock.h"

#include "qcustomplot.h"


Category::Category():
    id(0),
    name(""),
    systemCode(""),
    stocks(QStringList())
{
}

Category::Category(quint32 id, const QString &name, const QString &systemCode, const QStringList &stocks):
    id(id),
    name(name),
    systemCode(systemCode),
    stocks(stocks)
{
}

Category::~Category(){
    stocks.clear();
}


TradeExtraData::TradeExtraData() :
    time(0),
    preClose(0),
    volume(0),
    turnover(0),
    exchangeRatio(0)
{
}

TradeExtraData::TradeExtraData(uint key, double preClose, double volume_Hand, double turnover, double turnoverRate) :
    time(key),
    preClose(preClose),
    volume(volume_Hand),
    turnover(turnover),
    exchangeRatio(turnoverRate)
{
}

RealTimeQuoteData::RealTimeQuoteData(){
    code = "";
    time = "";
    ask1=ask2=ask3=ask4=ask5=0;
    askVol1=askVol2=askVol3=askVol4=askVol5=0;
    bid1=bid2=bid3=bid4=bid5=0;
    bidVol1=bidVol2=bidVol3=bidVol4=bidVol5=0;
}

RealTimeStatisticsData::RealTimeStatisticsData(){
    time = "";
    open=high=low=price=change=changePercent=yestClose=0;
    volume=turnover=exchangeRatio=0;
    tradableMarketCap=marketCap=pe=earnings=0;
    volChangeRatio=orderChangeRatio=0;
    fiveMinsChange=0;
}



Stock::Stock(const QString &code, const QString &name, QObject *parent)
    : QObject(parent),
      m_code(code),
      m_name(name)
{
    m_realTimeStatisticsData = new RealTimeStatisticsData();
    m_periodType = PERIOD_ONE_DAY;
    m_ohlcData.reset(new QCPFinancialDataContainer);
    m_tradeExtraData = new QMap<double, TradeExtraData>();
    m_futuresDeliveryDates = new QVector<double>();
}

Stock::~Stock(){
    delete m_realTimeStatisticsData;
    m_realTimeStatisticsData = 0;

    m_ohlcData.clear();

    delete m_tradeExtraData;
    m_tradeExtraData = 0;

    delete m_futuresDeliveryDates;
    m_futuresDeliveryDates = 0;
}

QString Stock::code() const{
    return m_code;
}

QString Stock::name() const{
    return m_name;
}

void Stock::setName(const QString &name){
    m_name = name;
}

RealTimeStatisticsData * Stock::realTimeStatisticsData(){
    return m_realTimeStatisticsData;
}

PeriodType Stock::periodType() const{
    return m_periodType;
}

QSharedPointer<QCPFinancialDataContainer> Stock::ohlcDataContainer(){
    QMutexLocker locker(&mutex);
    return m_ohlcData;
}

QMap<double, TradeExtraData> * Stock::tradeExtraDataMap(){
    QMutexLocker locker(&mutex);
    return m_tradeExtraData;
}

QVector<double> * Stock::futuresDeliveryDates(){
    return m_futuresDeliveryDates;
}

//RealTimeData * Stock::realTimeData(){
//    return m_realTimeData;
//}

void Stock::clear(){
    QMutexLocker locker(&mutex);

    m_ohlcData.clear();
    m_tradeExtraData->clear();
}



