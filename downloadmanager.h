
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


    void setLocalSaveDir(const QString &path);

signals:
    void networkError(const QUrl &url, const QString &errorString);
    void fileDownloaded(const QString &fileName, const QUrl &url);
    void realTimeQuoteDataReceived(const QByteArray &data);
    void realTimeStatisticsDataReceived(const QByteArray &data);

private slots:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void fileDownloadFinished();
    void fileDownloadReadyRead();

    void realTimeQuoteDataDownloadFinished();
    void realTimeStatisticsDataDownloadFinished();

    void error(QNetworkReply::NetworkError code);

private:
    QString saveFileName(const QUrl &url);



private:
    QNetworkAccessManager *m_networkAccessManager;
    QHash<QUrl, QFile*> m_fileDownloadHash; //URL, File

    bool m_overwriteoldFile;
    QString m_localSaveDir;

};

#endif
