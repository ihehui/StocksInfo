#ifndef QCPCANDLECHART_H
#define QCPCANDLECHART_H

#include "qcustomplot.h"
#include <QObject>

class QCPCandleChart : public QCPFinancial
{
    Q_OBJECT
public:
    explicit QCPCandleChart(QCPAxis *keyAxis, QCPAxis *valueAxis);

    bool zoom(const double& factor, const double& center);

    void adjustValueRange(double marginRatio = 0.1);  ///<指示应该在四边留出多少空白边距
    ///factor表示新宽度和老宽度的比值
    void adjustKeyRangeOnResize(const double& factor);

    ///重置整个布局,以合适的宽度/位置来显示整个图形
    void initAdjustAll();

    bool getFocusKeyAndValue(const QPointF &pos, double &key, double &value);

    bool setKeyAxisAutoFitGrid();

private:
    bool canZoom(const double& requestFactor, double& preferFactor, \
                 const double& requestCenter, double& preferCenter);

    //drag
public:
    bool move(const double& startKey, const int& step, double& preferKey, double& preferValue);
    bool canDrag(const double& preferPixelWidth);
private:
    bool canDragInner(double& preferCoordsWidth);

private slots:
    void onKeyAxisRangeChanged(const QCPRange& range);
protected:
    virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
    virtual void drawIndicatorText(QCPPainter *painter);
private:
    void getRatioBoundValuesInVisibleRange\
    (double &minValue, double &maxValue,
     double marginRatioH = 0.0, double marginRatioV = 0.1) const;
    void getRatioVisibleDataBounds(QCPFinancialDataContainer::const_iterator &begin, \
                                   QCPFinancialDataContainer::const_iterator &end, \
                                   double ratio = 0.9) const;

    double pixelWidthToCoordsWidth(double width);
    double coordsWidthToPixelWidth(double width);
    bool coordsToPoint(const double &key, const double &value, QPointF &pos);

    //    void getBoundValuesInVisibleRange(double &minValue, double &maxValue, uint &itemCount, double *leftKey = 0, double *rightkey = 0) const;
    //    void getBoundValuesInVisibleRange(QCPRange &valueRange, QCPRange &leftBoxRange, QCPRange &rightBoxRange) const;
};

#endif // QCPCANDLECHART_H
