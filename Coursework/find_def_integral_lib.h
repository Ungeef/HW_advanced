#ifndef FIND_DEF_INTEGRAL_LIB_H
#define FIND_DEF_INTEGRAL_LIB_H

#include "types.h"
#include <stdio.h>


float calcIntegralSquare(float xl, float xr, size_t n, function f);
float calcIntegralTrap(float xl, float xr, size_t n, function f); 
float calcIntegralMonteCarlo(float xl, float xr, float fmax, size_t n, function f); 
float calcIntegralSimpson(float xl, float xr, size_t n, function f);
float integral(float (*f)(float), float a, float b, float eps2);

#endif