#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 10000

// Z-функция
static void z_function(const char *s, size_t n, size_t *z) {
    z[0] = 0;
    size_t l = 0, r = 0;
    for (size_t i = 1; i < n; ++i) 
	{
        if (i < r) 
		{
            z[i] = z[i - l] < (r - i) ? z[i - l] : (r - i);
        } else 
		{
            z[i] = 0;
        }
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) 
            ++z[i];
        if (i + z[i] > r) 
		{ 
            l = i; 
            r = i + z[i]; 
        }
    }
}

// Находит длину наибольшего префикса X, являющегося суффиксом Y
static size_t prefix_suffix_max(const char *X, const char *Y) 
{
    size_t nx = strlen(X), ny = strlen(Y);
    
    //cоздаём строку X+$+Y
    size_t n = nx + 1 + ny;
    char *S = (char*)malloc(n + 1);
    size_t *Z = (size_t*)malloc(n * sizeof(size_t));
    
    
    strcpy(S, X);
    S[nx] = '$'; //вставляем разделитель
    strcpy(S + nx + 1, Y);
    
    // Вычисляем Z-функцию
    z_function(S, n, Z);
    
    size_t ans = 0;
    //проверяем все суффиксы Y 
    for (size_t i = nx + 1; i < n; ++i) {
        if (i + Z[i] == n && Z[i] > ans) 
            ans = Z[i];
    }
    
    free(Z);
    free(S);
    return ans;
}

int main(void) 
{
    char s1[MAX_LEN + 2], s2[MAX_LEN + 2];
    
    fgets(s1, sizeof s1, stdin);
    fgets(s2, sizeof s2, stdin);
    
    s1[strcspn(s1, "\n")] = '\0';
    s2[strcspn(s2, "\n")] = '\0';
    
    size_t a = prefix_suffix_max(s1, s2);
    size_t b = prefix_suffix_max(s2, s1);
    
    printf("%d %d", a, b);
    return 0;
}