#include <stdio.h>
#include <stdint.h>


int main()
{
	uint32_t input;
	uint8_t rotate;
	scanf("%u", &input);
	scanf("%hhu", &rotate);
	
	uint32_t output;
    output = (input >> rotate)|(input << (32 - rotate));
	printf("%u", output);

	return 0;
}