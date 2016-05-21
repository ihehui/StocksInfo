#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QThread>
#include <QMap>
#include <QVector>



class QCPFinancialData;
class TradeExtraData;
class DownloadManager;


class DataManager : public QThread
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = 0);
    ~DataManager();

    QMap<double, QCPFinancialData> * ohlcData() {return m_ohlcData;}
    QMap<double, TradeExtraData> * tradeExtraData() const {return m_tradeExtraData;}
    static QMap<QString, QString> * allStocks() {return &m_allStocksMap;}
    QVector< double > * futuresDeliveryDates() {return &m_futuresDeliveryDates;}


signals:
    void historicalDataRead(const QString &stockCode);

public slots:

    //读取交易数据
    bool readHistoricalData(QString *code, int offset = 0);

    //下载交易数据
    bool downloadData(const QString &code);
    void dataDownloaded(const QString &fileName, const QUrl &url);

private slots:
    //TODO:MUTEX
    void readStocksList();

protected:
    void run();

private:
    //Code,Name
    static QMap<QString, QString> m_allStocksMap;

    QString m_stockCode;
    QString m_stockName;

    QMap<double, QCPFinancialData> *m_ohlcData; //index,QCPFinancialData. 基本交易数据
    QMap<double, TradeExtraData> *m_tradeExtraData; //index,TradeExtraData. 交易数据
    QVector<double> m_futuresDeliveryDates; //index. Futures delivery,期指交割日，忽略放假顺延

    DownloadManager *m_downloadManager;

    QString m_localSaveDir;



};

#endif // DATAMANAGER_H
