#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#define MIN_Y  2
#define CONTROLS 3
#define FOOD_COUNT 5
#define PLAYERS

enum 
{
    TIME_1_MINUTE = 60,
    TIME_3_MINUTES = 180,
    TIME_5_MINUTES = 300
};

enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=7, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=3};

enum multiplayer_mode {
    MODE_NONE,       // Для одиночной игры
    MODE_LOCAL_COOP, // Игра на одной клавиатуре
    MODE_VS_AI,      // Игрок против ИИ
    MODE_NETWORK     // Локальная игра
};

// Здесь храним коды управления змейкой
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
}control_buttons;

struct control_buttons default_controls[CONTROLS] = 
{
    {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT}, //Стрелки
    {'s', 'w', 'a', 'd'},                     //wasd
    {'S', 'W', 'A', 'D'}                      //WASD
};

//Настройки игры
typedef struct game_settings 
{
    int player_count;
    int player1_color;
    int player2_color;
    enum multiplayer_mode mp_mode;
    bool edible_tails_mode; // Чекбокс нового подрежима - съедобный хвост? (придумать название)
    int game_time_seconds;  // Время для подрежима иначе игар будет вечной
} game_settings;

time_t game_start_time;

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
    attron(COLOR_PAIR(3));
	mvprintw(fp->y, fp->x, "%s", spoint);
    attroff(COLOR_PAIR(3));
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
    struct control_buttons controls;
    int player_num; //Используем для определения цвета
     int color_id;
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


void initSnake(snake_t *head[], size_t size, int x, int y, int i, int color_id)
{
    head[i]    = (snake_t*)malloc(sizeof(snake_t));
    tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head[i], x, y);
    head[i]->tail     = tail; // прикрепляем к голове хвост
    head[i]->tsize    = size+1;
    head[i]->controls = default_controls[i];
    //head[i]->controls = default_controls[0];
    head[i]->player_num = i + 1;
    head[i]->color_id = color_id;
}


/*
 Движение головы с учетом текущего направления движения
 */
void go(struct snake_t *head)
{
    char ch = '@';
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x); 
    mvprintw(head->y, head->x, " "); 
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
    //Рисуем цветную голову
    attron(COLOR_PAIR(head->color_id));
    mvprintw(head->y, head->x, "%c", ch);
    attroff(COLOR_PAIR(head->color_id));

    refresh();
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
	if (key == snake->controls.down)
    {
    	snake->direction = DOWN;
	}
    else if (key == snake->controls.up)
    {
    	snake->direction = UP;
	}
    else if (key == snake->controls.right)
    {
    	snake->direction = RIGHT;
	}
    else if (key == snake->controls.left)
    {
    	snake->direction = LEFT;
	}
}
int checkDirection(snake_t* snake, int32_t key)
{
	if ((key == snake->controls.down && snake->direction != UP) ||
	    (key == snake->controls.up && snake->direction != DOWN) ||
	    (key == snake->controls.right && snake->direction != LEFT) ||
	    (key == snake->controls.left && snake->direction != RIGHT))
    {
		return 1; // Движение разрешено
    }
	
	return 0; // Нажата не та клавиша, или это движение назад
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
        {
            attron(COLOR_PAIR(head->color_id));
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
            attroff(COLOR_PAIR(head->color_id));
        }
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

int isCrush(snake_t *head)
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

int isCrushWithOtherSnake(snake_t *s1, snake_t *s2)
{
    // Цикл от 0 чтобы проверить и столкновение лоб в лоб
    for (size_t i = 0; i < s2->tsize; i++)
    {
        if (s1->x == s2->tail[i].x && s1->y == s2->tail[i].y)
        {
            return 1; 
        }
    }
    return 0; 
}

bool haveEat(struct snake_t *head, struct food f[])
{
    for(int i = 0; i < MAX_FOOD_SIZE; i++)
    {
        if(head->x == f[i].x  && head->y == f[i].y && f[i].enable == 1)
        {
            f[i].enable = 0;
            return 1;
        }
    }
    return 0;
}

 /*
 Увеличение хвоста на 1 элемент
 */
void addTail(struct snake_t *head)
{
    if (head->tsize < MAX_TAIL_SIZE)
    {
        head->tsize++;
    }
}

void repairSeed(struct food f[], size_t nfood, struct snake_t *head)
 {
    for( size_t i=0; i<head->tsize; i++ )
        for( size_t j=0; j<nfood; j++ )
        {
            /* Если хвост совпадает с зерном */
            if(f[j].x == head->tail[i].x && f[j].y == head->tail[i].y && f[j].enable)
            {
                putFoodSeed(&f[j]);
            }
        }
    for( size_t i=0; i<nfood; i++ )
        for( size_t j=i + 1; j<nfood; j++ )
        {
            /* Если два зерна на одной точке */
            if(f[j].enable && f[i].enable && f[i].x == f[j].x && f[i].y == f[j].y)
            {
                putFoodSeed(&f[j]);
            }
        }
 }

void setColor(int objectType)
{
    attroff(COLOR_PAIR(1));
    attroff(COLOR_PAIR(2));
    attroff(COLOR_PAIR(3));
    switch (objectType)
    {
        case 1:
        { // SNAKE1
            attron(COLOR_PAIR(1));
            break;
        }
        case 2:
        { // SNAKE2
            attron(COLOR_PAIR(2));
            break;
        }
        case 3:
        { // FOOD
            attron(COLOR_PAIR(3));
            break;
        }
    }
}

// Возвращает 1 если нужно запустить клиент, 0 в остальных случаях
int startMenu(game_settings* settings)
{
    // Настройки по умолчанию
    *settings = (game_settings){.player_count = 1, .mp_mode = MODE_NONE, .player1_color = 1, .player2_color = 2, .edible_tails_mode = false, .game_time_seconds = TIME_3_MINUTES};
    
    int choice = 0;
    // 0 - главное, 1 - мультиплеер, 2 - выбор цвета 1, 3 - выбор цвета 2, 4 - сетевая игра 
    int menu_stage = 0; 
    int key;
    
    int time_options[] = {TIME_1_MINUTE, TIME_3_MINUTES, TIME_5_MINUTES};
    char* time_labels[] = {"1 Minute", "3 Minutes", "5 Minutes"};
    int time_choice = 1; // По умолчанию 3 минуты

    clear();
    
    while(1)
    {
        clear();
        
        // ГЛАВНОЕ МЕНЮ
        if(menu_stage == 0) 
        {
            attron(COLOR_PAIR(3));
            mvprintw(5, 20, "===== SNAKE GAME =====");
            attroff(COLOR_PAIR(3));
            mvprintw(7, 25, "Welcome to Snake!");
            mvprintw(9, 20, "Select game mode:");
            
            if(choice == 0) attron(A_REVERSE);
            mvprintw(11, 20, "1. Single Player");
            attroff(A_REVERSE);
            
            if(choice == 1) attron(A_REVERSE);
            mvprintw(12, 20, "2. Multiplayer");
            attroff(A_REVERSE);
            
            if(choice == 2) attron(A_REVERSE);
            mvprintw(13, 20, "3. Exit");
            attroff(A_REVERSE);
            
            mvprintw(15, 15, "Use UP/DOWN arrows to navigate, ENTER to select");
            
            key = getch();
            
            switch(key)
            {
                case KEY_UP: choice = (choice - 1 + 3) % 3; break;
                case KEY_DOWN: choice = (choice + 1) % 3; break;
                case '\n': case KEY_ENTER:
                    if(choice == 0) 
                    { // Одиночка
                        settings->player_count = 1;
                        settings->mp_mode = MODE_NONE;
                        menu_stage = 2; // выбор цвета 1
                    }
                    else if(choice == 1) 
                    { // МУЛЬТИПЛЕЕР
                        menu_stage = 1; // Подменю мультиплеера
                    }
                    else if(choice == 2) 
                    { 
                        endwin();
                        exit(0);
                    }
                    choice = 0;
                    break;
            }
        }
        // ПОДМЕНЮ МУЛЬТИПЛЕЕРА
        else if(menu_stage == 1) 
        {
            attron(COLOR_PAIR(3));
            mvprintw(5, 20, "===== MULTIPLAYER MODE =====");
            attroff(COLOR_PAIR(3));
            
            if(choice == 0) attron(A_REVERSE);
            mvprintw(8, 20, "Local Coop (Player vs Player)");
            attroff(A_REVERSE);
            
            if(choice == 1) attron(A_REVERSE);
            mvprintw(9, 20, "Player vs AI");
            attroff(A_REVERSE);

            if(choice == 2) attron(A_REVERSE);
            mvprintw(10, 20, "Network Play");
            attroff(A_REVERSE);

            if(choice == 3) attron(A_REVERSE);
            mvprintw(12, 20, "[%c] Edible Tails Mode", settings->edible_tails_mode ? 'X' : ' ');
            attroff(A_REVERSE);
            
            if (settings->edible_tails_mode) 
            {
                if(choice == 4) attron(A_REVERSE);
                mvprintw(13, 20, "Game Time: <%s>", time_labels[time_choice]);
                attroff(A_REVERSE);
            }

            if(choice == (settings->edible_tails_mode ? 5 : 4)) attron(A_REVERSE);
            mvprintw(15, 20, "Back to Main Menu");
            attroff(A_REVERSE);
            
            int max_choice = settings->edible_tails_mode ? 5 : 4;

            key = getch();
            switch(key) 
            {
                case KEY_UP: choice = (choice - 1 + (max_choice + 1)) % (max_choice + 1); break;
                case KEY_DOWN: choice = (choice + 1) % (max_choice + 1); break;
                case KEY_LEFT:
                    if (choice == 4 && settings->edible_tails_mode) time_choice = (time_choice - 1 + 3) % 3;
                    break;
                case KEY_RIGHT:
                    if (choice == 4 && settings->edible_tails_mode) time_choice = (time_choice + 1) % 3;
                    break;
                case '\n': case KEY_ENTER:
                    if (choice == 3) 
                    {
                        settings->edible_tails_mode = !settings->edible_tails_mode;
                    } else if (choice == max_choice) 
                    {
                        menu_stage = 0;
                        choice = 0;
                    } else if (choice == 2) 
                    {
                        menu_stage = 4;
                        choice = 0;
                    } else 
                    {
                        settings->player_count = 2;
                        if (choice == 0) settings->mp_mode = MODE_LOCAL_COOP;
                        else if (choice == 1) settings->mp_mode = MODE_VS_AI;
                        
                        if (settings->edible_tails_mode) settings->game_time_seconds = time_options[time_choice];
                        else settings->game_time_seconds = 0;
                        
                        menu_stage = 2;
                        choice = 0;
                    }
                    break;
            }
        }
        // ВЫБОР ЦВЕТА ИГРОКА 1
        else if(menu_stage == 2)
        {
            mvprintw(5, 20, "===== SELECT COLOR =====");
            mvprintw(7, 20, "Player 1 - Choose your color:");

            char* colors[] = {"Red", "Blue", "Green", "Yellow", "Magenta", "Cyan"};
            for (int i = 0; i < 6; i++) 
            {
                if(choice == i) attron(A_REVERSE);
                attron(COLOR_PAIR(i + 1));
                mvprintw(9 + i, 20, "%d. %s", i + 1, colors[i]);
                attroff(COLOR_PAIR(i + 1));
                attroff(A_REVERSE);
            }
            mvprintw(16, 15, "Controls: Arrow Keys (or WASD in Single Player)");
            
            key = getch();
            switch(key) 
            {
                case KEY_UP: choice = (choice - 1 + 6) % 6; break;
                case KEY_DOWN: choice = (choice + 1) % 6; break;
                case '\n': case KEY_ENTER:
                    settings->player1_color = choice + 1;
                    if(settings->player_count == 2) 
                    {
                        menu_stage = 3;
                        choice = 0;
                    } else 
                    {
                        return 0;
                    }
                    break;
            }
        }
        // ВЫБОР ЦВЕТА ИГРОКА 2 
        else if(menu_stage == 3) 
        {
            mvprintw(5, 20, "===== SELECT COLOR =====");
            char player2_title[50];
            if (settings->mp_mode == MODE_VS_AI) sprintf(player2_title, "Choose AI color:");
            else sprintf(player2_title, "Player 2 - Choose your color:");
            mvprintw(7, 20, "%s", player2_title);

            int option = 0;
            char* colors[] = {"Red", "Blue", "Green", "Yellow", "Magenta", "Cyan"};
            for(int i = 1; i <= 6; i++) 
            {
                if(i == settings->player1_color) continue;
                if(option == choice) attron(A_REVERSE);
                attron(COLOR_PAIR(i));
                mvprintw(9 + option, 20, "%d. %s", option + 1, colors[i-1]);
                attroff(COLOR_PAIR(i));
                attroff(A_REVERSE);
                option++;
            }
            if (settings->mp_mode == MODE_LOCAL_COOP) mvprintw(16, 15, "Controls: WASD keys");
            
            key = getch();
            switch(key) 
            {
                case KEY_UP: choice = (choice - 1 + 5) % 5; break;
                case KEY_DOWN: choice = (choice + 1) % 5; break;
                case '\n': case KEY_ENTER:
                    int color_index = 1;
                    int current_option = 0;
                    for(int i = 1; i <= 6; i++) 
                    {
                        if(i == settings->player1_color) continue;
                        if(current_option == choice) 
                        {
                            color_index = i;
                            break;
                        }
                        current_option++;
                    }
                    settings->player2_color = color_index;
                    return 0;
            }
        }
        // ВЫБОР ХОСТ/КЛИЕНТ
        else if(menu_stage == 4) 
        {
            attron(COLOR_PAIR(3));
            mvprintw(5, 20, "===== NETWORK MODE =====");
            attroff(COLOR_PAIR(3));

            if(choice == 0) attron(A_REVERSE);
            mvprintw(9, 20, "1. Host Game (You are Player 1)");
            attroff(A_REVERSE);
            
            if(choice == 1) attron(A_REVERSE);
            mvprintw(10, 20, "2. Join Game (You are Player 2)");
            attroff(A_REVERSE);
            
            if(choice == 2) attron(A_REVERSE);
            mvprintw(12, 20, "3. Back");
            attroff(A_REVERSE);

            mvprintw(14, 10, "NOTE: Both players must be in the same network folder!");

            key = getch();
            switch(key) 
            {
                case KEY_UP: choice = (choice - 1 + 3) % 3; break;
                case KEY_DOWN: choice = (choice + 1) % 3; break;
                case '\n': case KEY_ENTER:
                    if (choice == 0) 
                    { // Host
                        settings->player_count = 2;
                        settings->mp_mode = MODE_NETWORK;
                        settings->player1_color = 1;
                        settings->player2_color = 2;
                        return 0;
                    }
                    else if (choice == 1) 
                    { // Client
                        return 1;
                    }
                    else if (choice == 2) 
                    { // Back
                        menu_stage = 1;
                        choice = 0;
                    }
                    break;
            }
        }
        
        refresh();
    }
}

void turn_tail_into_food(snake_t* snake, int from, struct food food_array[], int max_food_size) 
{
    for (int i = from; i < snake->tsize; i++) 
    {
        for (int j = 0; j < max_food_size; j++) 
        {
            if (!food_array[j].enable) 
            {
                food_array[j].x = snake->tail[i].x;
                food_array[j].y = snake->tail[i].y;
                food_array[j].point = '$'; // Превращаем хвост в еду
                food_array[j].put_time = time(NULL);
                food_array[j].enable = 1;
                
                // Рисуем новый кусок еды
                attron(COLOR_PAIR(snake->color_id));
                mvprintw(food_array[j].y, food_array[j].x, "%c", food_array[j].point);
                attroff(COLOR_PAIR(snake->color_id));
                
                break; // Нашли слот, переходим к следующему куску хвоста
            }
        }
    }
    // Укорачиваем хвост
    snake->tsize = from;
}

// Обработка столкновений в режиме съедобный хвост
void handle_edible_collision(snake_t* s1, snake_t* s2, struct food food_array[], int max_food_size) 
{
    // Проверяем не врезалась ли голова s1 в хвост s2
    // Чтобы не съесть голову i = 1 (Я так и не придумал что делать при столкновении лоб в лоб)
    for (int i = 1; i < s2->tsize; i++) 
    {
        if (s1->x == s2->tail[i].x && s1->y == s2->tail[i].y) 
        {
            // Съедаем кусок хвоста
            s1->tsize++; // Змейка-агрессор растет
            
            // Превращаем остаток хвоста s2 в еду
            turn_tail_into_food(s2, i, food_array, max_food_size);
            return; 
        }
    }
}

void gameOverScreen(game_settings settings, snake_t* snakes[]) 
{
    clear(); 
    
    attron(COLOR_PAIR(1));
    mvprintw(5, 20, "=====================");
    mvprintw(6, 20, "      GAME OVER      ");
    mvprintw(7, 20, "=====================");
    attroff(COLOR_PAIR(1));

    if (settings.player_count == 1)
    {
        // В одиночной игре показываем длину змейки + 
        mvprintw(10, 18, "Your score (snake length): %zu", snakes[0]->tsize);
    }
    else // В мультиплеере определяем победителя -
    {
        // кто длиннее тот и победил - надо передалать!
        if (snakes[0]->tsize > snakes[1]->tsize)
        {
            mvprintw(10, 20, "Player 1 WINS!");
        }
        else if (snakes[1]->tsize > snakes[0]->tsize)
        {
            mvprintw(10, 20, "Player 2 WINS!");
        }
        else
        {
            mvprintw(10, 25, "DRAW!");
        }
        
        mvprintw(12, 18, "Player 1 length: %zu", snakes[0]->tsize);
        mvprintw(13, 18, "Player 2 length: %zu", snakes[1]->tsize);
    }

    mvprintw(16, 15, "Press any key to exit...");
    
    refresh();
    timeout(-1); // Ждем нажатия любой клавиши бесконечно
    getch();
}

void update(struct snake_t *head)
{
    go(head);
    goTail(head);
}

struct food* find_closest_food(snake_t* snake, struct food f[], int food_count) 
{
    struct food* closest_food = NULL;
    int min_distance = -1;

    for (int i = 0; i < food_count; i++) 
    {
        // Проверяем что еда съедобна
        if (f[i].enable) 
        {
            // Вычисляем Манхэттенское расстояние
            int distance = abs(snake->x - f[i].x) + abs(snake->y - f[i].y);
            
            if (closest_food == NULL || distance < min_distance) 
            {
                min_distance = distance;
                closest_food = &f[i];
            }
        }
    }
    return closest_food;
}

void ai_make_move(snake_t* ai_snake, struct food* target_food) 
{
    if (target_food == NULL) 
    {
        return; // Если еды нет - ничего не делаем
    }

    // Разница по координатам
    int dx = target_food->x - ai_snake->x;
    int dy = target_food->y - ai_snake->y;

    // Приоритет движения по горизонтали
    if (abs(dx) > abs(dy)) 
    {
        if (dx > 0 && ai_snake->direction != LEFT) 
        {
            ai_snake->direction = RIGHT;
        } else if (dx < 0 && ai_snake->direction != RIGHT) 
        {
            ai_snake->direction = LEFT;
        }
    } 
    // Приоритет движения по вертикали
    else 
    {
        if (dy > 0 && ai_snake->direction != UP) 
        {
            ai_snake->direction = DOWN;
        } else if (dy < 0 && ai_snake->direction != DOWN) 
        {
            ai_snake->direction = UP;
        }
    }


    // Если основное направление заблокировано пытаемся выбрать альтернативное
    if (dx > 0 && ai_snake->direction == LEFT) 
    {
        if (dy > 0 && ai_snake->direction != UP) ai_snake->direction = DOWN;
        else if (dy < 0 && ai_snake->direction != DOWN) ai_snake->direction = UP;
    }
    else if (dx < 0 && ai_snake->direction == RIGHT) 
    {
        if (dy > 0 && ai_snake->direction != UP) ai_snake->direction = DOWN;
        else if (dy < 0 && ai_snake->direction != DOWN) ai_snake->direction = UP;
    }
    // Если по Y нельзя пробуем по X
    else if (dy > 0 && ai_snake->direction == UP) 
    {
        if (dx > 0 && ai_snake->direction != LEFT) ai_snake->direction = RIGHT;
        else if (dx < 0 && ai_snake->direction != RIGHT) ai_snake->direction = LEFT;
    }
    else if (dy < 0 && ai_snake->direction == DOWN) 
    {
        if (dx > 0 && ai_snake->direction != LEFT) ai_snake->direction = RIGHT;
        else if (dx < 0 && ai_snake->direction != RIGHT) ai_snake->direction = LEFT;
    }
}


// НАЧАЛО БЛОКА СЕТЕВОЙ ИГРЫ (БОЛИ)
// Клиент записывает свое нажатие в input.txt
void write_client_input(int direction) 
{
    FILE *f = fopen("input.txt", "w");
    if (f == NULL) return;
    fprintf(f, "%d", direction);
    fclose(f);
}

// Хост читает нажатие клиента из input.txt
int read_client_input() 
{
    int direction = 0;
    FILE *f = fopen("input.txt", "r");
    if (f == NULL) return 0; // Если файла нет считаем, что нажатия не было
    fscanf(f, "%d", &direction);
    fclose(f);
    return direction;
}

// Хост записывает полное состояние игры в gamestate.txt
void write_game_state(game_settings settings, snake_t* snakes[], struct food f[], int food_count, int gameover) 
{

    FILE *file = fopen("gamestate.tmp", "w");
    if (file == NULL) return;

    // Состояние игры (0 = идет, 1 = game over)
    fprintf(file, "%d\n", gameover);

    // Данные по каждой змейке
    for (int i = 0; i < settings.player_count; i++) 
    {
        // Голова: x y длина_хвоста цвет
        fprintf(file, "%d %d %zu %d\n", snakes[i]->x, snakes[i]->y, snakes[i]->tsize, snakes[i]->color_id);
        // Хвост: x1 y1 ...
        for (size_t j = 0; j < snakes[i]->tsize; j++) 
        {
            fprintf(file, "%d %d ", snakes[i]->tail[j].x, snakes[i]->tail[j].y);
        }
        fprintf(file, "\n");
    }

    // Данные по еде
    int active_food_count = 0;
    for (int i = 0; i < food_count; i++) 
    {
        if (f[i].enable) active_food_count++;
    }
    fprintf(file, "%d\n", active_food_count);
    for (int i = 0; i < food_count; i++) 
    {
        if (f[i].enable) 
        {
            fprintf(file, "%d %d\n", f[i].x, f[i].y);
        }
    }

    fclose(file);

    remove("gamestate.txt");
    rename("gamestate.tmp", "gamestate.txt");
}



typedef struct GameState 
{
    int gameover;
    snake_t snakes[2]; // Храним полные структуры змеек
    tail_t snake_tails[2][MAX_TAIL_SIZE]; // И их хвосты
    struct food foods[FOOD_COUNT];
    int food_count;
} GameState;
// Клиент читает состояние игры и отрисовывает его
// Возвращает 1 если игра окончена, 0 - если продолжается
int read_game_state_from_file(GameState* state) 
{
    FILE *file = fopen("gamestate.txt", "r");
    if (file == NULL) return 0;

    fscanf(file, "%d", &state->gameover);
    if (state->gameover) 
    {
        fclose(file);
        return 1;
    }

    for (int i = 0; i < 2; i++) 
    {
        state->snakes[i].tail = state->snake_tails[i]; 
        fscanf(file, "%d %d %zu %d", &state->snakes[i].x, &state->snakes[i].y, &state->snakes[i].tsize, &state->snakes[i].color_id);
        for (size_t j = 0; j < state->snakes[i].tsize; j++) 
        {
            fscanf(file, "%d %d", &state->snakes[i].tail[j].x, &state->snakes[i].tail[j].y);
        }
    }

    fscanf(file, "%d", &state->food_count);
    for (int i = 0; i < state->food_count; i++) 
    {
        fscanf(file, "%d %d", &state->foods[i].x, &state->foods[i].y);
    }

    fclose(file);
    return 1;
}

//  функция которая ТОЛЬКО рисует aw
void draw_client_view(GameState* state) 
{
    clear();
    mvprintw(0, 1, "CLIENT VIEW. Use WASD for control. Press 'F10' for EXIT");

    if (state->gameover) return;

    // Рисуем змеек
    for (int i = 0; i < 2; i++) 
    {
        attron(COLOR_PAIR(state->snakes[i].color_id));
        for (size_t j = 0; j < state->snakes[i].tsize; j++) 
        {
            mvprintw(state->snakes[i].tail[j].y, state->snakes[i].tail[j].x, "*");
        }
        mvprintw(state->snakes[i].y, state->snakes[i].x, "@");
        attroff(COLOR_PAIR(state->snakes[i].color_id));
    }

    // Рисуем еду
    attron(COLOR_PAIR(3));
    for (int i = 0; i < state->food_count; i++) 
    {
        mvprintw(state->foods[i].y, state->foods[i].x, "$");
    }
    attroff(COLOR_PAIR(3));
    
    refresh(); 
}
void write_client_input(int direction);
int read_client_input();
void write_game_state(game_settings settings, snake_t* snakes[], struct food f[], int food_count, int gameover);
int read_and_draw_game_state();


// Основной цикл для хоств - то что было в в прошлой версии main'а
void host_game_loop(game_settings settings, snake_t* snakes[]) 
{
    mvprintw(0, 1,"HOST. Use arrows for control. Press 'F10' for EXIT");
    timeout(10);
    int key_pressed = 0;
    int gameover = 0;

    srand(time(NULL));
    initFood(food, MAX_FOOD_SIZE);
    putFood(food, FOOD_COUNT);

    const double SECONDS_PER_UPDATE = 0.1;
    clock_t last_update_time = clock();
    
    
    while( key_pressed != STOP_GAME && gameover != 1) 
    {
        //ОБРАБОТКА ВВОДА
        key_pressed = getch();
        if (key_pressed != ERR) 
        {
            // Хост управляется напрямую
            if (checkDirection(snakes[0], key_pressed)) 
            {
                changeDirection(snakes[0], key_pressed);
            }
        }
        // Игрок 2 управляется через файл
        int client_input_dir = read_client_input(); // Читаем направление 
if (client_input_dir != 0) 
{
    // Проверяем что это не разворот
    if ((client_input_dir == UP && snakes[1]->direction != DOWN) ||
        (client_input_dir == DOWN && snakes[1]->direction != UP) ||
        (client_input_dir == LEFT && snakes[1]->direction != RIGHT) ||
        (client_input_dir == RIGHT && snakes[1]->direction != LEFT))
    {
        snakes[1]->direction = client_input_dir; // Напрямую меняем направление
    }
}

        //ОБНОВЛЕНИЕ КАДРА
        clock_t current_time = clock();
        if ((double)(current_time - last_update_time) / CLOCKS_PER_SEC >= SECONDS_PER_UPDATE) 
        {
            // Обновляем всех
            for (int i = 0; i < settings.player_count; i++) 
            {
                update(snakes[i]);
                if (haveEat(snakes[i], food)) 
                {
                    addTail(snakes[i]);
                }
                refreshFood(food, FOOD_COUNT);
                repairSeed(food, FOOD_COUNT, snakes[i]);
            }
            last_update_time = current_time;
            
            // Проверяем столкновения
            for (int i = 0; i < settings.player_count; i++) 
            {
                for (int j = 0; j < settings.player_count; j++) 
                {
                    if (i == j) continue;
                    if (isCrushWithOtherSnake(snakes[i], snakes[j])) 
                    {
                        gameover = 1;
                        break;
                    }
                }
                if (gameover) break;
            }

            write_game_state(settings, snakes, food, FOOD_COUNT, gameover);
        }
    }
    
    // Если игра закончилась, пишем финальное состояние
    write_game_state(settings, snakes, food, FOOD_COUNT, 1);
    
    if (gameover) 
    {
        gameOverScreen(settings, snakes);
    }
}

// Основной игровой цикл для клиента
void client_game_loop() 
{
    timeout(50); // В 0 не ставить!
    int key_pressed = 0;
    
    GameState current_state = {0}; // Создаем структуру для хранения состояния

    while (key_pressed != STOP_GAME) 
    {
        key_pressed = getch();
        if (key_pressed != ERR) 
        {
            int dir = 0;
            if (key_pressed == 'w') dir = UP;
            if (key_pressed == 's') dir = DOWN;
            if (key_pressed == 'a') dir = LEFT;
            if (key_pressed == 'd') dir = RIGHT;
            if (dir != 0) 
            {
                write_client_input(dir);
            }
        }
        
        // Пытаемся прочитать новое состояние. Если получилось, обновляем current_state
        read_game_state_from_file(&current_state);

        // Рисуем текущее или старое(неважно) состояние
        draw_client_view(&current_state);

        if (current_state.gameover) 
        {
            break; // Гамовер - выходим - переделать, нужно показать результаты
        }
    }
}


int main()
{
    initscr();
    start_color();
    // Инициализация цветовых пар
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);

    // Настройки ncurses
    keypad(stdscr, TRUE);
    raw();
    noecho();
    curs_set(FALSE);

    game_settings settings;

    int is_client = startMenu(&settings);

    if (is_client) 
    {
        // Если мы клиент, запускаем простой цикл отрисовки и выходим
        client_game_loop();
    } else 
    {
        // Логика для Оффлайн и Хост
        clear();

        // Инициализация змеек согласно настройкам из меню
        snake_t* snakes[settings.player_count];
        for (int i = 0; i < settings.player_count; i++) 
        {
            int color_to_use = (i == 0) ? settings.player1_color : settings.player2_color;
            initSnake(snakes, START_TAIL_SIZE, 10 + i * 20, 10, i, color_to_use);
        }
        
        // В зависимости от режима запускаем нужный игровой цикл
        if (settings.mp_mode == MODE_NETWORK) 
        {
            host_game_loop(settings, snakes);
        } else 
        {
            // ОФФЛАЙН РЕЖИМ
            mvprintw(0, 1, "OFFLINE MODE. Press 'F10' for EXIT");
            timeout(0);
            int key_pressed = 0;
            int gameover = 0;
            
            srand(time(NULL));
            initFood(food, MAX_FOOD_SIZE);
            putFood(food, FOOD_COUNT);

            const double SECONDS_PER_UPDATE = 0.1;
            clock_t last_update_time = clock();
            game_start_time = time(NULL); // Засекаем время начала игры
            
            while( key_pressed != STOP_GAME && gameover != 1) 
            {
                // Обработка ввода
                key_pressed = getch();
                if (key_pressed != ERR) 
                {
                    if (settings.player_count == 1) 
                    {
                        // Универсальное управление для одного игрока
                        for (int j = 0; j < CONTROLS; j++) 
                        {
                            if ((key_pressed == default_controls[j].up && snakes[0]->direction != DOWN) ||
                                (key_pressed == default_controls[j].down && snakes[0]->direction != UP) ||
                                (key_pressed == default_controls[j].left && snakes[0]->direction != RIGHT) ||
                                (key_pressed == default_controls[j].right && snakes[0]->direction != LEFT)) 
                            {
                                if (key_pressed == default_controls[j].up) snakes[0]->direction = UP;
                                else if (key_pressed == default_controls[j].down) snakes[0]->direction = DOWN;
                                else if (key_pressed == default_controls[j].left) snakes[0]->direction = LEFT;
                                else if (key_pressed == default_controls[j].right) snakes[0]->direction = RIGHT;
                                break;
                            }
                        }
                    } else 
                    { // Раздельное управление для двух игроков
                        for (int i = 0; i < settings.player_count; i++) 
                        {
                            if (checkDirection(snakes[i], key_pressed)) 
                            {
                                changeDirection(snakes[i], key_pressed);
                            }
                        }
                    }
                }
                
                // Таймер для нового подрежима 
                if (settings.edible_tails_mode) 
                {
                    time_t current_time_for_timer = time(NULL);
                    int time_left = settings.game_time_seconds - (current_time_for_timer - game_start_time);
                    if (time_left < 0) time_left = 0;
                    
                    int max_x, max_y;
                    getmaxyx(stdscr, max_y, max_x);
                    mvprintw(0, max_x - 18, "Time left: %02d:%02d", time_left / 60, time_left % 60);

                    if (time_left <= 0) 
                    {
                        gameover = 1;
                    }
                }

                // Обновление кадра
                clock_t current_clock_time = clock();
                if ((double)(current_clock_time - last_update_time) / CLOCKS_PER_SEC >= SECONDS_PER_UPDATE) 
                {
                    // Логика бота
                    if (settings.mp_mode == MODE_VS_AI) 
                    {
                        struct food* target = find_closest_food(snakes[1], food, FOOD_COUNT);
                        ai_make_move(snakes[1], target);
                    }
                    
                    // Обновление состояния всех змеек
                    for (int i = 0; i < settings.player_count; i++) 
                    {
                        update(snakes[i]);
                        if (haveEat(snakes[i], food)) 
                        {
                            addTail(snakes[i]);
                        }
                        refreshFood(food, FOOD_COUNT);
                        repairSeed(food, FOOD_COUNT, snakes[i]);
                    }
                    last_update_time = current_clock_time;
                    
                    // ПРОВЕРКА СТОЛКНОВЕНИЙ В ЗАВИСИМОСТИ ОТ РЕЖИМА
                    if (settings.edible_tails_mode) 
                    {
                        handle_edible_collision(snakes[0], snakes[1], food, MAX_FOOD_SIZE);
                        handle_edible_collision(snakes[1], snakes[0], food, MAX_FOOD_SIZE);
                    } else {
                        // Старая логика проигрыша
                        for (int i = 0; i < settings.player_count; i++) 
                        {
                            if (settings.player_count == 1) 
                            {
                                if (isCrush(snakes[i])) { gameover = 1; break; }
                            } else {
                                for (int j = 0; j < settings.player_count; j++) 
                                {
                                    if (i == j) continue;
                                    if (isCrushWithOtherSnake(snakes[i], snakes[j])) { gameover = 1; break; }
                                }
                            }
                            if (gameover) break;
                        }
                    }
                }
            } // конец while
            
            // Показываем экран окончания игры, если она завершилась не по F10
            if (gameover) 
            {
                gameOverScreen(settings, snakes);
            }
        }
        
        // Освобождение памяти для всех режимов, кроме клиента
        for (int i = 0; i < settings.player_count; i++) 
        {
            free(snakes[i]->tail);
            free(snakes[i]);
        }
    }
    
    endwin();
    return 0;
}
