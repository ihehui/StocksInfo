#ifndef COMMON_H
#define COMMON_H

const long OneHundredMillion = 100000000;

bool isZero(double d);
bool isEqual(double d1, double d2);


enum PeriodType{
    PERIOD_REALTIME = 0,
    PERIOD_ONE_MIN,
    PERIOD_FIVE_MINS,
    PERIOD_TEN_MINS,
    PERIOD_FIFTEEN_MINS,
    PERIOD_THIRTY_MINS,
    PERIOD_ONE_HOUR,
    PERIOD_ONE_DAY,
    PERIOD_ONE_WEEK,
    PERIOD_ONE_MONTH,
    PERIOD_THREE_MONTHS,
    PERIOD_ONE_YEAR,


    PERIOD_CUSTOM_YEARS,
    PERIOD_CUSTOM_MONTHS,
    PERIOD_CUSTOM_DAYS,
    PERIOD_CUSTOM_MINS,

};












#endif // COMMON_H
