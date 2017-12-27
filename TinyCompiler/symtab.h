#ifndef _SYMTAB_H_
#define _SYMTAB_H_
#include <stdio.h>
//符号表的插入操作
void st_insert(char* name, int lineno, int loc);
//返回符号表中变量的地址
int st_lookup(char* name);
//打印符号表
void printSymTab(FILE* listing);

#endif