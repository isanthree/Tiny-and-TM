#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/*
���ű���Ϣ��Ϊ�ϳ����ԣ�ͨ�����������ɽ�����
���ͼ����Ϊ�̳����ԣ�ͨ��ǰ�������ɼ�顣
*/
static int location = 0;
extern int Error;
//��׼�ı�������:ע�����ﺯ��ָ������ã��β��Ǻ���ָ�룬���Խ��������ݽ���
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

// nullProc,ʲôҲ����������ͳһǰ�����������������һ��������������ǰ������ͺ������������ʵ�ֿ�����ĺ���
static void nullProc(TreeNode* t){
	if (t == NULL) return;
	else return;
}

//����ڵ�Ĳ���,��������һ���ڵ�֮�󣬷�������ڵ�����ͣ�����st_insert�������뵽���ű�
static void insertNode(TreeNode* t){
	switch (t->nodekind){
	case StmtK://������ͣ�Assign��read�в���id, write����д���exp���͡�
		switch (t->kind.stmt){
		case AssignK:
		case ReadK:
			if (st_lookup(t->attr.name) == -1)//���ڷ��ű�
				st_insert(t->attr.name, t->lineno, location++);
			else//�ڷ��ű��У��Ͳ������locλ����
				st_insert(t->attr.name, t->lineno, 0);
			break;
		default:
			break;
		}
		break;
	case ExpK://���ʽ����
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
	default://Ĭ��
		break;
		}
}

//�������ű��ܺ���
void buildSymtab(TreeNode* syntaxTree){
	//���ʱ���insetNode����ǰ������Ķ���
	traverse(syntaxTree, insertNode, nullProc);
	if (TraceAnalyze){
		fprintf(listing, "\nSymbol table:\n\n");
		printSymTab(listing);
	}
}

static void typeError(TreeNode* t, char* message){
	fprintf(listing, "type error at line %d: %s\n", t->lineno, message);
	Error = TRUE;//main�����еı�������parse.cpp�кͱ��ļ��ж������ã���ʾ��ǰ�׶��Ƿ�����ָʾ��־
}

static void checkNode(TreeNode* t){
	switch (t->nodekind){
	//ExpK
	case ExpK:
		switch (t->kind.exp){//����ı��ʽ����
		case OpK:
			if ((t->child[0]->type != Integer) || (t->child[1]->type != Integer))
				typeError(t, "op applied to non-integer");//ǰ�����
			if ((t->attr.op == EQ) || (t->attr.op == LT))// �Բ����ڵ㸳ֵ����
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
			if (t->child[0]->type == Integer)//����Ҫ��if���ʽ���жϱ�����boolean���͵�
				typeError(t->child[0], "if test is not Boolean");
			break;
		case AssignK:
			if (t->child[0]->type != Integer)//��ֵ���ʽ������һ������
				typeError(t->child[0], "assignment of non-integer value");
			break;
		case WriteK:
			if (t->child[0]->type!=Integer)//д���ı��ʽ���ͱ���������
				typeError(t->child[0], "write of non-integer value");
			break;
		case RepeatK:
			if (t->child[1]->type == Integer)//ѭ���ı��ʽ���ͱ���������
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

//���ͼ�麯��,���ݽ�ȥ���Ǻ���������ͼ��Ķ���
void typeCheck(TreeNode* syntaxTree){
	traverse(syntaxTree, nullProc, checkNode);
}