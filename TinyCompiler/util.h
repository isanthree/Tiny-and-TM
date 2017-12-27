#ifndef _UTIL_H_
#define _UNTIL_H_

//打印记号
void printToken(TokenType, const char*);
//剩下的函数现在还有没有用到
TreeNode* newStmtNode(StmtKind);
TreeNode* newExpNode(ExpKind);
char* copyString(char*);
void printTree(TreeNode*);

#endif