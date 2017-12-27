#ifndef _SCAN_H_
#define _SCAN_H_

#define MAXTOKENLEN 40

extern char tokenString[MAXTOKENLEN + 1];

extern int EchoSource;
extern int TraceScan;
//从source中取得下一个记号
TokenType getToken(void);

#endif