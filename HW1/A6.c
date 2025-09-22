#include <stdio.h>
#include <stdint.h>


int main()
{
	uint32_t input;
	scanf("%u", &input);

    	uint32_t output = input ^ 0xFF000000; //не сразу догадался до простого варианта
    
    	/*uint32_t temp = (~(input >> 24))<<24;
    	uint32_t output = ((input<<8)>>8) | temp; */
        
    	printf("%u", output);

	return 0;
}