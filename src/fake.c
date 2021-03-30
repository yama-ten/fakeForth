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

int *pop_int()
{
	if (sp > stack)  {
		// 返すのは int のポインタ
		return (int *)--sp;
	} else {
		print("stack underflow.\n");
		return NULL;
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
 * プリミティブ ワード
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
	int num;
	int *t;
	if ((t = pop_int()) != NULL) num = *t; else return;
	push_int(num);
	push_int(num);
}

void swap()
{
	int n1, n2;
	int *t;
	if ((t = pop_int()) != NULL) n1 = *t; else return;
	if ((t = pop_int()) != NULL) n2 = *t; else return;
	push_int(n1);
	push_int(n2);
}

void drop()
{
	pop_int();
}

void over()
{
	int n1,n2;
	int *t;
	if ((t = pop_int()) != NULL) n1 = *t; else return;
	if ((t = pop_int()) != NULL) n2 = *t; else return;
	push_int(n2);
	push_int(n1);
	push_int(n2);
}

void rot()
{
	int n1,n2,n3;
	int *t;
	if ((t = pop_int()) != NULL) n1 = *t; else return;
	if ((t = pop_int()) != NULL) n2 = *t; else return;
	if ((t = pop_int()) != NULL) n3 = *t; else return;
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
	int num;
	int *t;
	if ((t = pop_int()) != NULL) num = *t; else return;
//	print_int(num);
	print_num(num, base);
	putc(' ', stdout);
}

void int_add()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int(x + y);
}

void int_sub()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int(x - y);
}

void int_mul()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int(x * y);
}

void int_div()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int(x / y);
}

void int_mod()
{
	int x, y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int(x % y);
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
	int x;
	int *t;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int(-x);
}

void int_abs()
{
	int x;
	int *t;
	if ((t = pop_int()) != NULL) x = *t; else return;
	if (x < 0)
		push_int(-x);
	else
		push_int(x);
}

void int_max()
{
	int x,y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	if (x > y)
		push_int(x);
	else
		push_int(y);
}

void int_min()
{
	int x,y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	if (x < y)
		push_int(x);
	else
		push_int(y);
}

void times_div()
{
	int x,y,z;
	int *t;
	if ((t = pop_int()) != NULL) z = *t; else return;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int( x * y / z );
}

void times_div_mod()
{
	int x,y,z;
	int *t;
	if ((t = pop_int()) != NULL) z = *t; else return;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	x *= y;
	push_int( x % z );
	push_int( x / z );
}

void div_mod()
{
	int x,y;
	int *t;
	if ((t = pop_int()) != NULL) y = *t; else return;
	if ((t = pop_int()) != NULL) x = *t; else return;
	push_int( x % y );
	push_int( x / y );
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
	int num = *pop_int();
	*(int*)mem = num;
}

void quest()
{
	fetch();
	dot();
}

void int_base()
{
	int *t;
	if ((t = pop_int()) != NULL) base = *t; else return;
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

//		/* print h */
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

struct PROC prim[] = {
		{ "bye",	bye },
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
		{ "?",		quest },

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

void create_var(char* var_name)
{
	append_name(var_name);
	dic.entry_step++;

	uintptr_t var_addr = (uintptr_t)hp;
	*(unsigned*)var_addr = 0;

	append_body("VAR $");
	dic.append_pos--;
	char buff[20];
	strcpy(buff, numtos(var_addr, 16));
	append_body(buff);
	append_body("$0");
	append_body(";");

	if (sp > stack)
		*hp = *pop_int();

	hp += sizeof(int);
}

void create_const(char* const_name)
{
	int num;
	if (sp > stack)
		num = *(int*)pop_int();
	else
		print("ERRROR:no value on stack.\n");

	append_name(const_name);
	dic.entry_step++;

	append_body("CONST");
	char buff[20];
	strcpy(buff, numtos(num, 10));
	append_body(buff);
	append_body(";");

	hp += sizeof(int);
}

void see(char *str)
{
	char *wd = lookup_word(str);
	if (wd) {
		print(wd);
	}
	else {
		print("ERROR:'");
		print(str);
		print("' is not defined.");
	}
}


struct PROC prim_2[] = {
		{ "variable",	create_var },
		{ "constant",	create_const },
		{ "see",	see },

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
		dic.prev_word = dic.append_pos;
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
	if (is_num(str)) {
		print("Can't dic entry! word='");
		print(str);
		println("' is number.");
		return false;
	}

	if (!append_addr(dic.last_word, true))
		return false;

	dic.last_word =
	dic.curr_word = dic.append_pos;
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

	dic.last_word = dic.curr_word;

	if (strcmp(str, ";") == 0) {
		*(dic.append_pos-1) = '\0';
		dic.prev_word = dic.curr_word;
		append_addr(dic.prev_word, false);
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
			dic.prev_word = dic.curr_word;
			append_addr(dic.prev_word, false);

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
	print("dic.prev_word:"); print_addr(dic->prev_word); print("\n");
	print("dic.last_word:"); print_addr(dic->last_word); print("\n");
	print("dic.curr_word:"); print_addr(dic->curr_word); print("\n");

	char* p = dic->dic_buff;
	//char* addr;
	while (p < dic->append_pos) {
		// prev_word addr;
//		print("\nprev_word:");
		char **addr = (char**)p;
//		print_addr((char*)*addr);
//		print("\n");
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


/***
 * ワードの評価
 */
void eval(char *str)
{
	if (is_num(str))
		push_num(str);
	else if (is_hex(str))
		push_int(atou64(str));
	else {
		void (*pf)() = lookup_prim(str);
		if (pf != NULL) {
			(*pf)();
		}
		else {
			print("ERROR:'");
			print(str);
			print("' is not defined.\n");
		}
	}
}

void println(char* str)
{
	print(str);
	print("\n");
}

char* get_token(char **str 		// 取り出し元の文字列
				, char *tok		// 取り出し先のバッファ
				, int tok_len	// バッファのサイズ
				)
{
	if (str == NULL)
		return NULL;

	// 引数 str は戻るときに検査した分だけ進められている.
	char *src = *str;
	char *dst = tok;
	*dst = '\0';

	while (*src && *src <= ' ') src++;
	*str = src;			// 検査分引数のアドレスが進む
	if (*src == '\0')
		return NULL;	// 取り出すものがなかったとき

	int siz = tok_len;
	while (*src > ' ' && siz--) {
		*dst++ = *src++;
	}
	*dst = '\0';
	*str = src;			// 検査分引数のアドレスが進む

	return tok;			// 取り出した結果
}

void proc(char *str)
{
	char token[20];
	char *tok;
	while ((tok = get_token(&str, token, sizeof(token))) != NULL) {
		if (*token != '\0') {
			if (dic.entry_step == 0) {
				void (*pf2)(char*) = lookup_prim_2(tok);
				if (pf2 != NULL) {
					char next_word[20];
					char* str2 = str;
					char* next = get_token(&str2, next_word, sizeof(next_word));
					(*pf2)(next);
					str = str2;
				}
				else {
					char *wd = lookup_word(tok);
					if (wd)
						proc(wd);
					else
						eval(tok);
				}
			}
			else {
				dic_entry(tok);
			}
		}
	}
}


void init()
{
	println("++ Fake Fotrh ++");

	// stack
	sp = stack;

	// heap
	hp = heap;

	// dictionary
	dic.entry_step = 0;
	dic.prev_word = dic.dic_buff;
	dic.last_word = dic.dic_buff;
	dic.append_pos = dic.dic_buff;

	// digit base
	base = 10;

}

int main(void)
{
	init();

	char *str;
	while ((str = input()) != NULL) {
		proc(str);
		print("ok ");
	}
	return EXIT_SUCCESS;
}


/*
 * 変数の実装
 *
 * 宣言： vbariable x
 * variable → 次の語(x) 取得。
 * (x) が変数テーブルにあるか？ ある → （？再定義する？）おわり。
 * ない → (x) を変数テーブルに追加。
 * (x) を辞書に追加。 名前=x / 本体=”VAR" 。
 *
 * 使用: x
 * 辞書検索 (x) ？ ない → エラー
 * あった （”VAR” だった） → 変数テーブル検索 (x) ない → エラー
 * あった → テーブル中のアドレス取得。
 * アドレス プッシュ。
 * おわり
 *
 */
