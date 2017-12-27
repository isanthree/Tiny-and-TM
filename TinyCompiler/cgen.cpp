/*
code.cpp的代码没有完全看懂，在这里能不能看懂呢？
差不多已经看懂了，关键的技术点是在跳转的时候，如何记录需要回填的地址呢，这里采用的临时变量记录的方式savedLoc。
对于符号表的利用，实际上，内存的分配在符号表构建的时候已经完成了，现在需要把符号和地址联系起来一起编写到代码里。
可以看到对符号的操作，实际上就是对全局变量地址内容的操作，把变量和地址联系，变量值和地址内容联系起来。
编译器针对TM机器，也编写了Standard prelude 标准的前部分，实际上相当于编译器初始化运行环境，是不是这样呢。就好比c语言中的这个区的初始化
如果要用到栈和堆结构的话，这个Standard prelude或许需要初始化栈指针和初始化管理堆的操作，简单的是这里并没有这样的操作。
唯一需要栈的地方是表达式求值，但是这点运用局部变量tmpOffset就可以解决了。
运行时环境需要管理这些堆啊，栈啊，全局变量什么的。可以类比c内存管理。
*/
#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/*临时存储器的偏移量，每次临时存储时递减，当再次装入时递增。*/
static int tmpOffset = 0;

static void cGen(TreeNode* tree);

static void genStmt(TreeNode* tree){
	TreeNode *p1, *p2, *p3;
	int savedLoc1, savedLoc2, currentLoc;
	int loc;
	switch (tree->kind.stmt){
	case IfK:
		if (TraceCode)
			emitComment("-> if");
		p1 = tree->child[0];
		p2 = tree->child[1];
		p3 = tree->child[2];

		cGen(p1);
		savedLoc1 = emitSkip(1);//逻辑表达式后跳到else的部分代码，地址需要回填
		emitComment("if: jump to else belongs here");

		cGen(p2);
		savedLoc2 = emitSkip(1);//跳过else部分代码
		emitComment("if: jump to end belongs here");
		currentLoc = emitSkip(0);
		emitBackup(savedLoc1);
		emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");//修补第一个指令
		emitRestore();

		cGen(p3);
		currentLoc = emitSkip(0);
		emitBackup(savedLoc2);//修补第二个指令，因为else已经写完了
		emitRM_Abs("LDA", pc, currentLoc, "jmp to end");
		emitRestore();
		if (TraceCode)
			emitComment("<- if");
		break;

	case RepeatK:
		if (TraceCode)
			emitComment("-> repeat");
		p1 = tree->child[0];
		p2 = tree->child[1];
		savedLoc1 = emitSkip(0);
		emitComment("repeat: jump after body comes back here");

		cGen(p1);
		cGen(p2);

		emitRM_Abs("JEQ", ac, savedLoc1, "repeat: jmp back to body");
		if (TraceCode)
			emitComment("<- repeat");
		break;

	case AssignK:
		if (TraceCode)
			emitComment("-> assign");
		cGen(tree->child[0]);
		loc = st_lookup(tree->attr.name); //怎么通过符号表地位地址
		emitRM("ST", ac, loc, gp, "assign: store value");//将数值存储到全局变量中
		if (TraceCode)
			emitComment("<- assign");
		break;

	case ReadK:
		emitRO("IN", ac, 0, 0, "read integer value");
		loc = st_lookup(tree->attr.name);
		emitRM("ST", ac, loc, gp, "read:store value");//存储数值
		break;

	case WriteK:
		cGen(tree->child[0]);
		emitRO("OUT", ac, 0, 0, "write ac");
		break;

	default:
		break;

	}
}

static void genExp(TreeNode* tree){
	int loc;
	TreeNode *p1, *p2;
	switch (tree->kind.exp){
	case ConstK:
		if (TraceCode)
			emitComment("-> Const");
		emitRM("LDC", ac, tree->attr.val, 0, "load const");
		if (TraceCode)
			emitComment("<- Const");
		break;
	case IdK:
		if (TraceCode)
			emitComment("-> Id");
		loc = st_lookup(tree->attr.name);//注意id是怎么分配内存的，是在符号表中就分配了内存，然后写入指令当中
		emitRM("LD", ac, loc, gp, "load id value");
		if (TraceCode)
			emitComment("<- Id");
		break;
	case OpK:
		if (TraceCode)
			emitComment("-> Op");
		p1 = tree->child[0];
		p2 = tree->child[1];
		cGen(p1);
		emitRM("ST", ac, tmpOffset--, mp, "op: push left");//将左孩子压入计算栈中，其实tmpOffset就是一个栈
		cGen(p2);
		emitRM("LD", ac1, ++tmpOffset, mp, "op: load left");
		switch (tree->attr.op){
		case PLUS:
			emitRO("ADD", ac, ac1, ac, "op +");//临时结果放入 ac
			break;
		case MINUS:
			emitRO("SUB", ac, ac1, ac, "op -");
			break;
		case TIMES:
			emitRO("MUL", ac, ac1, ac, "op *");
			break;
		case OVER:
			emitRO("DIV", ac, ac1, ac, "op /");//表达式没有说明数据的所占字节数，反正都是整型
			break;
		case LT:
			emitRO("SUB", ac, ac1, ac, "op <");//编译器采用ac的值来判断跳转，这是编译器的策略，在生成if语句的时候也能看出，生成的语句要求测试ac的值（编写的指令，本文35行）实现跳转。
			emitRM("JLT", ac, 2, pc, "br if true"); 
			emitRM("LDC", ac, 0, ac, "false case");//失败 ac 写入 0
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");//成功的话 ac 写入 1
			break;
		case EQ:
			emitRO("SUB", ac, ac1, ac, "op ==");
			emitRM("JEQ", ac, 2, pc, "br if true");//与上一个测试条件不同，如果相等，ac 就是1，不相等，就是0
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");//ac为0的话要跳过下一句语句。因为下一句是设置ac为1的语句
			emitRM("LDC", ac, 1, ac, "true case");
		}
		if (TraceCode)
			emitComment("<- Op");
		break;

	}
}

static void cGen(TreeNode* tree){
	if (tree!= NULL){
		switch (tree->nodekind){
		case StmtK:
			genStmt(tree);
			break;
		case ExpK:
			genExp(tree);
			break;
		default:
			break;
		}
		cGen(tree->sibling);
	}
}

void codeGen(TreeNode* syntaxTree, char* codefile){
	//这一个相当于运行环境的布置，利用TM机器的特性，利用代码初始化TM中的环境
	char* s = (char*)malloc(strlen(codefile) + 7);
	strcpy(s, "File: ");
	strcat(s, codefile);
	emitComment("TINY compilation to TM code");
	emitComment(s);
	emitComment("Standard prelude:");

	emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
	emitRM("ST", ac, 0, ac, "clear location 0");
	emitComment("End of standard prelude.");

	cGen(syntaxTree);
	emitComment("end of ececution.");
	emitRO("HALT", 0, 0, 0, "");


}