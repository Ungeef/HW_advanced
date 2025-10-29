#include <stdio.h>
#include <stdint.h>

int main(void) {
    
 int32_t res = 0;
 int32_t input;
 int32_t N;
 
 scanf("%d", &N);
 for(int i = 0; i < N; i++)
 {
    scanf("%d", &input);
    res ^= input;
 }
 
 printf("%d", res);
 return 0;
 
}
