#ifndef _UTIL_H_
#define _UNTIL_H_

//��ӡ�Ǻ�
void printToken(TokenType, const char*);
//ʣ�µĺ������ڻ���û���õ�
TreeNode* newStmtNode(StmtKind);
TreeNode* newExpNode(ExpKind);
char* copyString(char*);
void printTree(TreeNode*);

#endif