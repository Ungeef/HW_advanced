#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct list 
{
    char word[20];
    struct list *next;
} list;

void print_list(list *p)
{
    while(p)
    {
        printf("%s ", p->word);
        p = p->next;
    }
}

void swap_elements(list *swap_1, list *swap_2)
{
    char a[20];
    strcpy(a, swap_1->word);
    strcpy(swap_1->word, swap_2->word);
    strcpy(swap_2->word, a);
}
void sort(list *head)
{
    if (head == NULL || head->next == NULL) 
    {
        return;
    }
    list *p1, *p2;
    for (p1 = head; p1 != NULL; p1 = p1->next) 
    {
         for (p2 = p1->next; p2 != NULL; p2 = p2->next)
        {
            if (strcmp(p1->word, p2->word) > 0)
            {
                swap_elements(p1, p2);
            }
        }
    }
}

void delete_list(list *head)
{
    list *p1 = head;
    list *next_p;
    while (p1 != NULL)
    {
        next_p = p1->next;
        free(p1);
        p1 = next_p;
    }
}

list *add_to_list(list *head, char new_word[20])
{
    list *new = calloc(1, sizeof(list));
    strcpy(new->word, new_word);
    if(head == NULL)
    {
        return new;
    }
    else 
    {
        list *p = head;
        while(p->next != NULL)
        {
            p = p->next;
        }
        p->next = new;
        return head;
    }
}

int main(void)
{
    list *head = NULL;
    //head = calloc(1, sizeof(list));
    char c;
    char new_word[20];
    int temp = 0;
    int i;

    scanf("%c", &c);
    while(c != '.')
    {
    i = 0;
        for(; c != ' ' && c != '.'; i++)
        {
            new_word[i] = c;
            scanf("%c", &c);
            temp++;
        }

        new_word[i] = '\0';
        head = add_to_list(head, new_word);
        if(c == ' ')//Пропускаем пробелы
        {
            scanf("%c", &c); 
        }
        
    }
    
    sort(head);
    print_list(head);
    delete_list(head);
    
    return 0;
}