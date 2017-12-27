#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/*
TM�������
��Ȼ��������ر��ر��ª����ȴ�ܺܺõ�˵�����⣬������ԣ�������������������������ͬ��ɹ����ġ�
������Զ�������ԵĴʷ����﷨������
�������������ʵ���������л���
������ͨ������Դ���룬���������ת���ɻ�����ص�Ŀ�������߻����޹ص��м����
�������ն���Ҫ����Ŀ�����ģ���һ�����ò�����Ŀ�������ʵ��Ӳ�������ָ���
��������Ŀ������ʱ����ʵ�ֺ���һ�㣬����������ָ���������*/
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/******����*******/
#define IADDR_SIZE 1024 //ָ��洢���Ĵ�С
#define DADDR_SIZE 1024 //�������Ĵ�С
#define NO_REGS 8 //�Ĵ���������
#define PC_REG 7 //pc�Ĵ������ ����code.h�е�pc ����Ӧ

#define LINESIZE 121
#define WORDSIZE 20

/*ָ������*/
typedef enum{
	opclRR, 
	opclRM,
	opclRA
}OPCLASS;

typedef enum{
	/*RR ����ָ��*/
	opHALT,
	opIN,
	opOUT,
	opADD,
	opSUB,
	opMUL,
	opDIV,
	opRRLim, // ��λר��

	/*RM ����ָ��*/
	opLD, //reg(r) = mem( d + reg(s) )
	opST, //
	opRMLim,

	/*RA ָ��*/
	opLDA,
	opLDC,
	opJLT,
	opJLE,
	opJGT,
	opJGE,
	opJEQ,
	opJNE,
	opRALim
}OPCODE;//������

typedef enum{
	srOKAY,
	srHALT,
	srIMEM_ERR,
	srDMEM_ERR,
	srZERODIVIDE
}STEPRESULT;//ÿһ��ִ�к�Ľ����Ҳ��ִ���������

typedef struct{
	int iop;
	int iarg1;
	int iarg2;
	int iarg3;
}INSTRUCTION;//ָ��ṹ

/*��������*/
int iloc = 0; 
int dloc = 0;
int traceflag = FALSE;//���ִ���׷��,ÿ��ִ�л��ӡִ�еĴ�����
int icountflag = FALSE;//��ӡִ�еĴ�������

INSTRUCTION iMem[IADDR_SIZE];//ָ��ṹ����ܳ���
int dMem[DADDR_SIZE];//�����������ر�Ľṹ�����������������
int reg[NO_REGS]; //�Ĵ���Ҳ����������ʵ��

char* opCodeTab[] = {
	"HALT", "IN", "OUT", "ADD", "SUB", "MUL", "DIV", "????",
	"LD", "ST", "????",
	"LDA", "LDC", "JLT", "JLE", "JGT", "JGE", "JEQ", "JNE", "????"
};
char* stepResultTab[] = {
	"OK", "Halted", "Instruction Memory Fault",
	"Data Memory Fault", "Division by 0" //��ö��ֵ���ַ�����ϵ����һ��
};

//�ļ���ȡ
char pgmName[20];
FILE* pgm = NULL;

//ȡ����ʱ���õ������ݽṹ
char in_Line[LINESIZE]; //����commend�����һ��
int lineLen;
int inCol;
int num; //��¼cmd�еĲ�������Ϣ
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

/*����������ָ���λ�ô�iMem�д�ӡ����ָ��*/
void writeInstruction(int loc){
	printf("%5d: ", loc);
	if ((loc >= 0) && (IADDR_SIZE)){
		printf("%6s%3d,", opCodeTab[iMem[loc].iop], iMem[loc].iarg1);
		switch (opClass(iMem[loc].iop)){//����ָ������ͣ�ѡ���ӡ��ʽ
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

/*����������Щ�������ǹ��ߣ����������ı��е�ָ�������iMem���еġ������������䵱�����е�ָ��Ľ�������
�������̶��������ף�ʵ���Ͼ��ǳ������һЩ�ַ��������ߺ�����Ҫ���ȶ�������������Ļ��ź���ƣ��ҿ��԰�˼·
д��ȫƪ����Ľӿ�����
�����õ���ȫ�ֱ�����
inCol ��ǰin_Line ����һ��λ��
lineLen in_Line����
ch ��ȡ���ַ�
word ��ȡ�ĵ���
num ��ȡ������
*/
void getCh(void){
	if (++inCol < lineLen)
		ch = in_Line[inCol];
	else ch = ' ';
}

int nonBlank(void){//�ǿ��жϣ��վ���false�� �ǿվ��� true
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
	if (nonBlank()){ //�ǿյĻ�����һ������
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

int atEOL(void){//�վ���true,�ǿ���false
	return (!nonBlank());
}

int error(char* msg, int lineNo, int instNo){
	printf("Line %d", lineNo);
	if (instNo >= 0)
		printf("Instrcution %d", instNo);
	printf("   %s\n", msg);
	return FALSE;
}

/*������������þ��൱�ڳ�������������ļ��еĴ�����ص��ڴ浱�У�������ִ��
ռ�ô����������ľ��ǳ����������ĺ��Ĵ������*/
int readInstructions(void){
	OPCODE op;
	int arg1, arg2, arg3;
	int loc, regNo, lineNo;
	//��ʼ���Ĵ�����ֵΪ��
	for (regNo = 0; regNo < NO_REGS; regNo++)
		reg[regNo] = 0;
	dMem[0] = DADDR_SIZE - 1;//TM�����Ĺ��򣬺�.tm����ĶԽ�Ҳ�����
	for (loc = 1; loc < DADDR_SIZE; loc++){
		dMem[loc] = 0;
	}
	//��ʼ��������
	for (loc = 0; loc < IADDR_SIZE;loc++){
		iMem[loc].iop = opHALT;
		iMem[loc].iarg1 = 0;
		iMem[loc].iarg2 = 0;
		iMem[loc].iarg3 = 0;
	}
	lineNo = 0;//�����ļ��е��ı�
	while (!feof(pgm)){
		fgets(in_Line, LINESIZE - 2, pgm);
		inCol = 0;
		lineNo++;
		lineLen = strlen(in_Line) - 1;
		if (in_Line[lineLen] == '\n')
			in_Line[lineLen] = '\0';
		else
			in_Line[++lineLen] = '\0';
		//����,����Ƿǿ����Ҳ���ע��
		if ((nonBlank()) && (in_Line[inCol] != '*')){
			if (!getNum())//ÿ�еĵ�һ��Ӧ�������֡�ͻȻ���ó�������ձ飬�����϶�Ҫ����
				return error("Bad location", lineNo, -1);//����ͷ���
			loc = num;//������־���ָ���λ��
			if (loc > IADDR_SIZE)
				return error("Location too large", lineNo, loc);//ָ���ַ̫���ˣ������洢����С
			if (!skipCh(':'))
				return error("Missing colon", lineNo, loc);//ð�Ŷ�ʧ
			if (!getWord())
				return error("Missing opcode", lineNo, loc);//�����붪ʧ
			op = opHALT;//���ñ����ʽ���в���ָ������
			while ((op < opRALim) && (strncmp(opCodeTab[op], (char*)word, 4) != 0))
				op = (OPCODE)((int)op + 1);//ö�����͵����㣬��Ҫת����������
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

/*ִ�к�����ÿ�ε����൱��CPU�ĵ���ִ�У����ص���ִ�к��״̬
��������ôʵ�ֵ��أ���һ�����������ôʵ����������ģ�ʵ���ϻ��ǵ���c�⺯�����ٽ�һ����������
��ʵ����ϵͳ��ϵͳ����ʵ�ֵġ����Կ���ÿ��ָ���ִ�У�������Ӧ���ǻ�����һ��ָ����������Ƶ�Ч��
��ֱ�����ٵ͡�����û�б�͵���˼�����������ѧ�õġ�*/
STEPRESULT stepTM(void){
	INSTRUCTION currentinstruction; 
	int pc;
	int r, s, t, m;
	int ok;
	pc = reg[PC_REG];//��ȡ��ǰָ��ĵ�ַ
	if ((pc<0)||(pc>IADDR_SIZE))
		return srIMEM_ERR;
	reg[PC_REG] = pc + 1;//ָ���Զ���һ��֮���ת��ָ���п����޸�reg[PC_REG]ֵ
	//ȡָ����
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
	}//ȡָ��������
	/*
	���ݲ�����ִ�в��������￴���Ĳ�������һ����ָ��������һ�¡�
	�������ǿ������κα����ģ�ֻ�мĴ������ڴ�����֮ǰ�ĸ߼����Ե����ﱻ�ֽ���
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
				printf("Illegal value\n");//ָ�����󣬲���ͼ��������Ϊ�ⲻ��TM������
			else
				reg[r] = num; //ִ�еĲ���
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
	����ִ�е�״̬
	*/
	return srOKAY;
}

/*������������ַ��������ִ��
Commands are:
	s(tep <n>		ִ��n(Ĭ����1)�� TMָ��
	g(o				ִ��TMָ��ֱ��HALTָ��
	r(egs			��ӡ�Ĵ���������
	i(Mem <b<n>>	��ӡ��b��ʼ��n��iMemλ��
	d(Mem <b<n>>	��ӡ��b��ʼ��n��dMemλ��
	t(race			�л�ָ����� �� toggle instructions trace
	p(rint			toggle print of total instructions executed ('go' only)
	c(lear			reset simulator for new execution of program
	h(elp			��ӡ�����
	q(uit			��ִֹ��TM
	
*/
int doComment(void){
	char cmd;
	int stepcnt = 0, i; // stepcnt��ʾִ�еĲ�����iѭ��ʹ��
	int printcnt; //��Ҫ��ӡ��ָ�����
	int stepResult;
	int regNo, loc;
	do{
		printf("Enter commemd: ");
		fflush(stdin);//�������
		gets_s(in_Line);//����ָ����
		//ԭ��û��lineLen����
		lineLen = strlen(in_Line);
		inCol = 0;
	} while (!getWord());//ֻȡ�ò�Ϊ�ո�ĵ�һ����

	cmd = word[0];//ֻȡ��һ���ַ�
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
		printf("s(tep <n>       ִ��n(Ĭ����1)��TMָ��\n");
		printf("g(o             ִ��ָ��ֱ������\n");
		printf("r(egs           ��ӡ�Ĵ���\n");
		printf("i(Mem <b<n>>    ��ӡ��b��ʼ��n��iMem����\n");
		printf("d(Mem <b<n>>    ��ӡ��b��ʼ��n��dMem����\n");
		printf("t(race          �л�ָ����ٿ���\n");
		printf("p(rint          toggle print of total instructions executed ('go' only)\n");
		printf("c(lear          ��������\n");
		printf("h(elp           �й������еİ���\n");
		printf("q(uit           ��ִֹ��\n");
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
		if (atEOL())//�ڽ�β˵��û�в����ˣ���Ĭ��ִ��һ��
			stepcnt = 1;
		else
		if (getNum())
			stepcnt = abs(num);
		else
			printf("Step count?\n");//�������֣����������
		break;
	case 'g':
		stepcnt = 1;
		break;
	case 'r':
		for (i = 0; i < NO_REGS; i++){
			printf("<reg%1d:%4d> ", i,reg[i]);
			if ((i % 4) == 3)
				printf("\n");//�ĸ�һ����ʾ
		}
		break;
	case 'i':
		printcnt = 1;
		if (getNum()){
			iloc = num;//��һ������¼�����￪ʼ��ӡ
			if (getNum())
				printcnt = num;//�ڶ�������¼��ӡ�ĸ���
		}
		if (!atEOL())//����
			printf("Instruction location?\n");
		else{//û�д�ʹ�ӡ
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
			dloc = num;//��һ������¼�����￪ʼ��ӡ
			if (getNum())
				printcnt = num;//�ڶ�������¼��ӡ�ĸ���
		}
		if (!atEOL())//����
			printf("Data location?\n");
		else{//û�д�ʹ�ӡ
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
		//�Ĵ���ȫ�����
		for (regNo = 0; regNo < NO_REGS; regNo++){
			reg[regNo] = 0;
		}
		//������ȫ���������
		dMem[0] = DADDR_SIZE - 1;
		for (loc = 1; loc < DADDR_SIZE; loc++){
			dMem[loc] = 0;
		}
		break;
	case 'q':
		return FALSE;//�������˳���
	default:
		printf("commend %c unknown.\n", cmd);
		break;
	}
	/*�����������������ʼִ�С�������ɧ�ģ�ִ��״̬ǿ�Ʊ���Ϊ��ȷ��֮ǰ�������Ļ���Ҳ�����ִ��
	������ϸ��һ�룬�����Ĳ���ϵͳ�������������ɵģ��Լ��Ľ��̳���Ҫô�˳����̣�Ҫô����ִ�г���ĳ��򣬲����ܰ�
	����ϵͳ�ص���*/
	stepResult = srOKAY;
	if (stepcnt > 0){
		if (cmd == 'g'){//ȫ��ִ��
			stepcnt = 0;
			while (stepResult == srOKAY){
				iloc = reg[PC_REG];
				if (traceflag) //׷�ٱ�־�����Ļ����ʹ�ӡ��ǰָ��
					writeInstruction(iloc);
				stepResult = stepTM();//ִ��һ��
				stepcnt++;
			}
			if (icountflag)//��¼����ִ�е�����
				printf("number of instructions executed = %d\n", stepcnt);
		}
		else{//ִ��stepcnt��
			while ((stepcnt > 0) && (stepResult == srOKAY)){
				iloc = reg[PC_REG];
				if (traceflag) //׷�ٱ�־�����Ļ����ʹ�ӡ��ǰָ��
					writeInstruction(iloc);
				stepResult = stepTM();//ִ��һ��
				stepcnt--;
			}
		}
		printf("%s\n", stepResultTab[stepResult]);	//���ִ�к��״̬
	}
	return TRUE;//����ִ����һ�Σ���Ҫִ��
}


/*
����������������ִ��
*/
int main(int argc, char* argv[])
{
	//�����ļ�
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

	//��������iMem
	if (!readInstructions())
		exit(1);
	//�������
	printf("TM simulation (enter h for help)...\n");//��������ô���ģ������кܺõ�ʾ��
	do
	done = !doComment();
	while (!done);

	printf("Aimulation done.\n");
	return 0;
}

