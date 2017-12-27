#include "globals.h"
#include "code.h"

//当前指令发送的TM位置号码
static int emitLoc = 0;
//迄今为止生成的最高TM位置用于连接
static int highEmitLoc = 0;

//打印注释
void emitComment(char* c){
	if (TraceCode)
		fprintf(code, "* %s\n", c);
}

void emitRO(char* op, int r, int s, int t, char* c){
	fprintf(code, "%3d:  %5s  %d,%d,%d", emitLoc++, op, r, s, t);
	if (TraceCode)
		fprintf(code, "\t%s", c); fprintf(code, "\n");
	if (highEmitLoc < emitLoc)
		highEmitLoc = emitLoc;
}

void emitRM(char* op, int r, int d, int s, char* c){
	fprintf(code, "%3d:  %5s  %d,%d(%d) ", emitLoc++, op, r, d, s);
	if (TraceCode)
		fprintf(code, "\t%s", c);
	fprintf(code,"\n");
	if (highEmitLoc < emitLoc)//hignEmitLoc始终指向最大的标号
		highEmitLoc = emitLoc;
}

int emitSkip(int howMany){//仅仅是改动标号的值
	int i = emitLoc;
	emitLoc += howMany;
	if (highEmitLoc < emitLoc)
		highEmitLoc = emitLoc;
	return i;
}

void emitBackup(int loc){
	if (loc>highEmitLoc)
		emitComment("Bug in emitBackup");
	emitLoc = loc;
}

void emitRestore(void){
	emitLoc = highEmitLoc;
}

void emitRM_Abs(char* op, int r, int a, char* c){
	fprintf(code, "%3d:  %5s  %d,%d(%d)", emitLoc, op, r, a - (emitLoc + 1), pc);//这个偏移是通过生成代码的当前位置和记录的回填位置做差求出来的
	++emitLoc;
	if (TraceCode)
		fprintf(code, "\t%s", c);
	fprintf(code, "\n");
	if (highEmitLoc < emitLoc);
}