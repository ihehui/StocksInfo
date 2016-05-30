#include "tradesummaryinfoview.h"
#include "ui_tradesummaryinfoview.h"

#include <QDateTime>

#include "qcustomplot.h"
#include "common.h"


TradeSummaryInfoView::TradeSummaryInfoView(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    ui(new Ui::TradeSummaryInfoView)
{
    ui->setupUi(this);
}

TradeSummaryInfoView::~TradeSummaryInfoView()
{
    delete ui;
}

void TradeSummaryInfoView::updateTradeInfo(const QCPFinancialData &ohlcData, const TradeExtraData &extraData, double curValue, const QString &stockName){
    setWindowTitle(stockName);
    ui->labelTime->setText(QDateTime::fromTime_t(extraData.time).toString("yyyy-MM-dd hh:mm"));

    char buffer[65];
    sprintf(buffer, "%.2f", curValue);
    ui->labelCurValue->setText(QString(buffer));

    ui->labelOpen->setText(QString::number(ohlcData.open));
    ui->labelHigh->setText(QString::number(ohlcData.high));
    ui->labelLow->setText(QString::number(ohlcData.low));
    ui->labelClose->setText(QString::number(ohlcData.close));
    ui->labelVolume->setText(QString::number(extraData.volume/OneHundredMillion)+" \344\272\277");
    ui->labelTurnover->setText(QString::number(extraData.turnover/OneHundredMillion)+" \344\272\277");
    ui->labelTurnoverRate->setText(QString::number(extraData.exchangeRatio)+"%");

}

void TradeSummaryInfoView::closeEvent(QCloseEvent *event){
    emit closed(false);
    event->accept();
}

void TradeSummaryInfoView::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}

void TradeSummaryInfoView::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) ) {
        move(event->globalPos() - dragPosition);
    }
}

