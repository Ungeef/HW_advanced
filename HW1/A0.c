#include <stdio.h>
#include <stdint.h>

int main() {
	// your code goes here
	int32_t a = 0;
	int32_t max = 0;
    scanf("%d",&a);
    scanf("%d",&max);
    int32_t temp = 0;
    uint16_t quantity = 1;
    for(int i = 0; i < a-1; i++)
    {
        scanf("%d",&temp);
        if(max == temp)
        {
            quantity++;
        }
        if(max < temp)
        {
            max = temp;
            quantity = 1;
        }
    }
    printf("%d", quantity);
}

