#include "common.h"
#include <math.h>

bool isZero(double d){
    return fabs(d) < 1e-15;
}

bool isEqual(double d1, double d2){
    return fabs(d1 - d2) < 1e-15;
}
