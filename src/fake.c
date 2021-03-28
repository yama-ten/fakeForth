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
	char *curr_word;
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

int print(char *str)
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

int print_int(int num)
{
	int len = 0;
	char buff[20];
	char *p = buff+sizeof(buff);
	*--p = '\0';
	do {
		if (p == buff) {
			print("too long num.\n");
			break;
		}
		++len;
		int dig = num % 10;
		*--p = '0'+dig;
	} while (num /= 10);

	print(p);

	return len;
}

int print_hex(int num)
{
	int len = 0;
	char buff[20];
	char *p = buff+sizeof(buff);
	*--p = '\0';
	do {
		if (p == buff) {
			print("too long num.\n");
			break;
		}
		++len;
		int dig = num % 16;
		*--p = (dig<10) ?  '0'+dig : 'A'+dig;
	} while (num /= 16);

	print("$");
	print(p);

	return len;
}

int print_addr(char* addr)
{
	unsigned long num = (unsigned long)addr;

	int len = 0;
	char buff[20];
	char *p = buff+sizeof(buff);
	*--p = '\0';
	int addr_len = sizeof(char*);
	for (int i=0; i<addr_len; i++) {
		++len;
		unsigned dig = num & 0x0f;
		*--p = (dig<10) ?  '0'+dig : 'A'-10+dig;
		num >>= 4;
	}
	print("$");
	print(p);

	/*
	int len = 0;
	char buff[20];
	char *p = buff+sizeof(buff);
	*--p = '\0';
	do {
		if (p == buff) {
			print("too long num.\n");
			break;
		}
		++len;
		unsigned dig = num & 0x0f;
		*--p = (dig<10) ?  '0'+dig : 'A'-10+dig;
	} while (num >>= 4);

	print("$");
	print(p);
	*/

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
	print_int(num);
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


bool (*lookup_prim(char *str))()
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

char* lookup_dict(struct DIC* dic, char *str)
{
	char* p = dic->last_word;
	while (p > dic->dic_buff) {
		if (stricmp(str, p)==0)
			return p;

		char **prev_word = (char**)(p - sizeof(char*));
		p = *prev_word;
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
		pf = lookup_prim(str);
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
		print("words buffer overfllow.\n");
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

bool append_addr(struct DIC* dic, char *addr, bool inc_pos)
{
	if (!check_rest(dic, sizeof(char*))) {
		print("words buffer overfllow.\n");
		return false;
	}

	char **prev_word = (char**)dic->append_pos;
	*prev_word = addr;
	if (inc_pos)
		dic->append_pos += sizeof(char *);

	return true;
}

bool append_name(struct DIC *dic, char* str)
{
	if (!append_addr(dic, dic->last_word, true))
		return false;

	dic->last_word =
	dic->curr_word = dic->append_pos;
	if (!append_word(dic, str))
		return false;		// 登録失敗

	return true;
}

bool append_body(struct DIC *dic, char* str)
{
	if (!append_word(dic, str)) {
		// 登録失敗
		return false;
	}
	// 本体は語のあとはスペースにする
	dic->append_pos--;
	append_word(dic, " \0");
	dic->append_pos--;
//	append_addr(dic, dic->curr_word, false);
	dic->last_word = dic->curr_word;

	return true;
}

// [prev_word_addr] <-----------------------+
// [this_word.name] <-- curr word			|
// [null]									|
// [this_word."1st-word"]					|
// ['sp']									|
// [this_word."2nd-word"]					|
// ['sp']									|
// ...										|
// ['sp']									|
// [this_word."last-word"]					|
// ['sp']									|
// [this_word.";"]							|
// ['sp'] 									|
// [null] <-- next append_pos;				|
// [prev-word-addr] ------------------------+

void dic_entry(struct DIC* dic, char* str)
{
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
			*(dic->append_pos-1) = '\0'; // 最後のスペースをNULLにする.
			dic->prev_word = dic->curr_word;
			append_addr(dic, dic->prev_word, false);

			dic->entry_step = 0;
		}
		break;

	case 3: // end
		break;
	}
}

void dump_dic(struct DIC *dic)
{
	print("dic:"); print_addr((char*)dic); print("\n");
	print("dic.append_pos:"); print_addr(dic->append_pos); print("\n");
	print("dic.prev_word:"); print_addr(dic->prev_word); print("\n");
	print("dic.last_word:"); print_addr(dic->last_word); print("\n");
	print("dic.curr_word:"); print_addr(dic->curr_word); print("\n");

	char* p = dic->dic_buff;
	//char* addr;
	while (p < dic->append_pos) {
		// prev_word addr;
		print("\nprev_word:");
		char **addr = (char**)p;
		print_addr((char*)*addr);
		print("\n");
		p += sizeof(char*);

		//word_addr = p;
		print("\nword_addr:");
		print_addr(p);
		print(": ");
		p += print(p);
		p++;
		print("\n");

		//words text
		p += print(p);
		p++;
		print("\n");
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
	print("> ");
	char *p = fgets(input_buff, sizeof(input_buff), stdin);

	return p;
}


void proc(struct DIC* dic, char *str)
{
	char* tok;
	char buff[INPUT_MAX];
	strncpy(buff, str, sizeof(buff));

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
	print("++ Fake Fotrh ++");

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

	char a[] = "addr test";
	print_addr(a);
	char test1[] = ": test1 1 2 + . ; \n";
	char test2[] = ": test2 2 3 * . ; \n";
	char test3[] = ": test3 3 4 - . ; \n";
	proc(&dic, test1);
	dump_dic(&dic);
	proc(&dic, test2);
	dump_dic(&dic);
	proc(&dic, test3);
	dump_dic(&dic);
	char test[] = "test\n";
	proc(&dic, test);

	char *str;
	while ((str = input()) != NULL) {
		proc(&dic, str);
		print("ok ");
	}
	return EXIT_SUCCESS;
}
