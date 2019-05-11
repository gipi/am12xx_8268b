#include "poweroff.h"



int static m_write(unsigned int filesize)
{
	int m_fp;
	unsigned int j_loop;
	unsigned int fileoffset=0;
	char w_filename[50]="/mnt/udisk/poweroff.bin";
	unsigned int w_filesize=filesize;
	char TmpBuf[32*512];
	memset(TmpBuf,0xaa,32*512);

	m_fp = open(w_filename, O_RDWR|O_CREAT, 0);
	if(m_fp < 0)
	{
		Trace_MSG("<Test Info> filename:%s,write file fail\n",w_filename);
		return 0x01;
	}
	
	for (j_loop=0;j_loop<w_filesize;j_loop+=0x4000)
	{
		fileoffset=j_loop;
		lseek(m_fp,fileoffset,SEEK_SET);
		if (w_filesize-fileoffset>0x4000)
		{
			write(m_fp,TmpBuf,0x4000);
		} 
		else
		{
			write(m_fp,TmpBuf,w_filesize-fileoffset);
		}
	}
	
    close(m_fp);

	return 0x00;

}


//return 0:ok  1:fail
int PowerOff_Protect(unsigned int sum_param,char * param_buf)
{
	int param[10];
	char t_buf[10];
	char *tp=param_buf;
	int t_i=0;

    unsigned int w_filesize;
	/*int m_fp;
	unsigned int j_loop;
	unsigned int fileoffset=0;
	char w_filename[50]="/mnt/udisk/poweroff.bin";
	unsigned int w_filesize;
	char TmpBuf[32*512];
	memset(TmpBuf,0xaa,32*512);*/
	
	for (t_i=0;t_i<sum_param;t_i++)
	{
		strcpy(t_buf,tp);
		param[t_i]=atoi(t_buf);
		tp+=strlen(tp)+1;
	}//完成参数的转化，char-->int

	if (param[0]==0)
	{
		w_filesize=5*1024*1024;
	} 
	else
	{
		w_filesize=10*1024*1024;
	}

	/*m_fp = open(w_filename, O_RDWR|O_CREAT, 0);
	if(m_fp < 0)
	{
		Trace_MSG("<Test Info> filename:%s,write file fail\n",w_filename);
		return 0x01;
	}

	for (j_loop=0;j_loop<w_filesize;j_loop+=0x4000)
	{
		fileoffset=j_loop;
		lseek(m_fp,fileoffset,SEEK_SET);
		if (w_filesize-fileoffset>0x4000)
		{
			write(m_fp,TmpBuf,0x4000);
		} 
		else
		{
			write(m_fp,TmpBuf,w_filesize-fileoffset);
		}
	}

    close(m_fp);*/

    m_write(w_filesize);
    return 0x00;

}