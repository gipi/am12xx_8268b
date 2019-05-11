#ifndef Table_H
#define Table_H

extern unsigned short *uni_sort;
extern unsigned char *gb_sort;
extern unsigned short *uni_table;
void table_init();
void table_exit();

int Gb2312ToUni(unsigned short * pOut, int OutLen, char *pIn, int InLen);
void Gb2312ToUtf8(char * pOut,char *pText, int pLen);
void Utf8ToGb2312(char *pOut, char *pText, int pLen);

#endif
