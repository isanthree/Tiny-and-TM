#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/*
符号表信息作为合成属性，通过后序遍历完成建立。
类型检查作为继承属性，通过前序遍历完成检查。
*/
static int location = 0;
extern int Error;
//标准的遍历函数:注意这里函数指针的作用，形参是函数指针，可以将函数传递进来
static void traverse(TreeNode* t, void(*preProc)(TreeNode*), void(*postProc)(TreeNode*)){
	if (t != NULL){
		preProc(t);
		{
			int i;
			for (i = 0; i < MAXCHILDREN; i++)
				traverse(t->child[i], preProc, postProc);
		}
		postProc(t);
		traverse(t->sibling, preProc, postProc);
	}
}

// nullProc,什么也不做，用来统一前后遍历操作。可以用一个遍历函数进行前序遍历和后序遍历，具体实现看后面的函数
static void nullProc(TreeNode* t){
	if (t == NULL) return;
	else return;
}

//插入节点的操作,当遍历到一个节点之后，分析这个节点的类型，调用st_insert函数插入到符号表
static void insertNode(TreeNode* t){
	switch (t->nodekind){
	case StmtK://语句类型：Assign和read中才有id, write语句中带有exp类型。
		switch (t->kind.stmt){
		case AssignK:
		case ReadK:
			if (st_lookup(t->attr.name) == -1)//不在符号表
				st_insert(t->attr.name, t->lineno, location++);
			else//在符号表中，就不用添加loc位置了
				st_insert(t->attr.name, t->lineno, 0);
			break;
		default:
			break;
		}
		break;
	case ExpK://表达式类型
		switch (t->kind.exp){
		case IdK:
			if (st_lookup(t->attr.name) == -1)
				st_insert(t->attr.name, t->lineno, location++);
			else
				st_insert(t->attr.name, t->lineno, 0);
			break;
		default:
			break;
		}
		break;
	default://默认
		break;
		}
}

//建立符号表总函数
void buildSymtab(TreeNode* syntaxTree){
	//这个时候把insetNode当做前序遍历的动作
	traverse(syntaxTree, insertNode, nullProc);
	if (TraceAnalyze){
		fprintf(listing, "\nSymbol table:\n\n");
		printSymTab(listing);
	}
}

static void typeError(TreeNode* t, char* message){
	fprintf(listing, "type error at line %d: %s\n", t->lineno, message);
	Error = TRUE;//main函数中的变量，在parse.cpp中和本文件中都有引用，表示当前阶段是否出错的指示标志
}

static void checkNode(TreeNode* t){
	switch (t->nodekind){
	//ExpK
	case ExpK:
		switch (t->kind.exp){//具体的表达式类型
		case OpK:
			if ((t->child[0]->type != Integer) || (t->child[1]->type != Integer))
				typeError(t, "op applied to non-integer");//前序遍历
			if ((t->attr.op == EQ) || (t->attr.op == LT))// 对操作节点赋值类型
				t->type = Boolean;
			else
				t->type = Integer;
			break;
		case ConstK:
		case IdK:
			t->type = Integer;
			break;
		default:
			break;
		}
		break;
	//StmtK
	case StmtK:
		switch (t->kind.stmt){
		case IfK:
			if (t->child[0]->type == Integer)//语义要求if表达式的判断必须是boolean类型的
				typeError(t->child[0], "if test is not Boolean");
			break;
		case AssignK:
			if (t->child[0]->type != Integer)//赋值表达式必须是一个整型
				typeError(t->child[0], "assignment of non-integer value");
			break;
		case WriteK:
			if (t->child[0]->type!=Integer)//写语句的表达式类型必须是整型
				typeError(t->child[0], "write of non-integer value");
			break;
		case RepeatK:
			if (t->child[1]->type == Integer)//循环的表达式类型必须是整型
				typeError(t->child[1], "repeat of not boolean");
			break;
		default:
			break;
		}
		break;
	//default
	default:
		break;
	}
}

//类型检查函数,传递进去的是后序遍历类型检查的动作
void typeCheck(TreeNode* syntaxTree){
	traverse(syntaxTree, nullProc, checkNode);
}