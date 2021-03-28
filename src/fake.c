/*
 ============================================================================
 Name        : fake.c
 Author      : tenshi
 Version     :
 Copyright   : yama-ten
 Description : Hello World in C, Ansi-style
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


#define STACK_SIZE 16
#define INPUT_MAX 100
#define DIC_SIZE 1000

int stack[STACK_SIZE];
int *sp;
char input_buff[INPUT_MAX];
char dict_buff[DIC_SIZE];

// 辞書登録フラグ
// 0: 実行モード, 辞書登録状態でない.
//    ':' を検出したら, 登録開始. フラグは 1.
// 1: 名前登録モード, name 登録中. 語を名前として登録する("NAME" + NULL).
//    名前の登録ができたら フラグは 2.
// 2: 本体登録モード, word 登録中. ';' まで繰り返し("WORD" + SPACE+NULL).
//    ';' を検出したら,登録終了処理. フラグは 0.
//
int dic_entry = 0;


bool is_num(char *str)
{
	for (char *p=str; *p; p++) {
		if (!isdigit(*p))
			return false;
	}
	return true;
}

void push_int(int num)
{
	*sp++ = num;
}

void push_num(char *str)
{
	int num = atoi(str);
	push_int(num);
}


int *pop_int()
{
	if (sp > stack)  {
		return --sp;
	} else {
		puts("stack underflow.");
		return NULL;
	}
}

int put_str(char *str)
{
	int len = 0;
	if (str != NULL) {
		char *p = str;
		while (str!=NULL && *p) {
			++len;
			putc(*p++, stdout);
		}
	}
	return len;
}

int put_int(int num)
{
	int len = 0;
	char buff[20];
	char *p = buff+sizeof(buff);
	*--p = '\0';
	do {
		if (p == buff) {
			put_str("too long num.\n");
			break;
		}
		++len;
		int dig = num % 10;
		*--p = '0'+dig;
	} while (num /= 10);

	put_str(p);

	return len;
}

bool bye()
{
	exit(0);
}

bool dup()
{
	int num;
	int *t;
	if ((t = pop_int()) != NULL) num = *t; else return false;
	push_int(num);
	push_int(num);

	return true;
}

bool swap()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return false;
	if ((t = pop_int()) != NULL) x = *t; else return false;
	push_int(y);
	push_int(x);

	return true;
}

bool dot()	// print
{
	int num;
	int *t;
	if ((t = pop_int()) != NULL) num = *t; else return false;
	put_int(num);
	putc(' ', stdout);

	return true;
}

bool int_add()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return false;
	if ((t = pop_int()) != NULL) x = *t; else return false;
	push_int(x + y);

	return true;
}

bool int_sub()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return false;
	if ((t = pop_int()) != NULL) x = *t; else return false;
	push_int(x - y);

	return true;
}

bool int_mul()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return false;
	if ((t = pop_int()) != NULL) x = *t; else return false;
	push_int(x * y);

	return true;
}

bool int_div()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return false;
	if ((t = pop_int()) != NULL) x = *t; else return false;
	push_int(x / y);

	return true;
}

bool int_mod()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return false;
	if ((t = pop_int()) != NULL) x = *t; else return false;
	push_int(x % y);

	return true;
}


bool dic_entry_open(char *str)
{
	dic_entry = 1;
}

bool dic_entry_close(char *str)
{
	dic_entry = 0;
}

/***
 * プリミティブ ワード
 *
 */

struct PROC {
	char* name;
	bool (*func)();
};

struct PROC prim[] = {
		{ "bye",	bye },
		{ ":",		dic_entry_open },
		{ ";", 		dic_entry_close },
		{ ".",		dot },
		{ "dup",	dup },
		{ "swap",	swap },
		{ "+", 		int_add },
		{ "-", 		int_sub },
		{ "*", 		int_mul },
		{ "/", 		int_div },
		{ "%", 		int_mod },
		{ NULL, NULL }
};

bool (*lookup(char *str))()
{
	struct PROC *p = prim;
	while(p->name != NULL) {
		if (stricmp(str, p->name) == 0) {
			return p->func;
		}
		p++;
	}
	return NULL;
}

/***
 * ワードの評価
 */
void eval(char *str)
{
	bool (*pf)();

	if (is_num(str))
		push_num(str);
	else {
		pf = lookup(str);
		if (pf != NULL)
			(*pf)();
		else {
			char msg[80];
			strcpy(msg, "ERROR:'");
			strcat(msg, str);
			strcat(msg, "' is not defined.\n");
			puts(msg);
		}
	}
}



/***
 * １行入力
 *
 * コンソール入力をデバッグするには、GDB コマンドファイルに「.gdbinit」を指定。
 * プロジェクトのルートに .gdbinit を置き、`set new-console on` と書き込む。
 * プロジェクトのデバッグの構成で 「Use external console for inferior (open a new console window for input/output)」 をチェック。
 */
char *input()
{
	put_str("> ");
	char *p = fgets(input_buff, sizeof(input_buff), stdin);

	return p;
}


int main(void)
{
	sp = stack;
	dic_entry = false;

	put_str("++ Fake Fotrh ++");

	char *str;
	while ((str = input()) != NULL) {
		while ((str = strtok(str, " \t\n")) != NULL) {
			eval(str);
			str = NULL;
		}
		put_str("ok ");
	}
	return EXIT_SUCCESS;
}
