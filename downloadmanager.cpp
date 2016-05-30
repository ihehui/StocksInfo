
#include "downloadmanager.h"

#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QApplication>
#include <stdio.h>
#include <QThread>

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent),
    m_overwriteoldFile(true)
{

    m_localSaveDir = QApplication::applicationDirPath() + "/temp";
    m_networkAccessManager = new QNetworkAccessManager();

}

DownloadManager::~DownloadManager(){
    //qDebug()<<"DownloadManager::~DownloadManager()";

    delete m_networkAccessManager;

    foreach (QFile *file, m_fileDownloadHash.values()) {
        file->close();
        delete file;
    }
    m_fileDownloadHash.clear();
}


void DownloadManager::requestFileDownload(const QStringList &urlList)
{
    foreach (QString url, urlList)
        requestFileDownload(QUrl::fromEncoded(url.toLocal8Bit()));

    //if (downloadQueue.isEmpty())
    //   QTimer::singleShot(0, this, SIGNAL(finished()));

}

void DownloadManager::requestFileDownload(const QUrl &url)
{
    if(m_fileDownloadHash.contains(url)){
        qWarning()<<QString("URL '%1' is already been downloading!").arg(url.toString());
        return;
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkAccessManager->get(request);
    //connect(currentFileDownload, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
    connect(reply, SIGNAL(readyRead()), SLOT(fileDownloadReadyRead()));
    connect(reply, SIGNAL(finished()), SLOT(fileDownloadFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));

    QFile *file = new QFile();
    m_fileDownloadHash.insert(url, file);

}

void DownloadManager::requestFileDownload(const QString &url){
    requestFileDownload(QUrl::fromEncoded(url.toLocal8Bit()));
}

void DownloadManager::requestRealTimeQuoteData(const QString &url){
    QNetworkRequest request(QUrl::fromEncoded(url.toLocal8Bit()));
    QNetworkReply *reply = m_networkAccessManager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(realTimeQuoteDataDownloadFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));

}

void DownloadManager::requestRealTimeStatisticsData(const QString &url){
    //qDebug()<<"DownloadManager::requestRealTimeStatisticsData()";

    QNetworkRequest request(QUrl::fromEncoded(url.toLocal8Bit()));
    QNetworkReply *reply = m_networkAccessManager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(realTimeStatisticsDataDownloadFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));

}



void DownloadManager::setLocalSaveDir(const QString &path){

    m_localSaveDir = path;
    QDir dir;
    dir.mkpath(m_localSaveDir);
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug()<<"DownloadManager::downloadProgress:"<<QThread::currentThreadId();

    // calculate the download speed
    //double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
//    QString unit;
//    if (speed < 1024) {
//        unit = "bytes/sec";
//    } else if (speed < 1024*1024) {
//        speed /= 1024;
//        unit = "kB/s";
//    } else {
//        speed /= 1024*1024;
//        unit = "MB/s";
//    }

   // qDebug()<<QString::fromLatin1("%1% %2").arg(speed, 3, 'f', 1).arg(unit);
}

void DownloadManager::fileDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply){return;}

    QUrl url = reply->url();
    QFile *file = m_fileDownloadHash.value(url);
    file->close();


    if (reply->error()) {
        qCritical()<<QString("Failed to download file '%1'! %2").arg(url.toString()).arg(reply->errorString());
    } else {
        emit fileDownloaded(file->fileName(), url);
    }

    m_fileDownloadHash.remove(url);
    file->deleteLater();
    reply->deleteLater();

}

void DownloadManager::fileDownloadReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply){return;}

    QUrl url = reply->url();
    QFile *file = m_fileDownloadHash.value(url);
    if(file->fileName().isEmpty()){
        QString header = QString(reply->rawHeader("Content-Disposition")).remove("attachment; filename=", Qt::CaseInsensitive);
        QString filename = m_localSaveDir + "/" + header;
        QDir dir;
        dir.mkpath(m_localSaveDir);
        if(header.isEmpty()){
            filename = saveFileName(url);
        }

        file->setFileName(filename);
        if (!file->open(QIODevice::WriteOnly)) {
            fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
                    qPrintable(filename), url.toEncoded().constData(),
                    qPrintable(file->errorString()));

            reply->deleteLater();
            file->deleteLater();
            m_fileDownloadHash.remove(url);
            return;
        }
    }

    file->write(reply->readAll());
}

void DownloadManager::realTimeQuoteDataDownloadFinished(){
    //qDebug()<<"DownloadManager::realTimeQuoteDataDownloadFinished()";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply){return;}

    if(reply->error() == QNetworkReply::NoError){
        emit realTimeQuoteDataReceived(reply->readAll());
    }
    reply->deleteLater();

}

void DownloadManager::realTimeStatisticsDataDownloadFinished(){
    //qDebug()<<"DownloadManager::realTimeStatisticsDataDownloadFinished() "<<QThread::currentThreadId();
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply){return;}

    if(reply->error() == QNetworkReply::NoError){
        emit realTimeStatisticsDataReceived(reply->readAll());
    }
    reply->deleteLater();
}

void DownloadManager::error(QNetworkReply::NetworkError code){
    qDebug()<<"NetworkError:"<<code;

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply){return;}
    emit networkError(reply->url(), reply->errorString());
}

QString DownloadManager::saveFileName(const QUrl &url)
{

    QString path = url.path();
    QString basename = QFileInfo(path).fileName();
    QString localPath = m_localSaveDir + "/" + basename;

    if (basename.isEmpty())
        basename = "download";

    if (QFile::exists(localPath)) {
        if(m_overwriteoldFile && QFile::remove(localPath)){
            return localPath;
        }else{
            qCritical()<<QString("Failed to remove file '%1'").arg(basename);
        }
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return m_localSaveDir + "/" + basename;
}


