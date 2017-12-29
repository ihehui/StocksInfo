#include "qcpcandlechart.h"
#include <QtMath>

const int CANDLE_MAX_PIXEL_WIDTH = 40;
const int CANDLE_MIN_PIXEL_WIDTH = 1;
const int CANDLE_PREFER_PIXEL_WIDTH = 10;

QCPCandleChart::QCPCandleChart(QCPAxis *keyAxis, QCPAxis *valueAxis)\
    : QCPFinancial(keyAxis, valueAxis)
{
}

void QCPCandleChart::draw(QCPPainter *painter)
{
    QCPFinancial::draw(painter);
    drawIndicatorText(painter);
}

void QCPCandleChart::drawIndicatorText(QCPPainter *painter)
{
//    QCPAxis *keyAxis = mKeyAxis.data();
//    QCPAxis *valueAxis = mValueAxis.data();
//    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }

//    ////start---------------EXTENDED BY HEHUI, MODIFIED BY Bringer-of-Light---------------////
//    double minYValue = std::numeric_limits<double>::max();
//    double maxYValue = std::numeric_limits<double>::min();
//    double keyOfMinYValue = keyAxis->range().lower;
//    double keyOfmaxYValue = keyAxis->range().upper;
//    ////end---------------EXTENDED BY HEHUI, MODIFIED BY Bringer-of-Light---------------////

//    if (keyAxis->orientation() == Qt::Horizontal)
//    {
//        for (QCPFinancialDataContainer::const_iterator it = begin; it != end; ++it)
//        {
//            if (isSelected && mSelectionDecorator)
//            {
//                mSelectionDecorator->applyPen(painter);
//                mSelectionDecorator->applyBrush(painter);
//            } else if (mTwoColored)
//            {
//                painter->setPen(it->close >= it->open ? mPenPositive : mPenNegative);
//                painter->setBrush(it->close >= it->open ? mBrushPositive : mBrushNegative);
//            } else
//            {
//                painter->setPen(mPen);
//                painter->setBrush(mBrush);
//            }
//            double keyPixel = keyAxis->coordToPixel(it->key);
//            double openPixel = valueAxis->coordToPixel(it->open);
//            double closePixel = valueAxis->coordToPixel(it->close);
//            // draw high:
//            painter->drawLine(QPointF(keyPixel, valueAxis->coordToPixel(it->high)), QPointF(keyPixel, valueAxis->coordToPixel(qMax(it->open, it->close))));
//            // draw low:
//            painter->drawLine(QPointF(keyPixel, valueAxis->coordToPixel(it->low)), QPointF(keyPixel, valueAxis->coordToPixel(qMin(it->open, it->close))));
//            // draw open-close box:
//            double pixelWidth = getPixelWidth(it->key, keyPixel);
//            painter->drawRect(QRectF(QPointF(keyPixel-pixelWidth, closePixel), QPointF(keyPixel+pixelWidth, openPixel)));

//            ////start---------------EXTENDED BY HEHUI, MODIFIED BY Bringer-of-Light---------------////
//            //TODO:if(it->open == 0){continue;}
//            double currentLow = it->low;
//            double currentHigh = it->high;
//            if (currentHigh >= maxYValue){
//                maxYValue = currentHigh;
//                keyOfmaxYValue = it->key;
//            }
//            if(currentLow <= minYValue){
//                minYValue = currentLow;
//                keyOfMinYValue = it->key;
//            }
//            ////end---------------EXTENDED BY HEHUI, MODIFIED BY Bringer-of-Light---------------////
//        }

//        ////start---------------EXTENDED BY HEHUI, MODIFIED BY Bringer-of-Light---------------////
//        {
//            //Draw MIN&MAX Value Tips
//            QString minValueString = QString("%1%2").arg("\342\206\220").arg(minYValue);
//            QString maxValueString = QString("%1%2").arg("\342\206\220").arg(maxYValue);

//            //QFont font = QGuiApplication::font();
//            QFontMetrics fm = QApplication::fontMetrics();
//            int pixelsMinValueStringWide = fm.width(minValueString);
//            int pixelsMaxValueStringWide = fm.width(maxValueString);
//            int pixelsHigh = fm.height();

//            QPointF minValuePoint = QPointF(keyAxis->coordToPixel(keyOfMinYValue), valueAxis->coordToPixel(minYValue)+pixelsHigh/2);
//            QPointF maxValuePoint = QPointF(keyAxis->coordToPixel(keyOfmaxYValue), valueAxis->coordToPixel(maxYValue)+pixelsHigh/2);
//            double rightKeyPointX = keyAxis->coordToPixel(keyAxis->range().upper);

//            if(rightKeyPointX - maxValuePoint.x() < pixelsMaxValueStringWide){
//                maxValueString = QString("%1%2").arg(maxYValue).arg("\342\206\222");
//                maxValuePoint.setX(maxValuePoint.x() - pixelsMaxValueStringWide);
//            }
//            if(rightKeyPointX - minValuePoint.x() < pixelsMinValueStringWide){
//                minValueString = QString("%1%2").arg(minYValue).arg("\342\206\222");;
//                minValuePoint.setX(minValuePoint.x() - pixelsMinValueStringWide);
//            }

//            painter->setPen(mPenPositive);
//            painter->drawText(maxValuePoint, maxValueString);

//            painter->setPen(mPenNegative);
//            painter->drawText(minValuePoint, minValueString);

//        }
//    }
}

bool QCPCandleChart::canZoom(const double& requestFactor, double& preferFactor)
{
    preferFactor = 1.0;
    if (!mKeyAxis) return false;
    if (mDataContainer->isEmpty()) return false;

    if (requestFactor == 1) return true;
    double pixels = coordsWidthToPixelWidth(mWidth);
    if (requestFactor < 1) //放大 zoom in
    {
        if (pixels >= CANDLE_MAX_PIXEL_WIDTH) return false;
        preferFactor =qMax((pixels/CANDLE_MAX_PIXEL_WIDTH),requestFactor);
    }
    else //缩小 zoom out
    {
        double axisRange = mKeyAxis->range().size();

        QCPFinancialDataContainer::const_iterator begin, end;
        begin = mDataContainer->constBegin();
        end = mDataContainer->constEnd();
        --end;
        if (begin>=end) return false;
        double dataRange = end->key - begin->key;
        if (pixels <= CANDLE_MIN_PIXEL_WIDTH || (dataRange <= axisRange)) \
            return false;

        preferFactor =qMin((dataRange/axisRange),requestFactor);
    }
    return true;
}

bool QCPCandleChart::zoom(const double& factor, const double& center)
{
    if (!mKeyAxis) return false;
    double preferFactor = 1.0;
    if (!canZoom(factor, preferFactor)) return false;
    mKeyAxis->scaleRange(preferFactor, center);
    return true;
}
void QCPCandleChart::adjustValueRange(double marginRatio)
{
    double minValue = 0, maxValue = 0;
    getRatioBoundValuesInVisibleRange(minValue, maxValue, marginRatio);

    if(mValueAxis->range().lower != minValue || \
            mValueAxis->range().upper != maxValue){
        mValueAxis->setRange(minValue, maxValue);
    }
}

void QCPCandleChart::getRatioBoundValuesInVisibleRange(double &minValue, double &maxValue, double marginRatio) const
{
    double minYValue = std::numeric_limits<double>::max();
    double maxYValue = std::numeric_limits<double>::min();

    // get ratio visible data range:
    QCPFinancialDataContainer::const_iterator lower, upper; // note that upper is the actual upper point, and not 1 step after the upper point
    getRatioVisibleDataBounds(lower, upper, 1-marginRatio);
    if (lower == mDataContainer->constEnd()) return ;

    QCPFinancialDataContainer::const_iterator it;
    for (it = lower; it != upper; ++it)
    {
        if(it->open == 0){continue;}

        double currentLow = it->low;
        double currentHigh = it->high;

        if (currentHigh > maxYValue){
            maxYValue = currentHigh;
        }
        if(currentLow < minYValue){
            minYValue = currentLow;
        }
    }


    double curRange = mValueAxis->range().upper - \
            mValueAxis->range().lower;
    double margins = marginRatio * curRange;
    minValue = minYValue - margins;
    maxValue = maxYValue + margins;
}

void QCPCandleChart::getRatioVisibleDataBounds(QCPFinancialDataContainer::const_iterator &begin, \
                                               QCPFinancialDataContainer::const_iterator &end,  \
                                               double ratio) const
{
    if (!mKeyAxis)
    {
        qDebug() << Q_FUNC_INFO << "invalid key axis";
        begin = mDataContainer->constEnd();
        end = mDataContainer->constEnd();
        return;
    }
    if (ratio>1) ratio = 1;
    if (ratio < 0) ratio = 0;

    QCPRange visibleRange = mKeyAxis.data()->range();
    // add half width of ohlc/candlestick to include partially visible data points
    // subtract half width of ohlc/candlestick to include partially visible data points
    double halfLength = 0.5*((visibleRange.upper+mWidth*0.5) \
                             - (visibleRange.lower-mWidth*0.5));
    double ratioLower = visibleRange.lower + halfLength * (1-ratio);
    double ratioUpper = visibleRange.upper - halfLength * (1-ratio);
    begin = mDataContainer->findBegin(ratioLower);
    end = mDataContainer->findEnd(ratioUpper);
}


//void QCPCandleChart::getBoundValuesInVisibleRange(double &minValue, double &maxValue, uint &itemCount, double *leftKey, double *rightkey) const
//{
//    double minYValue = std::numeric_limits<double>::max();
//    double maxYValue = std::numeric_limits<double>::min();

//    // get visible data range:
//    QCPFinancialDataContainer::const_iterator lower, upper; // note that upper is the actual upper point, and not 1 step after the upper point
//    getVisibleDataBounds(lower, upper);
//    if (lower == mDataContainer->constEnd()) return ;
//    itemCount = 0;

//    QCPFinancialDataContainer::const_iterator it;
//    for (it = lower; it != upper; ++it)
//    {
//        ++itemCount;
//        if(it->open == 0){continue;}

//        double currentLow = it->low;
//        double currentHigh = it->high;

//        if (currentHigh > maxYValue){
//            maxYValue = currentHigh;
//        }
//        if(currentLow < minYValue){
//            minYValue = currentLow;
//        }
//    }
//    minValue = minYValue;
//    maxValue = maxYValue;

//    if(leftKey){
//        *leftKey = lower->key;
//    }

//    if(rightkey){
//        *rightkey = upper->key;
//    }
//}

//void QCPCandleChart::getBoundValuesInVisibleRange(QCPRange &valueRange, QCPRange &leftBoxRange, QCPRange &rightBoxRange) const
//{
//    double minYValue = std::numeric_limits<double>::max();
//    double maxYValue = std::numeric_limits<double>::min();

//    // get visible data range:
//    QCPFinancialDataContainer::const_iterator lower, upper; // note that upper is the actual upper point, and not 1 step after the upper point
//    getVisibleDataBounds(lower, upper);
//    if (lower == mDataContainer->constEnd() \
//            || upper == mDataContainer->constEnd()) return ;

//    QCPFinancialDataContainer::const_iterator it;
//    for (it = lower; it != upper; ++it)
//    {
//        if(it->open == 0){continue;}

//        double currentLow = it->low;
//        double currentHigh = it->high;

//        if (currentHigh >= maxYValue){
//            maxYValue = currentHigh;
//        }
//        if(currentLow <= minYValue){
//            minYValue = currentLow;
//        }
//    }


//    valueRange.lower = minYValue;
//    valueRange.upper = maxYValue;

//    leftBoxRange.lower = lower->key-mWidth*0.5;
//    leftBoxRange.upper = lower->key+mWidth*0.5;

//    rightBoxRange.lower = upper->key-mWidth*0.5;
//    rightBoxRange.upper = upper->key+mWidth*0.5;


//}

bool QCPCandleChart::findFocusKey(const QPointF &pos, double &key, double &value) {

    QCPAxis *keyAxis = mKeyAxis.data();
    QCPAxis *valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) {
        qDebug() << Q_FUNC_INFO << "invalid key or value axis";
        return false;
    }

    QCPFinancialDataContainer::const_iterator begin, end; // note that upper is the actual upper point, and not 1 step after the upper point
    getVisibleDataBounds(begin, end);
    if (begin == mDataContainer->constEnd())
    {
        //qDebug()<<Q_FUNC_INFO<<"Empty Data!";
        return false;
    }

    //end++;
    QCPFinancialDataContainer::const_iterator it;
    if (keyAxis->orientation() == Qt::Horizontal)
    {
        for (it = begin; it != end; it++)
        {
            // determine whether pos is in open-close-box:
            QCPRange boxKeyRange(it->key-mWidth*0.5, it->key+mWidth*0.5);
            //QCPRange boxValueRange(it->close, it->open);
            double posKey, posValue;
            pixelsToCoords(pos, posKey, value);
            if (boxKeyRange.contains(posKey)) // is in open-close-box
            {
                key = it->key;
                return true;
            }
        }
    } else // keyAxis->orientation() == Qt::Vertical
    {
        for (it = begin; it != end; ++it)
        {
            // determine whether pos is in open-close-box:
            QCPRange boxKeyRange(it->key-mWidth*0.5, it->key+mWidth*0.5);
            //QCPRange boxValueRange(it->close, it->open);
            double posKey, posValue;
            pixelsToCoords(pos, posKey, value);
            if (boxKeyRange.contains(posKey)) // is in open-close-box
            {
                key = it->key;
                return true;
            }
        }
    }
    return false;
}

bool QCPCandleChart::coordsToPoint(const double &key, const double &value, QPointF &pos){
    if (mKeyAxis.isNull() || mValueAxis.isNull()){
        qDebug() << Q_FUNC_INFO << "invalid key or value axis";
        return false;
    }

    double x = mKeyAxis->coordToPixel(key);
    double y = mValueAxis->coordToPixel(value);
    pos = QPointF(x, y);
    return true;
}

double QCPCandleChart::coordsWidthToPixelWidth(double width)
{
    if (mKeyAxis.isNull()){
        qDebug() << Q_FUNC_INFO << "invalid key axis";
        return 0;
    }
    double begin = mKeyAxis->coordToPixel(0);
    double end = mKeyAxis->coordToPixel(width);
    return (end - begin);
}
