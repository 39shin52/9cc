#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>


// トークンの種類
typedef enum{
	TK_RESERVED, // 記号
	TK_IDENT,    // 識別子
	TK_NUM,      // 整数トークン
	TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;


typedef enum{
	ND_ADD, 	// +
	ND_SUB, 	// -
	ND_MUL, 	// *
	ND_DIV, 	// /
	ND_ASSIGN,  // =
	ND_LVAR,    // ローカル変数
	ND_NUM, 	//整数
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;

// トークン型
struct Token{
	TokenKind kind; // トークンの型
	Token *next;    // 次のトークン
	int val;        // kindがTK_NUMの場合、その数値
	char *str;      // トークン文字列
};

//抽象構文木のノード
struct Node{
	NodeKind kind;  // ノードの型
	Node *lhs;      // 左辺
	Node *rhs;      // 右辺
	int val;        // kindがND_NUMのときにのみ使う
	int offset;		// kindがND_LVARの時のみ使う
};

// ローカル変数の型
struct LVar {
	LVar *next;	// 次の変数かNULL
	char *name;	// 変数の名前
	int len;	// 名前の長さ
	int offset;	// RBPからのオフセット
};

// 現在注目しているトークン
Token *token;

// ローカル変数
LVar *locals:

// 入力プログラム
char *user_input;

// ノードを入力
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}
Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *expr();
Node *mul();
Node *primary();

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// エラー報告
// printfと同じ引数をとる
void error(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号のときには、トークンを一つ読み進めて
// trueを返す。それ以外はfalseを返す。
bool consume(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op) return false;
	token = token->next;
	return true;
}

// 次のトークンが期待している記号のときには、トークンを一つ読み進める。
// それ以外の場合にはエラーを返す
void expect(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op) error_at(token->str, "'%c'ではありません", op);
	token = token->next;
}

// 次のトークンが数値の場合、トークンを一つ読み進めてその数値を返す
// それ以外はエラーを返す
int expect_number(){
	if(token->kind != TK_NUM) error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof(){
	return token->kind == TK_EOF;
}
	
// 新しいtokenを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(){
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while(*p){
		// 空白文字をスキップ
		if(isspace(*p)){
			p++;
			continue;
		}

		if('a' <= *p && *p <= 'z'){
			cur = new_token(TK_IDENT, cur, p++);
			cur->len = 1;
			continue;
		}

		if(strchr("+-*/()", *p)){
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す
LVar *find_lvar(Token *tok){
	for(LVar *var = locals; var; var = var->next){
		if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)) return var;
	}
	return NULL;
}

// パーサ
Node *code[100];

Node *assign(){
	Node *node = equality();
	if(comnsume("=")) node = ew_node(ND_ASSIGN, node, assign());
	return node;
}

Node *expr(){
	/*
	Node *node = mul();

	for(;;){
		if(consume('+')) node = new_node(ND_ADD, node, mul());
		else if (consume('-')) node = new_node(ND_SUB, node, mul());
		else return node;
	}*/
	return assigin();
}
Node *stmt(){
	Node *node = expr();
	expect(;);
	return node;
}

void program(){
	int i = 0;
	while(!at_eof()) coode[i++] = stmt();
	code[i] = NULL;
}

Node *mul(){
	Node *node = primary();

	for(;;){
		if(consume('*')) node = new_node(ND_MUL, node, primary());
		else if(consume('/')) node = new_node(ND_DIV, node, primary());
		else return node;
	}
}
Node *primary(){
	// 次のトークンが"("なら、"(" expr ")"のはず
	if(consume('(')){
		Node *node = expr();
		expect(')');
		return node;
	}
	// そうでなければ数値のはず
	return new_node_num(expect_number());

	Token *tok = consume_ident();
	if(tok) {
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		//node->offset = (tok->str[0] - 'a' + 1) * 8;
		LVar *lvar = find_lvar(tok);
		if(lvar){
			node->offset = lvar->offset;
		} else {
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			lvar->offset = locals->offset + 8;
			node->offset = lvar->offset;
			locals = lvar;
		}
		return node;
	}
}

// スタックマシン
void gen_lval(Node *node){
	if(node->kind != ND_LVAR) error("代入の左辺値が変数ではありません");

	printf(" mov rax, rbp\n");
	printf(" sub rax, %d\n", node->offset);
	printf(" push rax\n");
}

void gen(Node *node){
	/*
	if(node->kind == ND_NUM){
		printf(" push %d\n", node->val);
		return;
	}*/

	switch (node->kind){
		case ND_NUM:
			printf(" push %d\n", node->val);
			return;
		case ND_LVAR:
			gen_lval(node);
			printf(" pop rax\n");
			printf(" mov rax, [rax]\n");
			printf(" push rax\n");
			return;
		case ND_ASSIGN:
			gen_lval(node->lhs);
			gen(node->rhs);

			printf(" pop rdi\n");
			printf(" pop rax\n");
			printf(" mov [rax], rdi\n");
			printf(" push rdi\n");
			return;
	}

	gen(node->lhs);
	gen(node-> rhs);

	printf(" pop rdi\n");
	printf(" pop rax\n");

	switch (node->kind) {
		case '+':
			printf(" add rax, rdi\n");
			break;
		case '-':
			printf(" sub rax, rdi\n");
			break;
		case '*':
			printf(" imul rax, rdi\n");
			break;
		case '/':
			printf(" cqo\n");
			printf(" idiv rax, rdi\n");
			break;
	}

	printf(" push rax\n");
}

int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// char *p = argv[1];
	// トークナイズする
	// token = tokenize(argv[1]);
	user_input = argv[1];
	token = tokenize();
	//Node *node =expr ();
	program();

	// アセンブリ前半部分を入力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	/*
	// 式の最初は数でなければならないのでそれをチェックして
	// 最初のmov命令を出力
	printf(" mov rax, %d\n", expect_number());

	// `+ <数>`あるいは`- <数>`というトークンの並びを消費しつつ
	// アセンブリを出力
	while(!at_eof()){
		if(consume('+')){
			printf(" add rax, %d\n", expect_number());
			continue;
		}
		
		expect('-');
		printf(" sub rax, %d\n", expect_number());
	}
	*/

	// プロローグ
	// 変数26個文の領域を確保する
	printf(" push rbp\n");
	printf(" mov rbp, rsp\n");
	printf(" sub rsp, 208\n");

	// 先頭の式から順にコード生成
	for(int i = 0; code[i]; i++){
		gen(code[i]);

		// 式の評価結果としてスタックに一つの値が残っている
		// はずなので、スタックが溢れないようにポップしておく
		printf(" pop rax\n");
	}

	// エピローグ

	// 抽象構文木を下りながらコード生成
	//gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをRAXにロードして関数からの返り値とする
	printf(" pop rax\n");
	printf(" ret\n");
	return 0;
}
			
