#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>


// トークンの種類
typedef enum{
	TK_RESERVED, // 記号
	TK_NUM,      // 整数トークン
	TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef enum{
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, //整数
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

// トークン型

struct Token{
	TokenKind kind; // トークンの型
	Token *next;    // 次のトークン
	int val;        // kindがTK_NUMの場合、その数値
	char *str;      // トークン文字列
};

struct Node{
	NodeKind kind;  // ノードの型
	Node *lhs;      // 左辺
	Node *rhs;      // 右辺
	int val;        // kindがND_NUMのときにのみ使う
};


// 現在注目しているトークン
Token *token;

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
Node *unary();

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

// パーサ
Node *expr(){
	Node *node = mul();

	for(;;){
		if(consume('+')) node = new_node(ND_ADD, node, mul());
		else if (consume('-')) node = new_node(ND_SUB, node, mul());
		else return node;
	}
}
Node *mul(){
	Node *node = unary();

	for(;;){
		if(consume('*')) node = new_node(ND_MUL, node, unary());
		else if(consume('/')) node = new_node(ND_DIV, node, unary());
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
}
Node *unary(){
	if(consume('+')) return primary();
	if(consume('-')) return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

// スタックマシン
void gen(Node *node){
	if(node->kind == ND_NUM){
		printf(" push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node-> rhs);

	printf(" pop rdi\n");
	printf(" pop rax\n");

	switch (node->kind) {
		case ND_ADD:
			printf(" add rax, rdi\n");
			break;
		case ND_SUB:
			printf(" sub rax, rdi\n");
			break;
		case ND_MUL:
			printf(" imul rax, rdi\n");
			break;
		case ND_DIV:
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
	token = tokenize(user_input);
	Node *node =expr ();

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

	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをRAXにロードして関数からの返り値とする
	printf(" pop rax\n");
	printf(" ret\n");
	return 0;
}
			
