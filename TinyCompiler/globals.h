/*
tiny ��ȫ�ֱ�����
��������ֵ���壬�Ǻ�ö������

*/
#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//����boolֵ
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
//���ı�ʶ������
#define MAXRESERVED 8
//�Ǻŵ�ö������
typedef enum{
	//�ļ�β������
	ENDFILE,ERROR,
	//if, then, else, end, repeat, until, read, write
	IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,
	// id, num
	ID,NUM,
	//
	ASSIGN,EQ,LT,PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,SEMI
}TokenType;

//������ⲿ����
extern FILE* source ;
extern FILE* listing;
extern FILE* code;
extern int lineno;

//syntax tree for parsing
typedef enum{StmtK , ExpK }NodeKind;
typedef enum{IfK , RepeatK , AssignK , ReadK , WriteK}StmtKind;
typedef enum{OpK , ConstK , IdK}ExpKind;
typedef enum{Void , Integer , Boolean}ExpType;

#define MAXCHILDREN 3

typedef struct treeNode{
	struct treeNode* child[MAXCHILDREN];
	struct treeNode* sibling;
	int lineno;
	NodeKind nodekind;
	union{ StmtKind stmt; ExpKind exp; } kind;
	union{ TokenType op;
			int val;
			char* name;} attr;
			ExpType type;
}TreeNode;

// flags for traceing
extern int EchoSource;
extern int TraceScan;
extern int TraceParse;
extern int TraceAnalyze;
extern int TraceCode;
extern int error;

#endif