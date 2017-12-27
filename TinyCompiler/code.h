/*
代码为微型编译器发布实用程序并与TM机器接口
*/
#ifndef _CODE_H_
#define _CODE_H_
//程序计数器
#define pc 7
//内存指针，指向内存的顶部，有关临时存储 memory pointer
#define mp 6
//全局变量指针，指向存储器的底部，指向全局变量
#define gp 5
//累加器
#define ac 0
#define ac1 1
extern int TraceCode;

/*代码生成函数，是TM的接口，只负责生成指令，认真辨别它是怎么和Cgen函数分开功能的*/

//生成RO指令
void emitRO(char* op, int r, int s, int t, char *c);
//生成RM指令
void emitRM(char* op, int r, int d, int s, char *c);
//用于跳过将来要反填的一些位置 emitSkip(0)不跳过位置，得到当前位置将来可以用
int emitSkip(int howMany);
//回填这些位置
void emitBackup(int loc);
//
void emitRestore(void);

void emitComment(char* c);

void emitRM_Abs(char* op, int r, int a, char* c);

#endif