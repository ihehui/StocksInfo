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

    Stock * stock(const QString &code);

signals:
    void historicalDataRead(Stock * stock);
//    void historicalDataRead(const QString &stockCode);
    void requestDownloadData(const QString &url);

public slots:

    //读取交易数据
    bool readHistoricalData(QString *code, int offset = 0);

    //下载交易数据
    bool downloadData(const QString &code);
    void dataDownloaded(const QString &fileName, const QUrl &url);

private slots:
    //TODO:MUTEX
    void readStocksList();
    void updateStocksAskInfo(const QString & jsonString);
    void updateStocksSummaryInfo(const QString & jsonString);

protected:
    void run();

private:
    QMutex mutex;


    //QMap<double, QCPFinancialData> *m_ohlcData; //index,QCPFinancialData. 基本交易数据
    //QMap<double, TradeExtraData> *m_tradeExtraData; //index,TradeExtraData. 交易数据
    //QVector<double> m_futuresDeliveryDates; //index. Futures delivery,期指交割日，忽略放假顺延

    QString m_localSaveDir;

    QMap<QString, Stock*> *m_allStocks; //Code,Stock

};

#endif // DATAMANAGER_H
