#ifndef _SCAN_H_
#define _SCAN_H_

#define MAXTOKENLEN 40

extern char tokenString[MAXTOKENLEN + 1];

extern int EchoSource;
extern int TraceScan;
//��source��ȡ����һ���Ǻ�
TokenType getToken(void);

#endif