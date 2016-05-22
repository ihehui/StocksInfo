#ifndef TRADESUMMARYINFOVIEW_H
#define TRADESUMMARYINFOVIEW_H

#include <QWidget>
#include "stock.h"


namespace Ui {
class TradeSummaryInfoView;
}

class QCPFinancialData;

class TradeSummaryInfoView : public QWidget
{
    Q_OBJECT

public:
    explicit TradeSummaryInfoView(QWidget *parent = 0, Qt::WindowFlags f = Qt::Tool | Qt::WindowStaysOnTopHint);
    ~TradeSummaryInfoView();

public slots:
    void updateTradeInfo(const QCPFinancialData &ohlcData, const TradeExtraData &extraData, double curValue, const QString &stockName);

signals:
    void closed(bool visible = false);

protected:
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);


private:
    Ui::TradeSummaryInfoView *ui;

    QPoint dragPosition;

};

#endif // TRADESUMMARYINFOVIEW_H
