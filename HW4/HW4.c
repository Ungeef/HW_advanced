#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#define MIN_Y  2
#define CONTROLS 3
#define FOOD_COUNT 5

enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=7, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=3};


// Здесь храним коды управления змейкой
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
}control_buttons;

struct control_buttons default_controls[CONTROLS] = {
    {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT}, //Стрелки
    {'s', 'w', 'a', 'd'},                     //wasd
    {'S', 'W', 'A', 'D'}                      //WASD
};

//Все о ЕДЕ

struct food
{
	int x;
	int y;
	time_t put_time;
	char point;
	uint8_t enable;
}food[MAX_FOOD_SIZE];

void initFood(struct food f[], size_t size)
{
	struct food init = {0,0,0,0,0};
	int max_y=0, max_x=0;
	getmaxyx(stdscr, max_y, max_x);
	for(size_t i=0; i<size; i++)
	{
		f[i] = init;
	}
}

/* Обновить/разместить текущее зерно на поле */
void putFoodSeed(struct food *fp)
{
	int max_x=0, max_y=0;
	char spoint[2] = {0};
	getmaxyx(stdscr, max_y, max_x);
	mvprintw(fp->y, fp->x, " ");
	fp->x = rand() % (max_x - 1);
	fp->y = rand() % (max_y - 2) + 1; //Не занимаем верхнюю строку
	fp->put_time = time(NULL);
	fp->point = '$';
	fp->enable = 1;
	spoint[0] = fp->point;
	mvprintw(fp->y, fp->x, "%s", spoint);
}

/* Разместить еду на поле */
void putFood(struct food f[], size_t number_seeds)
{
	for(size_t i=0; i<number_seeds; i++)
	{
		putFoodSeed(&f[i]);
	}
}

void refreshFood(struct food f[], int nfood)
{
	int max_x=0, max_y=0;
	char spoint[2] = {0};
	getmaxyx(stdscr, max_y, max_x);
	for(size_t i=0; i<nfood; i++)
	{
		if( f[i].put_time )
		{
			if( !f[i].enable || (time(NULL) - f[i].put_time) > FOOD_EXPIRE_SECONDS )
			{
				putFoodSeed(&f[i]);
			}
		}
	}
}


/*
 Голова змейки содержит в себе
 x,y - координаты текущей позиции
 direction - направление движения
 tsize - размер хвоста
 *tail -  ссылка на хвост
 */
typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
} snake_t;

/*
 Хвост это массив состоящий из координат x,y
 */
typedef struct tail_t
{
    int x;
    int y;
} tail_t;

void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}
void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
}

void initSnake(snake_t *head, size_t size, int x, int y)
{
tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size+1;
}

/*
 Движение головы с учетом текущего направления движения
 */
void go(struct snake_t *head)
{
    char ch = '@';
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x); // macro - размер терминала
    mvprintw(head->y, head->x, " "); // очищаем один символ
    switch (head->direction)
    {
        case LEFT:
        	if(head->x <= 0) // Циклическое движение, чтобы не уходить за пределы экрана
			   head->x = max_x;
            mvprintw(head->y, --(head->x), "%c", ch);
            
        break;
        case RIGHT:
        	if(head->x >= max_x) 
			   head->x = 0;
            mvprintw(head->y, ++(head->x), "%c", ch);
        break;
        case UP:
        	if(head->y <= 1) //Стартуем с 1 чтобы не задеть надпись
			   head->y = max_y;
            mvprintw(--(head->y), head->x, "%c", ch);
        break;
        case DOWN:
        	if(head->y >= max_y) 
			   head->y = 1;
            mvprintw(++(head->y), head->x, "%c", ch);
        break;
        default:
        break;
    }
    refresh();
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
	for (int i = 0; i < CONTROLS; i++)
 	{   
    if (key == default_controls[i].down)
    {
    	snake->direction = DOWN;
        return;
	}
    else if (key == default_controls[i].up)
    {
    	snake->direction = UP;
        return;
	}
    else if (key == default_controls[i].right)
    {
    	snake->direction = RIGHT;
        return;
	}
    else if (key == default_controls[i].left)
    {
    	snake->direction = LEFT;
        return;
	}
	}
}
int checkDirection(snake_t* snake, int32_t key)
{
	for(int i = 0; i < CONTROLS; i++)
	{
		if(key == default_controls[i].down && snake->direction != UP ||
	key == default_controls[i].up && snake->direction != DOWN ||
	key == default_controls[i].right && snake->direction != LEFT ||
	key == default_controls[i].left && snake->direction != RIGHT)
		return 1;	
	}
	return 0;
}

/*
 Движение хвоста с учетом движения головы
 */
void goTail(struct snake_t *head)
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

int check_collision(snake_t *head)
{
    //Проверка столкновения с хвостом
    for (size_t i = 1; i < head->tsize; i++) 
    {
        if (head->x == head->tail[i].x && head->y == head->tail[i].y)
        {
            return 1; 
        }
    }

    return 0; 
}

int main()
{
	snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake,START_TAIL_SIZE,10,10);
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    //Отключаем курсор
    mvprintw(0, 0,"Use arrows for control. Press 'F10' for EXIT");
    timeout(0);    //Отключаем таймаут после нажатия клавиши в цикле
    int key_pressed = 0;
    int gameover = 0;
    
    srand(time(NULL));
    initFood(food, MAX_FOOD_SIZE);
    putFood(food, FOOD_COUNT);

    const double SECONDS_PER_UPDATE = 0.1; //время одного шага
    clock_t last_update_time = clock();
    
    while( key_pressed != STOP_GAME && gameover != 1)
    {
    	key_pressed = getch(); // Считываем клавишу
    	if(checkDirection(snake, key_pressed))
    		changeDirection(snake, key_pressed);
    	
    	clock_t current_time = clock();
    	
    	if ((double)(current_time - last_update_time) / CLOCKS_PER_SEC >= SECONDS_PER_UPDATE)
        {
        	go(snake);
        	goTail(snake);
        	gameover = check_collision(snake); // проверка съедания хвоста
        	refreshFood(food, FOOD_COUNT);
        	
        	last_update_time = current_time;
		}
        
 		
        
    }
    free(snake->tail);
    free(snake);
    endwin(); // Завершаем режим curses mod
    return 0;
}