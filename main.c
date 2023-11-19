#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define DEBUG(...) \
	fprintf(stderr, __VA_ARGS__); 

enum Type {
	NOT_IDENTIFIED,
	BLANK_SPACE,
	IDENTIFIER,
	ADDITION,
	MULTIPLIER,
	NEGATIVE,
	LPAR, // left parentheses
	RPAR, // right parentheses
};

typedef struct Token {
	enum Type type;
	int isNegative; 
	struct Token *left;
	struct Token *right;
	size_t position;
	char *data;
} Token;

typedef struct Expression {
	Token *tkn;
	size_t len;
} Expression;

typedef struct AST {
	Token *root;
	size_t size;
} AST;

int checkWord(char *tkn){
	char word = *(char*)tkn;
	if((int)word >= 65 && (int)word <= 91)
		return IDENTIFIER;
	else if(word == '+') 
		return ADDITION;
	else if(word == '*')
		return MULTIPLIER;
	else if(word == '(') 
		return LPAR;
	else if(word == ')')
		return RPAR;
	else if((int)word == 32)
		return BLANK_SPACE;
	else
		return NOT_IDENTIFIED;
}

void createAST(AST** tree){
	(*tree) = malloc(sizeof(AST)); 
	(*tree)->root = malloc(sizeof(Token));
	(*tree)->size = 0;
}

Token* createNode(char *data){
	Token* node = malloc(sizeof(Token));
	node->left = NULL;
	node->right = NULL;
	node->type = NOT_IDENTIFIED;
	node->data = malloc(strlen(data)); 
	strcpy(node->data, data);
	node->isNegative = 0;
	node->position = -1;
	return node;
}

Expression* createExpression(Token *t, size_t len){
	Expression *exp = malloc(sizeof(Expression));
	exp->len = len;
	exp->tkn = t;
	return exp;
}

int compareToken(char *a, char *b){
	if((checkWord(a) == ADDITION || checkWord(a) == MULTIPLIER) && checkWord(b) == IDENTIFIER){
		return -1;
	} else if(checkWord(a) == IDENTIFIER && (checkWord(b) == ADDITION || checkWord(b) == MULTIPLIER)) {
		return 1;
	} else {
		DEBUG("Comparing two things not identified: a = %s, b = %s\n", a, b);
		exit(1);
	}
}

Token* insertHelper(AST* ast, Token *tkn, char *data){
	if(tkn == NULL){
		ast->size++;
		tkn = createNode(data);
		// TODO: compare the operator with operand to check if someone is gonna be the node neigh or root
	} else if(compareToken(data, tkn->data) < 0){
		tkn->left = insertHelper(ast, tkn->left, data);
	} else {
		tkn->right = insertHelper(ast, tkn->right, data);
	}	

	return tkn;
}

void insert(AST *ast, void *data){
	ast->root = insertHelper(ast, ast->root, data);
}

void printExpression(Expression *e){
	for(int i = 0; i < e->len; i++){
		DEBUG("tkn[%i] = %c\n", i, *e->tkn[i].data);
	}	
}

Expression* parseHelper(AST *ast, char* expr){
	size_t len = strlen(expr)-1; 
	Token *tkn = malloc(sizeof(Token)*len);
	// current expr: AC*(ABD) + ABC
	size_t i = 0;
	while(i < len){
		Token *aux = malloc(sizeof(Token));
		DEBUG("expression at parseHelper: %c\n", expr[i]);
		switch(expr[i]){
			case '*':
				aux = createNode(&expr[i]); 
				aux->type = MULTIPLIER; 
				tkn[i] = *aux;
				break;
			case '+': 
				aux = createNode(&expr[i]); 
				aux->type = ADDITION; 
				tkn[i] = *aux;
				break;
			case '(': 
				aux = createNode(&expr[i]); 
				aux->type = LPAR;
				tkn[i] = *aux;
				break;
			case ')':
				aux = createNode(&expr[i]); 
				aux->type = RPAR;
				tkn[i] = *aux;
				break;
			case '!':
				aux = createNode(&expr[i]); 
				aux->type = NEGATIVE;
				aux->isNegative = 1;
				tkn[i] = *aux;
				break;
			default:
				// TODO(rvlt): Check if syntax is correct
				if(checkWord(&expr[i]) == IDENTIFIER){
					aux = createNode(&expr[i]); 
					aux->type = IDENTIFIER;
					tkn[i] = *aux;
					break;
				} else if (checkWord(&expr[i]) == BLANK_SPACE) {
					aux = createNode(&expr[i]); 
					aux->type = BLANK_SPACE;
					tkn[i] = *aux;
					break;	
				}else {
					printf("Expression not parsed by tokenization: %d", (int)expr[i]);
					exit(-1);
				}
		}
		i++;
		free(aux);
	}
	Expression *exp = createExpression(tkn, len);
	free(tkn);
	return exp;
}

typedef struct ListNode {
	char *data;
	struct ListNode *next;
} ListNode;

typedef ListNode LN;

typedef struct LinkedList {
	LN *root;
	size_t size;
} LinkedList;

typedef LinkedList LL;

void createLinkedList(LL **l){
	(*l) = malloc(sizeof(LL)); 
	(*l)->root = NULL; 
	(*l)->size = 0;
}

LN *createListNode(char *data){
	LN *node = malloc(sizeof(LN)); 
	node->data = malloc(strlen(data));
	strcpy(node->data, data);
	node->next = NULL;

	return node;
}

LN* appendHelper(LL* list, LN *listNode, void *data){
	if(listNode == NULL){
		list->size++;
		listNode = createListNode(data);
	} else {
		listNode->next = appendHelper(list, listNode->next, data);
	} 
	  
	return listNode;
}

void append(LL *list, char *data){
	list->root = appendHelper(list, list->root, data);
}

LL *readFile(FILE *f){
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

	LL *l;
	createLinkedList(&l); 
	while((nread = getline(&line, &len, f)) != -1){		
		append(l, line);
	}

	return l;
}

void printList(LL *inputs){
	LN *aux = inputs->root;
	int it = 1;
	while(aux){
		printf("input %i: %s", it, aux->data);
		aux = aux->next;
		it += 1;
	}
}

void parse(AST *ast, char *expr){
	Expression *e; 
	e = parseHelper(ast, expr);
	printExpression(e);
}

void lexer(AST *ast, LL *inputs){
	LN *aux = inputs->root;
	int it = 1;
	while(aux){
		parse(ast, aux->data);
		aux = aux->next;
		it += 1;
	}
}

int main(int argc, char *argv[]){
	const char *filename = "input.txt"; 
	FILE *f = fopen(filename, "r+");

	if(f == NULL){
		fprintf(stderr, "Couldn't open file input.txt\n");
		exit(-1);
	}

	AST *ast; 
	createAST(&ast);

	// Linked list of inputs
	LL *inputs = readFile(f);
	lexer(ast, inputs);

	free(ast);
	fclose(f);
	return 0;
}
