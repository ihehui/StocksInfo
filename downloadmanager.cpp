/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
      downloadedCount(0),
      totalCount(0),
      overwriteoldFile(true)
{

    localSaveDir = QApplication::applicationDirPath() + "/data";
    curFileName = "";

    manager = new QNetworkAccessManager();


}

DownloadManager::~DownloadManager(){
    qDebug()<<"DownloadManager::~DownloadManager()";

    delete manager;
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

    if (fileDownloadQueue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));

    fileDownloadQueue.enqueue(url);
    ++totalCount;
}

void DownloadManager::requestFileDownload(const QString &url){
    requestFileDownload(QUrl::fromEncoded(url.toLocal8Bit()));
}

void DownloadManager::requestRealTimeQuoteData(const QString &url){

    QNetworkRequest request(QUrl::fromEncoded(url.toLocal8Bit()));
    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(realTimeStatisticsDataDownloadFinished()));

}

void DownloadManager::requestRealTimeStatisticsData(const QString &url){
    qDebug()<<"DownloadManager::requestRealTimeStatisticsData()";

    QNetworkRequest request(QUrl::fromEncoded(url.toLocal8Bit()));
    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(realTimeStatisticsDataDownloadFinished()));

}


QString DownloadManager::saveFileName(const QUrl &url)
{

    QString path = url.path();
    QString basename = QFileInfo(path).fileName();
    QString localPath = localSaveDir + "/" + basename;

    if (basename.isEmpty())
        basename = "download";

    if (QFile::exists(localPath)) {
        if(overwriteoldFile && QFile::remove(localPath)){
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

    return localSaveDir + "/" + basename;
}

void DownloadManager::setLocalSaveDir(const QString &path){

    localSaveDir = path;
    QDir dir;
    dir.mkpath(localSaveDir);
}

void DownloadManager::startNextDownload()
{
    qDebug()<<"DownloadManager:"<<QThread::currentThreadId();

    curFileName = "";
    if (fileDownloadQueue.isEmpty()) {
        printf("%d/%d files downloaded successfully\n", downloadedCount, totalCount);
        emit finished();
        return;
    }

    QUrl url = fileDownloadQueue.dequeue();
//    QString filename = saveFileName(url);
//    output.setFileName(filename);
//    if (!output.open(QIODevice::WriteOnly)) {
//        fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
//                qPrintable(filename), url.toEncoded().constData(),
//                qPrintable(output.errorString()));

//        startNextDownload();
//        return;                 // skip this download
//    }

    QNetworkRequest request(url);
    currentFileDownload = manager->get(request);
    connect(currentFileDownload, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)));
    connect(currentFileDownload, SIGNAL(finished()),
            SLOT(downloadFinished()));
    connect(currentFileDownload, SIGNAL(readyRead()),
            SLOT(downloadReadyRead()));

    // prepare the output
    printf("Downloading %s...\n", url.toEncoded().constData());
    downloadTime.start();
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug()<<"DownloadManager::downloadProgress:"<<QThread::currentThreadId();

    // calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

   // qDebug()<<QString::fromLatin1("%1% %2").arg(speed, 3, 'f', 1).arg(unit);
}

void DownloadManager::downloadFinished()
{
    output.close();

    if (currentFileDownload->error()) {
        // download failed
        fprintf(stderr, "Failed: %s\n", qPrintable(currentFileDownload->errorString()));
    } else {
        printf("Succeeded.\n");
        ++downloadedCount;

        emit dataDownloaded(curFileName, currentFileDownload->url());
    }

    currentFileDownload->deleteLater();
    startNextDownload();

}

void DownloadManager::downloadReadyRead()
{

    if(curFileName.trimmed().isEmpty()){
        QString header = QString(currentFileDownload->rawHeader("Content-Disposition")).remove("attachment; filename=", Qt::CaseInsensitive);
        QString filename = localSaveDir + "/" + header;
        if(filename.isEmpty()){
            filename = saveFileName(currentFileDownload->url());
        }

        output.setFileName(filename);
        if (!output.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
                    qPrintable(filename), currentFileDownload->url().toEncoded().constData(),
                    qPrintable(output.errorString()));

            currentFileDownload->deleteLater();
            startNextDownload();
            return;                 // skip this download
        }
        curFileName = filename;
    }

    output.write(currentFileDownload->readAll());
}

void DownloadManager::realTimeQuoteDataDownloadFinished(){
    qDebug()<<"DownloadManager::realTimeQuoteDataDownloadFinished()";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply){return;}

    if(reply->error() == QNetworkReply::NoError){
        emit realTimeQuoteDataReceived(reply->readAll());
    }
    reply->deleteLater();

}

void DownloadManager::realTimeStatisticsDataDownloadFinished(){
    qDebug()<<"DownloadManager::realTimeStatisticsDataDownloadFinished() "<<QThread::currentThreadId();
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply){return;}

    if(reply->error() == QNetworkReply::NoError){
        emit realTimeStatisticsDataReceived(reply->readAll());
    }
    reply->deleteLater();
}

void DownloadManager::error(QNetworkReply::NetworkError code){
    qDebug()<<"NetworkError:"<<code;
}

