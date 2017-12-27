#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

//hash表的长度
#define SIZE 211
#define SHIFT 4

//key值得出hash值
static int hash(char* key){
	int temp = 0;
	int i = 0;
	while (key[i] != '\0'){
		temp = (((temp << SHIFT) + key[i]) % SIZE);
		++i;
	}
	return temp;
}

//符号表要存储每个符号被引用的行数，所以创建一个节点类型，存储这样的信息
typedef struct LineListRec{
	int lineno;
	struct LineListRec* next;
}*LineList;

//桶链表中的数据结构包括：名字，内存中的位置，被引用的行号信息，还有下一个桶节点的指针
typedef struct BucketListRec{
	char* name;
	LineList lines;
	int memloc;
	struct BucketListRec* next;
}* BucketList;

//hash表，插入位置由hash函数指示
static BucketList hashTable[SIZE];

//位置信息只插入一次，第二次接下来的引用则忽略
void st_insert(char* name, int lineno, int loc){
	int h = hash(name);
	BucketList  l = hashTable[h];
	//如果这个hash位置不空，则顺序找到name 对应的节点，直到找到或者到末尾。
	while ((l != NULL) && (strcmp(name, l->name) != 0))
		l = l->next;
	//是第一次插入
	if (l == NULL){
		l = (BucketList)malloc(sizeof(struct LineListRec));
		l->name = name;
		l->lines = (LineList)malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->memloc = loc;//loc值为怎么分配的？传入的值设为多少呢？
		l->lines->next = NULL;
		l->next = hashTable[h];
		hashTable[h] = l;//将新的节点插入到开头的位置
	}
	else{//l不为空则说明不是第一次插入,只插入行号就行了
		LineList t = l->lines;
		while (t->next != NULL)
			t = t->next;
		t->next = (LineList)malloc(sizeof(struct LineListRec));
		t->next->lineno = lineno;
		t->next->next = NULL;
	}
}

//函数lookup返回一个变量名字的地址
int st_lookup(char* name){
	int h = hash(name);
	BucketList l = hashTable[h];
	while ((l != NULL)&& (strcmp(name, l->name)!= 0))
		l = l->next;
	if (l == NULL)
		return -1;//没找到
	else
		return l->memloc;
}

//打印符号表
void printSymTab(FILE* listing){
	int i;
	fprintf(listing, "Variable Name  Location  Line Numbers\n");
	fprintf(listing, "-------------  --------  ------------\n");
	for (i = 0; i < SIZE; ++i){
		if (hashTable[i] != NULL){
			BucketList l = hashTable[i];
			while (l != NULL){//打印一个hash节点的所有变量
				LineList t = l->lines;
				fprintf(listing, "%-14s ", l->name);
				fprintf(listing, "%-8d  ", l->memloc);
				while (t != NULL){
					fprintf(listing, "%4d", t->lineno);
					t = t->next;
				}
				fprintf(listing, "\n");
				l = l->next;//指针移动到下一个节点
			}
		}
	}
}