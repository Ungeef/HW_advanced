#ifndef TYPES_H
#define TYPES_H


typedef float(*function)(float);
typedef struct // структура для присоединения имени к функции для красивого вывода статистики
{
    function func;
    const char* name;
} NamedFunction;
typedef float(*function2arg)(NamedFunction, float);

#endif