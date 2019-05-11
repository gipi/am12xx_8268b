/*
* @defgroup 
* @author zengtao
* @version 1.0
* @date 2010-04-22
* @{
*/

/**
*@addtogroup sysbuf
*@{
*/

#ifndef SYS_BUF_H
#define SYS_BUF_H


/** <
  usage : 
           struct mem_dev example ;
           example.request_size = XXXX;
           example.buf_attr = uncache;
           
           fd = open("/dev/sysbuf",MODE);
           ioctl(fd,MEM_GET,&example);
           void *logic_add = (void*)example.logic_address;
           physic_start = example.physic_address;
           .......
	    example.physic_address = physic_start;
	    ioctl(fd,MEM_PUT,&example);
           
           you can use address logic_add now as the  buf. the physic address             
           of the buf is example.physic_address ,can use it when needed
           for example  : fdXXX = open("/dev/lcm",MODE);
                          ioctl(fdXXX,SET_DE_ADDR,example.phyaddress);
                          

*/

/*memctl operation number*/

#define MEM_GET		0x1			/**<allocate memory from sysbuf*/
#define MEM_PUT		0x2			/**<free allocated memory from sysbuf*/
#define MEM_QUERY	0x4			/**<query max continuous available memory in sysbuf*/
#define MEM_COPY	0x5			/**<copy data between memory allocated from sysbuf*/
#define VirADDR2_PHYADDR  0x8

enum MEM_ATTR{
	UNCACHE =0,
	CACHE,
	MEM_FS
};

struct mem_dev{
	unsigned long  physic_address;	
	unsigned long  logic_address;
	unsigned long  mem_pgoff;
	unsigned long  request_size;
	enum MEM_ATTR  buf_attr;
	int ret;
};

struct mem_copy_t{
	unsigned long src;
	unsigned long dst;
	unsigned int size;
};

#endif
/**
 *@}
 */