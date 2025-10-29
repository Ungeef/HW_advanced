#include <stdio.h>

int main()
{
	int n; //Сдвиг
	char c; //Обрабатываемый символ
	char p; //Новый символ
	
	scanf("%d", &n);
	scanf(" ");
	
	while(c!='.')
	{
		scanf("%c", &c);
		if(c >= 97) //Если символ строчный
		{
			p = (((c + n) - 97) % 26) + 97;
			printf("%c", p);
		}
		else if(c == 32 || c == 46)
		{
			printf("%c", c);
		}
		else //Если символ прописной
		{
			p = (((c + n) - 65) % 26) + 65;
			printf("%c", p);
		}
	}
	
	
	
	return 0;
}