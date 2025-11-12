#ifndef SEARCH_ROOTS_LIB_H
#define SEARCH_ROOTS_LIB_H

#include "types.h"


float rootFindLineSearch(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2);
float rootFindDiv(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2);
int signF(float x, NamedFunction nf1, NamedFunction nf2);
float rootFindChord(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2);
float rootFindTangent(float xn, float eps, NamedFunction nf1, NamedFunction nf2, function2arg df1);
float rootFindCombine(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2, function2arg df1, function2arg ddf);
float root(NamedFunction f, NamedFunction g, float a, float b, float eps1); 

extern int print_statistic;
extern int total_iter;

#endif