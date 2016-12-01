#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#define R 33
#define C 77

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

#define ENTRIES "entries_list"

#define oops(m, x) {perror(m); exit(x);}

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

typedef struct _cell_entry{
	int x, y;
	cell_type type;
} cell_entry;

void init_board(void*);
void dress(d_code, char*, ...);

void draw(void*, int, int);
void draw_cell(cell_type);

void evolve(void*, int, int);
cell_type get_evolved_cell(int);

void* smalloc(int);
cell_entry** read_cell_entries(void);
void draw_cell_entries(cell_entry**, void*, int);

void run(void*, int, int);

int entries;

int main(){
	cell_type board[R][C] = { CT_NONE, };
	int r, c;

	srand(time(NULL));

	dress(CLEAR, "2");
	init_board(board);

	draw_cell_entries(read_cell_entries(), board, C);
	run(board, R, C);

	return 0;
}

void init_board(void *_b){
	cell_type (*board)[C] = _b;
	int i, j;

	for(i = 0; i < R; i++){
		for(j = 0; j < C; j++){
			if(!(i*j*(i-R+1)*(j-C+1)))
				board[i][j] = CT_WALL;
		}
	}
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
		case CT_NONE : printf(" "); break;
		case CT_WALL : dress(B_WHITE, ""); printf(" "); dress(INIT, ""); break;
		case CT_1_BRUISER : dress(F_GREEN, ""); printf("+"); dress(INIT, ""); break;
		case CT_1_ASSASSIN : dress(F_GREEN, ""); printf("#"); dress(INIT, ""); break;
		case CT_1_COMMANDER : dress(F_GREEN, ""); printf("@"); dress(INIT, ""); break;
		case CT_2_BRUISER : dress(F_RED, ""); printf("+"); dress(INIT, ""); break;
		case CT_2_ASSASSIN : dress(F_RED, ""); printf("#"); dress(INIT, ""); break;
		case CT_2_COMMANDER : dress(F_RED, ""); printf("@"); dress(INIT, ""); break;
		default: printf("Unhandled cell type: %d\n", cell);
	}
}
void evolve(void *_b, int rows, int cols){
	cell_type (*board)[cols] = _b;
	cell_type new[rows][cols];
	int cell_at[] = { 0 , 0, CAT_BRU, CAT_ASS, CAT_COM, CAT_BRU, CAT_ASS, CAT_COM};
	int cell_hp[] = { 0 , 0, CHP_BRU, CHP_ASS, CHP_COM, CHP_BRU, CHP_ASS, CHP_COM};
	int cell_sc[] = { 0 , 0, -CSC_BRU, -CSC_ASS, -CSC_COM, CSC_BRU, CSC_ASS, CSC_COM};
	int cell_ow[] = { 0 , 0, -1, -1, -1, 1, 1, 1};
	int r, c, n;
	int x, y;
	int as, sur, t, hp;

	for(r=0; r<rows; r++){
		for(c=0; c<cols; c++){
			if(board[r][c] == CT_WALL){
				new[r][c] = CT_WALL;
				continue;
			}

			t = cell_ow[board[r][c]];
			n = 0;
			as = 0;
			sur = 0;
			hp = cell_hp[board[r][c]];

			// 주변 유닛 수 계산 (음수는 플레이어1, 양수는 플레이어2)
			for(y=r-1; y<=r+1; y++){
				for(x=c-1; x<=c+1; x++){
					sur = (int)board[(y+rows)%rows][(x+cols)%cols];
					n += cell_sc[sur];
					if(t + cell_ow[sur] == 0) as += cell_at[sur];
				}
			}


			// 생사 결정
			if(board[r][c] == CT_NONE){
				new[r][c] = get_evolved_cell(n);
			}else if(hp <= as){
				new[r][c] = CT_NONE;
			}else{
				new[r][c] = board[r][c];
			}
		}
	}
	//원래 보드에 이번 턴의 생사 결정이 완료된 임시 보드를 복사
	for(r=0; r<rows; r++){
		for(c=0; c<cols; c++){
			board[r][c] = new[r][c];
		}
	}
}

cell_type get_evolved_cell(int n){
	switch (n) {
		case -CCO_BRU:	return CT_1_BRUISER;
		case -CCO_ASS:	return CT_1_ASSASSIN;
		case -CCO_COM:	return CT_1_COMMANDER;
		case CCO_BRU:	return CT_2_BRUISER;
		case CCO_ASS:	return CT_2_ASSASSIN;
		case CCO_COM:	return CT_2_COMMANDER;
		default:		return CT_NONE;
	}
}

void* smalloc(int size){
	void *res;

	if((res = malloc(size)) == NULL)
		oops("malloc error", 1);
	return res;
}

cell_entry** read_cell_entries(void){
	cell_entry **res;
	FILE *fp;
	int i, j;
	int entry_size;

	fp = fopen(ENTRIES, "r");

	fscanf(fp, "%d", &entries);

	res = smalloc(sizeof(cell_entry*) * entries);

	for(i = 0; i < entries; i++){
		fscanf(fp, "%d", &entry_size);
		res[i] = smalloc(sizeof(cell_entry) * entry_size);
		for(j = 0; j < entry_size; j++){
			fscanf(fp, "%d,%d,%d", &res[i][j].x, &res[i][j].y, &res[i][j].type);
		}
	}

	return res;
}

void draw_cell_entries(cell_entry** entries_list, void *_b, int cols){
	cell_type (*board)[cols] = _b;
	cell_entry* temp;
	int i, j;

	for(i = 0; i < entries; i++){
		temp = entries_list[i];
		for(j = 0; temp[j].x != -1; j++){
			board[temp[j].x][temp[j].y] = temp[j].type;
		}
	}
}
