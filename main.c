#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

typedef void * (*ast_constructor)(void *);
typedef void (*ast_destructor)(void *);
typedef int (*ast_comparator)(const void *, const void *);

void* constructor(void *data){
	void *ptr = malloc(sizeof(char));
	memcpy(ptr, data, sizeof(char));
	return ptr;
}

void destructor(void *data){
	free(data);
}

int comparator(const void *a, const void *b){
	if(*(char*)a > *(char*)b){
		return -1;
	} else if(*(char*) a == *(char*) b){
		return 0;
	} else {
		return 1;
	}
}

enum Type {
	NOT_IDENTIFIED,
	IDENTIFIER,
	OPERATOR,
	KEYWORD
};

typedef struct Token {
	enum Type type;
	struct Token *left;
	struct Token *right;
	size_t position;
	void *data;
} Token;

typedef struct Expression {
	Token *tkn;
	size_t len;
} Expression;

typedef struct AST {
	Token *root;
	size_t size;
	ast_constructor constructor;
	ast_destructor destructor;
	ast_comparator comparator;
} AST;

void createAST(AST** tree, ast_constructor constructor, ast_destructor destructor, ast_comparator comparator){
	(*tree) = malloc(sizeof(AST)); 
	(*tree)->root = malloc(sizeof(Token));
	(*tree)->size = 0;
	(*tree)->constructor = constructor;
	(*tree)->destructor = destructor;
	(*tree)->comparator = comparator;
}

Token* createNode(void *data, ast_constructor constructor){
	Token* node = malloc(sizeof(Token));
	node->left = NULL;
	node->right = NULL;
	node->type = NOT_IDENTIFIED;
	node->data = constructor(data);
	node->position = -1;
	return node;
}

Expression* createExpression(Token *t, size_t len){
	Expression *exp = malloc(sizeof(Expression));
	exp->len = len;
	exp->tkn = t;
	return exp;
}

Token* insertHelper(AST* ast, Token *tkn, void *data){
	if(tkn == NULL){
		ast->size++;
		tkn = createNode(data, ast->constructor);
	} else if(ast->comparator(data, tkn->data) < 0){
		tkn->left = insertHelper(ast, tkn->left, data);
	} else {
		tkn->right = insertHelper(ast, tkn->right, data);
	}	

	return tkn;
}

void insert(AST *ast, void *data){
	ast->root = insertHelper(ast, ast->root, data);
}

void printExpression(Expression *args, int qtdArgs){
	int i = 0;
	int j = 0;
	for(i = 0; i < qtdArgs; i++){
		for(j = 0; j < args[i].len; j++){
			printf("%c", *(char*)args[i].tkn[j].data); 
		}
	}
}
Expression* parse(AST *ast, void *expr, int currentTokenPosition){
	size_t len = strlen((char*)expr); 
	Token *tkn = malloc(sizeof(Token)*len);
	// current expr: AC*(ABD) + ABC
	size_t i = 0;
	while(i < len){
		char word = *(char*)expr;
		switch(word){
			case '*':
			case '+': 
				tkn[i].data = ast->constructor(expr); 
				tkn[i].type = OPERATOR; 
				break;
			case '(': 
			case ')':
				tkn[i].data = ast->constructor(expr);
				tkn[i].type = KEYWORD;
				break;
			default:
				if((int)word >= 65 && (int)word <= 91){
					// TODO(rvlt): Check if syntax is correct
					tkn[i].data = ast->constructor(expr); 
					tkn[i].type = IDENTIFIER;
					break;
				} else {
					printf("Expression not parsed by tokenization: %c", word);
					exit(-1);
				}
		}
		i++;
		expr++;
	}
	Expression *exp = createExpression(tkn, len);
	free(tkn);
	return exp;
}

void iterateOverArgs(AST *ast, void **expr, size_t len){
	assert(len > 0);
	// Skip the first arg which is the program call-out
	int i = 1;
	int j = 0;
	Expression *exp; 
	Expression *args = malloc(sizeof(Expression) * len);

	while(i < len){
		printf("expr[%d] = %s\n", i, (char*)expr[i]);
		exp = parse(ast, expr[i], i);
		args[j] = *exp;
		i++;
		j++;
	}
	
	printExpression(args, len);
	free(exp);
	free(args);
}

int main(int argc, char *argv[]){
	assert(argc > 1);
	// AB + ABC
	// (AB)C + CBA
	// current expr: AC*(ABD) + ABC
	void **expr = (void**)argv;
	// Less ./main
	//printf("expr: %s\n", (char*)expr[2]); 
	AST *ast; 
	createAST(&ast, constructor, destructor, comparator);
	iterateOverArgs(ast, expr, argc);
	destructor(ast);
	return 0;
}
