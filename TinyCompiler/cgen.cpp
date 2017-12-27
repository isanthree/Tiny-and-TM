/*
code.cpp�Ĵ���û����ȫ�������������ܲ��ܿ����أ�
����Ѿ������ˣ��ؼ��ļ�����������ת��ʱ����μ�¼��Ҫ����ĵ�ַ�أ�������õ���ʱ������¼�ķ�ʽsavedLoc��
���ڷ��ű�����ã�ʵ���ϣ��ڴ�ķ����ڷ��ű�����ʱ���Ѿ�����ˣ�������Ҫ�ѷ��ź͵�ַ��ϵ����һ���д�������
���Կ����Է��ŵĲ�����ʵ���Ͼ��Ƕ�ȫ�ֱ�����ַ���ݵĲ������ѱ����͵�ַ��ϵ������ֵ�͵�ַ������ϵ������
���������TM������Ҳ��д��Standard prelude ��׼��ǰ���֣�ʵ�����൱�ڱ�������ʼ�����л������ǲ��������ء��ͺñ�c�����е�������ĳ�ʼ��
���Ҫ�õ�ջ�Ͷѽṹ�Ļ������Standard prelude������Ҫ��ʼ��ջָ��ͳ�ʼ������ѵĲ������򵥵������ﲢû�������Ĳ�����
Ψһ��Ҫջ�ĵط��Ǳ��ʽ��ֵ������������þֲ�����tmpOffset�Ϳ��Խ���ˡ�
����ʱ������Ҫ������Щ�Ѱ���ջ����ȫ�ֱ���ʲô�ġ��������c�ڴ����
*/
#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/*��ʱ�洢����ƫ������ÿ����ʱ�洢ʱ�ݼ������ٴ�װ��ʱ������*/
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
		savedLoc1 = emitSkip(1);//�߼����ʽ������else�Ĳ��ִ��룬��ַ��Ҫ����
		emitComment("if: jump to else belongs here");

		cGen(p2);
		savedLoc2 = emitSkip(1);//����else���ִ���
		emitComment("if: jump to end belongs here");
		currentLoc = emitSkip(0);
		emitBackup(savedLoc1);
		emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");//�޲���һ��ָ��
		emitRestore();

		cGen(p3);
		currentLoc = emitSkip(0);
		emitBackup(savedLoc2);//�޲��ڶ���ָ���Ϊelse�Ѿ�д����
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
		loc = st_lookup(tree->attr.name); //��ôͨ�����ű��λ��ַ
		emitRM("ST", ac, loc, gp, "assign: store value");//����ֵ�洢��ȫ�ֱ�����
		if (TraceCode)
			emitComment("<- assign");
		break;

	case ReadK:
		emitRO("IN", ac, 0, 0, "read integer value");
		loc = st_lookup(tree->attr.name);
		emitRM("ST", ac, loc, gp, "read:store value");//�洢��ֵ
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
		loc = st_lookup(tree->attr.name);//ע��id����ô�����ڴ�ģ����ڷ��ű��оͷ������ڴ棬Ȼ��д��ָ���
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
		emitRM("ST", ac, tmpOffset--, mp, "op: push left");//������ѹ�����ջ�У���ʵtmpOffset����һ��ջ
		cGen(p2);
		emitRM("LD", ac1, ++tmpOffset, mp, "op: load left");
		switch (tree->attr.op){
		case PLUS:
			emitRO("ADD", ac, ac1, ac, "op +");//��ʱ������� ac
			break;
		case MINUS:
			emitRO("SUB", ac, ac1, ac, "op -");
			break;
		case TIMES:
			emitRO("MUL", ac, ac1, ac, "op *");
			break;
		case OVER:
			emitRO("DIV", ac, ac1, ac, "op /");//���ʽû��˵�����ݵ���ռ�ֽ�����������������
			break;
		case LT:
			emitRO("SUB", ac, ac1, ac, "op <");//����������ac��ֵ���ж���ת�����Ǳ������Ĳ��ԣ�������if����ʱ��Ҳ�ܿ��������ɵ����Ҫ�����ac��ֵ����д��ָ�����35�У�ʵ����ת��
			emitRM("JLT", ac, 2, pc, "br if true"); 
			emitRM("LDC", ac, 0, ac, "false case");//ʧ�� ac д�� 0
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");//�ɹ��Ļ� ac д�� 1
			break;
		case EQ:
			emitRO("SUB", ac, ac1, ac, "op ==");
			emitRM("JEQ", ac, 2, pc, "br if true");//����һ������������ͬ�������ȣ�ac ����1������ȣ�����0
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");//acΪ0�Ļ�Ҫ������һ����䡣��Ϊ��һ��������acΪ1�����
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
	//��һ���൱�����л����Ĳ��ã�����TM���������ԣ����ô����ʼ��TM�еĻ���
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