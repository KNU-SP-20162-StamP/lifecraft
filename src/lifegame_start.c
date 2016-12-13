#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
#include<unistd.h>
#include<time.h>

#include<termio.h>
#define R 33
#define C 77
/*
typedef struct
{
int up;
int down;
int left;
int right;
int assasin;
int bruiser;
int commander;
}key_table;

key_table p1;// = {p1_up, p1_down, p1_left, p1_right, p1_assasin, p1_bruiser, p1_commander};
key_table p2;// = {p2_up, p2_down, p2_left, p2_right, p2_assasin, P2_bruiser, p2_commander};
*/

int p1_up			=	105;	//w
int p1_down		=	107;	//s
int p1_left		=	106;	//a
int p1_right		=	108;	//d
int p1_assasin		=	49;	//1
int p1_bruiser		=	50;	//2
int p1_commander	=	51;	//3

int p1_assasin_score	=	3;
int p1_bruiser_score 	=	3;
int p1_commander_score 	=	3;

int p2_up			=	0x415B1B;	//방향키 up
int p2_down		=	0x425B1B;	//방향키 down
int p2_left		=	0x445B1B;	//방향키 left
int p2_right		=	0x435B1B;	//방향키 right
int p2_assasin		=	56;	//8
int p2_bruiser		=	57;	//9
int p2_commander	=	48;	//0


int p2_assasin_score    =	3;
int p2_bruiser_score    =	3;
int p2_commander_score  =	3;

typedef enum _cell_type{
	CT_NONE,
	CT_WALL,
	CT_1_ASSASIN,
	CT_1_BRUISER,
	CT_1_COMMANDER,
	CT_2_ASSASIN,
	CT_2_BRUISER,
	CT_2_COMMANDER
} cell_type;

cell_type board[R][C];
void set_cr_noecho_mode(void);
void operation_key();
void board_reset();

void main()
{
	operation_key();
}


void board_reset()	//board에 none값 넣는함수
{
	int r, c;

	for(r=0; r<R; r++)
	{
		for(c=0; c<C; c++)
			board[r][c] = CT_NONE;
	}
}
void operation_key()
{

int key = 0;
int set_unit = 0;
static int p1_row = 0, p1_col = 0 , p2_row = 0, p2_col = 0;

set_cr_noecho_mode();
board_reset();
while((p1_assasin_score != 0) || (p1_bruiser_score != 0) ||  (p1_commander_score != 0) ||  (p2_assasin_score != 0) ||  (p2_bruiser_score != 0) ||  (p2_commander_score != 0))
	{
	key = getchar();
	printf("%d",key);

	if(key ==  p1_up)
	{
		if(p1_row != 0)
		p1_row -=1;
	}

	else if( key ==  p1_down)
	{
		if(p1_row != R-1)
		p1_row += 1;
	}
	else if (key == p1_left)
	{
		if(p1_col != 0)
		p1_col-=1;
	}
	else if(key == p1_right)
	{
		if(p1_col != C-1)
		p1_col +=1;
	}

	else if(key ==  p2_up)
	{
                if(p2_row != 0)
	        p2_row -= 1;
	}

	else if(key == p2_down)
	{
	        if(p2_row != R-1)
	        p2_row += 1;
	}

	else if(key ==  p2_left)
	{
                if(p2_col != 0)
		p2_col -=1;
	}

	else if (key == p2_right)
	{
                if(p2_col != C-1)
		p2_col +=1;
	}
	else if (key == p1_assasin)
	{
		if((p1_assasin_score!=0)&&(board[p1_row][p1_col] == NULL ))
		{
			p1_assasin_score--;
			board[p1_row][p1_col] = CT_1_ASSASIN;
		}
	}
	else if (key ==  p1_bruiser)
	{
	        if((p1_bruiser_score!=0)&&(board[p1_row][p1_col] == NULL ))
                {
                         p1_bruiser_score--;
                         board[p1_row][p1_col] = CT_1_BRUISER;
                }
        }
	else if (key ==  p1_commander)
	{
		if((p1_commander_score!=0) && (board[p1_row][p1_col] == NULL))
		{
			p1_commander_score--;
			board[p1_row][p1_col] = CT_1_COMMANDER;
		}
	}
	else if (key ==  p2_assasin)
	{
		if((p2_assasin_score!=0) &&(board[p2_row][p2_col] ==NULL))
			{
			p2_assasin_score--;
			board[p2_row][p2_col] = CT_2_ASSASIN;
			}
	}
	else if (key ==  p2_bruiser)
	{
		if((p2_bruiser_score!=0) &&(board[p2_row][p2_col] == NULL))
		{
			p2_bruiser_score--;
			board[p2_row][p2_col] = CT_2_BRUISER;
		}
	}
	else if (key ==  p2_commander)
	{
		if((p2_commander_score!=0) && (board[p2_row][p2_col] == NULL))
		{
			p2_commander_score--;
			board[p2_row][p2_col] = CT_2_COMMANDER;
		}
	}
    }
}
void set_cr_noecho_mode(void)
{
        struct termios ttystate;

        tcgetattr( 0, &ttystate);
        ttystate.c_lflag &= ~ICANON;
        ttystate.c_lflag &= ~ECHO;
        ttystate.c_cc[VMIN] = 1;
        tcsetattr( 0, TCSANOW, &ttystate);
}
