/*
 * fake.h
 *
 *  Created on: 2021/03/28
 *      Author: tenshi
 */

#ifndef FAKE_H_
#define FAKE_H_

#define STACK_SIZE 16
#define INPUT_MAX 100
#define DIC_SIZE 1000

//unsigned stack[STACK_SIZE];
//unsigned *sp;
//char input_buff[INPUT_MAX];

#define S0  stack[0];
#define S1  stack[1];
#define S2  stack[2];
#define S3  stack[3];
#define S4  stack[4];
#define S5  stack[5];
#define S6  stack[6];
#define S7  stack[7];
#define S8  stack[8];
#define S9  stack[9];

struct DIC {

	int entry_step;
	// 辞書登録フラグ
	// 0: 実行モード, 辞書登録状態でない.
	//    ':' を検出したら, 登録開始. フラグは 1.
	// 1: 名前登録モード, name 登録中. 語を名前として登録する("NAME" + NULL).
	//    名前の登録ができたら フラグは 2.
	// 2: 本体登録モード, word 登録中. ';' まで繰り返し("WORD" + SPACE+NULL).
	//    ';' を検出したら,登録終了処理. フラグは 0.
	//

	char *append_pos;
	char *prev_word;
	char *last_word;
	char *curr_word;
	char dic_buff[DIC_SIZE];
};


struct PROC {
	char* name;
	bool (*func)();
};


bool is_num(char *str);
void push_int(int num);
void push_num(char *str);
int *pop_int();int print(char *str);
int print_int(int num);
int print_hex(int num);
int print_addr(char* addr);
int print(char*);
void println(char*);

bool dic_entry_open();
bool dic_entry_close();

bool (*lookup_prim(char *str))();
char* lookup_word(char *str);

void eval(char *str);
bool check_rest(int len);
bool append_word(char*);
bool append_addr(char *addr, bool inc_pos);
bool append_name(char*);
bool append_body(char*);

void dic_entry(char* str);
void dump_dic();
char *input();
void proc(char *str);


#endif /* FAKE_H_ */
