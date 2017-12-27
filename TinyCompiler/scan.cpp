/*
编程的技巧：static的变量和方法不会暴露给外面的文件。因此也不用写到头文件里面去，这样的话就实现了隐藏细节。
extern 引入外部的变量，防止变量的重复定义。

*/
#include "globals.h"
#include "util.h"
#include "scan.h"

//DNF的状态类型
typedef enum{
	//开始，冒号，注释，数字，标识符或保留字，完成
	START,INASSIGN,INCOMMENT,INNUM,INID,DONE
}StateType;

char tokenString[MAXTOKENLEN + 1];

#define BUFLEN 256

static char lineBuf[BUFLEN];//存储当前行
static int linepos = 0;//当前字符在行中的位置
static int bufsize = 0;//这个当前行的大小

/*
对外表示返回一个字符
如果到达文件尾部的话就返回EOF
*/
static char getNextChar(void){
	//下一个位置超出了当前行的大小，因此要再读入新的一行
	if (!(linepos < bufsize)){
		lineno++;//行号加一
		if (fgets(lineBuf, BUFLEN - 1, source)){
			//技巧：用全局的标记位进行输出
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

/*先行的字符被退出*/
static void ungetNextChar(void){
	linepos--;
}

//结构体数组，保存保留字
static struct{
	char* str;
	TokenType tok;
}reservesWorks[MAXRESERVED] = {
	{ "if", IF }, { "then", THEN }, { "else", ELSE }, { "end", END }, {"repeat",REPEAT},
	{ "until", UNTIL }, { "read", READ }, {"write",WRITE} };

/*判断标识符是不是保留字,如果不在保留字列表里，那他就是一个普通的标识符*/
static TokenType reservedLookup(char* s){
	int i;
	for (i = 0; i < MAXRESERVED; i++){
		if (!strcmp(s, reservesWorks[i].str))
			return reservesWorks[i].tok;
	}
	return ID;
}

/*
scan.cpp 唯一对外公开的接口，就是getToken()获得下一个字符，只返回字符的类型，用于分析程序已经足够了
保留字，标识符，数字，各种操作符的类型，文件尾，如果允许输出的话还能输出获得的字符串
*/
TokenType getToken(void){
	int tokenStringIndex = 0;
	TokenType currentToken;
	StateType state = START;
	int save;
	//前面的铺垫就是为了很好的判断一个记号的类型和串值
	while (state != DONE)
	{
		char c = getNextChar();//获取下一个字符
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
			if (c == '}')state = START;//没有考虑嵌套，如果{后还有{,则被当成注释的内容忽略，只有}被响应
			break;
		case INASSIGN:
			state = DONE;
			if (c=='=')
				currentToken = ASSIGN;
			else{
				ungetNextChar();
				save = FALSE;
				currentToken = ERROR;//错误也是接受态
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
				currentToken = ID; // 保留字和标识符都保存成标识符，之后在识别 保留字
			}
			break;
		case DONE:
		default:
			fprintf(listing, "Scanner Bug:state=%d\n", state);
			state = DONE;
			currentToken = ERROR;
			break;
		}
		//决定是否保留字符并且检查保留字
		if ((save) && (tokenStringIndex <= MAXTOKENLEN))
			tokenString[tokenStringIndex++] = c;
		if (state == DONE){
			tokenString[tokenStringIndex] = '\0';
			if (currentToken == ID){
				currentToken = reservedLookup(tokenString);
			}
		}
	}
	//输出记号并且返回
	if (TraceScan){
		fprintf(listing, "\t%d", lineno);
		printToken(currentToken, tokenString);
	}
	return currentToken;
}