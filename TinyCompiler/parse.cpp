#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
/*
˵�����⻰��ͷ�ļ��İ���˳��Ҳ�Ƿǳ���Ҫ�ģ�parse.h�õ���globals.h�Ľṹ�嶨�壬����һ��Ҫ���ں��棬���߻������
�ṹ��û�ж���
*/
/* TINY��EBNF�ķ�����,���ֺŵ����ս�����������Ƿ��ս��,�ݹ��½�����Ҫ���ķ�����EBNF���������Ļ�
�ñ�д�㷨��
����ı�д���Լ����ķ��Ƿֲ�����ϵ�ġ���tiny���ԣ��ж���������ͣ���Ҫ�ж��ٵĴ������ǵ�����ÿ�����Ĵ���ʽ��
�������Ƶģ�����ѭ����ͬ��ģʽ��

program			->	stmt-sequence 
stmt-sequence	->	statement{; statement} ���һ����䲻���Դ��ֺ�
statement		->	if-stmt|repeat-stmt|assign-stmt|read-stmt|write-stmt	��������������
if-stmt			->	if exp then stmt-squence [else stmt-swquence] end	if�����һ����ѡ��else��֧��֧�����Ƕ�׹���
repeat-stmt		->	repeat stmt-sequence until exp	ѭ�����е����һ������ǲ����зֺŵ�
assign-stmt		->	identifier := exp ��ֵ���
read-stmt		->	read identifier �������ֵ
write-stmt		->	write exp дһ�����ʽ��ֵ�����
exp				->	simple-exp [comparison-op simple-exp] ���ʽ�����������֣�������ʽop,�������ʽconst,��ʶ�����ʽid
comparison-op	->	<|= ���µĹ�����Ϊ��ƥ�������Ժ����ȼ�����Ƶ�
simple-exp		->	term{addop term}
addop			->	+|-
term			->	factor{mulop factor}
mulop			->	*|/
factor			->	(exp)|number|identifier
*/
static TokenType token;//��ǰ�ļǺ�,�˷������̲��õ��ǵݹ��½�����
extern int Error;//main�����еı����������ж��Ƿ��ķ���������
//�õ��ĺ�������
static TreeNode* stmt_sequence(void);
static TreeNode* statement(void);
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);

//�����ӡ
static void syntaxError(char* message){
	fprintf(listing, "\n>>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = TRUE;//������Ҫ�õ���������������Ƿ��ӡ���ű�Ƚṹ��
}

static void match(TokenType expected){
	if (token == expected)
		token = getToken();//����һ���Ǻţ���getToken()����parse()��һ���������ˡ�
	else{
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		fprintf(listing, "    ");
	}
}

TreeNode* stmt_sequence(void){
	TreeNode* t = statement();//���еĽڵ�
	TreeNode* p = t;//��ǰ�Ĺ����ڵ�
	//�ļ���β��������end else until ��ʶ��
	while ((token != ENDFILE) && (token != END) && (token != ELSE) && (token != UNTIL)){
		//����Ƕ�������
		TreeNode* q;
		match(SEMI);
		q = statement();
		if (q != NULL){
			if (t == NULL) t = p = q; //��������£�ָ�붼ָ�����һ����Ϊ�յ����
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	//���������󣬻����һ���������pʼ��ָ�����һ�����
	return t;
}

/*
������
����ֵ���Ǹ���������͵Ľڵ�
*/
static TreeNode* statement(void){
	TreeNode* t = NULL;
	switch (token){
	case IF:
		t = if_stmt();
		break;
	case REPEAT:
		t = repeat_stmt();
		break;
	case ID:
		t = assign_stmt();
		break;
	case READ:
		t = read_stmt();
		break;
	case WRITE:
		t = write_stmt();
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);//tokenStringΪȫ�ֱ������洢��ǰtoken�Ĵ�ֵ
		token = getToken();
		break;
	}
	return t;
}
/*���������������
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
*/
static TreeNode* if_stmt(void){
	TreeNode* t = newStmtNode(IfK);//��������
	match(IF);
	if (t != NULL)
		t->child[0] = exp();//��һ������ָ����ʽ
	match(THEN);
	if (t != NULL)
		t->child[1] = stmt_sequence();//�ڶ���������if�飬�������
	if (token == ELSE){//ÿ�εݹ��½�������������Ӧ���ַ�
		match(ELSE);
		if (t != NULL)
			t->child[2] = stmt_sequence();
	}
	match(END);
	return t;
}

static TreeNode* repeat_stmt(void){
	TreeNode *t = newStmtNode(RepeatK);
	match(REPEAT);
	if (t != NULL)
		t->child[0] = stmt_sequence();
	match(UNTIL);
	if (t != NULL)
		t->child[1] = exp();
	return t;
}

static TreeNode* assign_stmt(void){
	TreeNode* t = newStmtNode(AssignK);
	if ((t != NULL) && (token == ID)) //��ֵ���ʽһ��ʼ��һ��ID,��IDֵд��ڵ��name����
		t->attr.name = copyString(tokenString);
	match(ID);//����ID
	match(ASSIGN);
	if (t != NULL)
		t->child[0] = exp();//��ֵ���ĺ�����һ�����ʽ
	return t;
}

static TreeNode* read_stmt(void){
	TreeNode* t = newStmtNode(ReadK);
	match(READ);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);//read������һ����ʶ����ֵ
	return t;
}

static TreeNode* write_stmt(void){
	TreeNode* t = newStmtNode(WriteK);
	match(WRITE);
	if (t != NULL)
		t->child[0] = exp();//write������һ�����ʽ��ֵ
	return t;
}

/*
���ʽ�����ķ�
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);
*/
static TreeNode* exp(void){
	TreeNode* t = simple_exp();
	//���ʽ����LT������EQ��ʾ��һ����ϵ���ʽ
	if ((token == LT) || (token == EQ)){
		TreeNode* p = newExpNode(OpK);//���ɹ�ϵ���ʽ�ڵ�
		if (p != NULL){
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);//���ĵ�����Ǻš��������ǹ�ϵ���ʽ����һ��
		if (t != NULL)
			t->child[1] = simple_exp();
	}
	return t;
}

static TreeNode* simple_exp(void){
	TreeNode* t = term();
	while ((token == PLUS) || (token == MINUS)){
		TreeNode* p = newExpNode(OpK);//�򵥱��ʽҲ��һ���������ڵ��ǲ���
		if (p != NULL){
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			t->child[1] = term();//���Һ��Ӷ���term
		}
	}
	return t;
}

static TreeNode* term(void){
	TreeNode* t = factor();
	while ((token == TIMES) || (token == OVER)){
		TreeNode* p = newExpNode(OpK);
		if (p != NULL){
			p->child[0] = t;//��һ������
			p->attr.op = token;
			t = p;//��tָ��������ڵ�
			match(token);
			p->child[1] = factor();
		}
	}
	return t;
}

static TreeNode* factor(void){
	TreeNode* t = NULL;
	switch (token){
	case NUM://����һ���������ʽ�ڵ�
		t = newExpNode(ConstK);
		if ((t != NULL) && (token == NUM))
			t->attr.val = atoi(tokenString); // atoi()���ַ���ת��������
		match(NUM);
		break;
	case ID:
		t = newExpNode(IdK);
		if ((t != NULL) && (token == ID))
			t->attr.name = copyString(tokenString);
		match(ID);
		break;
	case LPAREN: //������
		match(LPAREN);
		t = exp();
		match(RPAREN);
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken(); // ���ĵ�δ֪���ַ���ֻ���ڳ����ʱ�����ʾ����getToken(),��������match()
		break;
	}
	return t;
}

/*������*/
TreeNode* parse(void){
	TreeNode* t; //����һ����
	token = getToken();//��һ���Ǻ�
	t = stmt_sequence(); // ���ص�ʵ����һ���������
	if (token != ENDFILE)//�ļ�β����,��Ӧ����stmt_sequence()�����е��ж���䣬��ΪENDFILEʱ�򣬷���NULL,�������ֹͣ
		syntaxError("code ends before file\n");
	return t;
}