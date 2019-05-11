#include "destory_boot.h"

//param1:要擦除的块数 
//param2:擦除方式标志，0为指定块号擦除，1为指定范围擦除
//param3:存放块号的数组
//return:  1:ok    0:fail   

void static hex_printf2(  const INT8U* pData, INT16U inLen)
{
	INT16U iLoop;   
//	printf("%s",pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%02x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  /*%4d*/\n",iLoop);
		}
	}
	printf("\n");

}

int static m_eraseblks(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	unsigned int i;
	if (0==flag)
	{
		//指定块号进行erase
		for (i=0;i<sum_blks;i++)
		{
			NAND_PhyBlk_Erase(m_param[i], 1);
		}
	} 
	else
	{
		//指定范围进行erase
		for (i=0;i<sum_blks;i++)
		{
			NAND_PhyBlk_Erase(*m_param+i, 1);
		}
	}
	return 0x01;
}

//param1:要物理写的块数 
//param2:物理写方式标志，0为指定块号物理写，1为指定范围物理写,只写物理块的前4个sector
//注意写的单位为page，至少要有4个sector
//param3:存放块号的数组
//return:  1:ok    0:fail  
int static m_phywriteblks(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	unsigned int i;
	unsigned char *TmpBuf=(void *)malloc(4*512);
	memset(TmpBuf,0xaa,4*512);
    //hex_printf2(TmpBuf,512);
    
	if (0==flag)
	{
		//指定块号进行phywrite
		for (i=0;i<sum_blks;i++)
		{
		    //printf("m_param[%d]==%5d\n",i,m_param[i]);
            //NAND_PhyBoot_Read(m_param[i],0,1,testbuf);
            //hex_printf2(testbuf,512);
            //hex_printf2(TmpBuf,512);
            //NAND_PhyBlk_Erase(m_param[i],1);
			NAND_PhyBoot_Write(m_param[i],0,8,TmpBuf);
            //NAND_PhyBoot_Read(m_param[i],0,1,testbuf);
            //hex_printf2(testbuf,512);
		}
	} 
	else
	{
		//指定范围进行phywrite
		for (i=0;i<sum_blks;i++)
		{
			NAND_PhyBoot_Write(*m_param+i, 0,8, TmpBuf);
		}
	}
	free(TmpBuf);
	return 0x01;
}

//param1:要破坏的块数 
//param2:破坏方式标志，0为擦除，1为擦除后再写,2为复写
//param3:存放块号的数组
//return:  1:ok    0:fail  
int static assign_destory(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	switch (flag)
	{
	case 0:
		m_eraseblks(sum_blks,0,m_param);
		break;
	case 1:
		m_eraseblks(sum_blks,0,m_param);
		m_phywriteblks(sum_blks,0,m_param);
		break;
	case 2:
		m_phywriteblks(sum_blks,0,m_param);
		break;
	default:
		break;
	}
	return 0x01;
}

//param1:要破坏的块数 
//param2:破坏方式标志，0为擦除，1为擦除后再写,2为复写
//param3:开始破坏指针
//return:  1:ok    0:fail  
int static range_destory(unsigned int sum_blks,unsigned int flag,int *m_param)
{
	switch (flag)
	{
	case 0:
		m_eraseblks(sum_blks,1,m_param);
		break;
	case 1:
		m_eraseblks(sum_blks,1,m_param);
		m_phywriteblks(sum_blks,1,m_param);
		break;
	case 2:
		m_phywriteblks(sum_blks,1,m_param);
		break;
	default:
		break;
	}
	return 0x01;
}

int static random_destory(unsigned int flag,int *m_p)
{
    unsigned char i=0;
    unsigned char j;
    unsigned char k1,k2;
    unsigned int a;
    unsigned int *m_param=m_p;
    srand((unsigned int)time(0));
    k1=rand()%4;//得到mbrc要破坏的块数
    while (i<k1)
	{
		srand((unsigned int) time(0));
		a=rand()%4;
		for (j=0;j<i;j++)
		{
			if (a==m_param[j])
			{
				break;
			}
		}
		if (i==j)
		{
			m_param[i]=a;
			i++;
		}
	}//循环结束后得到互异的块号
    srand((unsigned int)time(0));
    k2=rand()%4;//得到brec要破坏的块数
    i=0;
    m_param=m_param+k1;
    while (i<k2)
	{
		srand((unsigned int) time(0));
		a=rand()%4+4;
		for (j=0;j<i;j++)
		{
			if (a==m_param[j])
			{
				break;
			}
		}
		if (i==j)
		{
			m_param[i]=a;
			i++;
		}
	}
    //两个while循环结束后数组中得到要破坏的块，且互异
    k2=k1+k2;//得到总共要破坏的块数
    printf("<Test Info> random destory %d blks: ",k2);
    for(i=0;i<k2;i++)
    {
        printf("%3d ",m_p[i]);
    }
    printf("\n");
    assign_destory(k2,flag,m_p);
    return 0x00;
}

//return 0:ok 1:fail
int Destory_xx_block(unsigned int sum_param,char * param_buf)
{
	int param[10];
	char t_buf[10];
	char *tp=param_buf;
	int t_i=0;
	
	int * p_blk=&param[2];//得到块号信息
	int destoryblk_num=0;
	for (t_i=0;t_i<sum_param;t_i++)
	{
		strcpy(t_buf,tp);
		param[t_i]=atoi(t_buf);
		tp+=strlen(tp)+1;
	}//完成参数的转化，char-->int

	//NAND_Init();

	switch (param[0])
	{
	case 0:
		/////////指定破坏模块函数
		printf("<Test Info> assign_destory begin...\n");
		destoryblk_num=sum_param-2;//得到要破坏的块数
		assign_destory(destoryblk_num,param[1],p_blk);
		break;
	case 1:
		/////////指定范围破坏函数
		printf("<Test Info> range_destory begin...\n");
		destoryblk_num=param[3]-param[2]+1;
		range_destory(destoryblk_num,param[1],p_blk);
		break;
	case 2:
		////////随机破坏函数
		printf("<Test Info> random_destory begin...\n");
        unsigned int p[6];
        random_destory(param[1],p);
		break;
	default:
		break;
	}

	return 0x00;

}