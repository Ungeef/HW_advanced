#include <stdio.h>
#include <inttypes.h>

typedef struct list {
uint64_t address;
size_t size;
char comment[64];
struct list *next;
} list;

uint64_t findMaxBlock(list *head)
{
    if (head == NULL) 
    {
        return 0;
    }
    
    size_t maxSize = head->size;
    uint64_t maxAddress = head->address;
    
    list *current = head->next;
    
    while (current != NULL) 
    {
        if (current->size > maxSize) 
        {
            maxSize = current->size;
            maxAddress = current->address;
        }
        current = current->next;
    }
    return maxAddress;
    
}

size_t totalMemoryUsage(list *head)
{
    if (head == NULL) 
    {
        return 0;
    }
    
    size_t sumSize = head->size;
    
    list *current = head->next;
    
    while (current != NULL) 
    {
        sumSize += current->size;
        current = current->next;
    }
    return sumSize;
    
}


int main() 
{
    list *head = (list*)malloc(sizeof(list));
    list *second = (list*)malloc(sizeof(list));
    list *third = (list*)malloc(sizeof(list));

    //Проверка на исходных данных
    head->address = 140525067852320;
    head->size = 10;
    head->next = second;

    second->address = 140525067852350;
    second->size = 30;
    second->next = third;

    third->address = 140525067852900;
    third->size = 100;
    third->next = NULL; 


    uint64_t result = findMaxBlock(head);
    printf("%llu\n", result);
    
    size_t result2 = totalMemoryUsage(head);
    printf("%zu\n", result2);
    


    free(head);
    free(second);
    free(third);

    return 0;

}