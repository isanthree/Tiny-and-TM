/*
��̵ļ��ɣ�static�ı����ͷ������ᱩ¶��������ļ������Ҳ����д��ͷ�ļ�����ȥ�������Ļ���ʵ��������ϸ�ڡ�
extern �����ⲿ�ı�������ֹ�������ظ����塣

*/
#include "globals.h"
#include "util.h"
#include "scan.h"

//DNF��״̬����
typedef enum{
	//��ʼ��ð�ţ�ע�ͣ����֣���ʶ�������֣����
	START,INASSIGN,INCOMMENT,INNUM,INID,DONE
}StateType;

char tokenString[MAXTOKENLEN + 1];

#define BUFLEN 256

static char lineBuf[BUFLEN];//�洢��ǰ��
static int linepos = 0;//��ǰ�ַ������е�λ��
static int bufsize = 0;//�����ǰ�еĴ�С

/*
�����ʾ����һ���ַ�
��������ļ�β���Ļ��ͷ���EOF
*/
static char getNextChar(void){
	//��һ��λ�ó����˵�ǰ�еĴ�С�����Ҫ�ٶ����µ�һ��
	if (!(linepos < bufsize)){
		lineno++;//�кż�һ
		if (fgets(lineBuf, BUFLEN - 1, source)){
			//���ɣ���ȫ�ֵı��λ�������
			if (EchoSource)
				fprintf(listing, "%4d:%s", lineno, lineBuf);
			bufsize = strlen(lineBuf);
			linepos = 0;
			return lineBuf[linepos++];
		}
		else {
			linepos++;
			return EOF;
		}
	}
	else return lineBuf[linepos++];
}

/*���е��ַ����˳�*/
static void ungetNextChar(void){
	linepos--;
}

//�ṹ�����飬���汣����
static struct{
	char* str;
	TokenType tok;
}reservesWorks[MAXRESERVED] = {
	{ "if", IF }, { "then", THEN }, { "else", ELSE }, { "end", END }, {"repeat",REPEAT},
	{ "until", UNTIL }, { "read", READ }, {"write",WRITE} };

/*�жϱ�ʶ���ǲ��Ǳ�����,������ڱ������б����������һ����ͨ�ı�ʶ��*/
static TokenType reservedLookup(char* s){
	int i;
	for (i = 0; i < MAXRESERVED; i++){
		if (!strcmp(s, reservesWorks[i].str))
			return reservesWorks[i].tok;
	}
	return ID;
}

/*
scan.cpp Ψһ���⹫���Ľӿڣ�����getToken()�����һ���ַ���ֻ�����ַ������ͣ����ڷ��������Ѿ��㹻��
�����֣���ʶ�������֣����ֲ����������ͣ��ļ�β�������������Ļ����������õ��ַ���
*/
TokenType getToken(void){
	int tokenStringIndex = 0;
	TokenType currentToken;
	StateType state = START;
	int save;
	//ǰ����̵����Ϊ�˺ܺõ��ж�һ���Ǻŵ����ͺʹ�ֵ
	while (state != DONE)
	{
		char c = getNextChar();//��ȡ��һ���ַ�
		save = TRUE;
		switch (state){
		case START:
			if (isdigit(c))
				state = INNUM;
			else if (isalpha(c))
				state = INID;
			else if (c == ':')
				state = INASSIGN;
			else if ((c == ' ') || (c == '\t') || (c == '\n'))
				save = FALSE;
			else if (c == '{')
			{
				save = FALSE;
				state = INCOMMENT;
			}
			else{
				state = DONE;
				switch (c){
				case EOF:
					save = FALSE;
					currentToken = ENDFILE;
					break;
				case '=':
					currentToken = EQ;
					break;
				case '<':
					currentToken = LT;
					break;
				case '+':
					currentToken = PLUS;
					break;
				case '-':
					currentToken = MINUS;
					break;
				case '*':
					currentToken = TIMES;
					break;
				case '/':
					currentToken = OVER;
					break;
				case '(':
					currentToken = LPAREN;
					break;
				case ')':
					currentToken = RPAREN;
					break;
				case ';':
					currentToken = SEMI;
					break;
				default:
					currentToken = ERROR;
					break;
				}
			}
			break;//START end
		case INCOMMENT:
			save = FALSE;
			if (c == '}')state = START;//û�п���Ƕ�ף����{����{,�򱻵���ע�͵����ݺ��ԣ�ֻ��}����Ӧ
			break;
		case INASSIGN:
			state = DONE;
			if (c=='=')
				currentToken = ASSIGN;
			else{
				ungetNextChar();
				save = FALSE;
				currentToken = ERROR;//����Ҳ�ǽ���̬
			}
			break;
		case INNUM:
			if (!isdigit(c)){
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = NUM;
			}
			break;
		case INID:
			if (!isalpha(c)){
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = ID; // �����ֺͱ�ʶ��������ɱ�ʶ����֮����ʶ�� ������
			}
			break;
		case DONE:
		default:
			fprintf(listing, "Scanner Bug:state=%d\n", state);
			state = DONE;
			currentToken = ERROR;
			break;
		}
		//�����Ƿ����ַ����Ҽ�鱣����
		if ((save) && (tokenStringIndex <= MAXTOKENLEN))
			tokenString[tokenStringIndex++] = c;
		if (state == DONE){
			tokenString[tokenStringIndex] = '\0';
			if (currentToken == ID){
				currentToken = reservedLookup(tokenString);
			}
		}
	}
	//����ǺŲ��ҷ���
	if (TraceScan){
		fprintf(listing, "\t%d", lineno);
		printToken(currentToken, tokenString);
	}
	return currentToken;
}