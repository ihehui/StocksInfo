#include "datamanager.h"

#include "qcustomplot.h"
#include "tradeinfo.h"
#include "common.h"

#include "downloadmanager.h"


QMap<QString, QString> DataManager::m_allStocksMap = QMap<QString, QString>();

DataManager::DataManager(QObject *parent) : QThread(parent)
{
    m_stockCode = "";
    m_stockName = "";

    m_ohlcData = new QMap<double, QCPFinancialData>();
    m_tradeExtraData = new QMap<double, TradeExtraData>();

    if(m_allStocksMap.isEmpty()){
        readStocksList();
    }

    m_downloadManager = new DownloadManager(this);
    connect(m_downloadManager, SIGNAL(dataDownloaded(const QString &, const QUrl &)), this, SLOT(dataDownloaded(const QString &, const QUrl &)));

    m_localSaveDir = QApplication::applicationDirPath()+"/data";
}

DataManager::~DataManager(){
//    delete m_ohlcData; //deleted by QCPFinancial
    delete m_tradeExtraData;
    delete m_downloadManager;
}

bool DataManager::readHistoricalData(QString *code, int offset){
    if(!code){return false;}
    QString newCode = *code;

    int size = m_allStocksMap.size();
    int newOffset = offset%size;
    if(!m_allStocksMap.contains(newCode)){return false;}
    if(newOffset){
        QStringList allStocks = m_allStocksMap.keys();
        int index = allStocks.indexOf(newCode);
        index += newOffset;

        if(newOffset > 0 && index > size){
            index = newOffset - (size - allStocks.indexOf(newCode));
        }else if(index < 0){
            index = size - (newOffset - allStocks.indexOf(newCode)) - 1;
        }
        newCode = allStocks.at(index);
        *code = newCode;
    }

    QFile file(m_localSaveDir+"/"+newCode+".csv");
    if(!file.exists()){
        downloadData(newCode);
        return false;
    }
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug()<<file.errorString();
        return false;
    }

    QTextStream in(&file);
    //表头
    QString title = in.readLine();
    if(title.trimmed().isEmpty()){
        return false;
    }
    qDebug()<<title;

    m_stockCode = newCode;
    m_stockName = "";
    m_ohlcData->clear();
    m_tradeExtraData->clear();
    m_futuresDeliveryDates.clear();


    QVector< double >timeVector, openVector, highVector, lowVector, closeVector, volVector;
    uint index = (std::numeric_limits<uint>::max)();
    while (!in.atEnd()) {
        QStringList dataList = in.readLine().split(",");
        if(dataList.size()<12){
            qDebug()<<"Invalid column count!";
            return false;
        }

        if(m_stockName.isEmpty()){
            m_stockName = dataList.at(2);
        }

        double open = dataList.at(6).toDouble();
        if(isZero(open)){continue;} //停牌
        openVector.append(open);

        QDateTime dateTime = QDateTime::fromString(dataList.at(0), "yyyy-MM-dd");
        dateTime.setTime(QTime(15, 0));
        uint time_t = dateTime.toTime_t();
        QDate date = dateTime.date();
        if(date.dayOfWeek() == 5){
            //Futures delivery,期指交割日，忽略放假顺延
            QDate firstDay = date.addDays(-(date.day()-1));
            int firstFridy = 0;
            int dayofWeek = firstDay.dayOfWeek();
            if(dayofWeek > 5){
                firstFridy = 7-dayofWeek+5+1;
            }else{
                firstFridy = 5-dayofWeek+1;
            }
            if(date.day() == firstFridy+14){
                m_futuresDeliveryDates.append(index);
            }
        }


        double high = dataList.at(4).toDouble();
        highVector.append(high);

        double low = dataList.at(5).toDouble();
        lowVector.append(low);

        double close = dataList.at(3).toDouble();
        closeVector.append(close);


        double preClose = dataList.at(7).toDouble();

        double volume_Hand = dataList.at(11).toDouble();
        volVector.append(volume_Hand);

        double turnover = dataList.at(12).toDouble();
        double turnoverRate = dataList.at(10).toDouble();

        m_ohlcData->insert(index, QCPFinancialData(index, open, high, low, close));
        m_tradeExtraData->insert(index, TradeExtraData(time_t, preClose, volume_Hand, turnover, turnoverRate));

        //数据文件为倒序。不可使用日期做KEY，日期有空档。
        index--;
        qApp->processEvents();
    }

    emit historicalDataRead(newCode);

    return true;
}

bool DataManager::downloadData(const QString &code){
    qDebug()<<"-----DataManager::downloadData(...)";
    //m_downloadManager->append(QUrl(QString("http://quotes.money.163.com/service/chddata.html?code=%1&start=20000720&end=20150508")));
    m_downloadManager->append(QUrl(QString("http://quotes.money.163.com/service/chddata.html?code=%1%2").arg(code.startsWith("6")?"0":"1").arg(code)));
    return true;
}

void DataManager::dataDownloaded(const QString &fileName, const QUrl &url){
    qDebug()<<"---DataManager::dataDownloaded(...)"<<" fileName:"<<fileName;
    QFileInfo fi(fileName);
    QString code = fi.baseName();

    readHistoricalData(&code, 0);
}

void DataManager::readStocksList(){
    m_allStocksMap.clear();

    QFile file(QApplication::applicationDirPath()+"/data/AllStocks.csv");
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug()<<file.errorString();
        m_allStocksMap.insert("000001", "\345\271\263\345\256\211\351\223\266\350\241\214");
        m_allStocksMap.insert("000002", "\344\270\207  \347\247\221\357\274\241");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList dataList = in.readLine().split(",");
        if(dataList.size()!=2){
            continue;
        }
        m_allStocksMap.insert(dataList.at(0), dataList.at(1));
    }

    if(m_allStocksMap.isEmpty()){
        m_allStocksMap.insert("000001", "\345\271\263\345\256\211\351\223\266\350\241\214");
        m_allStocksMap.insert("000002", "\344\270\207  \347\247\221\357\274\241");
    }


}

void DataManager::run(){
    exec();
}
