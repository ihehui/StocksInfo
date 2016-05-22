#include "stock.h"

#include "qcustomplot.h"


TradeExtraData::TradeExtraData() :
    time(0),
    preClose(0),
    volume_Hand(0),
    turnover(0),
    turnoverRate(0)
{
}

TradeExtraData::TradeExtraData(uint key, double preClose, double volume_Hand, double turnover, double turnoverRate) :
    time(key),
    preClose(preClose),
    volume_Hand(volume_Hand),
    turnover(turnover),
    turnoverRate(turnoverRate)
{
}

RealTimeData::RealTimeData(){
    code = "";
    time = "";
    ask1=ask2=ask3=ask4=ask5=0;
    askVol1=askVol2=askVol3=askVol4=askVol5=0;
    bid1=bid2=bid3=bid4=bid5=0;
    bidVol1=bidVol2=bidVol3=bidVol4=bidVol5=0;
    open=high=low=price=change=changePercent=0;
    volume_Hand=turnover=0;
}



Stock::Stock(const QString &code, const QString &name, QObject *parent)
    : QObject(parent),
      m_code(code),
      m_name(name),
      m_periodType(PERIOD_ONE_DAY)//日线
{

    m_ohlcData = new QMap<double, QCPFinancialData>();
    m_tradeExtraData = new QMap<double, TradeExtraData>();
    m_futuresDeliveryDates = new QVector<double>();

//    m_realTimeData = new RealTimeData();
}

Stock::~Stock(){
    delete m_ohlcData; //May be deleted by QCPFinancial
    delete m_tradeExtraData;
    //delete m_realTimeData;
}

QString Stock::code() const{
    return m_code;
}

QString Stock::name() const{
    return m_name;
}

PeriodType Stock::periodType() const{
    return m_periodType;
}

QMap<double, QCPFinancialData> * Stock::ohlcDataMap(){
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

    m_ohlcData->clear();
    m_tradeExtraData->clear();
}

void Stock::setRealTimeData(const QString &jsonString){

}



