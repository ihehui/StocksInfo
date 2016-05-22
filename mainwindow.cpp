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

    connect(ui->tabCandlestick, SIGNAL(historicalDataRequested(QString *, int)), m_dataManager, SLOT(readHistoricalData(QString *, int)), Qt::QueuedConnection);

    ui->tabCandlestick->setDataManager(m_dataManager);
    m_dataManager->moveToThread(&m_dataManagerThread);
    m_dataManagerThread.start();
    //m_dataManager->start();


    m_downloadManager = new DownloadManager();
    connect(m_downloadManager, SIGNAL(dataDownloaded(const QString &, const QUrl &)), m_dataManager, SLOT(dataDownloaded(const QString &, const QUrl &)));
    connect(m_dataManager, SIGNAL(requestDownloadData(const QString &)), m_downloadManager, SLOT(append(const QString &)));
    m_downloadManager->moveToThread(&m_downloadManagerThread);
    m_downloadManagerThread.start();

    ui->tabCandlestick->showCandlesticks("000001");


}

MainWindow::~MainWindow()
{
    delete ui;

    //m_dataManager->quit();
    //delete m_dataManager;

    m_dataManagerThread.quit();

    m_downloadManagerThread.quit();

}
