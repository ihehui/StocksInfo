#ifndef TRADEINFO_H
#define TRADEINFO_H

#include <QObject>
#include <math.h>

class TradeExtraData
{
public:
  TradeExtraData();
  TradeExtraData(uint time, double preClose, double volume_Hand, double turnover, double turnoverRate);

  uint time; //time_t
  double preClose;
  double volume_Hand;
  double turnover;
  double turnoverRate;

};
//typedef QMap<double, TradeExtraData> TradeExtraDataMap;




#endif // TRADEINFO_H
