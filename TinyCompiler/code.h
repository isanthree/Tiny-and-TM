/*
����Ϊ΢�ͱ���������ʵ�ó�����TM�����ӿ�
*/
#ifndef _CODE_H_
#define _CODE_H_
//���������
#define pc 7
//�ڴ�ָ�룬ָ���ڴ�Ķ������й���ʱ�洢 memory pointer
#define mp 6
//ȫ�ֱ���ָ�룬ָ��洢���ĵײ���ָ��ȫ�ֱ���
#define gp 5
//�ۼ���
#define ac 0
#define ac1 1
extern int TraceCode;

/*�������ɺ�������TM�Ľӿڣ�ֻ��������ָ�������������ô��Cgen�����ֿ����ܵ�*/

//����ROָ��
void emitRO(char* op, int r, int s, int t, char *c);
//����RMָ��
void emitRM(char* op, int r, int d, int s, char *c);
//������������Ҫ�����һЩλ�� emitSkip(0)������λ�ã��õ���ǰλ�ý���������
int emitSkip(int howMany);
//������Щλ��
void emitBackup(int loc);
//
void emitRestore(void);

void emitComment(char* c);

void emitRM_Abs(char* op, int r, int a, char* c);

#endif