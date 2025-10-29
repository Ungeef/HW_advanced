#include <stdio.h>
#include <stdint.h>


int main()
{
	uint32_t input;
	scanf("%u", &input);
	uint32_t quantity = 0;
	uint32_t mask = 1;
    
    for(int i = 0; i < 32; i++)
    {
        uint32_t temp = (input >> i) & mask;
        if(temp == 1)
            quantity++;
    }
	printf("%u", quantity);

	return 0;
}