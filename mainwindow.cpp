#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidgetExtraInfo->hide();


    m_dataManager = new DataManager(this);
    m_dataManager->start();

    ui->tabCandlestick->setDataManager(m_dataManager);
    ui->tabCandlestick->showCandlesticks("000001");


}

MainWindow::~MainWindow()
{
    delete ui;

    m_dataManager->quit();
    delete m_dataManager;
}
