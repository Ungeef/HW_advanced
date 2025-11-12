#include <stdio.h>
#include <math.h>
#include "search_roots_lib.h"
#include "types.h"

float rootFindLineSearch(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2) 
{
    float x, minx = xl, nextstep;
    nextstep = fabs(xr-xl)/(1/eps); //разбиваем на отрезки интервал
    int stepcount=0; 
    for(x=xl; x<xr; x += nextstep, stepcount++) 
    {
        if( fabs(nf1.func(x) - nf2.func(x)) < fabs(nf1.func(minx) - nf2.func(minx)) )
            minx = x;
    }
    if(print_statistic == 1 || print_statistic == 3)
        printf("root = %f between %s and %s\n", minx, nf1.name, nf2.name); //статистика
    total_iter += stepcount;
    return minx;
}

int signF(float x, NamedFunction nf1, NamedFunction nf2)
{
    return nf1.func(x) - nf2.func(x) == 0 ? 0 : (nf1.func(x) - nf2.func(x) < 0 ? -1 : 1);
}

float rootFindDiv(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2)
{
  int stepcount = 0; // число шагов
  float xm;
  while (fabs (xr - xl) > eps)// вещественный модуль разницы
    {
      stepcount++;
      xm = (xl + xr) / 2; // середина отрезка
      if (nf1.func(xr) - nf2.func(xr) == 0)
        { // нашли решение на правой границе
            printf("Find root for %d steps\n",stepcount);
            return xr;
        }
      if (nf1.func(xl) - nf2.func(xl) == 0) // нашли решение на левой границе
        { 
            printf("Find root for %d steps\n",stepcount);
            return xl;
        }
      if (signF(xl,nf1, nf2) != signF(xm, nf1, nf2)) // если знак отличается
        xr = xm;
      else
        xl = xm;
    }
    float answ = (xl + xr) / 2;
    if(print_statistic == 1 || print_statistic == 3)
        printf("root = %f between %s and %s\n", answ, nf1.name, nf2.name); //статистика
    total_iter += stepcount;
    return answ;
}

float rootFindChord(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2)
{
    int stepcount = 0;
    while (fabs(xr - xl) > eps)
        {
            xl = xr - (xr - xl) * (nf1.func(xr) - nf2.func(xr)) / ((nf1.func(xr) - nf2.func(xr)) - (nf1.func(xl) - nf2.func(xl)));
            xr = xl - (xl - xr) * (nf1.func(xl) - nf2.func(xl)) / ((nf1.func(xl) - nf2.func(xl)) - (nf1.func(xr) - nf2.func(xr)));
            stepcount++;
        }
    if(print_statistic == 1 || print_statistic == 3)
        printf("root = %f between %s and %s\n", xr, nf1.name, nf2.name); //статистика
    total_iter += stepcount;
    return xr;
}

float rootFindTangent(float xn, float eps, NamedFunction nf1, NamedFunction nf2, function2arg df1 )
{

        float x1  = xn - (nf1.func(xn)-nf2.func(xn)) / (df1(nf1, xn) - df1(nf2, xn));
        float x0  = xn;
        int stepcount=0;
        while(fabs(x0 - x1) > eps)
        {
            x0 = x1;
            x1 = x1 - (nf1.func(x1) - nf2.func(x1))/(df1(nf1, x1) - df1(nf2, x1));
            stepcount++;
        }
        if(print_statistic == 1 || print_statistic == 3)
            printf("root = %f between %s and %s\n", x1, nf1.name, nf2.name); //статистика
        total_iter += stepcount;
        return x1;
}

float rootFindCombine(float xl, float xr, float eps, NamedFunction nf1, NamedFunction nf2, function2arg df1, function2arg ddf ) 
{
    int stepcount = 0;
    
        while(fabs(xl-xr) > 2*eps) 
        {
            if((nf1.func(xl) - nf2.func(xl)) * (ddf(nf1, xl) - ddf(nf2, xl)) < 0)
                xl = xl - ((nf1.func(xl) - nf2.func(xl)) * (xl - xr)) / ((nf1.func(xl) - nf2.func(xl)) - (nf1.func(xr) - nf2.func(xr)));
            else
                xl = xl - (nf1.func(xl) - nf2.func(xl)) / (df1(nf1, xl) - df1(nf2, xl));
            if((nf1.func(xr) - nf2.func(xr)) * (ddf(nf1, xr) - ddf(nf2, xr)) < 0)
                xr = xr - ((nf1.func(xr) - nf2.func(xr))*(xr - xl))/((nf1.func(xr) - nf2.func(xr)) - (nf1.func(xl) - nf2.func(xl)));
            else
                xr = xr - (nf1.func(xr) - nf2.func(xr))/(df1(nf1, xr) - df1(nf2, xr));
            stepcount++;
        }
        if(print_statistic == 1 || print_statistic == 3)
            printf("root = %f between %s and %s\n", (xl+xr)/2, nf1.name, nf2.name); //статистика
        total_iter += stepcount;
        return (xl+xr)/2;
 }

// обертка согласно заданию
float root(NamedFunction f, NamedFunction g, float a, float b, float eps1) 
{
    // выбор метода по умолчанию
    return rootFindLineSearch(a, b, eps1, f, g);
}
