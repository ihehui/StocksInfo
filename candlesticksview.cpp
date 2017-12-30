#include "candlesticksview.h"
#include "stock.h"
#include <QColor>
const QColor TRANSPARENT_QCOLOR(0,0,0,0);

const QColor CANDLE_POSITIVE_QCOLOR(15,195,81);
const QColor CANDLE_NEGATIVE_QCOLOR(255,61,61);

const QColor CANDLE_BACKGROUND_QCOLOR(40,44,48);
const QColor CANDLE_GRIDLINE_QCOLOR(44,50,54);
const QColor CANDLE_TEXT_QCOLOR_WHITE(187,193,199);
const QColor CANDLE_CROSSHAIR_QCOLOR(82,98,113);

const QMargins MARGINS_PLOT(0,0,0,0);
const QMargins MARGINS_AXISRECT_LAYOUT(30,0,30,16);

const double CANDLE_BOX_WIDTH = 0.8;
const int CANDLE_BASE_PENWIDTH = 1;
const double CANDLE_BASE_MARGIN_RATIO = 0.1;
const double CANDLE_BASE_ZOOM_RATIO = 0.2;

const QPen CANDLE_TICKLABEL_PEN(CANDLE_TEXT_QCOLOR_WHITE, CANDLE_BASE_PENWIDTH, Qt::SolidLine);
const QPen CANDLE_GRIDLINE_PEN(CANDLE_GRIDLINE_QCOLOR, CANDLE_BASE_PENWIDTH, Qt::SolidLine);
const QPen CANDLE_CROSSHAIR_PEN(CANDLE_CROSSHAIR_QCOLOR, CANDLE_BASE_PENWIDTH, Qt::DashLine);

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
    m_vxAxis = 0;
    m_vyAxis = 0;

    m_draggingByMouse = false;
    m_leftKey = 0;
    m_focusedKey = 0;
    m_rightKey = 0;

    m_tracerVisible = true;
    m_tracerCandle = 0;
    m_tracerVolume = 0;

    initCandlesticks();
    adjustAllAndReplot();
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

    //ticker
    QSharedPointer<QCPCandleTickerNum> customTicker(new QCPCandleTickerNum());
    yAxis->setTicker(customTicker);
    yAxis2->setTicker(customTicker);


    //坐标轴联动
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

    //设置蜡烛图坐标轴
    xAxis->setVisible(false);
    initTickAndGridStyle(yAxis);
    initTickAndGridStyle(yAxis2);

    //柱状图-成交量
    m_vAxisRect = new QCPAxisRect(this);
    m_vAxisRect->setRangeDrag(Qt::Horizontal);
    m_vAxisRect->setMaximumSize(QSize(QWIDGETSIZE_MAX, 100));
    m_vAxisRect->setMargins(MARGINS_PLOT);
    m_vAxisRect->axis(QCPAxis::atBottom)->setLayer("axes");
    m_vAxisRect->axis(QCPAxis::atBottom)->grid()->setLayer("grid");

    //设置成交量图坐标轴
    QCPAxis* vxAxis = m_vAxisRect->axis(QCPAxis::atBottom);
    QCPAxis* vyAxis = m_vAxisRect->axis(QCPAxis::atLeft);
    QCPAxis* vyAxis2 = m_vAxisRect->axis(QCPAxis::atRight);
    initTickAndGridStyle(vxAxis);
    initTickAndGridStyle(vyAxis);
    initTickAndGridStyle(vyAxis2);

    m_vxAxis = vxAxis;
    m_vyAxis = vyAxis;

    //Tickers
    //ticker
    QSharedPointer<QCPCandleTickerNum> customTickerV(new QCPCandleTickerNum());
    vyAxis->setTicker(customTickerV);
    vyAxis2->setTicker(customTickerV);


    // configure axes of both main and bottom axis rect:
    QSharedPointer<QCPCandleTickerDateTime> dateTimeTicker(new QCPCandleTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::UTC);
    dateTimeTicker->setDateTimeFormat("yyyy/MM/dd HH:mm:ss");
    vxAxis->setTicker(dateTimeTicker);

    //设置成交量图格式
    m_volumePos = new QCPBars(vxAxis, vyAxis);
    m_volumeNeg = new QCPBars(vxAxis, vyAxis);
    m_volumePos->setWidth(CANDLE_BOX_WIDTH);
    m_volumePos->setPen(Qt::NoPen);
    m_volumePos->setBrush(CANDLE_POSITIVE_QCOLOR);
    m_volumeNeg->setWidth(CANDLE_BOX_WIDTH);
    m_volumeNeg->setPen(Qt::NoPen);
    m_volumeNeg->setBrush(CANDLE_NEGATIVE_QCOLOR);

    //设置整体布局
    plotLayout()->insertRow(plotLayout()->rowCount());
    plotLayout()->addElement(plotLayout()->rowCount()-1, 0, m_vAxisRect);
    plotLayout()->setRowStretchFactor(0,4);
    plotLayout()->setRowStretchFactor(1,1);

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


    //调整大小
    rescaleAxes();

    //创建十字光标
    m_tracerCandle = new QCPItemTracer(this);
    m_tracerCandle->position->setAxisRect(this->axisRect());
    m_tracerCandle->position->setAxes(xAxis, yAxis);   ///<very important
    m_tracerCandle->setClipAxisRect(this->axisRect());   ///<very important
    initTracer(m_tracerCandle);

    m_tracerVolume = new QCPItemTracer(this);
    m_tracerVolume->position->setAxisRect(m_vAxisRect);
    m_tracerVolume->position->setAxes(m_vxAxis, m_vyAxis);   ///<very important
    m_tracerVolume->setClipAxisRect(m_vAxisRect);   ///<very important
    initTracer(m_tracerVolume);

    //设置坐标显示格式
    yAxis->setNumberPrecision(7);
    yAxis2->setNumberPrecision(7);
    m_vyAxis->setNumberPrecision(7);
    m_vAxisRect->axis(QCPAxis::atRight)->setNumberPrecision(7);
//    yAxis->setNumberFormat("f");
//    yAxis2->setNumberFormat("f");
//    m_vyAxis->setNumberFormat("f");
//    m_vAxisRect->axis(QCPAxis::atRight)->setNumberFormat("f");

    //创建指示文字
    initTracerText();
}

void CandlesticksView::initTracer(QCPItemTracer* tracer)
{
    if (!tracer) return;
//    tracer->setInterpolating(true);
    tracer->setStyle(QCPItemTracer::tsCrosshair);
    tracer->setVisible(m_tracerVisible);
    tracer->setPen(CANDLE_CROSSHAIR_PEN);
    tracer->position->setType(QCPItemPosition::ptPlotCoords);
}

void CandlesticksView::initTracerText()
{
    //创建光标提示

//    QCPItemText* m_leftLabel, *m_rightLabel;
//    QCPItemText* m_leftVLabel, *m_rightVLabel, *m_bottomLabel;

    m_leftTracerLabel = new QCPItemText(this);
    QCPLayer* legnedLayer = this->layer("legend");
    ///move text label to legend layer so that it can plot on top of axis rect
    if (legnedLayer) m_leftTracerLabel->setLayer(legnedLayer);
    m_leftTracerLabel->setClipAxisRect(this->axisRect());
    m_leftTracerLabel->setClipToAxisRect(false);      ///< make visible out of axis rect

    m_leftTracerLabel->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_leftTracerLabel->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
    m_leftTracerLabel->position->setTypeY(QCPItemPosition::ptPlotCoords);
    m_leftTracerLabel->position->setParentAnchorY(m_tracerCandle->position);   ///< y direction anchor to tracer
    m_leftTracerLabel->position->setCoords(0,0);

    m_leftTracerLabel->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ///make left padding real big to get nice visual effect
    m_leftTracerLabel->setPadding(QMargins(100,2,yAxis->tickLabelPadding(),2));


    m_leftTracerLabel->setColor(CANDLE_TEXT_QCOLOR_WHITE);
    m_leftTracerLabel->setBrush(QBrush(CANDLE_CROSSHAIR_QCOLOR));
    m_leftTracerLabel->setText("");

    m_rightTracerLabel = new QCPItemText(this);
    ///move text label to legend layer so that it can plot on top of axis rect
    if (legnedLayer) m_rightTracerLabel->setLayer(legnedLayer);
    m_rightTracerLabel->setClipAxisRect(this->axisRect());
    m_rightTracerLabel->setClipToAxisRect(false);      ///< make visible out of axis rect

    m_rightTracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_rightTracerLabel->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
    m_rightTracerLabel->position->setTypeY(QCPItemPosition::ptPlotCoords);
    m_rightTracerLabel->position->setParentAnchorY(m_tracerCandle->position);   ///< y direction anchor to tracer
    m_rightTracerLabel->position->setCoords(1.0,0);

    m_rightTracerLabel->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ///make left padding real big to get nice visual effect
    m_rightTracerLabel->setPadding(QMargins(yAxis->tickLabelPadding(),2,100,2));

    m_rightTracerLabel->setColor(CANDLE_TEXT_QCOLOR_WHITE);
    m_rightTracerLabel->setBrush(QBrush(CANDLE_CROSSHAIR_QCOLOR));
    m_rightTracerLabel->setText("");


    m_bottomTracerLabel = new QCPItemText(this);
    ///move text label to legend layer so that it can plot on top of axis rect
    if (legnedLayer) m_bottomTracerLabel->setLayer(legnedLayer);
    m_bottomTracerLabel->position->setAxisRect(m_vAxisRect);
    m_bottomTracerLabel->setClipAxisRect(m_vAxisRect);
    m_bottomTracerLabel->setClipToAxisRect(false);      ///< make visible out of axis rect

    m_bottomTracerLabel->setPositionAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_bottomTracerLabel->position->setTypeX(QCPItemPosition::ptPlotCoords);
    m_bottomTracerLabel->position->setParentAnchorX(m_tracerCandle->position);   ///< y direction anchor to tracer
    m_bottomTracerLabel->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
    m_bottomTracerLabel->position->setCoords(0,1.0);

    m_bottomTracerLabel->setTextAlignment(Qt::AlignCenter);
    m_bottomTracerLabel->setPadding(QMargins(2,m_vxAxis->labelPadding(),\
                                       2,m_vxAxis->labelPadding()));
    m_bottomTracerLabel->setColor(CANDLE_TEXT_QCOLOR_WHITE);
    m_bottomTracerLabel->setBrush(QBrush(CANDLE_CROSSHAIR_QCOLOR));
    m_bottomTracerLabel->setText("");
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
    setTracerAndInfoVisible(!m_tracerVisible);
}
void CandlesticksView::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
    if (event->button() == Qt::LeftButton){
        setCursor(Qt::DragMoveCursor);
        m_draggingByMouse = true;
    }
}
void CandlesticksView::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "CandlesticksView mouseReleaseEvent";
    QCustomPlot::mouseReleaseEvent(event);
    m_draggingByMouse = false;
    setCursor(Qt::ArrowCursor);
}
void CandlesticksView::mouseMoveEvent(QMouseEvent *event){
    QPoint pos = event->pos();
    static QPoint lastPos(pos);

    updateTracerAndInfo(pos);

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
            //m_draggingByMouse = false;
            event->ignore();
        }
    }
    lastPos = pos;
}

void CandlesticksView::wheelEvent(QWheelEvent* event){
    QCustomPlot::wheelEvent(event);
    int wheelSteps = event->delta()/120.0;
    zoom(1-wheelSteps*CANDLE_BASE_ZOOM_RATIO);
    //emit historicalDataRequested(&m_stockCodeExpected, -wheelSteps);

    updateTracerAndInfo(event->pos());
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
        int step = -1;
        if(event->modifiers() == Qt::ControlModifier) step *= 30;
        moveTracer(step);
    }
        break;
    case Qt::Key_Right:
    {
        int step = 1;
        if(event->modifiers() == Qt::ControlModifier) step *= 30;
        moveTracer(step);
    }
        break;
    default:
        break;
    }
}

double CandlesticksView::getZoomCenter()
{
    return m_tracerCandle->position->coords().x();
}

void CandlesticksView::zoom(double zoomFactor)
{
    if (isEmpty()) return;

    if (false == \
            m_candleChart->zoom(zoomFactor, getZoomCenter()) ){
        //can not zoom
        return;
    }
    replot();
}

void CandlesticksView::adjustAllAndReplot()
{
    rescaleAxes();
    m_candleChart->initAdjustAll();
    updateTracerAndInfo(QPoint(0,0));
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

void CandlesticksView::setTracerAndInfoVisible(bool visible)
{
    m_tracerVisible = visible;
    m_tracerCandle->setVisible(visible);
    m_tracerVolume->setVisible(visible);
    replot();
    //TODOs
}

void CandlesticksView::moveTracer(const int& step)
{
    double oldKey = m_tracerCandle->position->coords().x();
    double newKey = oldKey, newValue;
    double volumeValue = -100;
    if (m_candleChart->move(oldKey, step, newKey,newValue))
    {
        updateTracerAndInfoInner(newKey, newValue,volumeValue,true, false);
    }
}

void CandlesticksView::updateTracerAndInfo(const QPoint& pos)
{
    double volumeValue = m_vyAxis->pixelToCoord(pos.y());
    double candleKey = xAxis->pixelToCoord(pos.x());
    double candleValue = yAxis->pixelToCoord(pos.y());
    double candleFocusKey = candleKey;
    double candleFocusValue = candleValue;
    if (m_candleChart->getFocusKeyAndValue\
            (pos, candleFocusKey, candleFocusValue)){
        //can found focus
    }

    bool bInCandle = false;
    bool bInVolume = false;
    if (this->axisRect()->rect().contains(pos)) bInCandle = true;
    else if (m_vAxisRect->rect().contains(pos)) bInVolume = true;
    updateTracerAndInfoInner(candleFocusKey,candleValue,volumeValue,bInCandle,bInVolume);
}

void CandlesticksView::updateTracerAndInfoInner(const double& key, const double& candleValue,\
                              const double& volumeValue, const bool& bInCandle,\
                              const bool& bInVolume)
{

    m_tracerCandle->setVisible(m_tracerVisible);
    m_tracerVolume->setVisible(m_tracerVisible);
    m_leftTracerLabel->setVisible(true);
    m_rightTracerLabel->setVisible(true);
    m_bottomTracerLabel->setVisible(true);

    //更新十字光标
    m_tracerCandle->position->setCoords(key,candleValue);
    //必须要设置position的axes,setCoords才能对上,否则会默认使用qcustomplot的坐标轴!!!
    m_tracerVolume->position->setCoords(key,volumeValue);
//    m_tracerVolume->position->\
//            setPixelPosition(QPointF(m_tracerCandle->position->pixelPosition().x(),pos.y()));

    //更新指示文字，首先判断光标位置

    //如果处于光标处于K线图内
    if (bInCandle)
    {
        QCPCandleTickerNum* ticker = dynamic_cast <QCPCandleTickerNum*>(yAxis->ticker().data());
        if (0 != ticker)
        {
            m_leftTracerLabel->setText(\
                        ticker->getTickLabel(candleValue, this->locale(),
                                             yAxis->numberFormat().at(0), yAxis->numberPrecision()));
        }
    }
    //如果光标处于volume图内
    if (bInVolume)
    {
        QCPCandleTickerNum* ticker = dynamic_cast <QCPCandleTickerNum*>(m_vyAxis->ticker().data());
        if (0 != ticker)
        {
            m_leftTracerLabel->setText(\
                        ticker->getTickLabel(volumeValue, this->locale(),
                                             m_vyAxis->numberFormat().at(0), m_vyAxis->numberPrecision()));
        }
    }

    m_rightTracerLabel->setText(m_leftTracerLabel->text());
    QCPCandleTickerDateTime * bTicker = dynamic_cast <QCPCandleTickerDateTime*>(m_vxAxis->ticker().data());
    if (0 != bTicker)
    {
        m_bottomTracerLabel->setText(bTicker->getTickLabel(key, this->locale()));
    }

    if (!bInCandle && !bInVolume)
    {
        m_tracerCandle->setVisible(false);
        m_tracerVolume->setVisible(false);
        m_leftTracerLabel->setVisible(false);
        m_rightTracerLabel->setVisible(false);
        m_bottomTracerLabel->setVisible(false);
    }
    replot();

    //TODOs
}
