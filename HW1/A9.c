#include <stdio.h>
#include <inttypes.h>
#include <math.h>

struct pack_array {
uint32_t array; // поле для хранения упакованного массива из 0 и 1
uint32_t count0 : 8; // счетчик нулей в array
uint32_t count1 : 8; // счетчик единиц в array
};

void array2struct(int arr[], struct pack_array *pack)
{
    
    pack->count1 = 0;
    pack->count0 = 0;
    pack->array = 0;
    
    for(int i = 0; i < 32; i++)
    {
        scanf("%u",&arr[i]);
        if(arr[i] == 1)
        {
            pack->count1++;
            pack->array |= (1 << (31 - i));
        }
        else
        {
            pack->count0++;
        }
    }
}

int main() 
{
    struct pack_array my_data;
    int a[32];
    //int a[32] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    array2struct(a, &my_data);
    printf("%u ", my_data.array);
    printf("%u ", my_data.count0);
    printf("%u", my_data.count1);

}