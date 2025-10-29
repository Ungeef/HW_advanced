#include <stdio.h>
#include <stdint.h>


int main()
{
	uint32_t input;
	uint8_t bias;
	scanf("%u", &input);
	scanf("%hhu", &bias);
	
	uint32_t output;
    	output = input << (32-bias);
    	output >>= (32-bias);
	printf("%u", output);

	return 0;
}