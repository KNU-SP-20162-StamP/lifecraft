#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "lifecraft.h"

pthread_mutex_t main_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t main_cond = PTHREAD_COND_INITIALIZER;
struct termios _ttystate;
int p1_av[3], p2_av[3];
int p1_row = 1, p1_col = 1, p2_row = 1, p2_col = 1;
int p1_alive_num = 0, p2_alive_num = 0;//몇개 살아있는지
int ready_status = 0;
int cell_null;
int entries;
int mode;
int key;

int main(){
	void (*menu_items[])(void*) = { NULL, menu_title, menu_ready, menu_go, menu_result, menu_help};
	menu (*prom_items[])(void*) = { NULL, prom_title, prom_ready, prom_go, prom_result, prom_help};

	cell_type board[R][C] = { CT_NONE, };
	pthread_t key_manager;
	menu state = TITLE, next;
	int r, c;

	srand(time(NULL));

	pthread_create(&key_manager, NULL, key_manage, NULL);
	pthread_mutex_lock(&main_lock);
	kio();
	signal(SIGINT, on_terminate);
	signal(SIGQUIT, on_terminate);

	while(1){
		DRESS_NEW;
		if(state == END) break;
		menu_items[state](board);
		printf("\n");
		pthread_cond_wait(&main_cond, &main_lock);
		if((next = prom_items[state](board)) != MENU_NONE) state = next;
	}
	pthread_mutex_unlock(&main_lock);
	pthread_cancel(key_manager);
	on_terminate(0);

	return 0;
}
void draw_option(char key, char *desc){
	dress(F_GREEN, "[%c]", key);
	DRESS_INIT;
	printf(" %s ", desc);
}
void menu_title(void *_b){
	int color_change=1;
        dress(F_CYAN, "" );
	printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	printf("□■      ■■■■■  ■■■■■■  ■■■■■■    ■■■■  ■■■■■      ■     ■■■■■■  ■■■■■■■□\n");
	printf("□■        ■    ■       ■        ■      ■    ■    ■ ■    ■          ■   □\n");
	printf("□■        ■    ■       ■       ■       ■    ■   ■   ■   ■          ■   □\n");
	printf("□■        ■    ■■■■    ■■■■■■  ■       ■■■■■   ■■■■■■■  ■■■■       ■   □\n");
	printf("□■        ■    ■       ■       ■       ■   ■   ■     ■  ■          ■   □\n");
	printf("□■        ■    ■       ■        ■      ■    ■  ■     ■  ■          ■   □\n");
	printf("□■■■■■  ■■■■■  ■       ■■■■■■    ■■■■  ■    ■  ■     ■  ■          ■   □\n");
	printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	DRESS_INIT;
	draw_option(K_MODE_1P, "One Player");
	draw_option(K_MODE_2P, "Two Players");
	draw_option(K_HELP, "Help");
	draw_option(K_QUIT, "Quit");
}
void menu_ready(void *_b){
	int p1_ok = ready_status & 0b01;
	int p2_ok = ready_status & 0b10;
	
	dress(CLEAR, "2");
	draw(_b, R, C);

	if(!p1_ok){
		dress(SET_POS, dress_pos(p1_row + 1, p1_col + 1));
		dress(B_GREEN, ":");
		DRESS_INIT;
	}
	if(mode == 2){
		if(!p2_ok){
			dress(SET_POS, dress_pos(p2_row + 1 , p2_col + 1));
			dress(B_RED, ":");
			DRESS_INIT;
		}
		// 2P 커서 위치 출력
	}
	dress(SET_POS, dress_pos(R+1, 0));

	if(mode == 1){
		draw_option(K_P1_apply, "OK");
		// 1P 선택
	}else{
		if(!p1_ok) draw_option(K_P1_apply, "1P OK");
		if(!p2_ok) draw_option(K_P2_apply, "2P OK");	// 2P 선택
	}
	draw_option(K_BACK, "Back");
}
void menu_go(void *_b){
	if(mode == 1)
		draw_cell_entries(read_cell_entries(), _b, C);
	run(_b, R, C);
	// run이 끝남
	printf("\n종료!\n계속하려면 아무 키나 누르십시오.\n");
}
void menu_result(void *_b){
        dress(F_CYAN, "" );
	printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	printf("□■■■■■   ■■■■■■   ■■■■   ■     ■  ■       ■■■■■■■□\n");
        printf("□■    ■  ■       ■       ■     ■  ■          ■   □\n");
        printf("□■    ■  ■       ■       ■     ■  ■          ■   □\n");
        printf("□■■■■■   ■■■■■■   ■■■■   ■     ■  ■          ■   □\n");
        printf("□■   ■   ■            ■  ■     ■  ■          ■   □\n");
        printf("□■    ■  ■            ■   ■   ■   ■          ■   □\n");
        printf("□■    ■  ■■■■■■   ■■■■     ■■■    ■■■■■■     ■   □\n");
        printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	DRESS_INIT;

                dress(F_GREEN, "");
                printf("1p : %d  ", p1_alive_num);
                dress(F_RED, "");
		if( mode == 2 )
                printf("2p : %d\n", p2_alive_num);
		else
		printf("AI : %d\n", p2_alive_num);
                DRESS_INIT;
		if(p1_alive_num > p2_alive_num)
		{	
                printf("The winner is...\n");
		dress(F_GREEN, "");
	        printf("■■■■■   ■         ■     ■     ■  ■■■■■■  ■■■■■     ■■   \n");
	        printf("■    ■  ■        ■ ■     ■   ■   ■       ■    ■   ■ ■   \n");
	        printf("■    ■  ■       ■   ■     ■ ■    ■       ■    ■  ■  ■   \n");
	        printf("■■■■■   ■      ■■■■■■■     ■     ■■■■■■  ■■■■       ■   \n");
	        printf("■       ■      ■     ■     ■     ■       ■   ■      ■   \n");
	        printf("■       ■      ■     ■     ■     ■       ■    ■     ■   \n");
	        printf("■       ■■■■■  ■     ■     ■     ■■■■■■  ■    ■  ■■■■■■■\n");
      		DRESS_INIT;
		}
		else if(p1_alive_num < p2_alive_num)
		{
                printf("The winner is...\n");        
	        dress(F_RED, "");
                if(mode ==2){
			printf("■■■■■   ■         ■     ■     ■  ■■■■■■  ■■■■■    ■■■■■■\n");
                	printf("■    ■  ■        ■ ■     ■   ■   ■       ■    ■        ■\n");
                	printf("■    ■  ■       ■   ■     ■ ■    ■       ■    ■        ■\n");
                	printf("■■■■■   ■      ■■■■■■■     ■     ■■■■■■  ■■■■     ■■■■■■\n");
                	printf("■       ■      ■     ■     ■     ■       ■   ■    ■     \n");
                	printf("■       ■      ■     ■     ■     ■       ■    ■   ■     \n");
               		printf("■       ■■■■■  ■     ■     ■     ■■■■■■  ■    ■   ■■■■■■\n");
        	        }
		else {
			printf("   ■     ■■■■■■■\n");
                        printf("  ■ ■       ■   \n");
                        printf(" ■   ■      ■   \n");
                        printf("■■■■■■■     ■   \n");
                        printf("■     ■     ■   \n");
                        printf("■     ■     ■   \n");
                        printf("■     ■  ■■■■■■■\n");

		}
		DRESS_INIT;
		}
		else
		{
		dress(F_YELLOW,"");
		printf("■■■■■■   ■■■■■      ■     ■       ■\n");
                printf("■     ■  ■    ■    ■ ■    ■       ■\n");
                printf("■     ■  ■    ■   ■   ■   ■   ■   ■\n");
                printf("■     ■  ■■■■■   ■■■■■■■  ■  ■ ■  ■\n");
                printf("■     ■  ■   ■   ■     ■  ■ ■   ■ ■\n");
                printf("■     ■  ■    ■  ■     ■  ■■     ■■\n");
                printf("■■■■■■   ■    ■  ■     ■  ■       ■\n");
		DRESS_INIT;
		}

	draw_option(K_RETRY, "Retry");
	draw_option(K_TITLE, "Back to Title");
}
void menu_help(void *_b)
{
	printf("조작키 설명\n");
	printf("1P\n");
	dress(F_GREEN,"");
        printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	printf("□      %12s%12s%12s%6s□\n", "r:BRUISER", "t:ASSASSIN", "y:COMMANDER", "u:OK");
        printf("□     w:상                                       □\n");
        printf("□a:좌 s:하 d:우                                  □\n");
	printf("□ %12s                                   □\n","x:DELETE");
    	printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	DRESS_INIT;
	printf("2P\n");
	dress(F_RED,"");
        printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	printf("□%-12s%-12s%-12s%-6s      □\n", "m:BRUISER", ",:ASSASSIN", ".:COMMANDER", "/:OK");
	printf("□                              %12s      □\n",";:DELETE");
        printf("□     					↑:상     □\n");
        printf("□		 	           ←:좌 ↓:하 →:우□\n");
	printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n");
	DRESS_INIT;
	dress(B_BLUE, "");
	printf("유닛 설명\n");
	DRESS_INIT;
		printf("+ : BRUISER   높은 방어력을 지닌 유닛(공격력: %d, 방어력(체력):%d)\n",CAT_BRU,CHP_BRU);
		printf("# : ASSASSIN  높은 공격력을 지닌 유닛(공격력: %d  방어력(체력):%d)\n",CAT_ASS,CHP_ASS);
		printf("@ : COMMANDER 적절한 공격력과 방어력을 지닌 유닛(공격력: %d  방어력(체력):%d)\n",CAT_COM,CHP_COM);
	dress(B_MAGENTA, "");
	printf("게임 설명\n");
	DRESS_INIT;
		printf("방향키를 이용해 유닛을 배치하세요. 유닛당 3마리까지 배치가 가능합니다.\n");
		dress(F_YELLOW, "");
		printf("<<유닛 생성 조건 설명>>\n");
		DRESS_INIT;
		printf("유닛당 점수는 BRUISER: %d, ASSASSIN: %d, COMMANDER: %d이며, 주위의 아군 유닛들의 점수 합을 계산, 합이 %d이면BRUISER, %d이면 ASSASSIN, %d이면 COMMANDER가 탄생합니다.\n",CSC_BRU,CSC_ASS,CSC_COM,CCO_BRU,CCO_ASS,CCO_COM);
		dress(F_YELLOW, "");
		printf("<<유닛 소멸 조건 설명>>\n");
		DRESS_INIT;
		printf("한 유닛 주위에 아군이 %d이상 있을경우 해당 유닛은 소멸합니다.\n 또한 주위 여덟칸의 적 유닛 공격력의 합이 유닛의 체력보다 크거나 같으면 해당 유닛은 전사합니다.\n",DIE_POINT);

		printf("\n▶여기서 주위란□□□\n");
		printf("              □@□\n");
		printf("              □□□ (네모 8칸)을 의미합니다\n");
	        printf("▶예시(생성) #□□\n");
                printf("            □■□\n");
                printf("            □+□ @:%d점 + #:%d점 = %d점따라서 ■에 # 생성\n",CSC_ASS,CSC_BRU,CCO_ASS);
                printf("▶예시(소멸) #□□\n");
                printf("            □■□\n");
                printf("            @□□ 주위에 유닛수가 %d 이상이므로 ■에 위치한 유닛은 소멸합니다.\n",DIE_POINT);

		printf("▶예시(전사)");
		dress(F_GREEN, "");
		printf(" #");
		DRESS_INIT;
		printf("□□\n");
                printf("            □");
		dress(F_RED,"");
		printf("@");
		DRESS_INIT;
		printf("□\n");
                printf("            □");
		dress(F_GREEN, "");
		printf("+");
		DRESS_INIT;
		printf("□ ");
		printf("(1p의 총 공격력: %d + %d = %d) > ( 2p유닛의 체력 %d )이므로 2p의 유닛은 전사합니다.\n",CAT_ASS,CAT_BRU,(CAT_ASS + CAT_BRU),CHP_COM);
		dress(F_YELLOW, "");
		printf("전략적인 배치를 통해 승리를 쟁취하십시오\n\n");
		DRESS_INIT;
	printf("계속하려면 아무 키나 누르십시오.\n");
}

menu prom_title(void *_b){
	switch(key){
		case K_MODE_1P: init_board(_b); return mode = 1, READY;
		case K_MODE_2P: init_board(_b); return mode = 2, READY;
		case K_HELP: return HELP;
		case K_QUIT: return END;
		default: break;
	}
	return MENU_NONE;
}

menu prom_ready(void *_b){
	BOARD(_b);
	int P1_PLACE[3] = { CT_1_BRUISER, CT_1_ASSASSIN, CT_1_COMMANDER };
	int P2_PLACE[3] = { CT_2_BRUISER, CT_2_ASSASSIN, CT_2_COMMANDER };
	int d_actor = 0b000, d_w, p2, i;
	/* d_actor 플래그: 0bIXP
		I	그 위치에서 추가
		X	그 위치에서 삭제
		P	1이면 2P, 0이면 1P
	*/
	cell_type *c;
	
	switch(key){
		case K_P1_apply: if(mode == 1) return GO; if((ready_status |= 0b01) == 0b11){
			ready_status = 0; return GO;
		} break;
		case K_P1_up: p1_row = MAX(p1_row-1, 1); break;
		case K_P1_down: p1_row = MIN(p1_row+1, R-2); break;
		case K_P1_left: p1_col = MAX(p1_col-1, 1); break;
		case K_P1_right: p1_col = MIN(p1_col+1, C-2); break;
		case K_P1_bruiser: c = &board[p1_row][p1_col]; d_actor = 0b100; d_w = CT_1_BRUISER; break;
		case K_P1_assassin: c = &board[p1_row][p1_col]; d_actor = 0b100; d_w = CT_1_ASSASSIN; break;
		case K_P1_commander: c = &board[p1_row][p1_col]; d_actor = 0b100; d_w = CT_1_COMMANDER; break;
		case K_P1_delete: c = &board[p1_row][p1_col]; d_actor = 0b010; break;
		case K_BACK: return TITLE;
		default: break;
	}
	if(mode == 2) switch(key){
		case K_P2_apply: if((ready_status |= 0b10) == 0b11){
			ready_status = 0; return GO;
		} break;
		case K_P2_up: p2_row = MAX(p2_row-1, 1); break;
		case K_P2_down: p2_row = MIN(p2_row+1, R-2); break;
		case K_P2_left: p2_col = MAX(p2_col-1, 1); break;
		case K_P2_right: p2_col = MIN(p2_col+1, C-2); break;
		case K_P2_bruiser: c = &board[p2_row][p2_col]; d_actor = 0b101; d_w = CT_2_BRUISER; break;
		case K_P2_assassin: c = &board[p2_row][p2_col]; d_actor = 0b101; d_w = CT_2_ASSASSIN; break;
		case K_P2_commander: c = &board[p2_row][p2_col]; d_actor = 0b101; d_w = CT_2_COMMANDER; break;
		case K_P2_delete: c = &board[p2_row][p2_col]; d_actor = 0b011; break;
		default: break;
	}
	p2 = d_actor % 2;
	if((!p2 && ready_status & 0b01) || (p2 && ready_status & 0b10)){
		// 준비 완료한 상태
	}else if(d_actor & 0b010) for(i=0; i<3; i++){
		if(!p2 && *c == P1_PLACE[i]){
			p1_av[i]++;
			*c = CT_NONE;
		}
		if(p2 && *c == P2_PLACE[i]){
			p2_av[i]++;
			*c = CT_NONE;
		}
	}else if(d_actor & 0b100) for(i=0; i<3; i++){
		if(!p2 && !*c && d_w == P1_PLACE[i] && p1_av[i] > 0){
			p1_av[i]--;
			*c = P1_PLACE[i];
		}
		if(p2 && !*c && d_w == P2_PLACE[i] && p2_av[i] > 0){
			p2_av[i]--;
			*c = P2_PLACE[i];
		}
	}
	return MENU_NONE;
}
menu prom_go(void *_b){
	return RESULT;
}
menu prom_result(void *_b){

	switch(key){

		case K_RETRY: init_board(_b); return READY;
		case K_TITLE: return TITLE;
		default: break;
	}
	return MENU_NONE;
}
menu prom_help(void *_b)
{
	 return TITLE;
}

void* key_manage(void*arg){
	while(1){
		key = getchar() & 0xFF;
		if(key == 27){
			key |= (getchar() & 0xFF) << 8;
			if(key == 0x5B1B){
				key |= (getchar() & 0xFF) << 16;
			}
		}
		pthread_cond_signal(&main_cond);
		// printf("Key: %d %d %d\n", key >> 16, key >> 8 & 0xFF, key & 0xFF);
	}
	return NULL;
}

void kio(){
	struct termios ttystate;
	int ff;

	tcgetattr(0, &_ttystate);
	tcgetattr(0, &ttystate);
	ttystate.c_lflag &= ~ICANON;
	ttystate.c_lflag &= ~ECHO;
	ttystate.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &ttystate);
}

void restore(){
	tcsetattr(0, TCSANOW, &_ttystate);
}

void on_terminate(int sig){
	struct termios ttystate;

	restore();
	exit(sig);
}

void dress(d_code code, char *msg, ...){
	char buf[BUFSIZ] = { 27, '[', };
	va_list args;
	int ign_msg = 0;

	switch(code){
		case INIT: strcat(buf, "0m"); break;

		case CLEAR: strcat(buf, msg); strcat(buf, "J"); ign_msg = 1; break;
		case CLEAR_LINE: strcat(buf, msg); strcat(buf, "K"); ign_msg = 1; break;
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
char* dress_pos(int r, int c){
	static char res[BUFSIZ];

	sprintf(res, "%d;%d", r, c);
	return res;
}

void init_board(void *_b){
	cell_type (*board)[C] = _b;
	int i, j;

	for(i = 0; i < R; i++){
		for(j = 0; j < C; j++){
			if(!(i*j*(i-R+1)*(j-C+1)))
				board[i][j] = CT_WALL;
			else
				board[i][j] = CT_NONE;
		}
	}
	p1_row = R/2; p1_col = C/4;
	p2_row = R/2; p2_col = C/4*3;
	p1_av[0] = p2_av[0] = P_bruiser_score;
	p1_av[1] = p2_av[1] = P_assassin_score;
	p1_av[2] = p2_av[2] = P_commander_score;
}
void run(void *_b, int rows, int cols){
	pthread_t board_manager;
	bm_args args = { rows, cols, _b };

	pthread_create(&board_manager, NULL, board_manage, &args);
	pthread_join(board_manager, NULL);
}
void* board_manage(void *_args){
	bm_args *args = (bm_args*)_args;
	cell_type (*board)[args->bma_c] = args->bma_b;
	int gen = 0;

	while(1){

		draw(board, args->bma_r, args->bma_c);
		printf("Gen #%d\n", gen++);

		p1_alive_num = 0;
		p2_alive_num = 0;

		evolve(board, args->bma_r, args->bma_c);
		
		dress(CLEAR_LINE, "2");
		dress(F_GREEN, "");
		printf("1p : %d  ", p1_alive_num);
		dress(F_RED, "");
		if(mode == 2) printf("2p : %d\n", p2_alive_num);
		else printf("AI : %d\n", p2_alive_num);
		DRESS_INIT;
			
		// 한 쪽이 전멸하거나
		if(p1_alive_num == 0 || p2_alive_num == 0)
			break;

		if(gen > MAX_GEN)	// 제한 시간이 지나는 경우 while문 탈출
			break;

		usleep(RUN_CYCLE);
	}
	return _args;
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
		case CT_WALL : dress(B_WHITE, ""); printf(" "); DRESS_INIT; break;
		case CT_1_BRUISER : dress(F_GREEN, ""); printf("+"); DRESS_INIT; break;
		case CT_1_ASSASSIN : dress(F_GREEN, ""); printf("#"); DRESS_INIT; break;
		case CT_1_COMMANDER : dress(F_GREEN, ""); printf("@"); DRESS_INIT; break;
		case CT_2_BRUISER : dress(F_RED, ""); printf("+"); DRESS_INIT; break;
		case CT_2_ASSASSIN : dress(F_RED, ""); printf("#"); DRESS_INIT; break;
		case CT_2_COMMANDER : dress(F_RED, ""); printf("@"); DRESS_INIT; break;
		default: printf("Unhandled cell type: %d\n", cell);
	}
}
void evolve(void *_b, int rows, int cols){
	cell_type (*board)[cols] = _b;
	cell_type new[rows][cols];
	cell_type temp;

	int cell_at[] = { 0 , 0, CAT_BRU, CAT_ASS, CAT_COM, CAT_BRU, CAT_ASS, CAT_COM};
	int cell_hp[] = { 0 , 0, CHP_BRU, CHP_ASS, CHP_COM, CHP_BRU, CHP_ASS, CHP_COM};
	int cell_sc[] = { 0 , 0, -CSC_BRU, -CSC_ASS, -CSC_COM, CSC_BRU, CSC_ASS, CSC_COM};
	int cell_ow[] = { 0 , 0, -1, -1, -1, 1, 1, 1};
	int r, c, n;
	int x, y;
	int as, sur, t, hp, sur_n;

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
			sur_n = 0;
			hp = cell_hp[board[r][c]];

			if(t == -1)	//현재 보드에 남아있는 유닛 수 계산
				p1_alive_num++;
			else if(t == 1)
				p2_alive_num++;
			
			// 주변 유닛 수 계산 (음수는 플레이어1, 양수는 플레이어2)
			for(y=r-1; y<=r+1; y++){
				for(x=c-1; x<=c+1; x++){
					sur = (int)board[(y+rows)%rows][(x+cols)%cols];
					n += cell_sc[sur];
					if(t == cell_ow[sur]) sur_n++;
					if(t + cell_ow[sur] == 0) as += cell_at[sur];
				}
			}

			// 생사 결정
			if(board[r][c] == CT_NONE){
				new[r][c] = get_evolved_cell(n);
			}else if(hp <= as || sur_n >= DIE_POINT){
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
		default:	return CT_NONE;
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
	int i;

	temp = entries_list[rand() % entries];
	for(i = 0; temp[i].x != -1; i++){
		board[temp[i].x][temp[i].y] = temp[i].type;
	}
}
