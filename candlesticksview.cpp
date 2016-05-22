#include "candlesticksview.h"
#include "stock.h"


CandlesticksView::CandlesticksView(QWidget *parent)
    : QCustomPlot(parent)
{
    qDebug()<<"CandlesticksView:"<<QThread::currentThreadId();

    m_dataManager = 0;
    m_curStock = 0;
    m_stockCode = "";
    m_stockName = "";
    m_stockCodeExpected = "";

    m_ohlcDataMap = 0;
    m_tradeExtraDataMap = 0;

    m_plotTitle = 0;
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
    if(!manager){return;}
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

    if(!m_infoView){
        setInfoViewVisible(true);
        //QPoint bottomLeft = frameGeometry().bottomLeft();
        //m_infoView->move(QPoint(bottomLeft.x(), bottomLeft.y()-m_infoView->frameGeometry().height()));
        updateTradeInfoView(event->pos(), true);

        //生成鼠标移动事件
        //QMouseEvent e(QEvent::MouseMove, event->localPos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        //slotMouseMove(&e);
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

    Q_UNUSED(event)
    //qDebug()<<QString("--mousePressEvent--");
}

void CandlesticksView::mouseMoveEvent(QMouseEvent *event){
    if(!m_curStock){return;}

    updateTradeInfoView(event->pos());
    //qDebug()<<QString("--mouseMoveEvent--");
}

void CandlesticksView::mouseReleaseEvent(QMouseEvent *event){
    if(!m_curStock){return;}

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

    switch (event->key()) {
    case Qt::Key_Escape:
    {
        //隐藏交易信息小窗口
        setInfoViewVisible(false);
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

        qDebug()<<QDateTime::fromTime_t( m_tradeExtraDataMap->value(m_leftKey).time).toString("L: MM-dd");
        qDebug()<<QDateTime::fromTime_t( m_tradeExtraDataMap->value(m_focusedKey).time).toString("C: MM-dd");
        qDebug()<<QDateTime::fromTime_t( m_tradeExtraDataMap->value(m_rightKey).time).toString("R: MM-dd");

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
        QCPFinancialData ohlcData = m_ohlcDataMap->value(m_focusedKey);
        if(isZero(ohlcData.open)){return;}
        updateTradeInfoView(m_focusedKey, ohlcData.close);

        QPointF pos;
        m_candlesticks->coordsToPoint(m_focusedKey, ohlcData.close, pos);
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
    qDebug()<<"----setAxisRange()------";

    QCPRange valueRange(0,0), leftBoxRange(0,0), rightBoxRange(0,0);
    m_candlesticks->getBoundValuesInVisibleRange(valueRange, leftBoxRange, rightBoxRange);
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
    qDebug()<<"----setAxisRange()------";

    double minValue = 0, maxValue = 0;
    uint itemCount = 0;
    m_candlesticks->getBoundValuesInVisibleRange(minValue, maxValue, itemCount, &m_leftKey, &m_rightKey);
    if(m_yAxis->range().lower != minValue || m_yAxis->range().upper != maxValue){
        m_yAxis->setRange(minValue, maxValue);
    }

    double xAxisLower = xAxis->range().lower;
    double xAxisUpper = xAxis->range().upper;

    double leftBoxRangeLlower = m_leftKey-m_candlesticks->width()*0.5;
    double leftBoxRangeUpper = m_leftKey+m_candlesticks->width()*0.5;
    double rightBoxRangeLlower = m_rightKey-m_candlesticks->width()*0.5;
    double rightBoxRangeUpper = m_rightKey+m_candlesticks->width()*0.5;
    if( ((m_leftKey == m_tradeExtraDataMap->firstKey()) && (xAxisLower < leftBoxRangeLlower))){
        xAxis->setRangeLower(leftBoxRangeLlower);
    }
    if(m_rightKey == m_tradeExtraDataMap->lastKey() && xAxisUpper > rightBoxRangeUpper){
        xAxis->setRangeUpper(rightBoxRangeUpper);
    }

    if( (xAxisLower > leftBoxRangeLlower && xAxisLower <= leftBoxRangeUpper)){
        //TODO:显示不完整
    }

    if( (xAxisUpper >= rightBoxRangeLlower && xAxisUpper < rightBoxRangeUpper) ){
        //TODO:显示不完整
    }


    updateVolumeYAxisRange();

}


void CandlesticksView::drawCandlesticks(){
    QFont titleFont = font();
    //titleFont.setPointSize(10);
    m_plotTitle = new QCPPlotTitle(this, "Stock");
    m_plotTitle->setFont(titleFont);
    plotLayout()->insertRow(0);
    plotLayout()->addElement(0, 0, m_plotTitle);

    m_candlesticks = new QCPFinancial(xAxis, yAxis);
    addPlottable(m_candlesticks);
    m_candlesticks->setName("Candlestick");
    m_candlesticks->setChartStyle(QCPFinancial::csCandlestick);


    //    readData("601398");

    double boxWith = 0.85;
    m_candlesticks->setWidth(boxWith);
    m_candlesticks->setTwoColored(true);
    m_candlesticks->setBrushPositive(QColor(255, 0, 0));
    m_candlesticks->setBrushNegative(QColor(0, 255, 0));
    m_candlesticks->setPenPositive(QPen(QColor(255, 0, 0)));
    m_candlesticks->setPenNegative(QPen(QColor(0, 255, 0)));
    m_candlesticks->setSelectable(false);

    xAxis->setBasePen(Qt::NoPen);
    xAxis->setTickLabels(false);
    xAxis->setTicks(true);
    xAxis->setAutoTicks(false);
    xAxis->setAutoTickStep(false);
    xAxis->setAutoSubTicks(false);
    xAxis->setTickStep(1);
    //xAxis->axisRect()->axis(QCPAxis::atBottom)->setTickLabelType(QCPAxis::ltDateTime);
    //xAxis->axisRect()->axis(QCPAxis::atBottom)->setDateTimeFormat("MM-dd");
    //xAxis->setScaleType(QCPAxis::stLinear);

    m_yAxis = yAxis;
    yAxis->setTickLabels(false);
    yAxis->setTicks(true);
    yAxis->setAutoTicks(true);
    yAxis->setAutoTickStep(true);
    yAxis2->setVisible(true);
    yAxis2->grid()->setVisible(true);
    yAxis->setVisible(false);


    QCPPlotTitle *volPlotTitle = new QCPPlotTitle(this, "Volume");
    volPlotTitle->setFont(titleFont);
    plotLayout()->insertRow(plotLayout()->rowCount());
    plotLayout()->addElement(plotLayout()->rowCount()-1, 0, volPlotTitle);

    volumeAxisRect = new QCPAxisRect(this);
    plotLayout()->insertRow(plotLayout()->rowCount());
    plotLayout()->addElement(plotLayout()->rowCount()-1, 0, volumeAxisRect);
    volumeAxisRect->setMaximumSize(QSize(QWIDGETSIZE_MAX, 100));
    QCPAxis *volumeBottomAxis = volumeAxisRect->axis(QCPAxis::atBottom);
    volumeBottomAxis->setLayer("axes");
    volumeBottomAxis->grid()->setLayer("grid");
    plotLayout()->setRowSpacing(0);
    volumeAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    volumeAxisRect->setMargins(QMargins(0, 0, 0, 0));

    m_volumePos = new QCPBars(volumeBottomAxis, volumeAxisRect->axis(QCPAxis::atLeft));
    m_volumeNeg = new QCPBars(volumeBottomAxis, volumeAxisRect->axis(QCPAxis::atLeft));
    addPlottable(m_volumePos);
    addPlottable(m_volumeNeg);
    m_volumePos->setWidth(boxWith);
    m_volumePos->setPen(Qt::NoPen);
    m_volumePos->setBrush(QColor(255, 0, 0));
    m_volumeNeg->setWidth(boxWith);
    m_volumeNeg->setPen(Qt::NoPen);
    m_volumeNeg->setBrush(QColor(0, 255, 0));


    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), volumeBottomAxis, SLOT(setRange(QCPRange)));
    //connect(volumeBottomAxis, SIGNAL(rangeChanged(QCPRange)), xAxis, SLOT(setRange(QCPRange)));


    volumeBottomAxis->setTickLabels(false);
    volumeBottomAxis->setTicks(false);
    volumeBottomAxis->setAutoTicks(false);
    volumeBottomAxis->setAutoTickStep(false);
    volumeBottomAxis->setAutoSubTicks(false);
    volumeBottomAxis->setTickStep(1);
    //volumeBottomAxis->setTickLabelType(QCPAxis::ltDateTime);
    //volumeBottomAxis->setDateTimeSpec(Qt::UTC);
    //volumeBottomAxis->setDateTimeFormat("MM-dd");
    //volumeBottomAxis->setTickLabelRotation(15);

    m_volumeLeftAxis = volumeAxisRect->axis(QCPAxis::atLeft);
    QCPAxis *volumeRightAxis = volumeAxisRect->axis(QCPAxis::atRight);
    connect(m_volumeLeftAxis, SIGNAL(rangeChanged(QCPRange)), volumeRightAxis, SLOT(setRange(QCPRange)));

    m_volumeLeftAxis->setVisible(false);
    volumeRightAxis->setVisible(true);
    volumeRightAxis->setAutoTickCount(4);

    QCPMarginGroup *group = new QCPMarginGroup(this);
    axisRect()->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    volumeAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);


//    readData("601398");
//    rescaleAxes();
//    xAxis->scaleRange(0.015, xAxis->range().upper);
//    m_focusedKey = m_tradeExtraData->lastKey();

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
    if(m_candlesticks->pointToCoords(pos, posKey, value)){
        m_focusedKey = posKey;
    }

    double key = 0, posValue = 0;
    QCPAxisRect *element = qobject_cast<QCPAxisRect*>(layoutElementAt(pos));
    if(element){
        element->pointToCoords(pos, key, posValue);
    }
    updateCrossCurvePoint(pos);

    updateTradeInfoView(m_focusedKey, posValue, updateWhenInvisible);
}

void CandlesticksView::updateTradeInfoView(double curKey, double curValue, bool updateWhenInvisible){
    if(!m_infoView){return;}
    if(!m_infoView->isVisible() && !updateWhenInvisible){return;}

    QCPFinancialData ohlcData = m_ohlcDataMap->value(m_focusedKey);
    if(ohlcData.open == 0){return;}
    TradeExtraData extraData = m_tradeExtraDataMap->value(m_focusedKey);
    m_infoView->updateTradeInfo(ohlcData, extraData, curValue, m_stockName);
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
    //axisRect(0)->setCrossCurvePoint(pos);
    //yAxis->setCrossCurvePoint(pos);
    replot();
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

    for(int i=0; i < axisRectCount(); i++){
        axisRect(i)->setCrossCurveVisible(visible);
    }
    replot();
}

void CandlesticksView::updateVolumeYAxisRange(){
    if(m_ohlcDataMap->isEmpty()){return;}

    double maxYValue = std::numeric_limits<double>::min();
    TradeExtraDataMap::const_iterator leftkeyIT = m_tradeExtraDataMap->lowerBound(m_leftKey);
    TradeExtraDataMap::const_iterator rightkeyIT = m_tradeExtraDataMap->lowerBound(m_rightKey);
    TradeExtraDataMap::const_iterator it;
    for (it = leftkeyIT; it != rightkeyIT; ++it)
    {
        TradeExtraData extraData = it.value();
        double currentVol = extraData.volume_Hand;
        if (currentVol > maxYValue){
            maxYValue = currentVol;
        }
    }
    m_volumeLeftAxis->setRange(0, maxYValue);
}

void CandlesticksView::historicalDataRead(Stock *stock){
    Q_ASSERT(stock);
    if(!stock || stock->code() != m_stockCodeExpected){return;}
    m_curStock = stock;
    m_stockCode = stock->code();
    m_stockName = stock->name();

    m_ohlcDataMap = stock->ohlcDataMap();
    m_candlesticks->setData(m_ohlcDataMap, false);

    QVector< double > * futuresDeliveryDates = stock->futuresDeliveryDates();
    xAxis->setTickVector(*futuresDeliveryDates);

    m_tradeExtraDataMap = stock->tradeExtraDataMap();

    for(QMap<double, TradeExtraData>::const_iterator it = m_tradeExtraDataMap->constBegin(); it!=m_tradeExtraDataMap->constEnd(); it++){
        QCPFinancialData ohlcData = m_ohlcDataMap->value(it.key());
        TradeExtraData extraData = it.value();
        (ohlcData.close < ohlcData.open ? m_volumeNeg : m_volumePos)->addData(it.key(), (extraData.volume_Hand));
    }

    rescaleAxes();
    xAxis->scaleRange(0.015, xAxis->range().upper);
    m_focusedKey = m_tradeExtraDataMap->lastKey();

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



