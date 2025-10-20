#include <stdio.h>
#include <stdint.h>

#define STACK_SIZE 256
int32_t stack[STACK_SIZE];
int top = 0; // Индекс первого свободного места в стеке

void push(int32_t value) {
    if (top >= STACK_SIZE) 
	{
		return; 
	}
    stack[top++] = value;
}

int32_t pop() {
    if (top <= 0) 
	{
		return 0;
	}
    return stack[--top];
}

int main(void)
{
char c;
int temp = 0;

while((c = getchar()) != '.')
{	
	if(c == '+' || c == '-' || c == '/' || c == '*')
	{
	    int32_t b = pop();
        int32_t a = pop();
        
		switch(c)
		{
			case('+'):
				push((a + b));
			break;
			case('-'):
				push((a - b));
			break;
			case('*'):
				push((a * b));
			break;
			case('/'):
			    if(b == 0)
			        {
			            printf("Division by zero\n");
			            return -1;
			        }
				push((a / b));
			break;
		}
	}
	else if(c != ' ')
	{
		temp = c - '0';
		while((c = getchar()) != ' ')
		{
			temp = temp*10 + (c - '0');
		}
		push(temp);
	}
}
printf("%d\n", pop());
}