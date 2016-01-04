#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include <fcntl.h>

#define PANEL         1
#define ERROR_MSG     2
#define SNAKE_COLOR   3
#define FOOD_COLOR    5

#define x		0
#define y		1

#define POINTS		5

/* === global variables === */

int snakesize = 7;
int food_x = 0;
int food_y = 0;
int dead = 0;
int points = 0;
int exitc = 0;
int speed = 80000;

/* === functions === */

int draw_snake(int **body);
void clear_snake(int **body);
int move_snake(int c, int ***body);
int check_position(int **body);

void pos_food(int **body);
void draw_food( void );
void eat_food(int ***body);

void draw_info( void );
void readkey( int *c );
void space( int _x, int _y, int l);
int quit(int ***body, int state);

int game ( int *c ) {
	char title[100];

	int **body;
	int i = 0;
	int dir = *c;
	int state = 0;

	snakesize = 7;
	food_x = 0;
	food_y = 0;
	points = 0;
	*c = KEY_RIGHT;

	clear();
	body = (int **) malloc(sizeof(int *)*snakesize);
	for ( i = 0; i != snakesize; i++ ) {
		body[i] = (int *) malloc(sizeof(int)*2);
	}

	attron(COLOR_PAIR(PANEL));
	for ( i = 0; i != COLS; i++ ) {
		mvaddch(0, i, ' ');
	}
	sprintf(title," ..:: Snakie v 1.0 ::.. ");
	mvaddstr(0, COLS/2-strlen(title)/2, title);
	attroff(COLOR_PAIR(PANEL));
	refresh();

	for ( i = 0; i!= snakesize; i++ ) {
		body[i][x] = (COLS/2)-i;
		body[i][y] = (LINES/2);
	}

	
	pos_food(body);
	draw_food();
	draw_snake(body);

	while ( 1 ) { 
		clear_snake(body);
		switch(*c) {
			case KEY_UP:
				dir = 'U';
				break;

			case KEY_DOWN:
				dir = 'D';
				break;

			case KEY_RIGHT:
				dir = 'R';
				break;

			case KEY_LEFT:
				dir = 'L';
				break;
		}

		if ( *c == 'q' ) { 
			if ( confirm() == 1 ) {
				state = 0; 
				break; 
			}
		}

		move_snake(dir, &body);
		if ( (state = check_position(body)) != 0 ) {
			dead = 1;
			break;
		}
		eat_food(&body);
		draw_food();
		draw_info();
		draw_snake(body);
		usleep( dir == 'L' || dir == 'R' ? speed*0.75 : speed );
	}
	int ret = quit(&body, state);
	return ret;
}

/* === main === */
int main( void ) {

	pthread_t tgame;

	int rexit = 0;
	int c = KEY_RIGHT;
	initscr();
	curs_set(0);
	start_color();
	cbreak();
	keypad(stdscr, 1);
	noecho(); 

	int _x = (COLS-32)/2;
	int _y = (LINES-14)/2;
	mvprintw(_y,_x,    "  *****            *   *          ");
	mvprintw(++_y,_x,  " *                 *              ");
	mvprintw(++_y,_x,  "  ****  ***   **   * * *  ***     ");
	mvprintw(++_y,_x,  "      * *  * *  *  **  * * *      ");
	mvprintw(++_y,_x,  " ****** *  *  **** * * *  ****    ");

	_y += 2;
	mvprintw(++_y,_x,"  press any key to start a game   ");
	getch();

	init_pair(PANEL, COLOR_BLACK, COLOR_WHITE);
	init_pair(ERROR_MSG, COLOR_WHITE, COLOR_RED);
	init_pair(SNAKE_COLOR, COLOR_GREEN, COLOR_GREEN);
	init_pair(FOOD_COLOR, COLOR_YELLOW, COLOR_BLACK);
	
	pthread_create(&tgame, NULL, (void *) game, &c);
	while ( rexit != 1 ) {
		readkey(&c);

		pthread_join(tgame, NULL);
		switch(c) {
			case 'r':
				break;
			case 27:
			case 'q':
				rexit = 1;
				break;
			default: 
				continue;
		}
	}
	endwin();
	return exitc;
}

int confirm() {
	char *quitmsg[3];
	quitmsg[0] = "+--------------------+";
	quitmsg[1] = "| Really quit? [y/N] |";
	quitmsg[2] = "+--------------------+";

	mvaddstr(LINES/2-1, COLS/2-strlen(quitmsg[0])/2, quitmsg[0]);
	mvaddstr(LINES/2, COLS/2-strlen(quitmsg[1])/2, quitmsg[1]);
	mvaddstr(LINES/2+1, COLS/2-strlen(quitmsg[2])/2, quitmsg[2]);

	char answer = getch();

	if ( answer == 'N' || answer == 'n' ) {
		return 0; 
	} else {
		return 1;
	}
}

int draw_snake(int **body) {
	int i = 0;

	for ( i = 0; i != snakesize; i++ ) {
		attron(COLOR_PAIR(SNAKE_COLOR));
		mvaddch(body[i][y], body[i][x], '#');
		attroff(COLOR_PAIR(SNAKE_COLOR));
	}
	refresh();
}

void clear_snake(int **body) {
	mvaddch(body[snakesize-1][y],body[snakesize-1][x], ' ');
}

int check_position(int **body) {
	int i = 0,
	    j = 0;

	for ( i = 0; i != snakesize-1; i++ ) {
		for ( j = i+1; j != snakesize; j++ ) {
			if ( body[i][x] == body[j][x] && body[i][y] == body[j][y] ){
				return 1;
			}
		}
	}

	if ( body[0][x] == COLS || body[0][x] == -1 || body[0][y] == LINES || body[0][y] == 0 ) {
		return 2;
	}

	
	return 0;
}

int move_snake(int c, int ***body) {
	int i = 0;

	switch(c) {
		case 'R':
			if ( (*body)[0][x] == (*body)[1][x]-1 ) {
				c = 'L';
			}
			break;

		case 'L':
			if ( (*body)[0][x] == (*body)[1][x]+1 ) {
				c = 'R';
			}
			break;

		case 'U':
			if ( (*body)[0][y] == (*body)[1][y]+1 ) {
				c = 'D';
			}
			break;

		case 'D':
			if ( (*body)[0][y] == (*body)[1][y]-1 ) {
				c =  'U';
			}
	}

	free((*body)[snakesize-1]);
	for ( i = (snakesize-1); i != 0; i-- ) {
        	(*body)[i] = (*body)[i-1]; 
	}

	(*body)[0] = (int *) malloc(sizeof(int)*2);
	switch(c) {
		case 'R':
			(*body)[0][x] = (*body)[1][x]+1;
			(*body)[0][y] = (*body)[1][y];
			break;
		case 'L':
			(*body)[0][x] = (*body)[1][x]-1;
			(*body)[0][y] = (*body)[1][y];
			break;
		case 'U':
			(*body)[0][y] = (*body)[1][y]-1;
			(*body)[0][x] = (*body)[1][x];
			break;
		case 'D':
			(*body)[0][y] = (*body)[1][y]+1;
			(*body)[0][x] = (*body)[1][x];
			break;
	}
	return 0;
}

void pos_food(int **body) {
	int state = 0;
	int i = 0;

	while ( !state ) {
		state = 1;
		srand(time(NULL));
		food_x = (rand()%COLS);
		srand(time(NULL)%22);
		food_y = (rand()%(LINES-1))+1;

		// check where is the snake, we can not put food into the snake 
		for ( i = 0; i != snakesize; i++ ) {
			if ( body[i][x] == food_x && body[i][y] == food_y ) {
				state = 0;
				break;
			}
		}
	}
				
}

void draw_food( void ) {
	attron(COLOR_PAIR(FOOD_COLOR));
	mvaddch(food_y, food_x, '@');
	attroff(COLOR_PAIR(FOOD_COLOR));
}

void eat_food(int ***body) {

	if ( (*body)[0][x] == food_x && (*body)[0][y] == food_y ) {
		/* fun begins */
		pos_food(*body);
		snakesize++;
		points += POINTS;

		if ( (points % 30) == 0 ) 
			speed -= 200;

		*body = realloc(*body, sizeof(int *)*snakesize);
		(*body)[snakesize-1] = (int *) malloc(sizeof(int)*2);

		if ( (*body)[snakesize-2][x] < (*body)[snakesize-3][x] )  {
			(*body)[snakesize-1][x] = (*body)[snakesize-2][x]-1;
			(*body)[snakesize-1][y] = (*body)[snakesize-2][y];
			return;
		}
		
		if ( (*body)[snakesize-2][x] > (*body)[snakesize-3][x] )  {
			(*body)[snakesize-1][x] = (*body)[snakesize-2][x]+1;
			(*body)[snakesize-1][y] = (*body)[snakesize-2][y];
			return;
		}
		
		if ( (*body)[snakesize-2][y] < (*body)[snakesize-3][y] )  {
			(*body)[snakesize-1][y] = (*body)[snakesize-2][y]-1;
			(*body)[snakesize-1][x] = (*body)[snakesize-2][x];
			return;
		}

		if ( (*body)[snakesize-2][y] > (*body)[snakesize-3][y] )  {
			(*body)[snakesize-1][y] = (*body)[snakesize-2][y]+1;
			(*body)[snakesize-1][x] = (*body)[snakesize-2][x];
			return;
		}

	}
}

void readkey( int *c ) {
	dead = 0;
	while ( dead != 1 && ((*c = getch())!='q') );
}

void draw_info( void ) {
	char msg[20];
	sprintf(msg, "Points: %d", points);
	attron(COLOR_PAIR(PANEL));
	attron(A_BOLD);
	mvaddstr(0,1,msg);
	attroff(A_BOLD);
	attroff(COLOR_PAIR(PANEL));
}

void space(int _x, int _y, int l) {
	int i = 0;
	for (i = 0; i != l; i++) {
		mvaddch(_y, _x+i, ' ');
	}
}

int quit(int ***body, int state) {
	int i = 0;
	
	char *messages[2];
	messages[0] = " .o0 Oouch, I've bitten myself! :'( ";
	messages[1] = " .o0 Ouch, a border, that hurts! O:( ";
	
	for ( i = 0; i!=snakesize; i++ ) {
		free((void *)(*body)[i]);
	}
	free((void *)(*body));

	switch(state) {
		case 1:
		case 2:
			attron(COLOR_PAIR(ERROR_MSG));
			int lng = strlen(messages[state-1]);
			space((COLS-lng)/2, (LINES/2)-1, lng);
			mvaddstr(LINES/2,(COLS-lng)/2,messages[state-1]);
			space((COLS-lng)/2, (LINES/2)+1, lng);
			space((COLS-lng)/2, (LINES/2)+2, lng);
			space((COLS-lng)/2, (LINES/2)+3, lng);
			space((COLS-lng)/2, (LINES/2)+4, lng);
			mvaddstr((LINES/2)+2,(COLS-27)/2, "  press ESC ESC to exit   ");
			mvaddstr((LINES/2)+3,(COLS-27)/2, " or press r to play again ");
			attroff(COLOR_PAIR(ERROR_MSG));
			refresh();
			return state;
	}
}			

