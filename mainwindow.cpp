#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidgetExtraInfo->hide();





    m_dataManager = new DataManager();
//    connect(m_dataManager, SIGNAL(historicalDataRead(QString)), ui->tabCandlestick, SLOT(historicalDataRead(QString)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(historicalDataRead(Stock*)), ui->tabCandlestick, SLOT(historicalDataRead(Stock*)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(realTimeAskDataUpdated(const RealTimeData &)), this, SLOT(updateRealTimeAskData(const RealTimeData &)), Qt::QueuedConnection);

    connect(ui->tabCandlestick, SIGNAL(historicalDataRequested(QString *, int)), m_dataManager, SLOT(readHistoricalData(QString *, int)), Qt::QueuedConnection);

    ui->tabCandlestick->setDataManager(m_dataManager);
    m_dataManager->moveToThread(&m_dataManagerThread);
    m_dataManagerThread.start();
    //m_dataManager->start();


    m_downloadManager = new DownloadManager();
    connect(m_downloadManager, SIGNAL(dataDownloaded(const QString &, const QUrl &)), m_dataManager, SLOT(dataDownloaded(const QString &, const QUrl &)));
    connect(m_downloadManager, SIGNAL(realTimeAskDataReceived(const QByteArray &)), m_dataManager, SLOT(realTimeAskDataReceived(const QByteArray &)));

    connect(m_dataManager, SIGNAL(requestDownloadData(const QString &)), m_downloadManager, SLOT(append(const QString &)));
    connect(m_dataManager, SIGNAL(requestRealTimeAskData(const QString &)), m_downloadManager, SLOT(requestRealTimeAskData(const QString &)));

    m_downloadManager->moveToThread(&m_downloadManagerThread);
    m_downloadManagerThread.start();

    m_timer.setInterval(5000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));

    ui->tabCandlestick->showCandlesticks("000001");


}

MainWindow::~MainWindow()
{
    delete ui;

    m_timer.stop();

    //m_dataManager->quit();
    //delete m_dataManager;

    m_dataManagerThread.quit();

    m_downloadManagerThread.quit();


}

void MainWindow::updateRealTimeAskData(const RealTimeData &data){

    Stock *stock = m_dataManager->stock(data.code);
    Q_ASSERT(stock);
    if(stock != ui->tabCandlestick->currentStock()){return;}

    ui->labelCode->setText(QString("%1 %2").arg(data.code).arg(stock->name()));
    ui->labelTopInfo->setText(tr("Update: %1").arg(data.time));
    ui->labelAsk5->setText(QString::number(data.ask5));
    ui->labelAsk4->setText(QString::number(data.ask4));
    ui->labelAsk3->setText(QString::number(data.ask3));
    ui->labelAsk2->setText(QString::number(data.ask2));
    ui->labelAsk1->setText(QString::number(data.ask1));
    ui->labelAskVol1->setText(QString::number(data.askVol1));
    ui->labelAskVol2->setText(QString::number(data.askVol2));
    ui->labelAskVol3->setText(QString::number(data.askVol3));
    ui->labelAskVol4->setText(QString::number(data.askVol4));
    ui->labelAskVol5->setText(QString::number(data.askVol5));

    ui->labelBID1->setText(QString::number(data.bid1));
    ui->labelBID2->setText(QString::number(data.bid2));
    ui->labelBID3->setText(QString::number(data.bid3));
    ui->labelBID4->setText(QString::number(data.bid4));
    ui->labelBID5->setText(QString::number(data.bid5));
    ui->labelBIDVol1->setText(QString::number(data.bidVol1));
    ui->labelBIDVol2->setText(QString::number(data.bidVol2));
    ui->labelBIDVol3->setText(QString::number(data.bidVol3));
    ui->labelBIDVol4->setText(QString::number(data.bidVol4));
    ui->labelBIDVol5->setText(QString::number(data.bidVol5));

    ui->labelCurPrice->setText(QString::number(data.price));
    ui->labelOpen->setText(QString::number(data.open));
    ui->labelHigh->setText(QString::number(data.high));
    ui->labelLow->setText(QString::number(data.low));
    ui->labelChange->setText(QString::number(data.change));
    ui->labelChangePercent->setText(QString::number(data.changePercent));

}

void MainWindow::timeout(){
    m_dataManager->downloadRealTimeAskData(ui->tabCandlestick->currentStock()->code());
}
