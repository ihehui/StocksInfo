#ifndef CANDLESTICKSVIEW_H
#define CANDLESTICKSVIEW_H

#include <QWidget>

#include "qcustomplot.h"
#include "tradesummaryinfoview.h"
#include "common.h"
#include "datamanager.h"


class CandlesticksView : public QCustomPlot
{
    Q_OBJECT
public:
    explicit CandlesticksView(QWidget *parent = 0);
    ~CandlesticksView();

    void setDataManager(DataManager *manager);
    Stock * currentStock();

signals:
    void historicalDataRequested(QString *code, int offset);


private slots:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);


    //区间统计
    void intervalStatistics();

    //区间放大
    void intervalZoomin();

    //右键
    void contextMenuRequest(QPoint pos);

    //设置坐标轴范围
    void setAxisRange2(const QCPRange &newRange, const QCPRange &oldRange);
    void setAxisRange();

    //绘制蜡烛图
    void drawCandlesticks();


    //生成交易信息小窗口和十字光标
    void createTradeInfoView();

    //更新交易信息小窗口内容及十字光标
    void updateTradeInfoView(const QPoint &pos, bool updateWhenInvisible = false);
    void updateTradeInfoView(double curKey, double curValue, bool updateWhenInvisible = false);
    void updateCrossCurvePoint(const QPointF &pos);

    //显示及交易信息小窗口和十字光标是否可见
    void setInfoViewVisible(bool visible);

    //更新成交量柱坐标轴范围
    void updateVolumeYAxisRange();

public slots:
    void historicalDataRead(Stock *stock);
    void showCandlesticks(const QString &code);


private:
    DataManager *m_dataManager;
    Stock *m_curStock;
    QString m_stockCode; //证券代码
    QString m_stockName; //证券名称
    QString m_stockCodeExpected; //证券代码


//    PeriodType m_periodType; //数据周期类型
    QCPFinancialDataMap *m_ohlcDataMap; //基本交易数据
    typedef QMap<double, TradeExtraData> TradeExtraDataMap;
    TradeExtraDataMap *m_tradeExtraDataMap; //交易数据

    QCPPlotTitle *m_plotTitle;
    QCPFinancial *m_candlesticks; //绘图类
    QCPAxis *m_yAxis; //Y轴
    QCPItemLine *m_horizontalLine, *m_verticalLine; //十字光标

    QCPAxisRect *volumeAxisRect;
    QCPBars *m_volumePos; //成交量柱
    QCPBars *m_volumeNeg;
    QCPAxis *m_volumeLeftAxis;

    TradeSummaryInfoView *m_infoView; //交易信息小窗口
    double m_leftKey, m_focusedKey, m_rightKey; //X轴的左中右坐标


};

#endif // CANDLESTICKSVIEW_H
