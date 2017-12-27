#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/*
TM虚拟机器
虽然这个机器特别特别简陋，它却能很好的说明问题，编程语言，编译器，虚拟机三者是如果共同完成工作的。
编程语言定义的语言的词法，语法，语义
虚拟机定义了真实机器的运行环境
编译器通过编译源代码，将编程语言转换成机器相关的目标代码或者机器无关的中间代码
不过最终都是要生成目标代码的，这一步不得不考虑目标机器的实际硬件情况，指令集。
并在生成目标代码的时候充分实现好这一点，具体体现在指令码的生成*/
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/******常量*******/
#define IADDR_SIZE 1024 //指令存储区的大小
#define DADDR_SIZE 1024 //数据区的大小
#define NO_REGS 8 //寄存器的数量
#define PC_REG 7 //pc寄存器编号 ，与code.h中的pc 相照应

#define LINESIZE 121
#define WORDSIZE 20

/*指令种类*/
typedef enum{
	opclRR, 
	opclRM,
	opclRA
}OPCLASS;

typedef enum{
	/*RR 种类指令*/
	opHALT,
	opIN,
	opOUT,
	opADD,
	opSUB,
	opMUL,
	opDIV,
	opRRLim, // 定位专用

	/*RM 种类指令*/
	opLD, //reg(r) = mem( d + reg(s) )
	opST, //
	opRMLim,

	/*RA 指令*/
	opLDA,
	opLDC,
	opJLT,
	opJLE,
	opJGT,
	opJGE,
	opJEQ,
	opJNE,
	opRALim
}OPCODE;//操作码

typedef enum{
	srOKAY,
	srHALT,
	srIMEM_ERR,
	srDMEM_ERR,
	srZERODIVIDE
}STEPRESULT;//每一步执行后的结果，也是执行情况描述

typedef struct{
	int iop;
	int iarg1;
	int iarg2;
	int iarg3;
}INSTRUCTION;//指令结构

/*参数变量*/
int iloc = 0; 
int dloc = 0;
int traceflag = FALSE;//保持代码追踪,每次执行会打印执行的代码行
int icountflag = FALSE;//打印执行的代码行数

INSTRUCTION iMem[IADDR_SIZE];//指令结构体的总长度
int dMem[DADDR_SIZE];//数据区不用特别的结构，用整数数组就行了
int reg[NO_REGS]; //寄存器也用整数数组实现

char* opCodeTab[] = {
	"HALT", "IN", "OUT", "ADD", "SUB", "MUL", "DIV", "????",
	"LD", "ST", "????",
	"LDA", "LDC", "JLT", "JLE", "JGT", "JGE", "JEQ", "JNE", "????"
};
char* stepResultTab[] = {
	"OK", "Halted", "Instruction Memory Fault",
	"Data Memory Fault", "Division by 0" //将枚举值和字符串联系在了一起
};

//文件读取
char pgmName[20];
FILE* pgm = NULL;

//取命令时候用到的数据结构
char in_Line[LINESIZE]; //保存commend读入的一行
int lineLen;
int inCol;
int num; //记录cmd中的步数等信息
char word[WORDSIZE];
char ch;

int done;

int opClass(int c){
	if (c <= opRRLim)
		return (opclRR);
	else if (c <= opRMLim)
		return (opclRM);
	else
		return (opclRA);
}

/*根据这个这个指令的位置从iMem中打印这条指令*/
void writeInstruction(int loc){
	printf("%5d: ", loc);
	if ((loc >= 0) && (IADDR_SIZE)){
		printf("%6s%3d,", opCodeTab[iMem[loc].iop], iMem[loc].iarg1);
		switch (opClass(iMem[loc].iop)){//根据指令的类型，选择打印方式
		case opclRR:
			printf("%1d,%1d", iMem[loc].iarg2, iMem[loc].iarg3);
			break;
		case opclRM:
		case opclRA:
			printf("%3d(%1d)", iMem[loc].iarg2, iMem[loc].iarg3);
		}
		printf("\n");
	}

}

/*接下来的这些函数都是工具，用来读入文本中的指令并解析到iMem当中的。还可以用来充当命令行的指令的解析工具
大框架流程都能想明白，实际上就是出错处理和一些字符串处理工具函数需要事先定义好需求，这样的话才好设计，我可以按思路
写出全篇代码的接口来。
工具用到的全局变量：
inCol 当前in_Line 的下一个位置
lineLen in_Line长度
ch 获取的字符
word 获取的单词
num 获取的数字
*/
void getCh(void){
	if (++inCol < lineLen)
		ch = in_Line[inCol];
	else ch = ' ';
}

int nonBlank(void){//非空判断，空就是false， 非空就是 true
	while ((inCol < lineLen) && (in_Line[inCol] == ' '))
		inCol++;
	if (inCol < lineLen){
		ch = in_Line[inCol];
		return TRUE;
	}
	else{
		ch = ' ';
		return FALSE;
	}
}

int getNum(void){
	int sign;
	int term;
	int temp = FALSE;
	num = 0;
	do{
		sign = 1;
		while (nonBlank() && ((ch == '+' || ch == '-'))){
			temp = FALSE;
			if (ch == '-')
				sign = -sign;
			getCh();
		}
		term = 0;
		nonBlank();
		while (isdigit(ch)){
			temp = TRUE;
			term = term * 10 + (ch - '0');
			getCh();
		}
		num = num + (term * sign);
	} while ((nonBlank()) && ((ch == '+') || (ch == '-')));
	return temp;
}

int getWord(void){
	int temp = FALSE;
	int length = 0;
	if (nonBlank()){ //非空的话读入一个单词
		while (isalnum(ch)){
			if (length < WORDSIZE - 1)
				word[length++] = ch;
			getCh();
		}
		word[length] = '\0';
		temp = (length != 0);
	}
	return temp;
}

int skipCh(char c){
	int temp = FALSE;
	if (nonBlank() && (ch == c)){
		getCh();
		temp = TRUE;
	}
	return temp;
}

int atEOL(void){//空就是true,非空是false
	return (!nonBlank());
}

int error(char* msg, int lineNo, int instNo){
	printf("Line %d", lineNo);
	if (instNo >= 0)
		printf("Instrcution %d", instNo);
	printf("   %s\n", msg);
	return FALSE;
}

/*这个函数的作用就相当于程序加载器，将文件中的代码加载到内存当中，方便今后执行
占用代码行数最多的就是出错处理，真正的核心代码很少*/
int readInstructions(void){
	OPCODE op;
	int arg1, arg2, arg3;
	int loc, regNo, lineNo;
	//初始化寄存器的值为零
	for (regNo = 0; regNo < NO_REGS; regNo++)
		reg[regNo] = 0;
	dMem[0] = DADDR_SIZE - 1;//TM机器的规则，和.tm代码的对接也很清楚
	for (loc = 1; loc < DADDR_SIZE; loc++){
		dMem[loc] = 0;
	}
	//初始化代码区
	for (loc = 0; loc < IADDR_SIZE;loc++){
		iMem[loc].iop = opHALT;
		iMem[loc].iarg1 = 0;
		iMem[loc].iarg2 = 0;
		iMem[loc].iarg3 = 0;
	}
	lineNo = 0;//解析文件中的文本
	while (!feof(pgm)){
		fgets(in_Line, LINESIZE - 2, pgm);
		inCol = 0;
		lineNo++;
		lineLen = strlen(in_Line) - 1;
		if (in_Line[lineLen] == '\n')
			in_Line[lineLen] = '\0';
		else
			in_Line[++lineLen] = '\0';
		//处理,如果是非空行且不是注释
		if ((nonBlank()) && (in_Line[inCol] != '*')){
			if (!getNum())//每行的第一个应该是数字。突然觉得出错检查很普遍，基本上都要进行
				return error("Bad location", lineNo, -1);//出错就返回
			loc = num;//这个数字就是指令的位置
			if (loc > IADDR_SIZE)
				return error("Location too large", lineNo, loc);//指令地址太大了，超出存储器大小
			if (!skipCh(':'))
				return error("Missing colon", lineNo, loc);//冒号丢失
			if (!getWord())
				return error("Missing opcode", lineNo, loc);//操作码丢失
			op = opHALT;//利用表的形式进行查找指令类型
			while ((op < opRALim) && (strncmp(opCodeTab[op], (char*)word, 4) != 0))
				op = (OPCODE)((int)op + 1);//枚举类型的运算，需要转换数据类型
			if (strncmp(opCodeTab[op], (char*)word, 4) != 0)
				return error("Illegal opcode", lineNo, loc);
			switch (opClass(op)){
			case opclRR:
				if ((!getNum()) || (num < 0) || (num >= NO_REGS))
					return error("bad first register", lineNo, loc);
				arg1 = num;
				if (!skipCh(','))
					return error("Missing comma", lineNo, loc);
				if ((!getNum()) || (num < 0) || (num >= NO_REGS))
					return error("bad second register", lineNo, loc);
				arg2 = num;
				if (!skipCh(','))
					return error("Missing comma", lineNo, loc);
				if ((!getNum()) || (num < 0) || (num >= NO_REGS))
					return error("bad third register", lineNo, loc);
				arg3 = num;
				break;
			case opclRM:
			case opclRA:
				if ((!getNum()) || (num < 0) || (num >= NO_REGS))
					return error("bad first register", lineNo, loc);
				arg1 = num;
				if (!skipCh(','))
					return error("Missing comma", lineNo, loc);
				if (!getNum())
					return error("Bad displacement", lineNo, loc);
				arg2 = num;
				if (!skipCh('(') && !skipCh(','))
					return error("Missing LParen", lineNo, loc);
				if ((!getNum()) || (num < 0) || (num >= NO_REGS))
					return error("bad second register", lineNo, loc);
				arg3 = num;
				break;
			}
			iMem[loc].iop = op;
			iMem[loc].iarg1 = arg1;
			iMem[loc].iarg2 = arg2;
			iMem[loc].iarg3 = arg3;
		}
	}
	return TRUE;
}

/*执行函数，每次调用相当于CPU的单步执行，返回的是执行后的状态
具体是怎么实现的呢？看一看，虚拟机怎么实现输入输出的，实际上还是调用c库函数，再进一步，是利用
真实操作系统的系统调用实现的。可以看出每条指令的执行，并不对应真是机器的一条指令，因此这样设计的效率
简直不能再低。不过没有贬低的意思。设计是做教学用的。*/
STEPRESULT stepTM(void){
	INSTRUCTION currentinstruction; 
	int pc;
	int r, s, t, m;
	int ok;
	pc = reg[PC_REG];//获取当前指令的地址
	if ((pc<0)||(pc>IADDR_SIZE))
		return srIMEM_ERR;
	reg[PC_REG] = pc + 1;//指令自动加一，之后的转移指令有可能修改reg[PC_REG]值
	//取指操作
	currentinstruction = iMem[pc];
	switch (opClass(currentinstruction.iop))
	{
	case opclRR:
		r = currentinstruction.iarg1;
		s = currentinstruction.iarg2;
		t = currentinstruction.iarg3;
		break;
	case opclRM:
		r = currentinstruction.iarg1;
		s = currentinstruction.iarg3;
		m = currentinstruction.iarg2 + reg[s];
		if ((m<0) || (m>DADDR_SIZE))
			return srDMEM_ERR;
	case opclRA:
		r = currentinstruction.iarg1;
		s = currentinstruction.iarg3;
		m = currentinstruction.iarg2 + reg[s];
		break;
	default:
		break;
	}//取指操作结束
	/*
	根据操作码执行操作在这里看到的操作都是一个个指令，所以体会一下。
	这里你是看不到任何变量的，只有寄存器，内存区，之前的高级语言到这里被分解了
	*/
	switch (currentinstruction.iop){
	case opHALT:
		printf("HALT: %1d,%1d,%1d\n", r, s, t);
		return srHALT;
	case opIN:
		do{
			printf("Enter value for IN instruction: ");
			fflush(stdin);
			gets_s(in_Line);
			lineLen = strlen(in_Line);
			inCol = 0;
			ok = getNum();
			if (!ok)
				printf("Illegal value\n");//指出错误，不试图纠正，因为这不是TM的任务
			else
				reg[r] = num; //执行的操作
		} while (!ok);
		break;
	case opOUT:
		printf("OUT instruction prints: %d\n", reg[r]);
		break;
	case opADD:reg[r] = reg[s] + reg[t]; break;
	case opSUB:reg[r] = reg[s] - reg[t]; break;
	case opMUL:reg[r] = reg[s] * reg[t]; break;
	case opDIV:if (reg[t] != 0)reg[r] = reg[s] * reg[t];else return srZERODIVIDE; break;

	case opLD:reg[r] = dMem[m];break;
	case opST:dMem[m] = reg[r];break;

	case opLDA: reg[r] = m; break;
	case opLDC: reg[r] = currentinstruction.iarg2; break;
	case opJLT: if (reg[r] < 0) reg[PC_REG] = m; break;
	case opJLE: if (reg[r] <= 0) reg[PC_REG] = m; break;
	case opJGT: if (reg[r] > 0) reg[PC_REG] = m; break;
	case opJGE: if (reg[r] >= 0) reg[PC_REG] = m; break;
	case opJEQ: if (reg[r] == 0) reg[PC_REG] = m; break;
	case opJNE: if (reg[r] != 0) reg[PC_REG] = m; break;
	}
	/*
	返回执行的状态
	*/
	return srOKAY;
}

/*命令解释器，分发命令并进行执行
Commands are:
	s(tep <n>		执行n(默认是1)步 TM指令
	g(o				执行TM指令直到HALT指令
	r(egs			打印寄存器的内容
	i(Mem <b<n>>	打印从b开始的n个iMem位置
	d(Mem <b<n>>	打印从b开始的n个dMem位置
	t(race			切换指令跟踪 ： toggle instructions trace
	p(rint			toggle print of total instructions executed ('go' only)
	c(lear			reset simulator for new execution of program
	h(elp			打印这个表
	q(uit			终止执行TM
	
*/
int doComment(void){
	char cmd;
	int stepcnt = 0, i; // stepcnt表示执行的步数，i循环使用
	int printcnt; //需要打印的指令个数
	int stepResult;
	int regNo, loc;
	do{
		printf("Enter commemd: ");
		fflush(stdin);//清除缓存
		gets_s(in_Line);//读入指令行
		//原文没有lineLen计算
		lineLen = strlen(in_Line);
		inCol = 0;
	} while (!getWord());//只取得不为空格的第一个词

	cmd = word[0];//只取第一个字符
	switch (cmd){
	case 't':
		traceflag = !traceflag;
		printf("Tracing now ");
		if (traceflag)
			printf("on.\n");
		else
			printf("off.\n");
		break;
	case 'h':
		printf("s(tep <n>       执行n(默认是1)步TM指令\n");
		printf("g(o             执行指令直到结束\n");
		printf("r(egs           打印寄存器\n");
		printf("i(Mem <b<n>>    打印从b开始的n个iMem内容\n");
		printf("d(Mem <b<n>>    打印从b开始的n个dMem内容\n");
		printf("t(race          切换指令跟踪开关\n");
		printf("p(rint          toggle print of total instructions executed ('go' only)\n");
		printf("c(lear          重启机器\n");
		printf("h(elp           有关命令行的帮助\n");
		printf("q(uit           终止执行\n");
		break;
	case 'p':
		icountflag = !icountflag;
		printf("printing instruction count now ");
		if (icountflag)
			printf("on.\n");
		else
			printf("off.\n");
		break;
	case 's':
		if (atEOL())//在结尾说明没有参数了，就默认执行一步
			stepcnt = 1;
		else
		if (getNum())
			stepcnt = abs(num);
		else
			printf("Step count?\n");//不是数字，出错的命令
		break;
	case 'g':
		stepcnt = 1;
		break;
	case 'r':
		for (i = 0; i < NO_REGS; i++){
			printf("<reg%1d:%4d> ", i,reg[i]);
			if ((i % 4) == 3)
				printf("\n");//四个一行显示
		}
		break;
	case 'i':
		printcnt = 1;
		if (getNum()){
			iloc = num;//第一个数记录从哪里开始打印
			if (getNum())
				printcnt = num;//第二个数记录打印的个数
		}
		if (!atEOL())//出错
			printf("Instruction location?\n");
		else{//没有错就打印
			while ((iloc >= 0) && (iloc<IADDR_SIZE) && (printcnt>0)){
				writeInstruction(iloc);
				iloc++;
				printcnt--;
			}
		}
		break;
	case 'd':
		printcnt = 1;
		if (getNum()){
			dloc = num;//第一个数记录从哪里开始打印
			if (getNum())
				printcnt = num;//第二个数记录打印的个数
		}
		if (!atEOL())//出错
			printf("Data location?\n");
		else{//没有错就打印
			while ((dloc >= 0) && (dloc<DADDR_SIZE) && (printcnt>0)){
				printf("%5d: %5d\n", dloc, dMem[dloc]);
				dloc++;
				printcnt--;
			}
		}
		break;
	case 'c':
		iloc = 0;
		dloc = 0;
		stepcnt = 0;
		//寄存器全部清空
		for (regNo = 0; regNo < NO_REGS; regNo++){
			reg[regNo] = 0;
		}
		//数据区全部清空重置
		dMem[0] = DADDR_SIZE - 1;
		for (loc = 1; loc < DADDR_SIZE; loc++){
			dMem[loc] = 0;
		}
		break;
	case 'q':
		return FALSE;//命令行退出了
	default:
		printf("commend %c unknown.\n", cmd);
		break;
	}
	/*结束了命令解析，开始执行。里是最骚的，执行状态强制被改为正确，之前如果出错的话，也会接着执行
	不过仔细想一想，真正的操作系统基本都是这样干的，自己的进程出错，要么退出进程，要么接着执行出错的程序，不可能把
	操作系统关掉啊*/
	stepResult = srOKAY;
	if (stepcnt > 0){
		if (cmd == 'g'){//全部执行
			stepcnt = 0;
			while (stepResult == srOKAY){
				iloc = reg[PC_REG];
				if (traceflag) //追踪标志开启的话，就打印当前指令
					writeInstruction(iloc);
				stepResult = stepTM();//执行一步
				stepcnt++;
			}
			if (icountflag)//记录代码执行的条数
				printf("number of instructions executed = %d\n", stepcnt);
		}
		else{//执行stepcnt步
			while ((stepcnt > 0) && (stepResult == srOKAY)){
				iloc = reg[PC_REG];
				if (traceflag) //追踪标志开启的话，就打印当前指令
					writeInstruction(iloc);
				stepResult = stepTM();//执行一步
				stepcnt--;
			}
		}
		printf("%s\n", stepResultTab[stepResult]);	//输出执行后的状态
	}
	return TRUE;//命令执行了一次，还要执行
}


/*
从主函数剖析整个执行
*/
int main(int argc, char* argv[])
{
	//读入文件
	if (argc != 2){
		printf("usage: %s <filename\n>", argv[0]);
		exit(1);
	}
	int argvlen = strlen(argv[1]);
	strcpy_s(pgmName, argvlen + 1, argv[1]);
	if (strchr(pgmName, '.') == NULL)
		strcat_s(pgmName,3, ".tm");
	fopen_s(&pgm,pgmName, "r");
	if (pgm == NULL){
		printf("file '%s' not found!\n", pgmName);
		exit(1);
	}

	//读入程序进iMem
	if (!readInstructions())
		exit(1);
	//命令解释
	printf("TM simulation (enter h for help)...\n");//解释器怎么做的，这里有很好的示例
	do
	done = !doComment();
	while (!done);

	printf("Aimulation done.\n");
	return 0;
}

