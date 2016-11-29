#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#define R 20
#define C 80

// 점수
#define CSC_BRU 1
#define CSC_ASS 2
#define CSC_COM 3
// 방어력(체력)
#define CHP_BRU 3
#define CHP_ASS 1
#define CHP_COM 2
// 공격력
#define CAT_BRU 1
#define CAT_ASS 3
#define CAT_COM 2
// 탄생 비용
#define CCO_BRU 2
#define CCO_ASS 3
#define CCO_COM 4

typedef enum _d_code{
	INIT,
	// 특수
	CLEAR, SET_POS, NEXT_LINE,
	// 글자 속성
	BOLD, UNDERLINE,
	// 글자 색상
	F_BLACK, F_RED, F_GREEN, F_YELLOW, F_BLUE, F_MAGENTA, F_CYAN, F_WHITE,
	// 배경 색상
	B_BLACK, B_RED, B_GREEN, B_YELLOW, B_BLUE, B_MAGENTA, B_CYAN, B_WHITE
} d_code;

typedef enum _cell_type{
	CT_NONE,
	CT_WALL,
	CT_1_BRUISER,
	CT_1_ASSASSIN,
	CT_1_COMMANDER,
	CT_2_BRUISER,
	CT_2_ASSASSIN,
	CT_2_COMMANDER
} cell_type;

void dress(d_code, char*, ...);
void evolve(void*, int, int);
void run(void*, int, int);
void draw(void*, int, int);
void draw_cell(cell_type);

int main(){
	cell_type board[R][C] = { CT_NONE, };
	int r, c;

	srand(time(NULL));

	dress(CLEAR, "2");
	// 보드 초기화
	for(r=0; r<R; r++){
		for(c=0; c<C; c++){
			board[r][c] = rand() < RAND_MAX / 10 ? ALIVE : NONE;
		}
	}
	run(board, R, C);

	return 0;
}
void dress(d_code code, char *msg, ...){
	char buf[BUFSIZ] = { 27, '[', };
	va_list args;
	int ign_msg = 0;

	switch(code){
		case INIT: strcat(buf, "0m"); break;

		case CLEAR: strcat(buf, msg); strcat(buf, "J"); ign_msg = 1; break;
		case SET_POS: strcat(buf, msg); strcat(buf, "H"); ign_msg = 1; break;
		case NEXT_LINE: strcat(buf, msg); strcat(buf, "E"); ign_msg = 1; break;

		case BOLD: strcat(buf, "1m"); break;
		case UNDERLINE: strcat(buf, "4m"); break;

		case F_BLACK: strcat(buf, "30m"); break;
		case F_RED: strcat(buf, "31m"); break;
		case F_GREEN: strcat(buf, "32m"); break;
		case F_YELLOW: strcat(buf, "33m"); break;
		case F_BLUE: strcat(buf, "34m"); break;
		case F_MAGENTA: strcat(buf, "35m"); break;
		case F_CYAN: strcat(buf, "36m"); break;
		case F_WHITE: strcat(buf, "37m"); break;

		case B_BLACK: strcat(buf, "40m"); break;
		case B_RED: strcat(buf, "41m"); break;
		case B_GREEN: strcat(buf, "42m"); break;
		case B_YELLOW: strcat(buf, "43m"); break;
		case B_BLUE: strcat(buf, "44m"); break;
		case B_MAGENTA: strcat(buf, "45m"); break;
		case B_CYAN: strcat(buf, "46m"); break;
		case B_WHITE: strcat(buf, "47m"); break;

		default: printf("Unhandled dress code: %d\n", code);
	}
	if(!ign_msg){
		va_start(args, msg);
		vsprintf(buf + strlen(buf), msg, args);
		va_end(args);
	}
	fputs(buf, stdout);
}
void run(void *_b, int rows, int cols){
	cell_type (*board)[cols] = _b;
	int gen = 0;

	while(1){
		draw(board, rows, cols);
		printf("Gen #%d\n", gen++);
		evolve(board, rows, cols);
		usleep(200000);
	}
}
void draw(void *_b, int rows, int cols){
	cell_type (*board)[cols] = _b;
	int r, c;

	dress(SET_POS, "1;1");
	for(r=0; r<rows; r++){
		for(c=0; c<cols; c++){
			draw_cell(board[r][c]);
		}
		dress(NEXT_LINE, "1");
	}
	fflush(stdout);
}
void draw_cell(cell_type cell){
	switch(cell){
		case NONE: printf(" "); break;
		case ALIVE: dress(UNDERLINE, ""); dress(B_GREEN, " "); dress(INIT, ""); break;
		default: printf("Unhandled cell type: %d\n", cell);
	}
}
void evolve(void *_b, int rows, int cols){
	cell_type (*board)[cols] = _b;
	cell_type new[rows][cols];
	int r, c, n;
	int x, y;

	for(r=0; r<rows; r++){
		for(c=0; c<cols; c++){
			n = 0;
			// 주변 유닛 수 계산
			for(y=r-1; y<=r+1; y++){
				for(x=c-1; x<=c+1; x++){
					if(board[(y+rows)%rows][(x+cols)%cols]) n++;
				}
			}
			if(board[r][c]) n--;
			// 생사 결정
			if(board[r][c]){
				new[r][c] = (n == 2 || n == 3) ? ALIVE : NONE;
			}else{
				new[r][c] = (n == 3) ? ALIVE : NONE;
			}
		}
	}
	for(r=0; r<rows; r++){
		for(c=0; c<cols; c++){
			board[r][c] = new[r][c];
		}
	}
}
