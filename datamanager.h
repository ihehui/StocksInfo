#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QThread>
#include <QMap>
#include <QVector>
#include <QtSql>

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

    QList<Stock*> categoryStocks(quint32 categoryID) const;

signals:
    void allStocksLoaded();

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
    bool readHistoricalTradeDataFile(const QString &fileName);

    //下载历史交易数据
    void downloadHistoricalData(const QString &code, const QString &startDate = "", const QString &endDate = "");
    void historicalDataDownloaded(const QString &fileName, const QUrl &url);

    //实时行情数据
    void downloadRealTimeQuoteData(const QString &code);
    void realTimeQuoteDataReceived(const QByteArray &data);

    void downloadRealTimeStatisticsData(int pageIndex, int count = 0, bool allFields = false);
    void realTimeStatisticsDataReceived(const QByteArray &data);

    bool loadAllStocks();

private slots:
    void readStocksList();

    bool saveStockInfoToDB(Stock * stock);
    bool saveStocksInfoToDB(const QList<Stock*> &stocks);

    bool loadAllCategories();
    bool saveCategory(Category *category);
    bool saveCategoryMember(quint32 categoryID, const QString &stockCode);

    bool loadHistoricalTradeData(Stock * stock);
    bool saveHistoricalTradeData(Stock * stock);

private:
    bool openDatabase(bool reopen = false);
    bool initLocalDatabase(QString *errorMessage = 0);

    QSqlQuery queryDatabase(const QString & queryString, bool localConfigDatabase) ;

    QSqlQuery queryDatabase(const QString & queryString, const QString &connectionName, const QString &driver,
                            const QString &host, int port, const QString &user, const QString &passwd,
                            const QString &databaseName, HEHUI::DatabaseType databaseType) ;



private:
    QString m_localSaveDir;

    QString localDataFilePath;
    QSqlDatabase localStocksDataDB;
    //SQLITE
    QString m_localDBConnectionName;
    QString m_localDBName;
    QString m_localDBDriver;
    //MySQL
    QString m_remoteDBConnectionName;
    QString m_remoteDBDriver;
    QString m_remoteDBServerHost;
    quint16 m_remoteDBServerPort;
    QString m_remoteDBUserName;
    QString m_remoteDBUserPassword;
    QString m_remoteDBName;

    DownloadManager *m_downloadManager;

    QMap<QString, Stock*> *m_allStocks; //Code,Stock
    QHash<quint32, Category*> m_allCategories; //id, Category
    int m_hsaTotalStocksCount;



};

#endif // DATAMANAGER_H
