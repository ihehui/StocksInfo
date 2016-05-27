
#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QFile>
#include <QObject>
#include <QQueue>
#include <QTime>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QThread>
#include <QWaitCondition>
#include <QNetworkReply>

class DownloadManager: public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = 0);
    ~DownloadManager();


public slots:
    void requestFileDownload(const QUrl &url);
    void requestFileDownload(const QString &url);
    void requestFileDownload(const QStringList &urlList);

    void requestRealTimeQuoteData(const QString &url);
    void requestRealTimeStatisticsData(const QString &url);


    QString saveFileName(const QUrl &url);
    void setLocalSaveDir(const QString &path);

signals:
    void finished();
    void dataDownloaded(const QString &fileName, const QUrl &url);
    void realTimeQuoteDataReceived(const QByteArray &data);
    void realTimeStatisticsDataReceived(const QByteArray &data);

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

    void realTimeQuoteDataDownloadFinished();
    void realTimeStatisticsDataDownloadFinished();

    void error(QNetworkReply::NetworkError code);

public slots:


private:

    QNetworkAccessManager *manager;
    QQueue<QUrl> fileDownloadQueue, realTimeQuoteQueue, realTimeStatisticsQueue;
    QNetworkReply *currentFileDownload;
    QFile output;
    QTime downloadTime;

    int downloadedCount;
    int totalCount;

    bool overwriteoldFile;
    QString localSaveDir;
    QString curFileName;


};

#endif
