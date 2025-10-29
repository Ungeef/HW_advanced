#include <stdio.h>
#include <stdint.h>


int main()
{
	uint32_t input;
	uint8_t K;
	scanf("%u", &input);
	scanf("%hhu", &K);
	uint32_t max = 0;
	
	
	uint32_t mask = (1u << K) - 1;  // Маска из K единиц
    
    for(int i = 0; i + K <= 32; i++)
    {
        uint32_t temp = (input >> i) & mask;
        if(temp > max)
            max = temp;
    }
	
	/*
	uint32_t temp;
	for(int i = 0; i+K <= 32; i++)
	{
	    temp = input<<(32-K-i);
	    temp = temp>>(32-K);
	    if(temp > max)
	        max = temp;
	}*/
	printf("%u", max);

	return 0;
}