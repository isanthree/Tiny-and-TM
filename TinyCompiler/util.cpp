#include "globals.h"
#include "util.h"

/*
打印一个记号
输入是一个记号的类型：token 和一个记号的串值 tokenString
*/
void printToken(TokenType token, const char* tokenString){
	switch (token){
	case IF:
	case THEN:
	case ELSE:
	case END:
	case REPEAT:
	case UNTIL:
	case READ:
	case WRITE:
		fprintf(listing, "reserved word:%s\n", tokenString);
		break;

	case ASSIGN:fprintf(listing, ":=\n");	break;
	case LT:fprintf(listing, "<\n");		break;
	case EQ:fprintf(listing, "=\n");		break;
	case LPAREN:fprintf(listing, "(\n");	break;
	case RPAREN:fprintf(listing, ")\n");	break;
	case SEMI:fprintf(listing, ";\n");		break;
	case PLUS:fprintf(listing, "+\n");		break;
	case MINUS:fprintf(listing, "-\n");		break;
	case TIMES:fprintf(listing, "*\n");		break;
	case OVER:fprintf(listing, "/\n");		break;
	case ENDFILE:fprintf(listing, "EOF\n");	break;

	case NUM:
		fprintf(listing, "NUM,val=%s\n", tokenString);
		break;
	case ID:
		fprintf(listing, "ID,name=%s\n", tokenString);
		break;
	case ERROR:
		fprintf(listing, "ERROR:%s\n", tokenString);
		break;
	default:
		fprintf(listing, "Unknow token:%d\n", token);
	}
}

char* copyString(char* s){
	int n;
	char* t;
	if (s == NULL) return NULL;
	n = strlen(s) + 1;
	t = (char*)malloc(n);
	if (t == NULL){
		fprintf(listing, "out of memory error at line %d\n", lineno);
	}
	else{
		strcpy(t, s);
	}
	return t;
}

/*创建一个新的节点，这个节点的类型是语句类型： 分为五类，if, write, raed, repeat, assign
表达式有三种，算符，常量，标识符表达式
*/
TreeNode* newStmtNode(StmtKind kind){
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL){
		fprintf(listing, "out of memery error at line %d\n", lineno);
	}
	else{
		for (i = 0; i < MAXCHILDREN; i++){
			t->child[i] = NULL;
		}
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
	}
	return t;
}

/*创建一个表达式节点，类型有三个*/
TreeNode* newExpNode(ExpKind kind){
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL){
		fprintf(listing, "out of memery error at line %d\n", lineno);
	}
	else{
		for (i = 0; i < MAXCHILDREN; i++){
			t->child[i] = NULL;
			t->sibling = NULL;
			t->nodekind = ExpK;
			t->kind.exp = kind;
			t->lineno = lineno;
			t->type = Void;
		}
	}
	return t;
}

/*使用变量 indentno 来存储printTree当前的缩进数量*/
static int indentno = 0;
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/*打印缩进*/
static void printSpaces(void){
	int i;
	for (i = 0; i < indentno; i++){
		fprintf(listing, " ");
	}
}

/*打印语法树:这个树的打印有很多值得学习的地方
先打印语句的根节点，然后遍历孩子，然后传向下一个语句序列
*/
void printTree(TreeNode* tree){
	int i;
	INDENT;
	while (tree != NULL){
		printSpaces();
		//如果节点类型是语句，说明语句的类型
		if (tree->nodekind == StmtK){
			switch (tree->kind.stmt){
			case IfK:
				fprintf(listing, "If\n");
				break;
			case RepeatK:
				fprintf(listing, "Repeat\n");
				break;
			case AssignK:
				fprintf(listing, "Assign to: %s\n",tree->attr.name);
				break;
			case ReadK:
				fprintf(listing, "Read: %s\n",tree->attr.name);
				break;
			case WriteK:
				fprintf(listing, "Write\n");
				break;
			default:
				fprintf(listing, "Unknown ExpNode kind\n");
				break;
			}
		}
		else if(tree->nodekind==ExpK){//节点类型是表达式类型
			switch (tree->kind.exp){
			case OpK:
				fprintf(listing, "Op:");
				printToken(tree->attr.op, "\0");//利用记号打印函数，打印这个操作符
				break;
			case ConstK:
				fprintf(listing, "const: %d\n", tree->attr.val);
				break;
			case IdK:
				fprintf(listing, "Id: %s\n", tree->attr.name);
				break;
			default:
				fprintf(listing, "Unkonwn ExpNode kind\n");
				break;
			}
		}
		else{
			fprintf(listing, "Unknown node kind\n");
		}
		//遍历自己的孩子
		for (i = 0; i < MAXCHILDREN; i++){
			printTree(tree->child[i]);
		}
		//遍历自己的兄弟
		tree = tree->sibling;
	}
	//缩进回退
	UNINDENT;
}