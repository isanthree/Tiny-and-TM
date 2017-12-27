// TINY.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "globals.h"

#define NO_PARSE  FALSE
#define NO_ANALYZE FALSE
#define NO_CODE FALSE

#include "util.h"
#if NO_PARSE
	#include "scan.h"
#else
	#include "parse.h"
	#if !NO_ANALYZE
		#include "analyze.h"
		#if !NO_CODE
			#include "cgen.h"
		#endif
	#endif
#endif

//�õ���ȫ�ֱ���
int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;
/*
����׷�ٱ�־����־�ķ�ʽ�ܺã�ѧ����
*/
int EchoSource = TRUE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = FALSE;

int Error = FALSE;

int main(int argc, char *argv[])
{
	TreeNode* syntaxTree;
	char pgm[20]; //Դ�ļ�����
	printf("argc = %d\n", argc);
	printf("argv[0] = %s\n", argv[0]);
	printf("argv[1] = %s\n", argv[1]);
	if (argc != 2){
		fprintf(stderr, "usage:%s<filename>\n", argv[0]);
		exit(1);
	}
	//����������Ҽ���ļ�����׺
	strcpy(pgm, argv[1]);
	if (strchr(pgm, '.') == NULL)
		strcat(pgm, ".tny");
	//��Դ�ļ�source,����ʼ��listingΪ��׼���
	source = fopen(pgm, "r");
	if (source == NULL){
		fprintf(stderr, "file %s not found!", pgm);
		exit(1);
	}
	listing = stdout;
	fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);
#if NO_PARSE
	while (getToken() != ENDFILE);
	system("pause");
#else
	syntaxTree = parse();
	if (TraceParse){
		fprintf(listing, "\nSyntax tree:\n");
		printTree(syntaxTree);
	}
#if !NO_ANALYZE
	if (!Error){
		fprintf(listing, "\nBuilding Symbol Table...\n");
		buildSymtab(syntaxTree);
		fprintf(listing, "\nChecking Types...\n");
		typeCheck(syntaxTree);
		fprintf(listing, "\nType Checking Finished!\n");
	}
#if !NO_CODE
	if (!Error){
		char* codefile;
		int fnlen = strcspn(pgm, ".");
		codefile = (char*)calloc(fnlen + 4, sizeof(char));
		strncpy(codefile, pgm, fnlen);
		strcat(codefile, ".tm");
		code = fopen(codefile, "w");
		if (code == NULL){
			printf("Unable to open %s\n", codefile);
			exit(1);
		}
		codeGen(syntaxTree, codefile);
		fclose(code);
	}
#endif
#endif
#endif
	system("pause");
	return 0;
}



//�򵥵ļ���������
/*
<exp>		->	<term> { <addop> <term> }
<addop>		->	+ | - 
<term>		->	<factor> { <nulop> <factor> }
<nulop>		->	*
<factor>	->	(exp) | Number
*/
/*
char token; //��һ���Ǻ�

int exp(void);
int term(void);
int factor(void);

void error(void){
fprintf(stderr, "error!");
exit(1);
}

void match(char expectedToken){
if (token == expectedToken)
token = getchar();
else error();
}

int main(){
int result;
token = getchar();
result = exp();
if (token == '\n')
printf("result = %d\n", result);
else
error();
system("pause");
return 0;
}

int exp(void){
int temp = term();
while ((token == '+') || (token == '-')){
switch (token){
case '+':
	match('+');
	temp += term();
	break;
case '-':
	match('-');
	temp -= term();
	break;
}
}
return temp;
}

int term(void){
int temp = factor();
while (token == '*'){
match('*');
temp *= factor();
}
return temp;
}

int factor(void){
int temp;
if (token == '('){
match('(');
temp = exp();
match(')');
}
else{
if (isdigit(token)){
	ungetc(token, stdin);
	scanf("%d", &temp);
	token = getchar();
}
else error();
}
return temp;
}

*/