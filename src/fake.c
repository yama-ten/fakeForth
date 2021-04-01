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
#include "fake.h"

unsigned stack[STACK_SIZE];
unsigned *sp;

char heap[HEAP_SIZE];
char* hp;

char input_buff[INPUT_MAX];
int base = 10;

char* do_stack[DO_STACK];
char** do_sp;

int if_step = 0;
int if_nest = 0;

int do_step = 0;
int do_nest = 0;
int *do_i = 0;
int *do_j = 0;

/***
 * 辞書構造体
 */
struct DIC dic;

// 文字列数値変換
// 16進対応, 頭が "0x" または "$" なら16進
// その他は atoi() で 10進.
unsigned long long atou64(char *str)
{
	char *p = str;

	// 頭が "0x" か "$" なら16進
	if (*p == '$') p++;
	else if (!strncmp(str, "0x", 2)) p += 2;
	else
		return atoi(p);

	unsigned long long x = 0;
	while (*p) {
		char c = toupper(*p++);
		if (!isxdigit(c))
			return 0;
		x <<= 4;
		x |= (c <= '9') ? c-'0' : c-'A'+10;
	}

	return x;
}


bool is_num(char *str)
{
	for (char *p=str; *p; p++) {
		if (!isdigit(*p))
			return false;
	}
	return true;
}

bool is_hex(char *str)
{
	if (*str == '$')
		str++;
	else if (!strnicmp(str, "0x", 2))
		str += 2;
	else
		return false;

	for (char *p=str; *p; p++) {
		if (!isxdigit(*p))
			return false;
	}
	return true;
}

void push_int(int num)
{
	*sp++ = (unsigned)num;
}

void push_num(char *str)
{
	unsigned num = atou64(str);
	push_int(num);
}

void push_hex(char *str)
{
	unsigned num =  atou64(str);
	push_int(num);
}

void push_addr(void *addr)
{
	*(uintptr_t*)sp = (uintptr_t)addr;
	*sp += 2;
}

int pop_int()
{
	if (sp > stack)  {
		// 返すのは int のポインタ
		return *(int*)--sp;
	} else {
		print("stack underflow.\n");
		return 0;
	}
}

uintptr_t pop_addr()
{
	if (sp-1 > stack)  {
		sp -= 2;
		// 返すのは unsigned long long のポインタ
		return (uintptr_t)sp;
	} else {
		print("stack underflow.\n");
		return (uintptr_t)NULL;
	}
}

int print(char *str)
{
	int len = 0;
	if (str != NULL) {
		char *p = str;
		while (*p) {
			++len;
			putc(*p++, stdout);
		}
	}
	return len;
}

int print_int(int num)
{
	bool sign = (num<0);
	if (sign)
		num *= -1;

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

	if (sign)
		*--p = '-';

	print(p);

	return len;
}

char* numtos(int num, int _base)
{
	bool sign = (num<0);
	if (sign)
		num *= -1;

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
		int dig = num % _base;
		*--p = (dig<10) ? '0'+dig : 'A'+dig-10;
	} while (num /= _base);

	if (sign)
		*--p = '-';
	return p;
}

int print_num(int num, int _base)
{
	char buff[20];
	strcpy(buff, numtos(num, _base));
	int len = print(buff);
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
	unsigned long long num = (unsigned long long)addr;

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

	return len;
}

/***
 * 組込みワード
 *
 */

void bye()
{
	exit(0);
}

void nop()
{
}

void dup()
{
	int n1 = pop_int();
	push_int(n1);
	push_int(n1);
}

void swap()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int(n1);
	push_int(n2);
}

void drop()
{
	pop_int();
}

void over()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int(n2);
	push_int(n1);
	push_int(n2);
}

void rot()
{
	int n1 = pop_int();
	int n2 = pop_int();
	int n3 = pop_int();
	push_int(n2);
	push_int(n1);
	push_int(n3);
}

void depth()
{
	int n = sp-stack;
	push_int(n);
}

void dot()	// print
{
	int n1 = pop_int();
	print_num(n1, base);
	putc(' ', stdout);
}

void int_add()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int(n2 + n1);
}

void int_sub()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int(n2 - n1);
}

void int_mul()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int(n2 * n1);
}

void int_div()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int(n2 / n1);
}

void int_mod()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int(n2 % n1);
}


void dic_entry_open()
{
	dic.entry_step = 1;
}

void dic_entry_close()
{
	dic.entry_step = 0;
}

void dic_dump()
{
	dump_dic(&dic);
}

void cr()
{
	print("\n");
}

void words()
{
	char* p = dic.last_word;
	while (p > dic.dic_buff) {
		println(p);
		char **prev_word = (char**)(p - sizeof(char*));
		p = *prev_word;
	}
}

void minus()
{
	int n1 = pop_int();
	push_int(-n1);
}

void int_abs()
{
	int n1 = pop_int();
	if (n1 < 0)
		push_int(-n1);
	else
		push_int(n1);
}

void int_max()
{
	int n1 = pop_int();
	int n2 = pop_int();
	if (n2 > n1)
		push_int(n2);
	else
		push_int(n1);
}

void int_min()
{
	int n1 = pop_int();
	int n2 = pop_int();
	if (n2 < n1)
		push_int(n2);
	else
		push_int(n1);
}

void times_div()
{
	int n1 = pop_int();
	int n2 = pop_int();
	int n3 = pop_int();
	push_int( n3 * n2 / n1 );
}

void times_div_mod()
{
	int n1 = pop_int();
	int n2 = pop_int();
	int n3 = pop_int();
	n3 *= n2;
	push_int( n3 % n1 );
	push_int( n3 / n1 );
}

void div_mod()
{
	int n1 = pop_int();
	int n2 = pop_int();
	push_int( n2 % n1 );
	push_int( n2 / n1 );
}

void fetch()
{
	unsigned* mem = *(unsigned**)pop_addr();
	unsigned num = *mem;
	push_int((int)num);
}

void store()
{
	unsigned* mem = *(unsigned**)pop_addr();
	int num = pop_int();
	*(int*)mem = num;
}

void question()
{
	fetch();
	dot();
}

void int_base()
{
	base = pop_int();
}

void base_bin()
{
	base = 2;
}

void base_oct()
{
	base = 8;
}

void base_dec()
{
	base = 10;
}

void base_hex()
{
	base = 16;
}

void dump_stack()
{
	for (unsigned* p=sp-1; p >= stack; p--) {
		uintptr_t a = (uintptr_t)p;
		char buff[12];

//		/* print 64bit h */
//		unsigned h = (unsigned)(a>>32);
//		strcpy(buff, numtos(h,16));
//		for (int n = strlen(buff); n<8; n++)
//			putc('0', stdout);
//			print_num(h, 16);
//			putc('_', stdout);

		// print l
		unsigned l = (unsigned)(a & 0xffffffff);
		strcpy(buff, numtos(l,16));
		int n = 8-(int)strlen(buff);
		while(n--) putc('0', stdout);
		print(buff);

		print(":");
		print_num(*p , base);
		print("\n");
	}
}

void if_exec()
{
	int n1 = pop_int();

	if (n1 != 0)
		if_step = 1;	// ELSE か THEN まで実行するモード
	else
		if_step = 3;	// ELSE までスキップして THEN まで実行するモード
}

void if_then()
{
	if_step = 0;	// IF 　終了
}

void if_else()
{
	if_step = 2;	// THEN までスキップするモード
}


/** DO *****
 */
void do_exec()
{
	int n1 = pop_int();
	int n2 = pop_int();

	if (n2 - n1 > 0) {
		do_step = 1;	// loop まで実行するモード
		*do_sp++ = pc;
		push_int(n2);
		push_int(n1);
		do_j = do_i;
		do_i = (int*)sp-1; // == cnt_val
	}
	else {
	//	pop_int();	// cnt
	//	pop_int();	// end
		do_step = 3;	// loop までスキップするモード
	}
}

void do_loop()
{
	do_step = 0;	// loop 　終了
}

void int_i()
{
	push_int(*do_i);
}

void int_j()
{
	push_int(*do_j);
}


struct PROC prim[] = {
		{ "bye",	bye },
		{ "if",		if_exec },
		{ "then",	if_then },
		{ "else",	if_else },

		{ "do",		do_exec },
		{ "loop",	do_loop },
		{ "i",		int_i },
		{ "j",		int_j },

		{ "var",	nop },
		{ "const",	nop },

		{ "base",	int_base},
		{ "hex",	base_hex },
		{ "decimal",base_dec },
		{ "dec",	base_dec },
		{ "oct",	base_oct },
		{ "bin",	base_bin },
		{ "binary",	base_bin },

		{ ":",		dic_entry_open },
		{ ";", 		dic_entry_close },

		{ "?dic", 	dic_dump},
		{ "stack", 	dump_stack},
		{ "words", 	words},

		{ "minus",	minus },
		{ "abs", 	int_abs },
		{ "max", 	int_max },
		{ "min", 	int_min },

		{ "CR",		cr },
		{ ".",		dot },
		{ "@",		fetch },
		{ "!",		store },
		{ "?",		question },

		{ "dup",	dup },
		{ "swap",	swap },
		{ "drop",	drop },
		{ "over",	over },
		{ "rot",	rot },

		{ "depth",	depth },

		{ "+", 		int_add },
		{ "-", 		int_sub },
		{ "*", 		int_mul },
		{ "/", 		int_div },
		{ "%", 		int_mod },
		{ "/mod", 	div_mod },
		{ "/%", 	div_mod },
		{ "mod", 	int_mod },
		{ "*/",		times_div },
		{ "*/mod",	times_div_mod },
		{ "*/%",	times_div_mod },

		{ NULL, NULL }
};


void (*lookup_prim(char *str))()
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

void create_var(char** str)
{
	//char *next = *str;
	char buff[20];
	int len;
	char* name = get_token(str, &len);
	strncpy(buff, name, len);
	buff[len] = '\0';

	append_name(name);
	dic.entry_step++;

	uintptr_t var_addr = (uintptr_t)hp;
	*(unsigned*)var_addr = 0;

	append_body("VAR $");
	dic.append_pos--;
//	char buff[20];
	strcpy(buff, numtos(var_addr, 16));
	append_body(buff);
	append_body("$0");
	append_body(";");

	if (sp > stack)
		*hp = pop_int();

	hp += sizeof(int);
}

void create_const(char ** str)
{
	//char *next = *str;
	char buff[20];
	int len;
	char* name = get_token(str, &len);
	strncpy(buff, name, len);
	buff[len] = '\0';

	int num;
	if (sp > stack)
		num = pop_int();
	else
		print("ERRROR:no value on stack.\n");

	append_name(name);
	dic.entry_step++;

	append_body("CONST");
//	char buff[20];
	strcpy(buff, numtos(num, 10));
	append_body(buff);
	append_body(";");

	hp += sizeof(int);
}

void see(char** str)
{
	char *name;
	char buff[20];
	int len;
	name = get_token(str, &len);
	strncpy(buff, name, len);
	buff[len] = '\0';

	char *wd = lookup_word(buff);
	if (wd) {
		print(wd);
	}
	else {
		print("ERROR:'");
		print(name);
		print("' is not defined.");
	}
}


void double_quat(char** str)
{
	quat_flag = '"';
	char buff[20];
	int len;
	char *p = get_token(str, &len);
//	while (*p && *p!=quat_flag)
//		put
	strncpy(buff, buff, len);
	buff[len] = '\0';
	print(buff);
}

struct PROC prim_2[] = {
		{ "variable",	create_var },
		{ "constant",	create_const },
		{ "see",		see },
		{ ".\"",	double_quat },

		{ NULL, NULL }
};

void (*lookup_prim_2(char *str))()
{
	struct PROC *p = prim_2;
	while(p->name != NULL) {
		if (stricmp(str, p->name) == 0) {
			return p->func;
		}
		p++;
	}
	return NULL;
}

char* lookup_word(char *str)
{
	char* p = dic.last_word;
	while (p > dic.dic_buff) {
		if (stricmp(str, p)==0) {
			p += strlen(p);
			p += 1;
			return p;
		}
		char **prev_word = (char**)(p - sizeof(char*));
		p = *prev_word;
	}
	return NULL;
}

bool check_rest(int len)
{
	int size = DIC_SIZE;
	char *top = dic.dic_buff;
	char *cur = dic.append_pos;
	int rest = size - (cur - top);

	return (rest > len);
}

bool append_word(char* str)
{
	int len = strlen(str);
	if (!check_rest(len)) {
		print("words buffer overfllow.\n");
		return false;
	}
	else {
		strncpy((char*)dic.append_pos, str, len);
		dic.append_pos += len;
		*dic.append_pos++ = '\0';
	}
	return true;
}

bool append_addr(char *addr, bool inc_pos)
{
	if (!check_rest(sizeof(char*))) {
		print("words buffer overfllow.\n");
		return false;
	}

	char **prev_word = (char**)dic.append_pos;
	*prev_word = addr;
	if (inc_pos)
		dic.append_pos += sizeof(char *);

	return true;
}

bool append_name(char* str)
{
	// 数値を辞書登録しない
	if (is_num(str) || is_hex(str)) {
		print("Can't dic entry! word='");
		print(str);
		println("' is number.");
		return false;
	}

	if (!append_addr(dic.last_word, true))
		return false;

	dic.last_word = dic.append_pos;
	if (!append_word(str))
		return false;		// 登録失敗

	return true;
}

bool append_body(char* str)
{
	if (!append_word(str)) {
		// 登録失敗
		return false;
	}

	if (strcmp(str, ";") == 0) {
		*(dic.append_pos-1) = '\0';
		dic.entry_step = 0;
	}
	else {
		// 本体は語のあとはスペースにする
		dic.append_pos--;
		append_word(" \0");
		dic.append_pos--;
	}

	return true;
}
// [prev-word-addr] <---------------+
// [this_word.name] <-- curr word	|
// [null]							|
// [this_word."1st-word"]			|
// ['sp']							|
// [this_word."2nd-word"]			|
// ['sp']							|
// ...								|
// ['sp']							|
// [this_word."last-word"]			|
// ['sp']							|
// [this_word.";"]					|
// ['sp'] 							|
// [null]							|
// [prev-word-addr] ----------------+ <-- next append_pos

void dic_entry(char* str)
{
	switch (dic.entry_step) {
	case 0:	//
		break;

	case 1:	// name
		append_name(str);
		dic.entry_step++;
		break;

	case 2:// body
		append_body(str);
		if (strcmp(str, ";") == 0) {
			*(dic.append_pos-1) = '\0'; // 最後のスペースをNULLにする.
			append_addr(dic.last_word, false);

			dic.entry_step = 0;
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
	print("dic.last_word:"); print_addr(dic->last_word); print("\n");

	char* p = dic->dic_buff;
	//char* addr;
	while (p < dic->append_pos) {
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
 * プロジェクトのデバッグの構成で 「Use external console for inferior (open
 * 				a new console window for input/output)」 をチェック。
 */
char *input()
{
	print("> ");
	char *p = fgets(input_buff, sizeof(input_buff), stdin);

	return p;
}

/***
 * 表示改行
 */
void println(char* str)
{
	print(str);
	print("\n");
}

/***
 * 1語取得
 */
char* get_token(char **str, int* len)
{
	if (str == NULL)
		return NULL;

	// 引数 str は戻るときに検査した分だけ進められている.
	//char *src = *str;
	//char *dst = tok;
	char *ret = NULL;
	//*dst = '\0';

	while (**str && **str <= ' ') (*str)++;
	//*str = src;			// 検査分引数のアドレスが進む
	ret = *str;			// 返すアドレス
	if (**str == '\0')
		return NULL;	// 取り出すものがなかったとき

//	int siz = tok_len;
	pc = *str;
	*len = 0;
	//while (*src > ' ' && siz--) {
	while (**str > ' ' || quat_flag) {
		(*str)++;
		(*len)++;
		if (**str == quat_flag) {
			(*str)++;
			(*len)++;
			quat_flag = '\0';
			break;
		}
	}
	//*dst = '\0';
	//*str = *str;			// 検査分引数のアドレスが進む

	return ret;			// 取り出した結果
}

/***
 * ワードの評価
 */
void eval(char *str)
{
	char *str_save;
	char *tok;
	int len;
	char tok_buff[20];
	//char next_buff[20];
	str_save = str;
	while ((tok = get_token(&str, &len)) != NULL) {
		strncpy(tok_buff, tok, len);
		tok_buff[len] = '\0';
		// Proccess ENTRY dictionary
		if (dic.entry_step > 0) {
			dic_entry(tok_buff);
		}
		else if (quat_flag && str_save) {
			//str = ++str_save;
			//while (*str && *str != quat_flag) {
				print(tok_buff);
			//}
			//str++;
			quat_flag = '\0';
			str_save = str;
			continue;
		}
		// Proccess 'IF' statement.
		// step = 1 -> exec words untill "THEN" or "ELSE",
		//				if "THEN" step=0, "ELSE" step=2
		// step = 2 -> look for "TEHN", step=0, and exec next word.
		// step = 3 -> lock for "ELSE", step=1, and exec next word.
		// step 2or3 (nest if find) "IF" nest++ / "THEN" nest--
		else if (if_step>1) {
			// case 2= look for "TEHN"
			// case 3= lock for "ELSE"
			if (!stricmp(tok_buff, "IF")) {
				if_nest++;
			}
			else if (!stricmp(tok_buff, "THEN")) {
				if ((if_step==2 && if_nest == 0)
					)
					if_step = 0;
				else
					if_nest--;
			}
			else if (!stricmp(tok_buff, "ELSE")
					&& if_step==3 && if_nest == 0) {
				if_step = 1;
			}
		//	str_save = str;
		//	continue;
		}
		else if (do_step > 0) {
			// Proccess DO ... LOOP
			// step = 1 -> DOループ 実行開始
			// step = 2 -> DOループ 実行中
			// step = 2 -> LOOP までスキップ
			switch (do_step) {
			default:
				break;

			case 1:	// push prog pos.
				do_step++;
				break;

			case 2:	// push prog pos.
				if (!stricmp(tok_buff, "LOOP")) {
					int do_cnt = pop_int();
					push_int(++do_cnt);

					str_save = str;
					str = *--do_sp;
					do_step = 1;
				//	continue;
				}
				break;

			case 3: // skip to loop.
				if (!stricmp(tok_buff, "DO")) {
					do_nest++;
				}
				else if (!stricmp(tok_buff, "LOOP")) {
					if (do_step == 3 && do_nest == 0) {
						do_step = 0;
					}
					else
						do_nest--;
				}
				str_save = str;
				continue;
			}
		}
//		else {
		// Proccess double words statement.
		// 2語長命令
		void (*pf2)(char**) = lookup_prim_2(tok_buff);
		if (pf2 != NULL) {		// 2語長 組込みワード
			//char* next_tok = get_token(&str, next_buff, sizeof(next_buff));
			(*pf2)(&str);
		}
		else {
			// Proccess single word statement.
			// 1語長命令
			char *wd = lookup_word(tok_buff);
			if (wd)					// 辞書ワード
				eval(wd);
			else if (is_num(tok_buff) || is_hex(tok_buff))	// 数値
				push_int(atou64(tok_buff));
			else {
				void (*pf)() = lookup_prim(tok_buff);
				if (pf != NULL) {	// 1語長 組込みワード
					(*pf)();
				}
				else {
					print("ERROR:'");
					print(tok_buff);
					print("' is not defined.\n");
				}
			}
		}
//		}
		str_save = str;
	}
}


void init()
{
	println("++ fool Fotrh! 1.0 ++");

	// stack
	sp = stack;

	// heap
	hp = heap;

	// do loop
	do_sp = do_stack;
	pc = NULL;

	// dictionary
	dic.entry_step = 0;
	dic.last_word = dic.dic_buff;
	dic.append_pos = dic.dic_buff;

	// digit base
	base = 10;

	if_step = 0;
	if_nest = 0;

	quat_flag = 0;
}


int main(void)
{
	init();

	char *str;
	while ((str = input()) != NULL) {
		eval(str);
		print("ok ");
	}
	return EXIT_SUCCESS;
}


/*
 * 変数の実装 別の方法
 *
 * 宣言： vbariable x
 * variable → 次の語(x) 取得。
 * 変数テーブルに追加。
 * (x) を辞書に追加。 名前=x / 本体=”VAR $addr-L $addr-H ;" 。
 *
 * 使用: x
 * 辞書検索 (x) ？ ない → エラー
 * あった （”VAR” だった） → テーブル中のアドレス取得。
 * アドレス プッシュ。
 * おわり
 * 10 1 do i dup . % 3 if cr  ." fool"  else cr ." not fool" then loop
 */
