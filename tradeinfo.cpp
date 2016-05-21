#include "tradeinfo.h"

TradeExtraData::TradeExtraData() :
    time(0),
    preClose(0),
    volume_Hand(0),
    turnover(0),
    turnoverRate(0)
{
}

TradeExtraData::TradeExtraData(uint key, double preClose, double volume_Hand, double turnover, double turnoverRate) :
    time(key),
    preClose(preClose),
    volume_Hand(volume_Hand),
    turnover(turnover),
    turnoverRate(turnoverRate)
{
}

