#ifndef _SYMTAB_H_
#define _SYMTAB_H_
#include <stdio.h>
//���ű�Ĳ������
void st_insert(char* name, int lineno, int loc);
//���ط��ű��б����ĵ�ַ
int st_lookup(char* name);
//��ӡ���ű�
void printSymTab(FILE* listing);

#endif