#include <stdio.h>
#include <inttypes.h>
union floatbit {
 float value;
 struct {
 uint32_t mant : 23;
 uint32_t exp : 8;
 uint32_t sign : 1;
 } bit;
} f;
int main()
{
 scanf("%f", &f.value);
 printf("%u", f.bit.exp);

 return 0;
}