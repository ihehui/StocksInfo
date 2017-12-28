#include "candlesticksview.h"
#include "stock.h"
#include <QColor>

const QColor POSITIVE_QCOLOR = QColor(15,195,81);
const QColor NEGATIVE_QCOLOR = QColor(255,61,61);
const QColor TRANSPARENT_QCOLOR = QColor(0,0,0,0);

const QColor BACKGROUND_QCOLOR = QColor(40,44,48);
const QColor GRIDLINE_QCOLOR = QColor(44,50,54);
const QColor TEXT_QCOLOR_WHITE = QColor(187,193,199);

const QColor CROSSCRUVE_QCOLOR = QColor(82,98,113);

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

    //m_plotTitle = 0;
    m_candlesticks = 0;
    m_yAxis = 0;
    m_horizontalLine = 0;
    m_verticalLine = 0;

    volumeAxisRect = 0;
    m_volumePos = 0;
    m_volumeNeg = 0;
    m_volumeLeftAxis = 0;

    m_infoView = 0;
    m_leftKey = 0;
    m_focusedKey = 0;
    m_rightKey = 0;


    setInteractions(/*QCP::iRangeDrag | QCP::iRangeZoom |*/ QCP::iSelectAxes |
                    QCP::iSelectLegend | QCP::iSelectPlottables);


    //鼠标事件
    //connect(this, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(slotMouseDoubleClick(QMouseEvent*)));
    //connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(slotMousePress(QMouseEvent*)));
    //connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(slotMouseMove(QMouseEvent*)));
    //connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(slotMouseRelease(QMouseEvent*)));
    //connect(this, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(slotMouseWheel(QWheelEvent*)));


    //坐标轴
    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));
    //connect(yAxis2, SIGNAL(rangeChanged(QCPRange)), yAxis, SLOT(setRange(QCPRange)));
    connect(xAxis, SIGNAL(rangeChanged(const QCPRange &, const QCPRange &)), this, SLOT(setAxisRange()));
    //connect(xAxis, SIGNAL(rangeChanged(const QCPRange &, const QCPRange &)), this, SLOT(setAxisRange2(const QCPRange &, const QCPRange &)));


    //右键
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    drawCandlesticks();

}

CandlesticksView::~CandlesticksView(){
    delete m_infoView;
}

void CandlesticksView::setDataManager(DataManager *manager){
    //if(!manager){return;}
    m_dataManager = manager;
    //m_stockCode = m_allStocks.first();
    //connect(m_dataManager, SIGNAL(historicalDataRead(QString)), this, SLOT(historicalDataRead(QString)), Qt::QueuedConnection);
    //connect(this, SIGNAL(historicalDataRequested(QString *, int)), m_dataManager, SLOT(readHistoricalData(QString *, int)), Qt::QueuedConnection);
}

Stock * CandlesticksView::currentStock(){
    return m_curStock;
}

void CandlesticksView::mouseDoubleClickEvent(QMouseEvent *event){
    if(!m_curStock){return;}
    if(m_ohlcData->isEmpty()){return;}

    if(!m_infoView){
        setInfoViewVisible(true);
        QPoint bottomLeft = frameGeometry().bottomLeft();
        m_infoView->move(QPoint(bottomLeft.x(), bottomLeft.y()-m_infoView->frameGeometry().height()));
        updateTradeInfoView(event->pos(), true);

//        生成鼠标移动事件
//        QMouseEvent e(QEvent::MouseMove, event->localPos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
//        slotMouseMove(&e);
    }else{
        bool visible = m_infoView->isVisible();
        setInfoViewVisible(!visible);
        if(!visible){
            updateTradeInfoView(event->pos(), true);
        }
    }

    //qDebug()<<QString("--mouseDoubleClickEvent--");
}

void CandlesticksView::mousePressEvent(QMouseEvent *event){
    if(!m_curStock){return;}
    if(m_ohlcData->isEmpty()){return;}

    Q_UNUSED(event)
    //qDebug()<<QString("--mousePressEvent--");
}

void CandlesticksView::mouseMoveEvent(QMouseEvent *event){
    if(!m_curStock){return;}
    if(m_ohlcData->isEmpty()){return;}

    updateTradeInfoView(event->pos());
    //qDebug()<<QString("--mouseMoveEvent--");
}

void CandlesticksView::mouseReleaseEvent(QMouseEvent *event){
    if(!m_curStock){return;}
    if(m_ohlcData->isEmpty()){return;}

    Q_UNUSED(event)
    //qDebug()<<QString("--mouseReleaseEvent--");
}

void CandlesticksView::wheelEvent(QWheelEvent* event){
    if(!m_curStock){return;}

    int wheelSteps = event->delta()/120.0;
    if(event->modifiers() == Qt::ControlModifier){
        //CTRL+滚轮 进制缩放
        double factor;
        factor = qPow(0.85, wheelSteps);
        xAxis->scaleRange(factor, xAxis->pixelToCoord(event->pos().x()));
        replot();

        event->accept();
    }else{
        event->ignore();
    }

    Q_ASSERT(m_dataManager);
    //m_candlesticks->clearData();
    //m_dataManager->readHistoricalData(&m_stockCode, -wheelSteps);
    m_stockCodeExpected = m_stockCode;
    emit historicalDataRequested(&m_stockCodeExpected, -wheelSteps);

}

void CandlesticksView::keyPressEvent(QKeyEvent *event){
    if(!m_curStock){return;}
    if(event->key() != Qt::Key_Escape && m_ohlcData->isEmpty()){return;}


    switch (event->key()) {
    case Qt::Key_Escape:
    {
        if(m_infoView && m_infoView->isVisible()){
            //隐藏交易信息小窗口
            setInfoViewVisible(false);
        }else{
            emit escape();
        }
    }
        break;

    case Qt::Key_Up:
    case Qt::Key_Down:
    {
        //上箭头：放大， 下箭头：缩小
        double steps = (event->key()==Qt::Key_Up)?1:-1;
        double factor = qPow(0.85, steps);
        xAxis->scaleRange(factor, m_focusedKey);
        replot();

        event->accept();
    }
        break;

    case Qt::Key_Left:
    case Qt::Key_Right:
    {
        setInfoViewVisible(true);

        //左右箭头：左右移动十字光标，按住SHIFT平移
        if((event->key() == Qt::Key_Left && m_focusedKey == m_tradeExtraDataMap->firstKey())
                || (event->key() == Qt::Key_Right && m_focusedKey == m_tradeExtraDataMap->lastKey())
                ){
            return;
        }

        QMap<double, TradeExtraData>::const_iterator curkeyIT = m_tradeExtraDataMap->lowerBound(m_focusedKey);
        curkeyIT = (event->key() == Qt::Key_Left)?(--curkeyIT):(++curkeyIT);
        m_focusedKey = curkeyIT.key();
        if((event->key() == Qt::Key_Left) && m_focusedKey < m_leftKey){
            m_focusedKey = m_leftKey;
        }
        if((event->key() == Qt::Key_Right) && m_focusedKey > m_rightKey){
            m_focusedKey = m_rightKey;
        }

//        qDebug()<<QDateTime::fromTime_t( m_tradeExtraDataMap->value(m_leftKey).time).toString("L: MM-dd");
//        qDebug()<<QDateTime::fromTime_t( m_tradeExtraDataMap->value(m_focusedKey).time).toString("C: MM-dd");
//        qDebug()<<QDateTime::fromTime_t( m_tradeExtraDataMap->value(m_rightKey).time).toString("R: MM-dd");

        //移动坐标轴
        if(event->modifiers() == Qt::ShiftModifier
                || (isEqual(m_focusedKey, m_leftKey) && event->key() == Qt::Key_Left)
                || (isEqual(m_focusedKey, m_rightKey) && event->key() == Qt::Key_Right)
                ){

            double diff = m_candlesticks->width();
            if(event->key() == Qt::Key_Left){diff = -diff;}
            xAxis->setRange(xAxis->range().lower+diff, xAxis->range().upper+diff);
            replot();
        }


        //更新显示交易信息
        if(!m_infoView){
            createTradeInfoView();
            setInfoViewVisible(true);
            m_infoView->move(0, 0);
            m_focusedKey = m_rightKey;
        }
//        QCPFinancialData ohlcData = m_ohlcData->value(m_focusedKey);
//        if(isZero(ohlcData.open)){return;}
//        updateTradeInfoView(m_focusedKey, ohlcData.close);

        QPointF pos;
        //m_candlesticks->coordsToPoint(m_focusedKey, ohlcData.close, pos);
        updateCrossCurvePoint(pos);


        //updateVolumeYAxisRange();

    }
        break;


    default:
        break;
    }


}


void CandlesticksView::intervalStatistics(){
    //TODO
}

void CandlesticksView::intervalZoomin(){
    //TODO
}

void CandlesticksView::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction("Interval Statistics", this, SLOT(intervalStatistics()));
    menu->addAction("Interval Zoomin", this, SLOT(intervalZoomin()));

    menu->popup(mapToGlobal(pos));
}

void CandlesticksView::setAxisRange2(const QCPRange &newRange, const QCPRange &oldRange){

    QCPRange valueRange(0,0), leftBoxRange(0,0), rightBoxRange(0,0);
    //m_candlesticks->getBoundValuesInVisibleRange(valueRange, leftBoxRange, rightBoxRange);
    if(m_yAxis->range().lower != valueRange.lower || m_yAxis->range().upper != valueRange.upper){
        m_yAxis->setRange(valueRange);
    }

    double lower = xAxis->range().lower, upper = xAxis->range().upper;

    if(oldRange.lower < xAxis->range().lower){
        qDebug()<<"Zoomin";

        if(leftBoxRange.lower < xAxis->range().lower && leftBoxRange.upper > xAxis->range().lower){
            lower = leftBoxRange.upper;
            //xAxis->setRangeLower(leftBoxRange.upper);
        }

        if(rightBoxRange.upper > xAxis->range().upper && rightBoxRange.lower < xAxis->range().upper){
            upper = rightBoxRange.lower;
            //xAxis->setRangeUpper(rightBoxRange.lower);
            qDebug()<<"-i2-";
        }




    }else{
        qDebug()<<"Zoomout";



        if((leftBoxRange.upper < xAxis->range().lower && leftBoxRange.upper > xAxis->range().lower) || leftBoxRange.lower < xAxis->range().lower){
            lower = leftBoxRange.lower;
            // xAxis->setRangeLower(leftBoxRange.upper);
        }

        double minX = m_tradeExtraDataMap->firstKey() - m_candlesticks->width();
        if(lower < minX){
            lower = minX;
        }

        if((rightBoxRange.upper > xAxis->range().upper && rightBoxRange.lower < xAxis->range().upper) || rightBoxRange.upper < xAxis->range().upper){
            upper = rightBoxRange.upper;
            //xAxis->setRangeUpper(rightBoxRange.lower);
        }
    }

    if(lower != xAxis->range().lower || upper != xAxis->range().upper){
        xAxis->setRange(lower, upper);
    }


}

void CandlesticksView::setAxisRange(){
    //qDebug()<<"----setAxisRange()------";

    double minValue = 0, maxValue = 0;
    uint itemCount = 0;
    //m_candlesticks->getBoundValuesInVisibleRange(minValue, maxValue, itemCount, &m_leftKey, &m_rightKey);
    if(m_yAxis->range().lower != minValue || m_yAxis->range().upper != maxValue){
        m_yAxis->setRange(minValue, maxValue);
    }

    double xAxisLower = xAxis->range().lower;
    double xAxisUpper = xAxis->range().upper;

    double leftBoxRangeLlower = m_leftKey-m_candlesticks->width()*0.5;
    double leftBoxRangeUpper = m_leftKey+m_candlesticks->width()*0.5;
    double rightBoxRangeLlower = m_rightKey-m_candlesticks->width()*0.5;
    double rightBoxRangeUpper = m_rightKey+m_candlesticks->width()*0.5;
//    if( ((m_leftKey == m_tradeExtraDataMap->firstKey()) && (xAxisLower < leftBoxRangeLlower))){
//        xAxis->setRangeLower(leftBoxRangeLlower);
//    }
//    if(m_rightKey == m_tradeExtraDataMap->lastKey() && xAxisUpper > rightBoxRangeUpper){
//        xAxis->setRangeUpper(rightBoxRangeUpper);
//    }

//    if( (xAxisLower > leftBoxRangeLlower && xAxisLower <= leftBoxRangeUpper)){
//        //TODO:显示不完整
//    }

    if( (xAxisUpper >= rightBoxRangeLlower && xAxisUpper < rightBoxRangeUpper) ){
        //TODO:显示不完整
    }


    //updateVolumeYAxisRange();

}


void CandlesticksView::drawCandlesticks(){

    //设置为全都不抗锯齿,否则画出的图有点模糊,不够犀利
    this->setNotAntialiasedElements(QCP::aeAll);
    this->setBackground(QBrush(BACKGROUND_QCOLOR));
    this->setAutoAddPlottableToLegend(false);

    this->plotLayout()->setRowSpacing(10);

    double boxWidth = 0.8;
    //创建蜡烛图
    m_candlesticks = new QCPFinancial(xAxis, yAxis);
    m_candlesticks->setName("Candlestick");
    m_candlesticks->setChartStyle(QCPFinancial::csCandlestick);
    m_candlesticks->setWidth(boxWidth);
    m_candlesticks->setWidthType(QCPFinancial::wtPlotCoords);
    m_candlesticks->setTwoColored(true);
    m_candlesticks->setBrush(Qt::NoBrush);
    m_candlesticks->setBrushPositive(Qt::NoBrush);
    m_candlesticks->setPenPositive(QPen(POSITIVE_QCOLOR));
    m_candlesticks->setBrushNegative(NEGATIVE_QCOLOR);
    m_candlesticks->setPenNegative(QPen(NEGATIVE_QCOLOR));
    m_candlesticks->setSelectable(QCP::stNone);

    //设置蜡烛图格式等
    this->axisRect()->setCrossCurveVisible(true);
    this->axisRect()->layout()->setMargins(QMargins(10,10,10,10));

    QPen crossPen(CROSSCRUVE_QCOLOR);
    crossPen.setStyle(Qt::DashLine);
    crossPen.setWidth(1);
    this->axisRect()->setCrossCurvePen(crossPen);

    xAxis->setTickLabels(false);
    xAxis->setTicks(false);
    xAxis->setSubTicks(false);
    xAxis->setBasePen(Qt::NoPen);
    xAxis->setTickPen(Qt::NoPen);
    xAxis->grid()->setPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    xAxis->grid()->setSubGridPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    xAxis->grid()->setSubGridVisible(true);

    m_yAxis = yAxis;
    yAxis->setTickLabels(true);
    yAxis->setTicks(true);
    yAxis->setSubTicks(false);
    yAxis->setBasePen(Qt::NoPen);
    yAxis->setTickPen(Qt::NoPen);
    yAxis->setTickLabelColor(TEXT_QCOLOR_WHITE);
    yAxis->grid()->setPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    yAxis->grid()->setSubGridPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    yAxis->grid()->setSubGridVisible(true);

    yAxis2->setVisible(true);
    yAxis2->setTickLabels(true);
    yAxis2->setTicks(true);
    yAxis2->setSubTicks(false);
    yAxis2->setBasePen(Qt::NoPen);
    yAxis2->setTickPen(Qt::NoPen);
    yAxis2->setTickLabelColor(TEXT_QCOLOR_WHITE);
    yAxis2->grid()->setPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    yAxis2->grid()->setSubGridPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    yAxis2->grid()->setSubGridVisible(true);


    //柱状图-成交量
    volumeAxisRect = new QCPAxisRect(this);
    volumeAxisRect->setMaximumSize(QSize(QWIDGETSIZE_MAX, 100));
    volumeAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    volumeAxisRect->setMargins(QMargins(0, 0, 0, 0));
    volumeAxisRect->axis(QCPAxis::atBottom)->setLayer("axes");
    volumeAxisRect->axis(QCPAxis::atBottom)->grid()->setLayer("grid");

    QCPAxis* vxAxis = volumeAxisRect->axis(QCPAxis::atBottom);
    QCPAxis* vyAxis = volumeAxisRect->axis(QCPAxis::atLeft);
    vxAxis->setTickLabels(true);
    vxAxis->setTicks(true);
    vxAxis->setSubTicks(false);
    vxAxis->setBasePen(Qt::NoPen);
    vxAxis->setTickPen(Qt::NoPen);
    vxAxis->setTickLabelColor(TEXT_QCOLOR_WHITE);
    vxAxis->grid()->setPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    vxAxis->grid()->setSubGridPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    vxAxis->grid()->setSubGridVisible(true);
    //Tickers
    // configure axes of both main and bottom axis rect:
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::UTC);
    dateTimeTicker->setDateTimeFormat("yyyy/MM/dd HH:mm:ss");
    volumeAxisRect->axis(QCPAxis::atBottom)->setTicker(dateTimeTicker);

    vyAxis->setTickLabels(true);
    vyAxis->setTicks(true);
    vyAxis->setSubTicks(false);
    vyAxis->setBasePen(Qt::NoPen);
    vyAxis->setTickPen(Qt::NoPen);
    vyAxis->setTickLabelColor(TEXT_QCOLOR_WHITE);
    vyAxis->grid()->setPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    vyAxis->grid()->setSubGridPen(QPen(GRIDLINE_QCOLOR, 1, Qt::SolidLine));
    vyAxis->grid()->setSubGridVisible(true);

    m_volumePos = new QCPBars(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    m_volumeNeg = new QCPBars(volumeAxisRect->axis(QCPAxis::atBottom), volumeAxisRect->axis(QCPAxis::atLeft));
    m_volumePos->setWidth(boxWidth);
    m_volumePos->setPen(Qt::NoPen);
    m_volumePos->setBrush(POSITIVE_QCOLOR);
    m_volumeNeg->setWidth(boxWidth);
    m_volumeNeg->setPen(Qt::NoPen);
    m_volumeNeg->setBrush(NEGATIVE_QCOLOR);

    // interconnect x axis ranges of main and bottom axis rects:
    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), volumeAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
    connect(volumeAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), xAxis, SLOT(setRange(QCPRange)));

    plotLayout()->insertRow(plotLayout()->rowCount());
    plotLayout()->addElement(plotLayout()->rowCount()-1, 0, volumeAxisRect);
    plotLayout()->setRowSpacing(0);

    // make axis rects' left side line up:
    QCPMarginGroup *group = new QCPMarginGroup(this);
    axisRect()->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    volumeAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);

    //调整大小
    rescaleAxes(true);
    xAxis->scaleRange(1.025, xAxis->range().center());
    yAxis->scaleRange(1.1, yAxis->range().center());
}

void CandlesticksView::createTradeInfoView(){
    if(m_infoView){return;}
    m_infoView = new TradeSummaryInfoView(this);
    m_infoView->setVisible(true);
    connect(m_infoView, SIGNAL(closed(bool)), this, SLOT(setInfoViewVisible(bool)));
    QPoint bottomLeft = frameGeometry().bottomLeft();
    m_infoView->move(QPoint(bottomLeft.x(), bottomLeft.y()-m_infoView->frameGeometry().height()));


    m_horizontalLine = new QCPItemLine(this);
    //addItem(m_horizontalLine);
    //m_horizontalLine->setVisible(true);
    m_verticalLine = new QCPItemLine(this);
    //addItem(m_verticalLine);
    //m_verticalLine->setVisible(true);
}

void CandlesticksView::updateTradeInfoView(const QPoint &pos, bool updateWhenInvisible){
    double posKey = 0, value = 0;
//    if(m_candlesticks->pointToCoords(pos, posKey, value)){
//        m_focusedKey = posKey;
//    }

    double key = 0, posValue = 0;
    QCPAxisRect *element = qobject_cast<QCPAxisRect*>(layoutElementAt(pos));
//    if(element){
//        element->pointToCoords(pos, key, posValue);
//    }
    updateCrossCurvePoint(pos);

    updateTradeInfoView(m_focusedKey, posValue, updateWhenInvisible);
}

void CandlesticksView::updateTradeInfoView(double curKey, double curValue, bool updateWhenInvisible){
    if(!m_infoView){return;}
    if(!m_infoView->isVisible() && !updateWhenInvisible){return;}

//    QCPFinancialData ohlcData = m_ohlcData->value(m_focusedKey);
//    if(ohlcData.open == 0){return;}
    TradeExtraData extraData = m_tradeExtraDataMap->value(m_focusedKey);
//    m_infoView->updateTradeInfo(ohlcData, extraData, curValue, m_stockName);
    //    xAxis->setCrossCurvePoint(m_focusedKey, curValue);
    //    yAxis->setCrossCurvePoint(m_focusedKey, curValue);

    //    m_horizontalLine->start->setCoords(xAxis->range().lower, curValue);
    //    m_horizontalLine->end->setCoords(xAxis->range().upper, curValue);
    //    m_verticalLine->start->setCoords(curKey, m_yAxis->range().lower);
    //    m_verticalLine->end->setCoords(curKey, m_yAxis->range().upper);
    //    replot();
}

void CandlesticksView::updateCrossCurvePoint(const QPointF &pos){
    for(int i=0; i < axisRectCount(); i++){
        axisRect(i)->setCrossCurvePoint(pos);
    }
    replot(QCustomPlot::rpQueuedReplot);
}

void CandlesticksView::setInfoViewVisible(bool visible){

    if(m_infoView){
        m_infoView->setVisible(visible);
        //m_horizontalLine->setVisible(visible);
        //m_verticalLine->setVisible(visible);
    }else if(visible){
        createTradeInfoView();
    }
    //m_candlesticks->setCrossCurveVisible(visible);
    //xAxis->setCrossCurveVisible(visible);
    //yAxis2->setCrossCurveVisible(visible);

//    for(int i=0; i < axisRectCount(); i++){
//        axisRect(i)->setCrossCurveVisible(visible);
//    }
    replot();
}

void CandlesticksView::updateVolumeYAxisRange(){
    if(m_ohlcData->isEmpty()){return;}

    double maxYValue = std::numeric_limits<double>::min();
    TradeExtraDataMap::const_iterator leftkeyIT = m_tradeExtraDataMap->lowerBound(m_leftKey);
    TradeExtraDataMap::const_iterator rightkeyIT = m_tradeExtraDataMap->lowerBound(m_rightKey);
    TradeExtraDataMap::const_iterator it;
    for (it = leftkeyIT; it != rightkeyIT; ++it)
    {
        TradeExtraData extraData = it.value();
        double currentVol = extraData.volume;
        if (currentVol > maxYValue){
            maxYValue = currentVol;
        }
    }
    m_volumeLeftAxis->setRange(0, maxYValue);
}

void CandlesticksView::historicalDataRead(Stock *stock){
    //qDebug()<<"--CandlesticksView::historicalDataRead() "<<stock->name();
    Q_ASSERT(stock);
    if(!stock || stock->code() != m_stockCodeExpected){return;}
    m_curStock = stock;
    m_stockCode = stock->code();
    m_stockName = stock->name();
    //m_plotTitle->setText(m_stockName);
    emit stockChanged(m_stockCode);

    m_ohlcData = stock->ohlcDataContainer();
    m_candlesticks->setData(m_ohlcData); //TODO:optimize

    QVector< double > * futuresDeliveryDates = stock->futuresDeliveryDates();
    //xAxis->setTickVector(*futuresDeliveryDates);

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

//    for(QMap<double, TradeExtraData>::const_iterator it = m_tradeExtraDataMap->constBegin(); \
//        it!=m_tradeExtraDataMap->constEnd(); it++){
//        //const QCPFinancialData& ohlcData = m_ohlcData->at(it.)
//        TradeExtraData extraData = it.value();
//        (ohlcData.close < ohlcData.open ? m_volumeNeg : m_volumePos)->addData(it.key(), (extraData.volume));
//    }

    rescaleAxes();
    //xAxis->scaleRange(0.015, xAxis->range().upper);
    if(!m_tradeExtraDataMap->isEmpty()){
        m_focusedKey = m_tradeExtraDataMap->lastKey();
    }

    replot();
}

void CandlesticksView::showCandlesticks(const QString &code){
    if(code != m_stockCode){
        m_stockCode = code;
        //m_dataManager->readHistoricalData(&m_stockCode, wheelSteps);
        m_stockCodeExpected = code;
        emit historicalDataRequested(&m_stockCodeExpected, 0);
    }
}
