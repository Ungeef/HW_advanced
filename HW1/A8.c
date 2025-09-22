#include <stdio.h>

int main() {
    const int capacity = 1000000; // Вместимость чашки

    int weight;
    if (scanf("%d", &weight) != 1) {
        return 1;
    }

    // Первичная проверка на вместимость
    if (weight > capacity) {
        printf("-1\n");
        return 0;
    }
    long long total_pan_weight = 0; // Суммарная масса на чаше с гирями
    int temp_weight = weight;   
    long long p3 = 1;

    while (temp_weight > 0) {
        int remainder = temp_weight % 3;

        if (remainder == 1) {
            // Гиря p3 кладется на чашу с гирями
            total_pan_weight += p3;
            temp_weight = (temp_weight - 1) / 3;
        } else if (remainder == 2) {
            // Гиря p3 кладется на чашу с товаром. 
            temp_weight = (temp_weight + 1) / 3;
        } else { 
            // Гиря не используется
            temp_weight = temp_weight / 3;
        }
        
        // Переходим к следующей гире
        if (temp_weight > 0) {
            p3 *= 3;
        }
    }
    
    // Проверкана превышения суммарной массы на чаше
    if (total_pan_weight > capacity) {
        printf("-1\n");
        return 0;
    }
    int num_weights = 0; // Счетчик использованных гирь

    while (weight > 0) {
        int remainder = weight % 3;

        if (remainder == 0) {
            //гиря не нужна
            weight = weight / 3;
        } else if (remainder == 1) {
            //нужна одна гиря
            num_weights++;
            weight = (weight - 1) / 3;
        } else { 
            //нужна одна гиря.
            num_weights++;
            weight = (weight + 1) / 3;
        }
    }

    printf("%d\n", num_weights);

    return 0;
}