#include "candlesticksview.h"
#include "stock.h"
#include <QColor>
const QColor TRANSPARENT_QCOLOR(0,0,0,0);

const QColor CANDLE_POSITIVE_QCOLOR(15,195,81);
const QColor CANDLE_NEGATIVE_QCOLOR(255,61,61);

const QColor CANDLE_BACKGROUND_QCOLOR(40,44,48);
const QColor CANDLE_GRIDLINE_QCOLOR(44,50,54);
const QColor CANDLE_TEXT_QCOLOR_WHITE(187,193,199);
const QColor CANDLE_CROSSCRUVE_QCOLOR(82,98,113);

const QMargins MARGINS_PLOT(0,0,0,0);
const QMargins MARGINS_AXISRECT_LAYOUT(10,10,10,10);
const double CANDLE_BOX_WIDTH = 0.8;
const int CANDLE_BASEPENWIDTH = 1;
const double CANDLE_BASE_MARGIN_RATIO = 0.1;
const double CANDLE_BASE_ZOOM_RATIO = 0.1;

const QPen CANDLE_GRIDLINE_PEN(CANDLE_GRIDLINE_QCOLOR, CANDLE_BASEPENWIDTH, Qt::SolidLine);
const QPen CANDLE_CROSS_PEN(CANDLE_CROSSCRUVE_QCOLOR, CANDLE_BASEPENWIDTH, Qt::DashLine);

CandlesticksView::CandlesticksView(QWidget *parent)
    : QCustomPlot(parent)
{
    qDebug()<<"CandlesticksView:"<<QThread::currentThreadId();

    m_dataManager = 0;
    m_curStock = 0;
    m_stockCode = "";
    m_stockName = "";
    m_stockCodeExpected = "";
    m_ohlcData = 0;
    m_tradeExtraDataMap = 0;

    m_candleChart = 0;
    m_vAxisRect = 0;
    m_volumePos = 0;
    m_volumeNeg = 0;
    m_vyAxis = 0;

    m_draggingByMouse = false;
    m_leftKey = 0;
    m_focusedKey = 0;
    m_rightKey = 0;

    m_crossVisble = true;

    initCandlesticks();
}


void CandlesticksView::initCandlesticks(){

    //设置为全都不抗锯齿,否则画出的图有点模糊,不够犀利
    this->setNotAntialiasedElements(QCP::aeAll);
    this->setBackground(QBrush(CANDLE_BACKGROUND_QCOLOR));
    this->setAutoAddPlottableToLegend(false);
    this->plotLayout()->setMargins(MARGINS_PLOT);

    this->axisRect()->layout()->setMargins(MARGINS_AXISRECT_LAYOUT);
    this->setInteractions(QCP::iRangeDrag);
    this->axisRect()->setRangeDrag(Qt::Horizontal);

    //坐标轴
    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), \
            xAxis2, SLOT(setRange(QCPRange)));
    connect(yAxis, SIGNAL(rangeChanged(QCPRange)),
            yAxis2, SLOT(setRange(QCPRange)));

    //创建蜡烛图
    m_candleChart = new QCPCandleChart(xAxis, yAxis);
    m_candleChart->setName("Candlestick");
    m_candleChart->setChartStyle(QCPFinancial::csCandlestick);
    m_candleChart->setWidth(CANDLE_BOX_WIDTH);
    m_candleChart->setWidthType(QCPFinancial::wtPlotCoords);
    m_candleChart->setTwoColored(true);
    m_candleChart->setBrush(Qt::NoBrush);
    m_candleChart->setBrushPositive(Qt::NoBrush);
    m_candleChart->setPenPositive(QPen(CANDLE_POSITIVE_QCOLOR));
    m_candleChart->setBrushNegative(CANDLE_NEGATIVE_QCOLOR);
    m_candleChart->setPenNegative(QPen(CANDLE_NEGATIVE_QCOLOR));
    m_candleChart->setSelectable(QCP::stNone);

    //设置蜡烛图格式等
    this->axisRect()->setCrossCurveVisible(true);
    this->axisRect()->setCrossCurvePen(CANDLE_CROSS_PEN);

    xAxis->setVisible(false);
    initTickAndGridStyle(yAxis);
    initTickAndGridStyle(yAxis2);

    //柱状图-成交量
    m_vAxisRect = new QCPAxisRect(this);
    m_vAxisRect->setRangeDrag(Qt::Horizontal);
    m_vAxisRect->setMaximumSize(QSize(QWIDGETSIZE_MAX, 100));
    //volumeAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    m_vAxisRect->setMargins(MARGINS_PLOT);
    m_vAxisRect->axis(QCPAxis::atBottom)->setLayer("axes");
    m_vAxisRect->axis(QCPAxis::atBottom)->grid()->setLayer("grid");
    //volumeAxisRect->layout()->setMargins(axisRectLayoutMargins);
    m_vAxisRect->setCrossCurveVisible(true);
    m_vAxisRect->setCrossCurvePen(CANDLE_CROSS_PEN);

    QCPAxis* vxAxis = m_vAxisRect->axis(QCPAxis::atBottom);
    QCPAxis* vyAxis = m_vAxisRect->axis(QCPAxis::atLeft);
    QCPAxis* vyAxis2 = m_vAxisRect->axis(QCPAxis::atRight);
    initTickAndGridStyle(vxAxis);
    initTickAndGridStyle(vyAxis);
    initTickAndGridStyle(vyAxis2);

    //Tickers
    // configure axes of both main and bottom axis rect:
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::UTC);
    dateTimeTicker->setDateTimeFormat("yyyy/MM/dd HH:mm:ss");
    vxAxis->setTicker(dateTimeTicker);

    m_volumePos = new QCPBars(vxAxis, vyAxis);
    m_volumeNeg = new QCPBars(vxAxis, vyAxis);
    m_volumePos->setWidth(CANDLE_BOX_WIDTH);
    m_volumePos->setPen(Qt::NoPen);
    m_volumePos->setBrush(CANDLE_POSITIVE_QCOLOR);
    m_volumeNeg->setWidth(CANDLE_BOX_WIDTH);
    m_volumeNeg->setPen(Qt::NoPen);
    m_volumeNeg->setBrush(CANDLE_NEGATIVE_QCOLOR);

    plotLayout()->insertRow(plotLayout()->rowCount());
    plotLayout()->addElement(plotLayout()->rowCount()-1, 0, m_vAxisRect);

    // make axis rects' left side line up:
    QCPMarginGroup *group = new QCPMarginGroup(this);
    axisRect()->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    m_vAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);

    // interconnect x axis ranges of main and bottom axis rects:
    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), \
            vxAxis, SLOT(setRange(QCPRange)));
    connect(vxAxis, SIGNAL(rangeChanged(QCPRange)), \
            xAxis, SLOT(setRange(QCPRange)));
    connect(vxAxis, SIGNAL(rangeChanged(QCPRange)), \
            m_vAxisRect->axis(QCPAxis::atTop), SLOT(setRange(QCPRange)));
    connect(vyAxis, SIGNAL(rangeChanged(QCPRange)), \
            vyAxis2, SLOT(setRange(QCPRange)));

    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), \
            this, SLOT(adjustVolumeYRange()));

    m_vyAxis = vyAxis;
    //调整大小
    rescaleAxes();
}

void CandlesticksView::initTickAndGridStyle(QCPAxis* axis)
{
    if (Q_NULLPTR == axis) return;
    axis->setVisible(true);
    axis->setTickLabels(true);
    axis->setTicks(true);
    axis->setSubTicks(false);
    axis->setBasePen(Qt::NoPen);
    axis->setTickPen(Qt::NoPen);
    axis->setTickLabelColor(CANDLE_TEXT_QCOLOR_WHITE);
    axis->grid()->setPen(CANDLE_GRIDLINE_PEN);
    //axis->grid()->setSubGridPen(CANDLE_GRIDLINE_PEN);
    //axis->grid()->setSubGridVisible(true);
    axis->grid()->setZeroLinePen(CANDLE_GRIDLINE_PEN);
}

void CandlesticksView::setDataManager(DataManager *manager){
    m_dataManager = manager;
}

Stock * CandlesticksView::currentStock(){
    return m_curStock;
}


bool CandlesticksView::isEmpty()
{
    return (!m_curStock || m_ohlcData.isNull() || m_ohlcData->isEmpty() || !m_candleChart);
}

void CandlesticksView::historicalDataRead(Stock *stock){
    qDebug()<<"--CandlesticksView::historicalDataRead() "<<stock->name();
    Q_ASSERT(stock);
    if(!stock || stock->code() != m_stockCodeExpected){return;}
    m_curStock = stock;
    m_stockCode = stock->code();
    m_stockName = stock->name();
    emit stockChanged(m_stockCode);

    m_ohlcData = stock->ohlcDataContainer();
    m_candleChart->setData(m_ohlcData);

    m_tradeExtraDataMap = stock->tradeExtraDataMap();

    QSharedPointer<QCPBarsDataContainer> negData, posData;
    negData.reset(new QCPBarsDataContainer());
    posData.reset(new QCPBarsDataContainer());
    QCPFinancialDataContainer::const_iterator it = m_ohlcData->constBegin();
    while(it != m_ohlcData->constEnd()){
        TradeExtraData extraData = m_tradeExtraDataMap->value(it->key);
        if (it->close < it->open){
            negData->add(QCPBarsData(it->key, extraData.volume));
        }else{
            posData->add(QCPBarsData(it->key, extraData.volume));
        }
        ++it;
    }
    m_volumeNeg->setData(negData);
    m_volumePos->setData(posData);

    if(!m_tradeExtraDataMap->isEmpty()){
        m_focusedKey = m_tradeExtraDataMap->lastKey();
    }
    adjustAllAndReplot();
}

void CandlesticksView::showStock(const QString &code){
    if(code != m_stockCode){
        m_stockCode = code;
        m_stockCodeExpected = code;
        emit historicalDataRequested(&m_stockCodeExpected, 0);
    }
}

void CandlesticksView::mouseDoubleClickEvent(QMouseEvent *event){
    QCustomPlot::mouseDoubleClickEvent(event);
    if (isEmpty()) return;
    setCrossAndInfoVisible(!m_crossVisble);
}

void CandlesticksView::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
    if (event->button() == Qt::LeftButton){
        m_draggingByMouse = true;
    }
}
void CandlesticksView::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "CandlesticksView mouseReleaseEvent";
    QCustomPlot::mouseReleaseEvent(event);
    m_draggingByMouse = false;
}
void CandlesticksView::mouseMoveEvent(QMouseEvent *event){
    QPoint pos = event->pos();
    static QPoint lastPos(pos);

    if (isEmpty()){
        lastPos = pos;
        QCustomPlot::mouseMoveEvent(event);
        return;
    }
    if (m_draggingByMouse)
    {
        if (m_candleChart->canDrag(pos.x()-lastPos.x())){
            QCustomPlot::mouseMoveEvent(event);
            m_candleChart->setKeyAxisAutoFitGrid();
            replot();
        }else{
            m_draggingByMouse = false;
        }
    }

    lastPos = pos;

    //    double key = getFocusKey(pos);
    //TODO
    //    updateInfoView(key);
    //    updateCrossCurvePoint(getFocusPoint(key));

}

void CandlesticksView::wheelEvent(QWheelEvent* event){
    //QCustomPlot::wheelEvent(event);
    if (isEmpty()) return;
    //    int wheelSteps = event->delta()/120.0;
    //    //    if(event->modifiers() == Qt::ControlModifier){
    //    //        //CTRL+滚轮 进制缩放
    //    //        double factor;
    //    //        factor = qPow(0.85, wheelSteps);
    //    //        xAxis->scaleRange(factor, xAxis->pixelToCoord(event->pos().x()));
    //    //        replot();

    //    //        event->accept();
    //    //    }else{
    //    //        event->ignore();
    //    //    }
    //    m_stockCodeExpected = m_stockCode;
    //    emit historicalDataRequested(&m_stockCodeExpected, -wheelSteps);
}

void CandlesticksView::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    if (isEmpty()) {return;}
    double factor = (1.0*size().width())/event->oldSize().width();
    m_candleChart->adjustKeyRangeOnResize(factor);
    replot();
}

void CandlesticksView::keyPressEvent(QKeyEvent *event){
    QCustomPlot::keyPressEvent(event);

    if(event->key() != Qt::Key_Escape && isEmpty()){return;}

    switch (event->key()) {
    case Qt::Key_Escape:
    {
        emit escape();
    }
        break;
    case Qt::Key_Up:
    case Qt::Key_Plus:
    {
        zoom(1-CANDLE_BASE_ZOOM_RATIO);
        event->accept();
    }
        break;
    case Qt::Key_Minus:
    case Qt::Key_Down:
    {
        zoom(1+CANDLE_BASE_ZOOM_RATIO);
        event->accept();
    }
        break;
    case Qt::Key_Left:
    {
    }
        break;
    case Qt::Key_Right:
    {
    }
        break;
    default:
        break;
    }
}

double CandlesticksView::getFocusKey(const QPointF& point)
{
    //TODO
    return 0;
}
QPointF CandlesticksView::getFocusPoint(const double& key)
{
    //TODO
    return QPointF();
}

double CandlesticksView::getZoomCenter()
{
    return m_focusedKey;
}

void CandlesticksView::zoom(double zoomFactor)
{
    if (isEmpty()) return;

    if (false == \
            m_candleChart->zoom(zoomFactor, getZoomCenter()) )
    {
        //can not zoom
        return;
    }
    replot();
}

void CandlesticksView::adjustAllAndReplot()
{
    rescaleAxes();
    if (isEmpty()) return;
    //TODO adjust X range
    m_candleChart->initAdjustAll();
    replot();
}

void CandlesticksView::adjustVolumeYRange(){

    if (isEmpty()) return;

    double maxYValue = std::numeric_limits<double>::min();
    TradeExtraDataMap::const_iterator leftkeyIT = m_tradeExtraDataMap->lowerBound(xAxis->range().lower);
    TradeExtraDataMap::const_iterator rightkeyIT = m_tradeExtraDataMap->lowerBound(xAxis->range().upper);
    TradeExtraDataMap::const_iterator it;
    for (it = leftkeyIT; it != rightkeyIT; ++it)
    {
        TradeExtraData extraData = it.value();
        double currentVol = extraData.volume;
        if (currentVol > maxYValue){
            maxYValue = currentVol;
        }
    }
    maxYValue += CANDLE_BASE_MARGIN_RATIO*maxYValue;
    m_vyAxis->setRange(0, maxYValue);
}

void CandlesticksView::setCrossAndInfoVisible(bool visible)
{
    m_crossVisble = visible;
    axisRect()->setCrossCurveVisible(m_crossVisble);
    m_vAxisRect->setCrossCurveVisible(m_crossVisble);
    if (isEmpty()) return;

    //TODOs
}

void CandlesticksView::updateCrossCurvePoint(const QPointF &pos){
    for(int i=0; i < axisRectCount(); i++){
        axisRect(i)->setCrossCurvePoint(pos);
    }
    replot(QCustomPlot::rpQueuedReplot);
}

void CandlesticksView::updateInfoView(const double& key)
{
    if (isEmpty()) return;
    //TODO
}

////==========================================================

//class CandlesticksView : public QCustomPlot
//{
//    Q_OBJECT
//public:
//    explicit CandlesticksView(QWidget *parent = 0);

//    void setDataManager(DataManager *manager);
//    Stock * currentStock();

//    //data
//signals:
//    void historicalDataRequested(QString *code, int offset);
//    void stockChanged(const QString &code);
//public slots:
//    void historicalDataRead(Stock *stock);
//private:
//    DataManager *m_dataManager;
//    Stock *m_curStock;
//    QString m_stockCode; //证券代码
//    QString m_stockName; //证券名称
//    QString m_stockCodeExpected; //证券代码
//    QSharedPointer<QCPFinancialDataContainer> m_ohlcData; //基本交易数据
//    typedef QMap<double, TradeExtraData> TradeExtraDataMap;
//    TradeExtraDataMap *m_tradeExtraDataMap; //交易数据

//    //candle chart
//private:
//    void drawCandlesticks();
//    void setTickAndGridStyle(QCPAxis* axis);
//    void showCandlesticks(const QString &code);
//    QPointF getFocusPoint();
//    double getFocusKey();
//private slots:

//    void zoomIn();
//    void zoomOut();
//    void moveLeft();
//    void moveRight();

//    void updateyAxisRange();
//    void updateVolumeYAxisRange();
//private:
//    QCPFinancial *m_candlesticks; //绘图类
//    QCPAxisRect* m_vAxisRect;
//    QCPBars *m_volumePos; //成交量柱
//    QCPBars *m_volumeNeg;
//    QCPAxis *m_vyAxis;

//    //control
//signals:
//    void escape();
//protected:
//    void mouseDoubleClickEvent(QMouseEvent *event);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *event);
//    void wheelEvent(QWheelEvent *event);
//    void keyPressEvent(QKeyEvent *event);
//private:
//    double m_leftKey, m_focusedKey, m_rightKey; //X轴的左中右坐标

//    //indicator
//    //cross and info view
//private slots:
//    void toggleCrossAndInfoVisible();
//    void updateCrossCurvePoint(const QPointF &pos);
//    void updateInfoView(const double& key);
//private:
//    bool m_crossVisble;
//};


