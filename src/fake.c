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

/***
 * 辞書構造体
 */
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
	char dic_buff[DIC_SIZE];
}	dic;


/***
 * プリミティブ ワード
 *
 */

struct PROC {
	char* name;
	bool (*func)();
};

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
		puts("stack underflow.\n");
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
	dic.entry_step = 1;
	return true;
}

bool dic_entry_close(char *str)
{
	dic.entry_step = 0;
	return true;
}

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

bool check_rest(struct DIC *dic, int len)
{
	int size = DIC_SIZE;
	char *top = dic->dic_buff;
	char *cur = dic->append_pos;
	int rest = size - (cur - top);

	return (rest > len);
}

bool append_word(struct DIC *dic, char* str)
{
	int len = strlen(str);
	if (!check_rest(dic, len)) {
		put_str("words buffer overfllow.\n");
		return false;
	}
	else {
		strncpy(dic->append_pos, str, len);
		dic->prev_word = dic->append_pos;
		dic->append_pos += len;
		*dic->append_pos++ = '\0';
	}
	return true;
}

bool append_name(struct DIC *dic, char* str)
{
	if (!check_rest(dic, sizeof(char*))) {
		put_str("words buffer overfllow.\n");
		return false;
	}

	char *last_word = dic->last_word;
	char **prev_word = (char**)dic->append_pos;
	*prev_word = last_word;
	dic->append_pos += sizeof(*prev_word);

	if (!append_word(dic, str)) {
		// 登録失敗
		return false;
	}
	return true;
}

bool append_body(struct DIC *dic, char* str)
{
	if (!append_word(dic, str)) {
		// 登録失敗
		return false;
	}
	// 本体は語のあとはスペースにする
	append_word(dic, " \0");

	return true;
}

// [prev_word]
// [this_word.name] <-- curr prev_word
// [null]
// [this_word."1st-word"]
// ['sp']
// [this_word."2nd-word"]
// ['sp']
// ...
// ['sp']
// [this_word."last-word"]
// ['sp']
// [this_word.";"]
// ['sp']
// [null] <-- next append_pos;


void dic_entry(struct DIC* dic, char* str)
{
	//if バッファ溢れ虫

	switch (dic->entry_step) {
	case 0:	//
		break;

	case 1:	// name
		append_name(dic, str);
		dic->entry_step++;
		break;

	case 2:// body
		append_body(dic, str);
		if (strcmp(str, ";") == 0) {
			dic->entry_step = 0;
		}
		break;

	case 3: // end
		break;
	}
}

void dump_dic(struct DIC *dic)
{
	char* p = dic->dic_buff;
	while (p < dic->append_pos) {
		if (*p < ' ')
			putc('\n', stdout);
		else
			putc(*p, stdout);
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

void proc(struct DIC* dic, char *str)
{
	char* tok = str;
	while ((tok = strtok(str, " \t\n")) != NULL) {
		if (dic->entry_step == 0)
			eval(tok);
		else
			dic_entry(dic, tok);

		str = NULL;
	}
}


void init()
{
	put_str("++ Fake Fotrh ++");

	// stack
	sp = stack;

	// dictionary
	dic.entry_step = 0;
	dic.prev_word = dic.dic_buff;
	dic.last_word = dic.dic_buff;
	dic.append_pos = dic.dic_buff;
}

int main(void)
{
	init();

	char *test = ": 1 2 + . ;";
	proc(&dic, test);
	dump_dic(&dic);

	char *str;
	while ((str = input()) != NULL) {
		proc(&dic, str);
//		while ((str = strtok(str, " \t\n")) != NULL) {
//			if (dic.entry_step == 0)
//				eval(str);
//			else
//				dic_entry(str);
//			str = NULL;
//		}
		put_str("ok ");
	}
	return EXIT_SUCCESS;
}
