#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//#define SYMBOLTEST // 一致性测试


// 每个symbol所包含的bit数
#define SYMBOLBITS		9
// 可纠的错误数
#define ERRCAP			4

#define RSCODELEN 		((1 << SYMBOLBITS) - 1)
// 定义校验的位数
#define NPAR 			2*ERRCAP

#define NMSG			(RSCODELEN - NPAR)

#define ROM_EXPLEN		(RSCODELEN + 1)
#define ROM_LOGLEN		(RSCODELEN + 1)
#define ROM_INVLEN		(RSCODELEN + 1)
// 定义了最大的多项式阶数
// 这样方便硬件实现时多项式乘法器的复用
// 在RS码中，一般设置为可以纠正错误数的4倍即4t
#define MAXDEG			NPAR
// 两个多项式相乘的最大阶数
#define DMAXDEG			(2*MAXDEG)

typedef short BIT9;
typedef short BIT4;
typedef short BIT3;
typedef short BIT2;
typedef short BIT1;
//-------------------------------------------------------------------------------------
#endif
