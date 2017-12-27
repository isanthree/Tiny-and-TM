#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
/*
说句题外话，头文件的包含顺序也是非常重要的，parse.h用到了globals.h的结构体定义，所以一定要放在后面，否者会出错。报
结构体没有定义
*/
/* TINY的EBNF文法规则,带分号的是终结符，其他的是非终结符,递归下降方法要求文法符合EBNF规则，这样的话
好编写算法。
程序的编写和自己的文法是分不开关系的。像tiny语言，有多少语句类型，就要有多少的处理，但是单体上每个语句的处理方式又
都是类似的，都遵循着相同的模式。

program			->	stmt-sequence 
stmt-sequence	->	statement{; statement} 最后一个语句不可以带分号
statement		->	if-stmt|repeat-stmt|assign-stmt|read-stmt|write-stmt	语句的种类有五种
if-stmt			->	if exp then stmt-squence [else stmt-swquence] end	if语句有一个可选的else分支，支持最近嵌套规则
repeat-stmt		->	repeat stmt-sequence until exp	循环体中的最后一个语句是不会有分号的
assign-stmt		->	identifier := exp 赋值语句
read-stmt		->	read identifier 从输入读值
write-stmt		->	write exp 写一个表达式的值到输出
exp				->	simple-exp [comparison-op simple-exp] 表达式的种类有三种：算符表达式op,常量表达式const,标识符表达式id
comparison-op	->	<|= 以下的规则是为了匹配左结合性和优先级而设计的
simple-exp		->	term{addop term}
addop			->	+|-
term			->	factor{mulop factor}
mulop			->	*|/
factor			->	(exp)|number|identifier
*/
static TokenType token;//当前的记号,此分析过程采用的是递归下降程序
extern int Error;//main函数中的变量，用于判断是否文法分析出错
//用到的函数调用
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

//错误打印
static void syntaxError(char* message){
	fprintf(listing, "\n>>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = TRUE;//主函数要用到这个变量，决定是否打印符号表等结构。
}

static void match(TokenType expected){
	if (token == expected)
		token = getToken();//消耗一个记号，将getToken()当做parse()的一部分来用了。
	else{
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		fprintf(listing, "    ");
	}
}

TreeNode* stmt_sequence(void){
	TreeNode* t = statement();//先行的节点
	TreeNode* p = t;//当前的工作节点
	//文件结尾，或者是end else until 标识符
	while ((token != ENDFILE) && (token != END) && (token != ELSE) && (token != UNTIL)){
		//里面嵌套着语句
		TreeNode* q;
		match(SEMI);
		q = statement();
		if (q != NULL){
			if (t == NULL) t = p = q; //这种情况下，指针都指向最后一个不为空的语句
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	//函数结束后，会产生一个语句链表，p始终指向最后一个语句
	return t;
}

/*
语句分派
返回值就是各个语句类型的节点
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
		printToken(token, tokenString);//tokenString为全局变量，存储当前token的串值
		token = getToken();
		break;
	}
	return t;
}
/*下面的五大语句类型
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
*/
static TreeNode* if_stmt(void){
	TreeNode* t = newStmtNode(IfK);//构造类型
	match(IF);
	if (t != NULL)
		t->child[0] = exp();//第一个孩子指向表达式
	match(THEN);
	if (t != NULL)
		t->child[1] = stmt_sequence();//第二个孩子是if块，语句序列
	if (token == ELSE){//每次递归下降，都会消耗相应的字符
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
	if ((t != NULL) && (token == ID)) //赋值表达式一开始是一个ID,将ID值写入节点的name属性
		t->attr.name = copyString(tokenString);
	match(ID);//消耗ID
	match(ASSIGN);
	if (t != NULL)
		t->child[0] = exp();//赋值语句的孩子是一个表达式
	return t;
}

static TreeNode* read_stmt(void){
	TreeNode* t = newStmtNode(ReadK);
	match(READ);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);//read语句接收一个标识符的值
	return t;
}

static TreeNode* write_stmt(void){
	TreeNode* t = newStmtNode(WriteK);
	match(WRITE);
	if (t != NULL)
		t->child[0] = exp();//write语句接收一个表达式的值
	return t;
}

/*
表达式类型文法
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);
*/
static TreeNode* exp(void){
	TreeNode* t = simple_exp();
	//表达式接收LT或者是EQ表示是一个关系表达式
	if ((token == LT) || (token == EQ)){
		TreeNode* p = newExpNode(OpK);//生成关系表达式节点
		if (p != NULL){
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);//消耗掉这个记号。往下走是关系表达式的另一边
		if (t != NULL)
			t->child[1] = simple_exp();
	}
	return t;
}

static TreeNode* simple_exp(void){
	TreeNode* t = term();
	while ((token == PLUS) || (token == MINUS)){
		TreeNode* p = newExpNode(OpK);//简单表达式也是一个树，根节点是操作
		if (p != NULL){
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			t->child[1] = term();//左右孩子都是term
		}
	}
	return t;
}

static TreeNode* term(void){
	TreeNode* t = factor();
	while ((token == TIMES) || (token == OVER)){
		TreeNode* p = newExpNode(OpK);
		if (p != NULL){
			p->child[0] = t;//第一个孩子
			p->attr.op = token;
			t = p;//将t指向操作符节点
			match(token);
			p->child[1] = factor();
		}
	}
	return t;
}

static TreeNode* factor(void){
	TreeNode* t = NULL;
	switch (token){
	case NUM://构造一个常量表达式节点
		t = newExpNode(ConstK);
		if ((t != NULL) && (token == NUM))
			t->attr.val = atoi(tokenString); // atoi()将字符创转换成整型
		match(NUM);
		break;
	case ID:
		t = newExpNode(IdK);
		if ((t != NULL) && (token == ID))
			t->attr.name = copyString(tokenString);
		match(ID);
		break;
	case LPAREN: //左括号
		match(LPAREN);
		t = exp();
		match(RPAREN);
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken(); // 消耗掉未知的字符，只有在出错的时候才显示调用getToken(),不出错用match()
		break;
	}
	return t;
}

/*主函数*/
TreeNode* parse(void){
	TreeNode* t; //返回一颗树
	token = getToken();//第一个记号
	t = stmt_sequence(); // 返回的实际是一个语句序列
	if (token != ENDFILE)//文件尾出错,对应的是stmt_sequence()函数中的判断语句，当为ENDFILE时候，返回NULL,语句序列停止
		syntaxError("code ends before file\n");
	return t;
}