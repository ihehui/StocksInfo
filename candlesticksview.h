#ifndef CANDLESTICKSVIEW_H
#define CANDLESTICKSVIEW_H

#include <QWidget>

#include "qcustomplot.h"
#include "qcpcandlechart.h"
#include "tradesummaryinfoview.h"
#include "common.h"
#include "datamanager.h"

class CandlesticksView : public QCustomPlot
{
    Q_OBJECT
public:
    explicit CandlesticksView(QWidget *parent = 0);

    void setDataManager(DataManager *manager);
    Stock * currentStock();
    //data
signals:
    void historicalDataRequested(QString *code, int offset);
    void stockChanged(const QString &code);
public slots:
    void historicalDataRead(Stock *stock);
    void showStock(const QString &code);
private:
    bool isEmpty();
private:
    DataManager *m_dataManager;
    Stock *m_curStock;
    QString m_stockCode; //证券代码
    QString m_stockName; //证券名称
    QString m_stockCodeExpected; //证券代码
    QSharedPointer<QCPFinancialDataContainer> m_ohlcData; //基本交易数据
    typedef QMap<double, TradeExtraData> TradeExtraDataMap;
    TradeExtraDataMap *m_tradeExtraDataMap; //交易数据

    //candle chart
private:
    void initCandlesticks();
    void initTickAndGridStyle(QCPAxis* axis);
    double getFocusKey(const QPointF& point);
    QPointF getFocusPoint(const double& key);

    double getZoomCenter();

private slots:
    void zoom(double zoomFactor);
    void adjustAllAndReplot();
    void adjustVolumeYRange();
private:
    QCPCandleChart *m_candleChart; //绘图类
    QCPAxisRect* m_vAxisRect;
    QCPBars *m_volumePos; //成交量柱
    QCPBars *m_volumeNeg;
    QCPAxis *m_vyAxis;

    //control
signals:
    void escape();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
private:
    bool m_mouseLeftPressing;
    double m_leftKey, m_focusedKey, m_rightKey; //X轴的左中右坐标

    //indicator
    //cross and info view
private slots:
    void setCrossAndInfoVisible(bool visible=true);
    void updateCrossCurvePoint(const QPointF &pos);
    void updateInfoView(const double& key);
private:
    bool m_crossVisble;
};

#endif // CANDLESTICKSVIEW_H
