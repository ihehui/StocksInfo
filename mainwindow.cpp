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

    connect(ui->tabCandlestick, SIGNAL(escape()), this, SLOT(switchPage()), Qt::QueuedConnection);

    m_dataManager = new DataManager();
    connect(m_dataManager, SIGNAL(historicalDataRead(Stock*)), ui->tabCandlestick, SLOT(historicalDataRead(Stock*)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(realTimeAskDataUpdated(const RealTimeQuoteData &)), this, SLOT(updateRealTimeAskData(const RealTimeQuoteData &)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(allStocksLoaded()), this, SLOT(allStocksLoaded()));
    connect(ui->tabCandlestick, SIGNAL(historicalDataRequested(QString *, int)), m_dataManager, SLOT(readHistoricalData(QString *, int)), Qt::QueuedConnection);

    ui->tableViewStocks->setDataManager(m_dataManager);
    ui->tabCandlestick->setDataManager(m_dataManager);
    m_dataManager->loadAllStocks();


    m_timer.setInterval(5000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));


    //qDebug()<<ui->tableViewStocks->height();
    //qDebug()<<ui->tableViewStocks->viewport()->height();
    //qDebug()<<ui->tableViewStocks->geometry().height();
    //qDebug()<<ui->tableViewStocks->frameGeometry().height();
    //m_tableModel->setRowCount(ui->tableViewStocks->height()/rowHeight);

    QTimer::singleShot(1000, this, SLOT(test()));

}

MainWindow::~MainWindow()
{
    delete ui;

    m_timer.stop();

    delete m_dataManager;

}

void MainWindow::closeEvent(QCloseEvent *event){
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
    ui->tabCandlestick->showCandlesticks(stock->code());
    ui->stackedWidget->setCurrentWidget(ui->pageTradeInfo);
}

void MainWindow::stockSelected(Stock *stock){
    qDebug()<<"stockSelected:"<<stock->name();

}

void MainWindow::allStocksLoaded(){
    qDebug()<<"--MainWindow::allStocksLoaded()";
    ui->tableViewStocks->showCategory(0);
}

void MainWindow::switchPage(){
    if(ui->stackedWidget->currentWidget() == ui->pageTradeInfo){
        ui->stackedWidget->setCurrentWidget(ui->pageStocksList);
    }else{
        ui->stackedWidget->setCurrentWidget(ui->pageTradeInfo);
    }
}

void MainWindow::timeout(){
    m_dataManager->downloadRealTimeQuoteData(ui->tabCandlestick->currentStock()->code());
}

void MainWindow::test(){

    //    m_dataManager->downloadRealTimeStatisticsData(0, 1, true);


}


