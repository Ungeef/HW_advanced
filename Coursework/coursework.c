#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "search_roots_lib.h"
#include "types.h"
#include "find_def_integral_lib.h"

int print_statistic = 0; // флаг для печати статистики: 0 - не печатем, 1 - абциссы, 2 - число итераций, 3 - абциссы + итерации
_Bool test = 0;          // флаг для тестирования поиска корня
_Bool test_int = 0;      // флаг для тестирования интеграла
int total_iter = 0;      // общее число итераций при поиске корней
int count_my_func = 0;   // число функций пользователя, не может быть > 2
float xl_int, xr_int;    // границы интегрирования
int n_int = 1000;        // количество разбиений
float fmax_int = 0;      // максимум функции для МонтеКарло
char *func_name_int;     // имя функции для интегрирования
char *method_int;        // название метода интегрирования
float val1, val2;        //границы для поиска корней при использовании ключа -t
float eps_client = 0.001; //eps которое вводит пользователь
char *str1, *str2;       //(названия функций) строки которые ввел пользователь f1, f2...
char *str3;              //название метода поиска корней
NamedFunction *func1;    // тут хранятся указатели на функции nf1, nf2...
NamedFunction *func2;
int Method = 0;          //Номер метода

int Method_int = 0;    // номер метода интегрирования

// представление польз функций
typedef struct {
    float coeff;      // коэффициент
    float x_pow;      // степень x
    float abs_pow;    // степень |x|
    
    // Для тригонометрических функций и ln
    float sin_pow;    // степень sin
    float sin_arg_a;  // коэффициент аргумента sin(a*x^b)
    float sin_arg_b;  // степень аргумента
    
    float cos_pow;    
    float cos_arg_a;  
    float cos_arg_b;  
    
    float ln_pow;     
    float ln_arg_a;   
    float ln_arg_b;   
} Term;

Term parsed_terms[2][10];

// Объявление пользовательских функций
float my_func0(float x);
float my_func1(float x);

float f1(float x)//Функции из задания
{
    return 0.6*x+3;
}

float f2(float x)
{
    return (x-2)*(x-2)*(x-2)-1;
}

float f3(float x)
{
    if (fabs(x) < 1e-10) // деление на 0
    {
        return INFINITY;
    }
    return 3/x;
}

float f0(float x)
{
    return 0;
}

NamedFunction nf1 = {f1, "f1"}; // обзываем функции для более красивого вывода
NamedFunction nf2 = {f2, "f2"};
NamedFunction nf3 = {f3, "f3"};
NamedFunction nf0 = {f0, "f0"};

// NamedFunction для пользовательских функций
NamedFunction nf_my = {my_func0, "my_func0"};
NamedFunction nf_my1 = {my_func1, "my_func1"};

NamedFunction *func_int;

float derivative(NamedFunction f, float x0)// вычисление производной
{// будет использоваться только если используется нестандартная функция заданная пользователем
    const float delta = 1.0e-6;
    float x1 = x0 - delta;
    float x2 = x0 + delta;
    float y1 = f.func(x1);
    float y2 = f.func(x2);
    return (y2 - y1) / (x2 - x1);
}

float derivative2(NamedFunction f, float x0) //вторая производная
{
    const float delta = 1.0e-6;
    float x_minus = x0 - delta;
    float x_center = x0;
    float x_plus = x0 + delta;
    
    float y_minus = f.func(x_minus);
    float y_center = f.func(x_center);
    float y_plus = f.func(x_plus);
    
    return (y_plus - 2*y_center + y_minus) / (delta * delta);
}

float df(NamedFunction func, float x)
{
    if (strcmp(func.name, "f1") == 0) return 0.6;
    if (strcmp(func.name, "f2") == 0) return 3*x*x - 12*x + 12;
    if (strcmp(func.name, "f3") == 0) return -3 / (x*x);
    if (strcmp(func.name, "my_func0") == 0) return derivative(func, x);
    if (strcmp(func.name, "my_func1") == 0) return derivative(func, x);
    return 0;
}

float ddf(NamedFunction func, float x) 
{
    if (strcmp(func.name, "f1") == 0) return 0;
    if (strcmp(func.name, "f2") == 0) return 6*x - 12;
    if (strcmp(func.name, "f3") == 0) return 6 / (x*x*x);
    if (strcmp(func.name, "my_func0") == 0) return derivative2(func, x);
    if (strcmp(func.name, "my_func1") == 0) return derivative2(func, x);
    return 0;
}

NamedFunction* getNamedFunction(const char* name) //В зависимости от имени функции который ввел пользователь в -t -i
{                                                 //будет возвращена соответсвующая функция
    if (strcmp(name, "f1") == 0) return &nf1;
    if (strcmp(name, "f2") == 0) return &nf2;
    if (strcmp(name, "f3") == 0) return &nf3;
    if (strcmp(name, "f0") == 0) return &nf0;
    if (strcmp(name, "my_func0") == 0) return &nf_my;
    if (strcmp(name, "my_func1") == 0) return &nf_my1;
    return NULL;  // если функция не найдена
}

int getIntegralMethod(const char* name)//аналогично getNamedFunction
{
    if(strcmp(name, "Square") == 0) return 1;
    if(strcmp(name, "Trap") == 0) return 2;
    if(strcmp(name, "Simpson") == 0) return 3;
    if(strcmp(name, "Monte") == 0) return 4;
    return 0;
}

int getNameMethod(const char* name)//аналогично getNamedFunction
{
    if(strcmp(name, "Line") == 0) return 1;
    if(strcmp(name, "Div") == 0) return 2;
    if(strcmp(name, "Chord") == 0) return 3;
    if(strcmp(name, "Tangent") == 0) return 4;
    if(strcmp(name, "Combine") == 0) return 5;
    return 0;  // если метод не найден
}

float eval_func_arg(float a, float b, float x)
{
    return a * pow(x, b);
}

float client_func_new(Term terms[10], float x) 
{
    float y = 0;
    
    for(int i = 0; i < 10; i++) 
    {
        if(terms[i].coeff == 0) continue;
        
        float result = terms[i].coeff;
        
        // Обычный x
        if(terms[i].x_pow != 0) 
        {
            result *= pow(x, terms[i].x_pow);
        }
        
        // Модуль |x|
        if(terms[i].abs_pow != 0) 
        {
            result *= pow(fabs(x), terms[i].abs_pow);
        }
        
        // sin с аргументом
        if(terms[i].sin_pow != 0) 
        {
            float arg = eval_func_arg(terms[i].sin_arg_a, terms[i].sin_arg_b, x);
            result *= pow(sin(arg), terms[i].sin_pow);
        }
        
        // cos с аргументом
        if(terms[i].cos_pow != 0) 
        {
            float arg = eval_func_arg(terms[i].cos_arg_a, terms[i].cos_arg_b, x);
            result *= pow(cos(arg), terms[i].cos_pow);
        }
        
        // ln с аргументом
        if(terms[i].ln_pow != 0) 
        {
            float arg = eval_func_arg(terms[i].ln_arg_a, terms[i].ln_arg_b, x);
            if(arg > 0) 
            {
                result *= pow(log(arg), terms[i].ln_pow);
            } else {
                return NAN; // логарифм от неположительного числа
            }
        }
        
        y += result;
    }
    
    return y;
}

/*Обёрточные функции для пользовательских функций, чтобы не передовать в функции 
поиска корней дополнительный мусор в виде массива members_func*/ 

float my_func0(float x) 
{
    return client_func_new(parsed_terms[0], x);
}

float my_func1(float x) 
{
    return client_func_new(parsed_terms[1], x);
}

// функция для извлечения аргумента из скобок
void parse_function_arg(const char* str, float* arg_a, float* arg_b) 
{
    *arg_a = 1.0;
    *arg_b = 1.0;
    
    char* open = strchr(str, '(');
    char* close = strchr(str, ')');
    
    if(!open || !close || close <= open) return;
    
    int len = close - open - 1;
    char arg[256];
    strncpy(arg, open + 1, len);
    arg[len] = '\0';
    
    // обрабатываем разные случаи аргументов
    
    // просто x
    if(strcmp(arg, "x") == 0) 
    {
        *arg_a = 1.0;
        *arg_b = 1.0;
        return;
    }
    
    // -x
    if(strcmp(arg, "-x") == 0) 
    {
        *arg_a = -1.0;
        *arg_b = 1.0;
        return;
    }
    
    // x^n
    char* caret = strchr(arg, '^');
    if(caret) 
    {
        // проверяем что перед ^ стоит x или -x
        char before[256];
        int before_len = caret - arg;
        strncpy(before, arg, before_len);
        before[before_len] = '\0';
        
        if(strcmp(before, "x") == 0) 
        {
            *arg_a = 1.0;
            *arg_b = atof(caret + 1);
        } else if(strcmp(before, "-x") == 0) 
        {
            *arg_a = -1.0;
            *arg_b = atof(caret + 1);
        } else {
            // может быть число*x^n
            char* x_pos = strstr(before, "x");
            if(x_pos) 
            {
                if(x_pos == before) 
                {
                    *arg_a = 1.0;
                } else {
                    char coeff_str[256];
                    int coeff_len = x_pos - before;
                    strncpy(coeff_str, before, coeff_len);
                    coeff_str[coeff_len] = '\0';
                    
                    // убираем * если есть
                    if(coeff_str[coeff_len-1] == '*') 
                    {
                        coeff_str[coeff_len-1] = '\0';
                    }
                    
                    *arg_a = atof(coeff_str);
                }
                *arg_b = atof(caret + 1);
            }
        }
    } else {
        // нет степени, проверяем на коэффициент
        char* x_pos = strstr(arg, "x");
        if(x_pos) 
        {
            if(x_pos == arg) 
            {
                *arg_a = 1.0;
            } else {
                char coeff_str[256];
                int coeff_len = x_pos - arg;
                strncpy(coeff_str, arg, coeff_len);
                coeff_str[coeff_len] = '\0';
                
                // убираем * если есть
                if(coeff_len > 0 && coeff_str[coeff_len-1] == '*') 
                {
                    coeff_str[coeff_len-1] = '\0';
                }
                
                if(strcmp(coeff_str, "-") == 0) 
                {
                    *arg_a = -1.0;
                } else {
                    *arg_a = atof(coeff_str);
                }
            }
            *arg_b = 1.0;
        }
    }
}


float parse_power(const char* str) 
{
    if(*str != '^') return 1.0;
    
    str++; // пропускаем ^
    
    // Если степень в скобках
    if(*str == '(') 
    {
        str++; // пропускаем (
        float power = atof(str);
        return power;
    }
    
    return atof(str);
}

void parse_term_new(const char* term, Term* result) 
{
    memset(result, 0, sizeof(Term));
    result->coeff = 1.0;
    
    // Ищем функции
    char* sin_pos = strstr(term, "sin");
    char* cos_pos = strstr(term, "cos");
    char* ln_pos = strstr(term, "ln");
    char* abs_pos = strstr(term, "abs");
    char* pipe_pos = strchr(term, '|'); // для |x|
    
    // находим первую функцию или переменную
    const char* first_func = term + strlen(term);
    
    // ищем x который не внутри функций
    char* x_pos = NULL;
    for(const char* p = term; *p; p++) 
    {
        if(*p == 'x') 
        {
            int inside_func = 0;
            
            // проверяем что x не внутри функций
            if(sin_pos) 
            {
                char* sin_open = strchr(sin_pos, '(');
                char* sin_close = strchr(sin_pos, ')');
                if(sin_open && sin_close && p > sin_open && p < sin_close) 
                {
                    inside_func = 1;
                }
            }
            if(cos_pos) 
            {
                char* cos_open = strchr(cos_pos, '(');
                char* cos_close = strchr(cos_pos, ')');
                if(cos_open && cos_close && p > cos_open && p < cos_close) 
                {
                    inside_func = 1;
                }
            }
            if(ln_pos) 
            {
                char* ln_open = strchr(ln_pos, '(');
                char* ln_close = strchr(ln_pos, ')');
                if(ln_open && ln_close && p > ln_open && p < ln_close) 
                {
                    inside_func = 1;
                }
            }
            if(abs_pos) 
            {
                char* abs_open = strchr(abs_pos, '(');
                char* abs_close = strchr(abs_pos, ')');
                if(abs_open && abs_close && p > abs_open && p < abs_close) 
                {
                    inside_func = 1;
                }
            }
            if(pipe_pos) 
            {
                char* second_pipe = strchr(pipe_pos + 1, '|');
                if(second_pipe && p > pipe_pos && p < second_pipe) 
                {
                    inside_func = 1;
                }
            }
            
            if(!inside_func) 
            {
                x_pos = (char*)p;
                break;
            }
        }
    }
    
    // определяем что идет первым
    if(x_pos && x_pos < first_func) first_func = x_pos;
    if(sin_pos && sin_pos < first_func) first_func = sin_pos;
    if(cos_pos && cos_pos < first_func) first_func = cos_pos;
    if(ln_pos && ln_pos < first_func) first_func = ln_pos;
    if(abs_pos && abs_pos < first_func) first_func = abs_pos;
    if(pipe_pos && pipe_pos < first_func) first_func = pipe_pos;
    
    // извлекаем коэффициент
    if(first_func != term) 
    {
        char coeff_str[64];
        int coeff_len = first_func - term;
        strncpy(coeff_str, term, coeff_len);
        coeff_str[coeff_len] = '\0';
        
        // убираем * в конце
        int len = strlen(coeff_str);
        if(len > 0 && coeff_str[len-1] == '*') 
        {
            coeff_str[len-1] = '\0';
        }
        
        if(strlen(coeff_str) > 0) 
        {
            if(strcmp(coeff_str, "-") == 0) 
            {
                result->coeff = -1.0;
            } else if(strcmp(coeff_str, "+") == 0) 
            {
                result->coeff = 1.0;
            } else {
                result->coeff = atof(coeff_str);
            }
        }
    }
    
    // парсим x
    if(x_pos) 
    {
        int is_division = (x_pos > term && *(x_pos - 1) == '/');
        char* caret = strchr(x_pos, '^');
        if(caret && caret == x_pos + 1) 
        {
            result->x_pow = parse_power(caret);
            if(is_division) 
            {
                result->x_pow = -result->x_pow;
            }
        } else {
            result->x_pow = is_division ? -1.0 : 1.0;
        }
    }
    
    // парсим |x| или abs(x)
    if(pipe_pos) 
    {
        char* second_pipe = strchr(pipe_pos + 1, '|');
        if(second_pipe) 
        {
            // проверяем степень после |x|
            if(*(second_pipe + 1) == '^') 
            {
                result->abs_pow = parse_power(second_pipe + 1);
            } else {
                result->abs_pow = 1.0;
            }
        }
    } else if(abs_pos) 
    {
        char* closing = strchr(abs_pos, ')');
        if(closing) 
        {
            if(*(closing + 1) == '^') 
            {
                result->abs_pow = parse_power(closing + 1);
            } else {
                result->abs_pow = 1.0;
            }
        }
    }
    
    // парсим sin с аргументом
    if(sin_pos) 
    {
        parse_function_arg(sin_pos, &result->sin_arg_a, &result->sin_arg_b);
        
        char* closing = strchr(sin_pos, ')');
        if(closing) 
        {
            if(*(closing + 1) == '^') 
            {
                result->sin_pow = parse_power(closing + 1);
            } else {
                result->sin_pow = 1.0;
            }
        }
    }
    
    // парсим cos с аргументом
    if(cos_pos) 
    {
        parse_function_arg(cos_pos, &result->cos_arg_a, &result->cos_arg_b);
        
        char* closing = strchr(cos_pos, ')');
        if(closing) 
        {
            if(*(closing + 1) == '^') 
            {
                result->cos_pow = parse_power(closing + 1);
            } else {
                result->cos_pow = 1.0;
            }
        }
    }
    
    // парсим ln с аргументом
    if(ln_pos) 
    {
        parse_function_arg(ln_pos, &result->ln_arg_a, &result->ln_arg_b);
        
        char* closing = strchr(ln_pos, ')');
        if(closing) 
        {
            if(*(closing + 1) == '^') 
            {
                result->ln_pow = parse_power(closing + 1);
            } else {
                result->ln_pow = 1.0;
            }
        }
    }
}

// основной парсер мат выражения
int parse_math_expression_new(const char* expr, Term result[10]) 
{
    char buffer[1024];
    strncpy(buffer, expr, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    // удаляем пробелы
    char clean[1024];
    int j = 0;
    for(int i = 0; buffer[i]; i++) 
    {
        if(buffer[i] != ' ') 
            clean[j++] = buffer[i];
    }
    clean[j] = '\0';
    
    memset(result, 0, sizeof(Term) * 10);
    
    int term_count = 0;
    char* term_start = clean;
    
    // разбиваем на термы по + и -, но только вне скобок
    int parenthesis_depth = 0;
    for(int i = 0; clean[i] && term_count < 10; i++) 
    {
        // Отслеживаем глубину скобок
        if(clean[i] == '(') 
        {
            parenthesis_depth++;
        } else if(clean[i] == ')') 
        {
            parenthesis_depth--;
        }
        
        // находим + или - НЕ В НАЧАЛЕ и ВНЕ СКОБОК!
        if ((clean[i] == '+' || clean[i] == '-') && i > 0 && parenthesis_depth == 0) 
        {
            char term[256];
            int len = &clean[i] - term_start;
            strncpy(term, term_start, len);
            term[len] = '\0';
            
            parse_term_new(term, &result[term_count]);
            term_count++;
            term_start = &clean[i];
        }
    }
    
    // парсим последний терм
    if(term_count < 10 && strlen(term_start) > 0) 
    {
        parse_term_new(term_start, &result[term_count]);
        term_count++;
    }
    
    return term_count;
}

int main(int argc, char *argv[]) 
{
	srand(time(NULL));
    int rez=0;
    int option_index = 0;
    static struct option long_options[] = {
    {"help",        no_argument,       0, 'h'}, 
    {0, 0, 0, 0}
    };
		while ( (rez = getopt_long(argc,argv,"hs::t:m:i:", long_options, &option_index)) != -1)
        {
            switch (rez)
			{
			    case 'h': //help
                    printf("\n-h or --help for help\n\n");

                    printf("STATISTICS:\n");
                    printf("-s for print all statistic\n");
                    printf("-s1 for print roots and -s2 for print total number of iterations \n\n");

                    printf("CUSTOM FUNCTIONS:\n");
                    printf("-m for add yr function, example: -m ''5*x^2-sin(x)+ln(x)^3'' \n");
                    printf("-m u can add up to two func, example: -m 0.6*x+3 -m 3/x \n\n");
                    
                    printf("TEST ROOT FINDING:\n");
                    printf("-t for find root between two functions, format: -t x1 x2 eps func#1 funct#2 method\n");
                    printf("-t for find root between two functions, example: -t -7 -5 0.001 f1 f3 Combine\n");
                    printf("All methods: Line, Div, Chord, Tangent, Combine\n\n");  // список методов

                    printf("TEST INTEGRATION:\n");
                    printf("-i for find definite integral, format: -i method xl xr n function [fmax for Monte]\n");
                    printf("Example: -i Square 0 2 1000 f1\n");
                    printf("Available methods: Square, Trap, Simpson, Monte\n");
                break; 
                case 'm': // добавление функции пользователя
                    if(count_my_func >= 2) 
                    {
                        printf("Error: maximum 2 custom functions allowed\n");
                        break;
                    }
    
                    //printf("Parsing expression %d: %s\n", count_my_func, optarg);
                    memset(&parsed_terms[count_my_func], 0, sizeof(Term) * 10);  // очищаем только нужную функцию
    
                    int terms = parse_math_expression_new(optarg, parsed_terms[count_my_func]);  // парсим в правильный индекс
    
                    /*printf("Parsed %d terms for function 'my_func%d':\n", terms, count_my_func); // это отладка парсинга
                    for(int i = 0; i < terms; i++) 
                    {
                        Term *t = &parsed_terms[count_my_func][i];
                        printf("Term %d: coeff=%.2f, x^%.2f, |x|^%.2f, sin(%.2f*x^%.2f)^%.2f, cos(%.2f*x^%.2f)^%.2f, ln(%.2f*x^%.2f)^%.2f\n", 
                        i, 
                        t->coeff, 
                        t->x_pow, 
                        t->abs_pow,
                        t->sin_arg_a, t->sin_arg_b, t->sin_pow,
                        t->cos_arg_a, t->cos_arg_b, t->cos_pow,
                        t->ln_arg_a, t->ln_arg_b, t->ln_pow);
                    }*/
                    count_my_func++;
                break;
                case 'i': // тестирование как -t, только для нахождения интеграла
                {
                    char *end1, *end2, *end3;
                    test_int = 1;
    
                    // Первый аргумент - имя метода
                    if(optarg == NULL || optarg[0] == '-') 
                    {
                        printf("Error: -i requires method name\n");
                        break;
                    }
                    method_int = optarg;
                    Method_int = getIntegralMethod(method_int);
    
                    if(Method_int == 0) 
                    {
                        printf("Error: unknown integration method '%s'\n", method_int);
                        printf("Available: Square, Trap, Simpson, Monte\n");
                        break;
                    }
    
                    // Второй аргумент - xl
                    if(optind >= argc) 
                    {
                        printf("Error: -i requires xl (left bound)\n");
                        break;
                    }
                    xl_int = strtof(argv[optind], &end1);
                    while (*end1 == ' ' || *end1 == '\t') end1++;
                    if (end1 == argv[optind] || *end1 != '\0') 
                    {
                        printf("Error: xl is not a valid number\n");
                        break;
                    }
                    optind++;
    
                    // Третий аргумент - xr
                    if(optind >= argc) 
                    {
                        printf("Error: -i requires xr (right bound)\n");
                        break;
                    }
                    xr_int = strtof(argv[optind], &end2);
                    while (*end2 == ' ' || *end2 == '\t') end2++;
                    if (end2 == argv[optind] || *end2 != '\0') 
                    {
                        printf("Error: xr is not a valid number\n");
                        break;
                    }
                    optind++;
    
                    // Четвертый аргумент - n (количество разбиений)
                    if(optind >= argc) 
                    {
                        printf("Error: -i requires n (number of intervals)\n");
                        break;
                    }
                    n_int = atoi(argv[optind]);
                    if(n_int <= 0) 
                    {
                        printf("Error: n must be positive integer\n");
                        break;
                    }
                    optind++;
    
                    // Пятый аргумент - имя функции
                    if(optind >= argc || argv[optind][0] == '-') 
                    {
                        printf("Error: -i requires function name\n");
                        break;
                    }
                    func_name_int = argv[optind];
                    optind++;
    
                    func_int = getNamedFunction(func_name_int);
                    if(func_int == NULL) 
                    {
                        printf("Error: unknown function '%s'\n", func_name_int);
                        break;
                    }
    
                    // Для метода Монте-Карло нужен дополнительный параметр fmax
                    if(Method_int == 4) 
                    {
                        if(optind >= argc) 
                        {
                            printf("Error: Monte Carlo method requires fmax parameter\n");
                            break;
                        }
                        fmax_int = strtof(argv[optind], &end3);
                        while (*end3 == ' ' || *end3 == '\t') end3++;
                        if (end3 == argv[optind] || *end3 != '\0') 
                        {
                            printf("Error: fmax is not a valid number\n");
                            break;
                        }
                        optind++;
        
                        printf("Integration: method=%s, xl=%f, xr=%f, n=%d, func=%s, fmax=%f\n", 
                        method_int, xl_int, xr_int, n_int, func_name_int, fmax_int);
                    } else {
                        printf("Integration: method=%s, xl=%f, xr=%f, n=%d, func=%s\n", 
                        method_int, xl_int, xr_int, n_int, func_name_int);
                    }
                }
                break;
                case 't': // Тестирование поиска корней
                {
                    char *end1, *end2, *end3;
                    val1 = strtof(optarg, &end1);
                    test = 1;
        
                    // проверяем, что первый аргумент валидное число
                    while (*end1 == ' ' || *end1 == '\t') end1++; // пропускаем пробелы
                    if (end1 == optarg || *end1 != '\0') 
                    {
                        printf("Error: first argument of -t is not a valid number\n");
                        break;
                    }
        
                    // проверяем, есть ли следующий аргумент
                    if (optind >= argc) 
                    {
                        printf("Error: -t requires two values\n");
                        break;
                    }

                    val2 = strtof(argv[optind], &end2);
                    // проверяем, что второй аргумент валидное число
                    while (*end2 == ' ' || *end2 == '\t') 
                        end2++;
                    if (end2 == argv[optind] || *end2 != '\0') 
                    {
                        printf("Error: second argument of -t is not a valid number\n");
                        break;
                    }
                    optind++;  // пропускаем второй аргумент

                    // проверяем, есть ли следующий аргумент
                    if (optind >= argc) 
                    {
                        printf("Error: -t requires two values\n");
                        break;
                    }

                    eps_client = strtof(argv[optind], &end3);
                    // проверяем, что третий аргумент валидное число
                    while (*end3 == ' ' || *end3 == '\t') 
                        end3++;
                    if (end3 == argv[optind] || *end3 != '\0') 
                    {
                        printf("Error: third argument of -t is not a valid number\n");
                        break;
                    }
                    optind++;  // пропускаем третий аргумент

                    // проверяем, есть ли первая строка
                    if (optind >= argc || argv[optind][0] == '-') 
                    {
                        printf("Error: -t requires three strings after numbers\n");
                        break;
                    }
                    str1 = argv[optind];
                    optind++;
        
                    // Проверяем, есть ли вторая строка
                    if (optind >= argc || argv[optind][0] == '-') 
                    {
                        printf("Error: -t requires three strings after numbers\n");
                        break;
                    }
                    str2 = argv[optind];
                    optind++;

                    // Проверяем, есть ли третья строка
                    if (optind >= argc || argv[optind][0] == '-') 
                    {
                        printf("Error: -t requires three strings after numbers\n");
                        break;
                    }
                    str3 = argv[optind];
                    optind++;
        
                    func1 = getNamedFunction(str1);
                    func2 = getNamedFunction(str2);
                    Method = getNameMethod(str3);
                

                    if (func1 == NULL) 
                    {
                        printf("Error: unknown function '%s'\n", str1);
                        break;
                    }
        
                    if (func2 == NULL) 
                    {
                        printf("Error: unknown function '%s'\n", str2);
                        break;
                    }
                    if (Method == 0) 
                    {
                        printf("Error: unknown method '%s'\n", str3);
                        break;
                    }
        
                    //printf("Got three values: %f and %f and %f, and two strings: '%s' and '%s and %s'\n", val1, val2, eps_client, str1, str2, str3);
                }
                break;
                case 's': //Печатаем статистику
                    if(optarg == NULL)
                    {
                        print_statistic = 3;
                    }
                    else if(atof(optarg) < 1 || atof(optarg) > 2)
                    {
                        printf("Wrong argument\n");
                    }
                    else
                    {
                        print_statistic = atof(optarg);
                    }
                    break;
                case '?':
				    printf("Wrong argument\n");
                    printf("Use -h or --help for help\n");
				    break;
            }
        }

    if(!test && !test_int) // если не тестируем, то выполняем основную функцию
    {
        /*printf("Square = %f\n",
        - calcIntegralSquare(rootFindLineSearch(-7,-5,0.001,nf1, nf3), rootFindLineSearch(-1,0,0.001,nf2, nf3), 1000, f3)
        - calcIntegralSquare(rootFindLineSearch(-1,0,0.001,nf2, nf3), rootFindLineSearch(3,4,0.001,nf2, nf3), 1000, f2)
        + calcIntegralSquare(rootFindLineSearch(0,1,0.001,nf1, nf3), rootFindLineSearch(3,4,0.001,nf2, nf3), 1000, f3)
        + calcIntegralSquare(rootFindLineSearch(-7,-5,0.001,nf1, nf3), rootFindLineSearch(0,1,0.001,nf1, nf3), 1000, f1)); */
        float eps = 0.001;
    
        // вычисляем каждый корень один раз
        float x1 = rootFindLineSearch(-7, -5, eps, nf1, nf3);  // f1 и f3 
        float x2 = rootFindLineSearch(-1, 0, eps, nf2, nf3);   // f2 и f3 
        float x3 = rootFindLineSearch(0, 1, eps, nf1, nf3);    // f1 и f3 
        float x4 = rootFindLineSearch(3, 4, eps, nf2, nf3);    // f2 и f3 
    
        float area = 
            - calcIntegralSquare(x1, x2, 1000, f3)
            - calcIntegralSquare(x2, x4, 1000, f2)
            + calcIntegralSquare(x3, x4, 1000, f3)
            + calcIntegralSquare(x1, x3, 1000, f1);
    
        printf("Square = %f\n", area);
    }
    else if(test) 
    {
        switch(Method)
        {
            case 1:
                rootFindLineSearch(val1, val2, eps_client, *func1, *func2);
            break;
            case 2:
                rootFindDiv(val1, val2, eps_client, *func1, *func2);
            break;
            case 3:
                rootFindChord(val1, val2, eps_client, *func1, *func2);
            break;
            case 4:
                rootFindTangent(val1, eps_client, *func1, *func2, df);
            break;
            case 5:
                rootFindCombine(val1, val2, eps_client, *func1, *func2, df, ddf);
            break;
        }
        
        
    }

    if(test_int) 
    {
        float result = 0;
        switch(Method_int) 
        {
            case 1: 
                result = calcIntegralSquare(xl_int, xr_int, n_int, func_int->func);
                printf("Integral (Square method) = %f\n", result);
                break;
            case 2:  
                result = calcIntegralTrap(xl_int, xr_int, n_int, func_int->func);
                printf("Integral (Trapezoid method) = %f\n", result);
                break;
            case 3:  
                result = calcIntegralSimpson(xl_int, xr_int, n_int, func_int->func);
                printf("Integral (Simpson method) = %f\n", result);
                break;
            case 4:  
                result = calcIntegralMonteCarlo(xl_int, xr_int, fmax_int, n_int, func_int->func);
                printf("Integral (Monte Carlo method) = %f\n", result);
                break;
        }
    }    

    if(print_statistic == 2 || print_statistic == 3) //выводим статистику по итерациям, учитываются только итерации на поиск корней
        printf("Total number iteration for search roots is %d\n", total_iter);
}