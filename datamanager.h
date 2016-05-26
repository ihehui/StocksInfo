#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QThread>
#include <QMap>
#include <QVector>
#include "stock.h"


class QCPFinancialData;
class TradeExtraData;
class DownloadManager;


class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = 0);
    ~DataManager();

    Stock * stock(const QString &code) const;
    QMap<QString, Stock*> * allStocks() const;

signals:
    void stocksLoaded(const QList<Stock*> &stocks);
    void stocksCountChanged();
    void historicalDataRead(Stock * stock);
//    void realTimeAskDataUpdated(Stock * stock);
    void realTimeAskDataUpdated(const RealTimeQuoteData &data);

//    void historicalDataRead(const QString &stockCode);
    void requestDownloadData(const QString &url);
    void requestRealTimeQuoteData(const QString &url);
    void requestRealTimeStatisticsData(const QString &url);


public slots:

    //读取交易数据
    bool readHistoricalData(QString *code, int offset = 0);

    //下载历史交易数据
    void downloadHistoricalData(const QString &code);
    void historicalDataDownloaded(const QString &fileName, const QUrl &url);

    //实时行情数据
    void downloadRealTimeQuoteData(const QString &code);
    void realTimeQuoteDataReceived(const QByteArray &data);

    void downloadRealTimeStatisticsData(int pageIndex, int count = 0, bool allFields = false);
    void realTimeStatisticsDataReceived(const QByteArray &data);

private slots:
    //TODO:MUTEX
    void readStocksList();


private:
    QMutex mutex;
    DownloadManager *m_downloadManager;


    //QMap<double, QCPFinancialData> *m_ohlcData; //index,QCPFinancialData. 基本交易数据
    //QMap<double, TradeExtraData> *m_tradeExtraData; //index,TradeExtraData. 交易数据
    //QVector<double> m_futuresDeliveryDates; //index. Futures delivery,期指交割日，忽略放假顺延

    QString m_localSaveDir;

    QMap<QString, Stock*> *m_allStocks; //Code,Stock
    int m_hsaTotalStocksCount;

};

#endif // DATAMANAGER_H
