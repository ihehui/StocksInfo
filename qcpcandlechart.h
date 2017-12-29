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
    bool canZoom(const double& requestFactor, double& preferFactor);

    void adjustValueRange(double marginRatio = 0.1);  ///<指示应该在四边留出多少空白边距

    bool findFocusKey(const QPointF &pos, double &key, double &value);
    bool coordsToPoint(const double &key, const double &value, QPointF &pos);
private:
    double coordsWidthToPixelWidth(double width);
protected:
    virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
    virtual void drawIndicatorText(QCPPainter *painter);
private:
    void getRatioBoundValuesInVisibleRange(double &minValue, double &maxValue, double marginRatio = 0.1) const;
    void getRatioVisibleDataBounds(QCPFinancialDataContainer::const_iterator &begin, \
                                                   QCPFinancialDataContainer::const_iterator &end, \
                                                   double ratio = 0.9) const;

//    void getBoundValuesInVisibleRange(double &minValue, double &maxValue, uint &itemCount, double *leftKey = 0, double *rightkey = 0) const;
//    void getBoundValuesInVisibleRange(QCPRange &valueRange, QCPRange &leftBoxRange, QCPRange &rightBoxRange) const;
};

#endif // QCPCANDLECHART_H
