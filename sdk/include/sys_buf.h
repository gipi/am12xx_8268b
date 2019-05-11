#ifndef SYS_BUF_H
#define SYS_BUF_H
/**
*@file sys_buf.h
*@brief definition for system buffer operation
*
*@note usage : 
*struct mem_dev example ;
*example.request_size = XXXX;
*example.buf_attr = uncache;
*           
*fd = open("/dev/sysbuf",MODE);
*ioctl(fd,MEM_GET,&example);
*void *logic_add = (void*)example.logic_address;
*physic_start = example.physic_address;
* .......
*example.physic_address = physic_start;
*ioctl(fd,MEM_PUT,&example);
*           
*you can use address logic_add now as the  buf. the physic address             
*of the buf is example.physic_address ,can use it when needed
* for example: 
*             - fdXXX = open("/dev/lcm",MODE);
*             - ioctl(fdXXX,SET_DE_ADDR,example.phyaddress);
*@author zengtao
*@version 1.0
*@date 2010-04-22
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup sysbuf
*@{
*/


#define INVALID_PHYSICADDRESS	0xffffffff

/**
*@name memctl operation number
*@{
*/
#define MEM_GET		0x1			/**<allocate memory from sysbuf*/
#define MEM_PUT		0x2			/**<free allocated memory from sysbuf*/
#define MEM_QUERY	0x4			/**<query max continuous available memory in sysbuf*/
#define MEM_COPY	0x5			/**<copy data between memory allocated from sysbuf*/
#define VirADDR2_PHYADDR  0x8	/**<translate virtual address to physical*/
/**
*@}
*/

/**
*@enum MEM_ATTR
*/
enum MEM_ATTR{
	UNCACHE =0,		/**<uncached memory*/
	CACHE,				/**<cached memory*/
	MEM_FS				/**<memory can be used in system fs*/
};

/**
*@brief struct for the memory device
*/
struct mem_dev{
	unsigned long  physic_address;		/**<physciacal address allocated by sysbuf*/
	unsigned long  logic_address;		/**<logical address allocated by sysbuf*/
	unsigned long  mem_pgoff;			/**<page off of the address*/
	unsigned long  request_size;		/**<size of the required memory*/
	enum MEM_ATTR  buf_attr;			/**<attribute of the memory*/
	int ret;
};

/**
*@brief struct of memcpy info
*/
struct mem_copy_t{
	unsigned long src;		/**<source address*/
	unsigned long dst;		/**<destination address*/
	unsigned int size;			/**<data size of memcpy*/
};

/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif

