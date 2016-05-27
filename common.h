#ifndef COMMON_H
#define COMMON_H

#ifdef Q_OS_WIN
#define OS_IS_WINDOWS 1
#else
#define OS_IS_WINDOWS 0
#endif

#include <QString>




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

namespace HEHUI {
enum DatabaseType {
        OTHER = 0,
        MYSQL = 1,
        SQLITE = 2,
        POSTGRESQL = 3,
        FIREBIRD = 4,
        DB2 = 5,
        ORACLE = 6,
        M$SQLSERVER = 7,
        M$ACCESS =8,

};
}

#endif // COMMON_H
