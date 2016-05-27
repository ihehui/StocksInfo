#include "datamanager.h"

#include "qcustomplot.h"
#include "stock.h"

#include "downloadmanager.h"
#include "database/databaseconnecter.h"
#include "database/databaseutility.h"

//Q_IMPORT_PLUGIN(qsqlite)
//Q_IMPORT_PLUGIN(qsqlmysql)

DataManager::DataManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<RealTimeQuoteData>("RealTimeQuoteData");
    qRegisterMetaType<QList<Stock*>>("QList<Stock*>");

    m_localSaveDir = QApplication::applicationDirPath() + "/data";
    localDataFilePath = m_localSaveDir + "/stocks.db";
    m_localDBConnectionName = "LOCAL_STOCKS_DB";
    m_localDBName = "stocks.db";
    m_localDBDriver = "QSQLITE";
    m_remoteDBConnectionName = "REMOTE_STOCKS_DB";
    m_remoteDBDriver = "QMYSQL";
    m_remoteDBServerHost = "127.0.0.1";
    m_remoteDBServerPort = 3306;
    m_remoteDBUserName = "root";
    m_remoteDBUserPassword = "";
    m_remoteDBName = "stocks";

    m_downloadManager = new DownloadManager();
    connect(m_downloadManager, SIGNAL(dataDownloaded(const QString &, const QUrl &)), this, SLOT(historicalDataDownloaded(const QString &, const QUrl &)));
    connect(m_downloadManager, SIGNAL(realTimeQuoteDataReceived(const QByteArray &)), this, SLOT(realTimeQuoteDataReceived(const QByteArray &)));
    connect(m_downloadManager, SIGNAL(realTimeStatisticsDataReceived(const QByteArray &)), this, SLOT(realTimeStatisticsDataReceived(const QByteArray &)));

    connect(this, SIGNAL(requestDownloadData(const QString &)), m_downloadManager, SLOT(requestFileDownload(const QString &)));
    connect(this, SIGNAL(requestRealTimeQuoteData(const QString &)), m_downloadManager, SLOT(requestRealTimeQuoteData(const QString &)));
    connect(this, SIGNAL(requestRealTimeStatisticsData(const QString &)), m_downloadManager, SLOT(requestRealTimeStatisticsData(const QString &)));




    m_allStocks = new QMap<QString, Stock*>();
    m_hsaTotalStocksCount = 0;

    qDebug()<<"DataManager0:"<<QThread::currentThreadId();

   //QTimer::singleShot(0, this, SLOT(loadAllStocks()));

}

DataManager::~DataManager(){
    qDebug()<<"DataManager::~DataManager()";

//    delete m_ohlcData; //deleted by QCPFinancial
//    delete m_tradeExtraData;
    delete m_downloadManager;

    qDeleteAll(m_allStocks->begin(), m_allStocks->end());
    delete m_allStocks;
    m_allStocks = 0;

}

Stock * DataManager::stock(const QString &code) const{
    Stock *stock = m_allStocks->value(code);
    if(!stock){
        qDebug()<<QString("Stock '%1' Not Found!").arg(code);
    }
    return stock;
}

QMap<QString, Stock*> * DataManager::allStocks() const{
    return m_allStocks;
}

QList<Stock*> DataManager::categoryStocks(quint32 categoryID) const{
    qDebug()<<"--DataManager::categoryStocks()";
    if(categoryID == 0){
        return m_allStocks->values();
    }

    Category *cat = m_allCategories.value(categoryID);
    if(!cat){return QList<Stock*>();}

    QList<Stock*> list;
    QStringList codes = cat->stocks;
    foreach (QString code, codes) {
        Stock *stock = m_allStocks->value(code);
        Q_ASSERT(stock);
        list.append(stock);
    }

    return list;

}

bool DataManager::readHistoricalData(QString *code, int offset){
    qDebug()<<"DataManager-readHistoricalData:"<<QThread::currentThreadId();

    QMutexLocker locker(&mutex);

    if(!code){return false;}
    QString newCode = *code;

    int size = m_allStocks->size();
    if(!size){return false;}
    int newOffset = offset%size;
    if(!m_allStocks->contains(newCode)){return false;}
    if(newOffset){
        QStringList allStocks = m_allStocks->keys();
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
        downloadHistoricalData(newCode);
        return false;
    }
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug()<<file.errorString();
        return false;
    }

    Stock *stock = m_allStocks->value(newCode);
    if(!stock){
        stock = new Stock(newCode, "");
        m_allStocks->insert(newCode, stock);
    }

    QTextStream in(&file);
    //表头
    QString title = in.readLine();
    if(title.trimmed().isEmpty()){
        return false;
    }
    qDebug()<<title;

    QMap<double, QCPFinancialData> *ohlcDataMap = stock->ohlcDataMap();
    QMap<double, TradeExtraData>  *tradeExtraDataMap = stock->tradeExtraDataMap();
    QVector<double> *futuresDeliveryDates = stock->futuresDeliveryDates();

    QVector< double >timeVector, openVector, highVector, lowVector, closeVector, volVector;
    uint index = (std::numeric_limits<uint>::max)();
    while (!in.atEnd()) {
        QStringList dataList = in.readLine().split(",");
        if(dataList.size()<12){
            qDebug()<<"Invalid column count!";
            return false;
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
                futuresDeliveryDates->append(index);
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

        ohlcDataMap->insert(index, QCPFinancialData(index, open, high, low, close));
        tradeExtraDataMap->insert(index, TradeExtraData(time_t, preClose, volume_Hand, turnover, turnoverRate));

        //数据文件为倒序。不可使用日期做KEY，日期有空档。
        index--;
        qApp->processEvents();
    }

//    emit historicalDataRead(newCode);
    emit historicalDataRead(stock);

    return true;
}

void DataManager::downloadHistoricalData(const QString &code){
    qDebug()<<"-----DataManager::downloadData(...)"<<" currentThreadId:"<<QThread::currentThreadId();

    //m_downloadManager->append(QUrl(QString("http://quotes.money.163.com/service/chddata.html?code=%1&start=20000720&end=20150508")));

    QString url = QString("http://quotes.money.163.com/service/chddata.html?code=%1%2").arg(code.startsWith("6")?"0":"1").arg(code);
    emit requestDownloadData(url);

//    m_downloadManager->append();
}

void DataManager::historicalDataDownloaded(const QString &fileName, const QUrl &url){
    qDebug()<<"---DataManager::dataDownloaded(...)"<<" fileName:"<<fileName<<" currentThreadId:"<<QThread::currentThreadId();
    QFileInfo fi(fileName);
    QString code = fi.baseName();

    readHistoricalData(&code, 0);
}

void DataManager::downloadRealTimeQuoteData(const QString &code){
    //API:http://api.money.126.net/data/feed/1000001,money.api
    QString url = QString("http://api.money.126.net/data/feed/%1%2,money.api").arg(code.startsWith("6")?"0":"1").arg(code);
    emit requestRealTimeQuoteData(url);
}

void DataManager::realTimeQuoteDataReceived(const QByteArray &data){
    //API:http://api.money.126.net/data/feed/1000001,money.api
    if(data.isEmpty()){return;}

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError){
        qCritical()<<error.errorString();
        return;
    }
    QJsonObject object = doc.object();
    if(object.isEmpty()){return;}

    QString code = object["symbol"].toString();
    Stock *stock = m_allStocks->value(code);
    if(!stock){return;}

    RealTimeQuoteData rtData;
    rtData.code = code;
    rtData.time = object["time"].toString();
    rtData.ask1 = object["ask1"].toDouble();
    rtData.ask2 = object["ask2"].toDouble();
    rtData.ask3 = object["ask3"].toDouble();
    rtData.ask4 = object["ask4"].toDouble();
    rtData.ask5 = object["ask5"].toDouble();
    rtData.askVol1 = object["askvol1"].toDouble();
    rtData.askVol2 = object["askvol2"].toDouble();
    rtData.askVol3 = object["askvol3"].toDouble();
    rtData.askVol4 = object["askvol4"].toDouble();
    rtData.askVol5 = object["askvol5"].toDouble();
    rtData.bid1 = object["bid1"].toDouble();
    rtData.bid2 = object["bid2"].toDouble();
    rtData.bid3 = object["bid3"].toDouble();
    rtData.bid4 = object["bid4"].toDouble();
    rtData.bid5 = object["bid5"].toDouble();
    rtData.bidVol1 = object["bidvol1"].toDouble();
    rtData.bidVol2 = object["bidvol2"].toDouble();
    rtData.bidVol3 = object["bidvol3"].toDouble();
    rtData.bidVol4 = object["bidvol4"].toDouble();
    rtData.bidVol5 = object["bidvol5"].toDouble();

    RealTimeStatisticsData * statisticsData = stock->realTimeStatisticsData();
    statisticsData->open = object["open"].toDouble();
    statisticsData->high = object["high"].toDouble();
    statisticsData->low = object["low"].toDouble();
    statisticsData->price = object["price"].toDouble();
    statisticsData->change = object["updown"].toDouble();
    statisticsData->changePercent = object["percent"].toDouble();
    statisticsData->yestClose = object["yestclose"].toDouble();
    statisticsData->volume = object["volume"].toDouble();
    statisticsData->turnover = object["turnover"].toDouble();

    emit realTimeAskDataUpdated(rtData);

}

void DataManager::downloadRealTimeStatisticsData(int pageIndex, int count, bool allFields){
   QString fields = "SYMBOL,NAME,PRICE,PERCENT,UPDOWN,FIVE_MINUTE,OPEN,YESTCLOSE,HIGH,LOW,VOLUME,TURNOVER,HS,LB,WB,ZF,PE,MCAP,TCAP,MFSUM";
    if(!allFields){
        fields = "SYMBOL,PRICE,PERCENT,UPDOWN,FIVE_MINUTE,HIGH,LOW,VOLUME,TURNOVER,HS,LB,WB,ZF,MFSUM";
    }

    //API:http://quotes.money.163.com/hs/service/diyrank.php?page=0&query=STYPE%3AEQA&fields=NO%2CSYMBOL%2CNAME%2CPRICE%2CPERCENT%2CUPDOWN%2CFIVE_MINUTE%2COPEN%2CYESTCLOSE%2CHIGH%2CLOW%2CVOLUME%2CTURNOVER%2CHS%2CLB%2CWB%2CZF%2CPE%2CMCAP%2CTCAP%2CMFSUM%2CMFRATIO.MFRATIO2%2CMFRATIO.MFRATIO10%2CSNAME%2CCODE%2CANNOUNMT%2CUVSNEWS&sort=SYMBOL&order=desc&count=24&type=query
    QString url = QString("http://quotes.money.163.com/hs/service/diyrank.php?page=%1&count=%2&sort=SYMBOL&order=desc&type=query&query=STYPE:EQA&fields=%3").arg(pageIndex).arg(count).arg(fields);
    emit requestRealTimeStatisticsData(url);
}

void DataManager::realTimeStatisticsDataReceived(const QByteArray &data){
    if(data.isEmpty()){return;}

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError){
        qCritical()<<error.errorString();
        return;
    }

    QJsonObject object = doc.object();
    if(object.isEmpty()){return;}

    if(m_hsaTotalStocksCount < 1){
        m_hsaTotalStocksCount = object["total"].toInt();
    }
    bool initMode = m_allStocks->isEmpty();
    QList<Stock*> stocksToBeSaved;

    QString time = object["time"].toString();
    QJsonArray array = object["list"].toArray();
    for(int i=0;i<array.size();i++){
        qApp->processEvents();

        QJsonObject infoObj = array.at(i).toObject();
        if(infoObj.isEmpty()){continue;}

        QString code = infoObj["SYMBOL"].toString();
        QString name = infoObj["NAME"].toString();
        Stock *stock = m_allStocks->value(code);
        if(!stock){
            stock = new Stock(code, name, this);
            m_allStocks->insert(code, stock);
            stocksToBeSaved.append(stock);
        }

        RealTimeStatisticsData *statisticsData = stock->realTimeStatisticsData();
        statisticsData->time = time;
        statisticsData->open = infoObj["OPEN"].toDouble();
        statisticsData->high = infoObj["HIGH"].toDouble();
        statisticsData->low = infoObj["LOW"].toDouble();
        statisticsData->price = infoObj["PRICE"].toDouble();
        statisticsData->change = infoObj["UPDOWN"].toDouble();
        statisticsData->changePercent = infoObj["PERCENT"].toDouble();
        statisticsData->yestClose = infoObj["YESTCLOSE"].toDouble();
        statisticsData->volume = infoObj["VOLUME"].toDouble();
        statisticsData->turnover = infoObj["TURNOVER"].toDouble();
        statisticsData->exchangeRatio = infoObj["HS"].toDouble();
        statisticsData->tradableMarketCap = infoObj["MCAP"].toDouble();
        statisticsData->marketCap = infoObj["TCAP"].toDouble();
        statisticsData->pe = infoObj["PE"].toDouble();
        statisticsData->earnings = infoObj["MFSUM"].toDouble();
        statisticsData->volChangeRatio = infoObj["LB"].toDouble();
        statisticsData->orderChangeRatio = infoObj["WB"].toDouble();
        statisticsData->fiveMinsChange = infoObj["FIVE_MINUTE"].toDouble();

    }

    if(initMode){
        loadAllCategories();
        emit allStocksLoaded();
    }

    if(!stocksToBeSaved.isEmpty()){
        saveStocksInfoToDB(stocksToBeSaved);
    }



}

void DataManager::readStocksList(){
    QMutexLocker locker(&mutex);

    qDeleteAll(m_allStocks->begin(), m_allStocks->end());
    m_allStocks->clear();

    QFile file(QApplication::applicationDirPath()+"/data/AllStocks.csv");
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug()<<file.errorString();
        downloadRealTimeStatisticsData(0, 10000, true);
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList dataList = in.readLine().split(",");
        if(dataList.size()!=2){
            continue;
        }
        m_allStocks->insert(dataList.at(0), new Stock(dataList.at(0), dataList.at(1)));
    }

    if(m_allStocks->isEmpty()){
        downloadRealTimeStatisticsData(0, 10000, true);
    }


    //emit stocksLoaded(m_allStocks->values());
}

bool DataManager::loadAllStocks(){

    if(!localStocksDataDB.isValid()){
        if(!openDatabase()){
            return false;
        }
    }
    QSqlQuery query(localStocksDataDB);

    QString statement = QString("SELECT Code, Name From Stocks; ");
    qDebug()<<"statement:"<<statement;
    if(!query.exec(statement)){
        QSqlError error = query.lastError();
        QString msg = QString("Can not query stocks from database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        qCritical()<<msg;
        return false;
    }

    while (query.next()) {
        QString code = query.value(0).toString();
        QString name = query.value(1).toString();
        if(!m_allStocks->contains(code)){
            Stock * stock = new Stock(code, name, this);
            m_allStocks->insert(code, stock);
        }
        qApp->processEvents();
    }

    if(!m_allStocks->isEmpty()){
        loadAllCategories();
        emit allStocksLoaded();
    }


    downloadRealTimeStatisticsData(0, 5000, true);

    return true;

}

bool DataManager::saveStockInfoToDB(Stock * stock){
    if(!stock){return false;}
    if(!localStocksDataDB.isValid()){
        if(!openDatabase()){
            return false;
        }
    }
    QSqlQuery query(localStocksDataDB);
    QString statement = QString("INSERT INTO Stocks(Code, Name) VALUES('%1', '%2'); ").arg(stock->code()).arg(stock->name());
    qDebug()<<"statement:"<<statement;
    if(!query.exec(statement)){
        QSqlError error = query.lastError();
        QString msg = QString("Can not save stock info to database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        qCritical()<<msg;
        return false;
    }
    return true;
}

bool DataManager::saveStocksInfoToDB(const QList<Stock*> &stocks){
    if(stocks.isEmpty()){return false;}
    if(!localStocksDataDB.isValid()){
        if(!openDatabase()){
            return false;
        }
    }
    QSqlQuery query(localStocksDataDB);

    QString statement = "Begin Transaction;";
    query.exec(statement);
    foreach (Stock *stock, stocks) {
        statement = QString("INSERT INTO Stocks(Code, Name) VALUES('%1', '%2'); ").arg(stock->code()).arg(stock->name());
        query.exec(statement);

        qApp->processEvents();
    }
    statement = "Commit Transaction;";
    if(!query.exec(statement)){
        QSqlError error = query.lastError();
        QString msg = QString("Can not save stocks info to database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        qCritical()<<msg;
        return false;
    }
    return true;
}

bool DataManager::loadAllCategories(){

    if(!localStocksDataDB.isValid()){
        if(!openDatabase()){
            return false;
        }
    }
    QSqlQuery query(localStocksDataDB);

    QString statement = QString("SELECT * From Categories; ");
    if(!query.exec(statement)){
        QSqlError error = query.lastError();
        QString msg = QString("Can not query categories from database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        qCritical()<<msg;
        return false;
    }
    while (query.next()) {
        quint32 id = query.value(0).toUInt();
        QString name = query.value(1).toString();
        QString systemCode = query.value(2).toString();

        if(!m_allCategories.contains(id)){
            Category * cat = new Category(id, name, systemCode);
            m_allCategories.insert(id, cat);
        }
        qApp->processEvents();
    }


    statement = QString("SELECT * From CategoryMembers; ");
    if(!query.exec(statement)){
        QSqlError error = query.lastError();
        QString msg = QString("Can not query category members from database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        qCritical()<<msg;
        return false;
    }
    while (query.next()) {
        quint32 catID = query.value(0).toUInt();
        QString stockCode = query.value(1).toString();
        Category * cat = m_allCategories.value(catID);
        Q_ASSERT(cat);
        if(!cat){continue;}
        cat->stocks.append(stockCode);

        qApp->processEvents();
    }

    return true;

}

bool DataManager::saveCategory(Category *category){
    if(!localStocksDataDB.isValid()){
        if(!openDatabase()){
            return false;
        }
    }
    QSqlQuery query(localStocksDataDB);

    QString statement = QString("INSERT INTO Categories(Name, SystemCode) VALUES('%1', '%2'); ").arg(category->name).arg(category->systemCode);
    if(!query.exec(statement)){
        QSqlError error = query.lastError();
        QString msg = QString("Can not save category to database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        qCritical()<<msg;
        return false;
    }
    return true;
}

bool DataManager::saveCategoryMember(quint32 categoryID, const QString &stockCode){
    if(!localStocksDataDB.isValid()){
        if(!openDatabase()){
            return false;
        }
    }
    QSqlQuery query(localStocksDataDB);

    QString statement = QString("INSERT INTO CategoryMembers(CategoryID, StockCode) VALUES(%1, '%2'); ").arg(categoryID).arg(stockCode);
    if(!query.exec(statement)){
        QSqlError error = query.lastError();
        QString msg = QString("Can not save category member to database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
        qCritical()<<msg;
        return false;
    }
    return true;
}

bool DataManager::openDatabase(bool reopen){

    //Check Local Database
    bool needInitLocalDB = false;
    QDir dir;
    dir.mkpath(m_localSaveDir);
    if(!QFile(localDataFilePath).exists()){
        needInitLocalDB = true;
    }

    if(reopen){
        DatabaseUtility::closeDBConnection(m_localDBConnectionName);
    }

    //QSqlDatabase db = QSqlDatabase::database(LOCAL_USERDATA_DB_CONNECTION_NAME);
    localStocksDataDB = QSqlDatabase::database(m_localDBConnectionName);
    if(!localStocksDataDB.isValid()){
        QSqlError err;
        DatabaseUtility databaseUtility;
        err = databaseUtility.openDatabase(m_localDBConnectionName,
                                           m_localDBDriver,
                                           "",
                                           0,
                                           "",
                                           "",
                                           localDataFilePath,
                                           HEHUI::SQLITE);
        if (err.type() != QSqlError::NoError) {
            qCritical() << QString("An error occurred when opening the database! %1").arg(err.text());
            return false;
        }

    }

    localStocksDataDB = QSqlDatabase::database(m_localDBConnectionName);
    if(!localStocksDataDB.isOpen()){
        qCritical()<<QString("Database is not open! %1").arg(localStocksDataDB.lastError().text());
        return false;
    }


    if(needInitLocalDB){
        if(!initLocalDatabase()){
            return false;
        }
    }

    return true;



}

bool DataManager::initLocalDatabase(QString *errorMessage){

    if(!localStocksDataDB.isValid() || !localStocksDataDB.isOpen()){
        if(errorMessage){
            *errorMessage = tr("Database Invalid Or Not Open!");
        }
        return false;
    }

    QSqlQuery query;
    QSqlError error = DatabaseUtility::excuteSQLScriptFromFile(localStocksDataDB, "://resources/Stocks.sql", "UTF-8", &query, true);
    if(error.type() != QSqlError::NoError){
        QString msg = error.text();
        if(errorMessage){
            *errorMessage = msg;
        }
        return false;
    }


//    QString statement = QString("insert into Stocks(Code) values('%1')").arg("000001");
//    if(!query.exec(statement)){
//        QSqlError error = query.lastError();
//        QString msg = QString("Can not initialize user database! %1 Error Type:%2 Error NO.:%3").arg(error.text()).arg(error.type()).arg(error.number());
//        qCritical()<<msg;
//        if(errorMessage){
//            *errorMessage = msg;
//        }
//        return false;
//    }

    return true;

}

QSqlQuery DataManager::queryDatabase(const QString & queryString, bool localConfigDatabase) {

    QSqlQuery query;
    DatabaseUtility du;

    if(localConfigDatabase){
        query = du.queryDatabase(queryString,
                                 m_localDBConnectionName,
                                 m_localDBDriver,
                                 "",
                                 0,
                                 "",
                                 "",
                                 localDataFilePath,
                                 HEHUI::SQLITE);
    }else{
        query = du.queryDatabase(queryString,
                                 m_remoteDBConnectionName,
                                 m_remoteDBDriver,
                                 m_remoteDBServerHost,
                                 m_remoteDBServerPort,
                                 m_remoteDBUserName,
                                 m_remoteDBUserPassword,
                                 m_remoteDBName,
                                 HEHUI::MYSQL);
    }

    return query;


}


QSqlQuery DataManager::queryDatabase(const QString & queryString, const QString &connectionName, const QString &driver,
                                         const QString &host, int port, const QString &user, const QString &passwd,
                                         const QString &databaseName, HEHUI::DatabaseType databaseType) {


    QSqlQuery query;
    DatabaseUtility du(this);

    query = du.queryDatabase(queryString, connectionName, driver, host, port, user, passwd, databaseName, databaseType);

    return query;

}



