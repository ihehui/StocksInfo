#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <QtConcurrent/QtConcurrent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidgetExtraInfo->hide();

    connect(ui->tableViewStocks, SIGNAL(stockActivated(Stock*)), this, SLOT(stockActivated(Stock*)));
    connect(ui->tableViewStocks, SIGNAL(stockSelected(Stock*)), this, SLOT(stockSelected(Stock*)));

//    QFont ft = QApplication::font();
//    ft.setBold(true);
//    ft.setPointSize(ft.pointSize()*1.5);
//    QFontMetrics fm(ft);

//    int rowHeight = 20;
//    QHeaderView *verticalHeader = ui->tableViewStocks->verticalHeader();
//    verticalHeader->setDefaultSectionSize(rowHeight);
//    //verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

//    m_tableModel = new StocksTableModel(this);
//    m_sortFilterProxyModel = new SortFilterProxyModel(this);
//    m_sortFilterProxyModel->setSourceModel(m_tableModel);
//    m_sortFilterProxyModel->setDynamicSortFilter(true);
//    ui->tableViewStocks->setModel(m_sortFilterProxyModel);
    //m_tableModel->setStocks(m_dataManager->allStocks()->values());


    m_dataManager = new DataManager();
//    connect(m_dataManager, SIGNAL(historicalDataRead(QString)), ui->tabCandlestick, SLOT(historicalDataRead(QString)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(historicalDataRead(Stock*)), ui->tabCandlestick, SLOT(historicalDataRead(Stock*)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(realTimeAskDataUpdated(const RealTimeQuoteData &)), this, SLOT(updateRealTimeAskData(const RealTimeQuoteData &)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(allStocksLoaded()), this, SLOT(allStocksLoaded()));
    connect(ui->tabCandlestick, SIGNAL(historicalDataRequested(QString *, int)), m_dataManager, SLOT(readHistoricalData(QString *, int)), Qt::QueuedConnection);

//    m_tableModel->setStocks(m_dataManager->allStocks()->values());

    ui->tableViewStocks->setDataManager(m_dataManager);
    ui->tabCandlestick->setDataManager(m_dataManager);
    m_dataManager->loadAllStocks();
    //m_dataManager->moveToThread(&m_dataManagerThread);
    //m_dataManagerThread.start();
    //m_dataManager->start();


//    m_downloadManager = new DownloadManager();
//    connect(m_downloadManager, SIGNAL(dataDownloaded(const QString &, const QUrl &)), m_dataManager, SLOT(historicalDataDownloaded(const QString &, const QUrl &)));
//    connect(m_downloadManager, SIGNAL(realTimeQuoteDataReceived(const QByteArray &)), m_dataManager, SLOT(realTimeQuoteDataReceived(const QByteArray &)));
//    connect(m_downloadManager, SIGNAL(realTimeStatisticsDataReceived(const QByteArray &)), m_dataManager, SLOT(realTimeStatisticsDataReceived(const QByteArray &)));

//    connect(m_dataManager, SIGNAL(requestDownloadData(const QString &)), m_downloadManager, SLOT(requestFileDownload(const QString &)));
//    connect(m_dataManager, SIGNAL(requestRealTimeQuoteData(const QString &)), m_downloadManager, SLOT(requestRealTimeQuoteData(const QString &)));
//    connect(m_dataManager, SIGNAL(requestRealTimeStatisticsData(const QString &)), m_downloadManager, SLOT(requestRealTimeStatisticsData(const QString &)));

    //m_downloadManager->moveToThread(&m_downloadManagerThread);
    //m_downloadManagerThread.start();

//    QThreadPool::globalInstance()->setMaxThreadCount(4);
//    QtConcurrent::run(m_downloadManager, &DownloadManager::run);
//    QtConcurrent::run(clientPacketsParser, &ClientPacketsParser::startprocessOutgoingPackets);


    m_timer.setInterval(5000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));



//qDebug()<<ui->tableViewStocks->height();
//qDebug()<<ui->tableViewStocks->viewport()->height();
//qDebug()<<ui->tableViewStocks->geometry().height();
//qDebug()<<ui->tableViewStocks->frameGeometry().height();
//m_tableModel->setRowCount(ui->tableViewStocks->height()/rowHeight);

QTimer::singleShot(1000, this, SLOT(test()));
    ui->tabCandlestick->showCandlesticks("000001");

}

MainWindow::~MainWindow()
{
    delete ui;

    m_timer.stop();

    //m_dataManager->quit();
    delete m_dataManager;
    //delete m_downloadManager;

    //delete m_sortFilterProxyModel;
//    delete m_tableModel;

//    m_dataManagerThread.quit();
//    m_dataManagerThread.wait();
//    qDebug()<<"----------2-----------";

//    m_downloadManagerThread.quit();
//    m_dataManagerThread.wait();
//    qDebug()<<"----------3-----------";


}

void MainWindow::closeEvent(QCloseEvent *event){
    m_dataManagerThread.quit();
    m_dataManagerThread.wait();
    qDebug()<<"----------0-----------";

    m_downloadManagerThread.quit();
    m_downloadManagerThread.wait();
    qDebug()<<"----------1-----------";

    event->accept();
}

void MainWindow::updateRealTimeAskData(const RealTimeQuoteData &data){

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

    RealTimeStatisticsData * statisticsData = stock->realTimeStatisticsData();
    ui->labelCurPrice->setText(QString::number(statisticsData->price));
    ui->labelOpen->setText(QString::number(statisticsData->open));
    ui->labelHigh->setText(QString::number(statisticsData->high));
    ui->labelLow->setText(QString::number(statisticsData->low));
    ui->labelChange->setText(QString::number(statisticsData->change));
    ui->labelChangePercent->setText(QString::number(statisticsData->changePercent));

}

void MainWindow::stockActivated(Stock *stock){
    qDebug()<<"stockActivated:"<<stock->name();
}

void MainWindow::stockSelected(Stock *stock){
    qDebug()<<"stockSelected:"<<stock->name();

}

void MainWindow::allStocksLoaded(){
    qDebug()<<"--MainWindow::allStocksLoaded()";
    ui->tableViewStocks->showCategory(0);
}

void MainWindow::timeout(){
    m_dataManager->downloadRealTimeQuoteData(ui->tabCandlestick->currentStock()->code());
}

void MainWindow::test(){

//    m_dataManager->downloadRealTimeStatisticsData(0, 1, true);


}


