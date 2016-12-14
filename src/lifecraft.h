#define R 33
#define C 77
#define RUN_CYCLE 200000

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
//죽을 조건
#define DIE_POINT 2
// 키
#define K_QUIT 'q'
#define K_MODE_1P '1'
#define K_MODE_2P '2'
#define K_RETRY 'r'
#define K_TITLE 't'

#define K_P1_up 'w'	//w
#define K_P1_down 's'	//s
#define K_P1_left 'a'	//a
#define K_P1_right 'd'	//d
#define K_P1_assassin 'r'	//r
#define K_P1_bruiser 't'	//t
#define K_P1_commander 'y'	//y
#define K_P1_apply 'u'
#define K_P1_delete 'x'

#define K_P2_up 0x415B1B	//방향키 up
#define K_P2_down 0x425B1B	//방향키 down
#define K_P2_right 0x435B1B	//방향키 right
#define K_P2_left 0x445B1B	//방향키 left
#define K_P2_assassin 'm'	//m
#define K_P2_bruiser ','	//,
#define K_P2_commander '.'	//.
#define K_P2_apply '/'
#define K_P2_delete ';'

#define P_assassin_score 3
#define P_bruiser_score 3
#define P_commander_score 3

#define ENTRIES "./entries_list"
#define DRESS_INIT dress(INIT,"")
#define DRESS_NEW {dress(CLEAR,"2");dress(SET_POS,"1;1");}
#define BOARD(b) cell_type (*board)[C] = b;

#define MAX(a, b) ((a>=b)?a:b)
#define MIN(a, b) ((a<b)?a:b)
#define oops(m, x) {perror(m);exit(x);}

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

typedef enum _menu{
	MENU_NONE,
	TITLE, READY, GO, RESULT, END
} menu;

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

typedef struct _bm_args{
	int bma_r, bma_c;
	void *bma_b;
} bm_args;

void kio();
void restore();
void* key_manage(void*);
void on_terminate(int);

void init_board(void*);

void dress(d_code, char*, ...);
char* dress_pos(int, int);

void draw(void*, int, int);
void draw_cell(cell_type);

void evolve(void*, int, int);
cell_type get_evolved_cell(int);

void* smalloc(int);
cell_entry** read_cell_entries(void);
void draw_cell_entries(cell_entry**, void*, int);

void run(void*, int, int);
void* board_manage(void*);

void menu_title(void*);
void menu_ready(void*);
void menu_go(void*);
void menu_result(void*);
menu prom_title(void*);
menu prom_ready(void*);
menu prom_go(void*);
menu prom_result(void*);
