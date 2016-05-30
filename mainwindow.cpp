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
    connect(ui->tabCandlestick, SIGNAL(stockChanged(QString)), this, SLOT(resetRealTimeQuoteData(QString)), Qt::QueuedConnection);


    m_dataManager = new DataManager();
    connect(m_dataManager, SIGNAL(networkError(QUrl,QString)), this, SLOT(networkError(QUrl,QString)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(historicalDataRead(Stock*)), ui->tabCandlestick, SLOT(historicalDataRead(Stock*)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(realTimeQuoteDataUpdated(const RealTimeQuoteData &)), this, SLOT(updateRealTimeQuoteData(const RealTimeQuoteData &)), Qt::QueuedConnection);
    connect(m_dataManager, SIGNAL(allStocksLoaded()), this, SLOT(allStocksLoaded()));
    connect(ui->tabCandlestick, SIGNAL(historicalDataRequested(QString *, int)), m_dataManager, SLOT(readHistoricalData(QString *, int)), Qt::QueuedConnection);

    ui->tableViewStocks->setDataManager(m_dataManager);
    ui->tabCandlestick->setDataManager(m_dataManager);
    m_dataManager->loadAllStocks();


    m_timer.setInterval(5000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    QDateTime dateTime = QDateTime::currentDateTime();
    QDate date = dateTime.date();
    QTime time = dateTime.time();
    if(date.dayOfWeek() <= 5
            && ( (time > QTime(9, 30, 0) && time <= QTime(11, 30, 0)) || (time >= QTime(13, 0, 0) && time <= QTime(15, 0, 0)) )
            ){
        m_timer.start();
    }


    //qDebug()<<ui->tableViewStocks->height();
    //qDebug()<<ui->tableViewStocks->viewport()->height();
    //qDebug()<<ui->tableViewStocks->geometry().height();
    //qDebug()<<ui->tableViewStocks->frameGeometry().height();
    //m_tableModel->setRowCount(ui->tableViewStocks->height()/rowHeight);

    ui->tableViewStocks->setFocus();

}

MainWindow::~MainWindow()
{
    m_timer.stop();

    delete ui;

    delete m_dataManager;

}

void MainWindow::closeEvent(QCloseEvent *event){
    event->accept();
}

void MainWindow::updateRealTimeQuoteData(const RealTimeQuoteData &data){

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
    ui->labelAskVol1->setText(QString::number(int(data.askVol1/100)));
    ui->labelAskVol2->setText(QString::number(int(data.askVol2/100)));
    ui->labelAskVol3->setText(QString::number(int(data.askVol3/100)));
    ui->labelAskVol4->setText(QString::number(int(data.askVol4/100)));
    ui->labelAskVol5->setText(QString::number(int(data.askVol5/100)));

    ui->labelBID1->setText(QString::number(data.bid1));
    ui->labelBID2->setText(QString::number(data.bid2));
    ui->labelBID3->setText(QString::number(data.bid3));
    ui->labelBID4->setText(QString::number(data.bid4));
    ui->labelBID5->setText(QString::number(data.bid5));
    ui->labelBIDVol1->setText(QString::number(int(data.bidVol1/100)));
    ui->labelBIDVol2->setText(QString::number(int(data.bidVol2/100)));
    ui->labelBIDVol3->setText(QString::number(int(data.bidVol3/100)));
    ui->labelBIDVol4->setText(QString::number(int(data.bidVol4/100)));
    ui->labelBIDVol5->setText(QString::number(int(data.bidVol5/100)));

    RealTimeStatisticsData * statisticsData = stock->realTimeStatisticsData();
    ui->labelCurPrice->setText(QString::number(statisticsData->price));
    ui->labelOpen->setText(QString::number(statisticsData->open));
    ui->labelHigh->setText(QString::number(statisticsData->high));
    ui->labelLow->setText(QString::number(statisticsData->low));
    ui->labelChange->setText(QString::number(statisticsData->change));
    ui->labelChangePercent->setText(QString::number(statisticsData->changePercent*100)+"%");

}

void MainWindow::resetRealTimeQuoteData(const QString &code){
    Stock *stock = m_dataManager->stock(code);
    Q_ASSERT(stock);
    ui->labelCode->setText(QString("%1 %2").arg(code).arg(stock->name()));
    ui->labelTopInfo->setText("");
    ui->labelAsk5->setText("");
    ui->labelAsk4->setText("");
    ui->labelAsk3->setText("");
    ui->labelAsk2->setText("");
    ui->labelAsk1->setText("");
    ui->labelAskVol1->setText("");
    ui->labelAskVol2->setText("");
    ui->labelAskVol3->setText("");
    ui->labelAskVol4->setText("");
    ui->labelAskVol5->setText("");

    ui->labelBID1->setText("");
    ui->labelBID2->setText("");
    ui->labelBID3->setText("");
    ui->labelBID4->setText("");
    ui->labelBID5->setText("");
    ui->labelBIDVol1->setText("");
    ui->labelBIDVol2->setText("");
    ui->labelBIDVol3->setText("");
    ui->labelBIDVol4->setText("");
    ui->labelBIDVol5->setText("");

    RealTimeStatisticsData * statisticsData = stock->realTimeStatisticsData();
    ui->labelCurPrice->setText(QString::number(statisticsData->price));
    ui->labelOpen->setText(QString::number(statisticsData->open));
    ui->labelHigh->setText(QString::number(statisticsData->high));
    ui->labelLow->setText(QString::number(statisticsData->low));
    ui->labelChange->setText(QString::number(statisticsData->change));
    ui->labelChangePercent->setText(QString::number(statisticsData->changePercent));

    m_dataManager->downloadRealTimeQuoteData(code);

}

void MainWindow::networkError(const QUrl &url, const QString &errorString){

}

void MainWindow::stockActivated(Stock *stock){
    qDebug()<<"stockActivated:"<<stock->name();
    ui->tabCandlestick->showCandlesticks(stock->code());
    ui->stackedWidget->setCurrentWidget(ui->pageTradeInfo);
    ui->tabCandlestick->setFocus();

    m_dataManager->downloadRealTimeQuoteData(stock->code());
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
//        Stock *stock = ui->tabCandlestick->currentStock();
//        if(stock){
//            m_dataManager->downloadRealTimeQuoteData(stock->code());
//        }
    }
}

void MainWindow::timeout(){
    QDateTime dateTime = QDateTime::currentDateTime();
    QDate date = dateTime.date();
    QTime time = dateTime.time();
    if(date.dayOfWeek() > 5 || time < QTime(9, 30, 0) || time > QTime(15, 0, 0)){
        m_timer.stop();
    }

    if(ui->stackedWidget->currentWidget() == ui->pageTradeInfo){
        Stock *stock = ui->tabCandlestick->currentStock();
        if(stock){
            m_dataManager->downloadRealTimeQuoteData(stock->code());
        }
    }

    m_dataManager->downloadRealTimeStatisticsData(0, 3000, true);

}

void MainWindow::on_actionQuit_triggered(){
    qApp->quit();
}

void MainWindow::on_actionAbout_triggered(){
    QMessageBox::information(this, tr("About"), tr("<p align=center>Chinese stocks info view</p> Using customized QCustomPlot to draw candlesticks.<p align=right>Made by \350\264\272\350\276\211</p>"));
}



