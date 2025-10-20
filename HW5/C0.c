
#include <stdio.h>
#include <string.h>
#include <stdbool.h> 

#define MAX_DIGITS 255 

int main() 
{
    char n_str[MAX_DIGITS];
    scanf("%s", n_str);

    int len = strlen(n_str);

    if (len < 3) 
	{
        printf("0\n");
        return 0;
    }

    

    bool found[1000] = {false}; //отслеживание найденных уникальных чисел
    int unique_count = 0;


    for (int i = 0; i < len - 2; ++i)     //генерируем все комбинации из трех цифр
	{
        
        if (n_str[i] == '0') //трехзначное число не может начинаться с '0'
		{ 
            continue;
        }
        for (int j = i + 1; j < len - 1; ++j) 
		{
            for (int k = j + 1; k < len; ++k) 
			{
                //собираем трехзначное число из выбранных цифр
                int d1 = n_str[i] - '0'; 
                int d2 = n_str[j] - '0';
                int d3 = n_str[k] - '0';
                int number = d1 * 100 + d2 * 10 + d3;

                //проверяем встречали ли мы это число ранее
                if (!found[number]) 
				{
                    found[number] = true; 
                    unique_count++;       
                }
            }
        }
    }

    printf("%d\n", unique_count);

    return 0;
}