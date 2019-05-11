/**
*******************************************************************************
* @file	FileSelector.h
* @brief 文件选择器API
*
*   根据app需要，建立指定类型的文件列表
* 
* 
* @author 作者
* @version  版本
* @date	日期
* @warning 写上关于警告的说明
* @bug 写上关于bug的说明
* @deprecated 写上过时的说明，比如说，该文件到什么时候就过时，没用了
* @todo 写上将要做的事
* @see 写上要参看的文件或者变量
* @ingroup media_nav_s
********************************************************************************/


/**
* @addtogroup media_nav_s
* @{
*/

#ifndef _FILESELECTOR_H_
#define _FILESELECTOR_H_

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "ipc_key.h"


#ifdef __cplusplus                     
extern "C" {
#endif

#ifndef TRUE
#define TRUE	 1
#endif
#ifndef FALSE
#define FALSE	 0
#endif

typedef int bool;
#define FULL_PATH_SIZE	(1024)

#if 0
union semun {
    int val;						/* value for SETVAL */
    struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;		/* array for GETALL & SETALL */
    struct seminfo *__buf;		/* buffer for IPC_INFO */
    void *__pad;
};
#if 1
typedef enum
{
	NAND_DISK = 0,
	CARD_DISK = 1,
	HARD_DISK = 2,
	USB_DISK1 = 3,
	USB_DISK2 = 4,
	USB_DISK3 = 5,
	USB_DISK4 = 6,
	ROOT_DISK = 7,
	PRIV_DISK=8,
	DISK_NUM  = 9
}Disk_Type;
#endif

struct dirinfo;

/** 
 * @brief 文件搜索索引节点
*
* 详细说明…
*/
typedef struct dirinfo{
    int filenum;                      	/**< 当前文件夹下的有效图片个数 */
    struct dirinfo * children;         	/**< 指向第一个子文件夹节点*/
    struct dirinfo * brother;          	/**< 指向相邻的下一个文件夹节点*/
    struct dirinfo * parent;             /**< 指向父文件夹节点 */
 //   void * parent;
}dir_info_t;

typedef struct{
     struct dirinfo *  * buf;
     char              *  * ext;
     int num;
}dir_info_buffer_t;

#define MAX_DIR_NUM 1000 	// must be not less than  2 

#define FULL_PATH_SIZE	(1024)
#define MAX_EXT_LEN 		(12)
#define MAX_DIR_LEVEL	(10+3)

#define MAX_NAME_LEN	(768)

#define PATH_SEPARATOR	'/'

 struct scan_task_info;

#define mnavi_inf(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#define mnavi_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define mnavi_debug(fmt,arg...)  //printf("MDEBUG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)


/**the choices for pFsel->property*/
#define    NEED_FILE_NUM 	(0x01<<0)
#define    NEED_NAME_BUFFER 	(0x01<<1)
#define    NEED_EXIT	(0x01<<2)
#define    EXTERN_BUFFER 	(0x01<<3)

#define MNAVI_SECTOR_SIZE (512)


/** 
 * @brief 文件选择器
*
* 详细说明…
*/
typedef struct {
//    bool Idle;
    int StartNO;        				 /**< 选中的需要查询文件列表的起始位置*/
    char level;              				 /**< 当前工作目录de 目录层次*/			
    int FileTotal;    					 /**< 当前目录下有效文件总数(不包括子目录)*/
    int DirTotal;    					 /**< 工作目录下目录项总数(不包括子目录)*/
    int file_sub_total; 				 /**<  工作目录下文件总数(包括子目录内)*/
    int FileNo;      					 /**< 当前文件序号*/
    int file_sub_No; 	 			 /**< 文件在工作目录下的序号(包括子目录内)*/
    int start_file_sub;				 /**<开始搜索的文件序号(包括子目录)*/
    char *Ext;            				 /**< 文件过滤后缀名*/
    char * cur_ext;   				 /**< 当前后缀名*/
    char CurExt[MAX_EXT_LEN+1];  	 /**< 当前后缀名*/
    char sub;          					 /**< 子目录 控制*/
	pid_t scan_process_pid; /**<被创建的扫描进程的PID，后面在退出的时候需要等待进程结束>**/
   DIR *pDir;    					 /**< 目录项句柄    */
   //FS_Handle   pDir;      			 /**< 目录项句柄    */
//    FS_Entry_t  Entry;
   struct dirent *dirent; 				 /**< for save dir info from fs */
    char *path;    					 /**< root path*/
    unsigned char root_level;  		 /**< depth of the root path */
    long int dirpos[MAX_DIR_LEVEL];
    dir_info_t * dir_info;   			 /**< scaned dir info*/
    dir_info_t * cur_dir_info;			 /**< the dir info of the last searched file*/
   struct scan_task_info *scan_task; 	 /**< scan task's info*/
    //unsigned char task_hold;  		 /**< indicate whether  the fsel hold the task or not*/
    unsigned char property;  			 /**< some property of the fsel */
   char cur_file[FULL_PATH_SIZE];		 /**< buffer for saving  founded file path and so on */
   char cur_path[FULL_PATH_SIZE];  	 /**< worh path*/
   unsigned char not_back;     		 /**< useless now*/
   dir_info_buffer_t dir_info_buffer;
   struct{
       DIR *_pDir;
       char * _cur_path;
       int  _file_num;
       dir_info_t * _current;
       unsigned char _next;    
   }sync;
   char fsel_index;
   char ScanMode;					/**<ScanMode=1:create a task for scanning*/
								/**<ScanMode=0:the task will no be created*/
}Fsel_t;

/** 
 * @brief 后台扫描任务
*
* 详细说明…
*/
struct scan_task_info{
  // Fsel_t *pFsel;
   //OS_STK  * stack;   		/**<task stack */
   unsigned char  runing;      /**<task status */
   unsigned char prio;           /**<task priority */
   unsigned short ID;            /**<task ID */
   unsigned char task_hold;   
   unsigned char task_exit;
   //OS_EVENT * task_com;  /**<OS semphore,for waiting the scan task ready to be deleted  */
};

//struct scan_task_info scan_task;

enum{
	FT_DIR=0,// 目录名
	FT_FILE,// 文件名
	FT_MUSIC,
	FT_VIDEO,
	FT_IMAGE,
	FT_VVD,
	FT_TXT
};
#define MAX_FILE_TYPE FT_TXT

enum
{
    GET_TOTAL_FILE_NUM=0,
    GET_FILE_NUM,
    GET_DIR_NUM,
    START_SCAN_DIR,          		  // 开始目录扫描任务，建立文件信息
    DEL_SCAN_TASK,          			  // 删除目录扫描任务
    GET_SCANED_FILE_NUM,               //当前扫描到的文件总数
    GET_TASK_STATUS,
    REFRESH_FSEL
};

enum{
   EDLE=0,
   RUNING,
   COMPLETED
};


/** 
 * @brief 外部buffer，保存查询项目的路径等信息
*
* 详细说明…
*/

typedef struct
{
 	int type;                 			/**<item type,file or dir */
	unsigned int length; 			/**<length of the file name*/
	char content[FULL_PATH_SIZE]; 	/**<full path of the item */
	char filename[MAX_NAME_LEN];	/**<filename of the item*/
	char *path;        				/**<not uesed now */
	int index;            				/**<the item's index */
}filename_t;


/*typedef struct {
	int index;
	int valid;
	char  file[FULL_PATH_SIZE];
} LIST_NODE;*/

/**
* @brief  文件选择器初始化函数
*
* @param[in]  disk 希望操作的全路径.
* @param[in]  Ext 文件选择器的工作类型。 文件选择器通过设定的后缀名过滤文件
* @param[in]  property 文件选择器的一些参数
* -NEED_FILE_NUM 表示打开文件选择器的同时获得当前目录下文件夹和文件的个数
* -NEED_NAME_BUFFER 每次获取一定数量的文件名称等信息保存在buffer中
* @return	文件选择器句柄,打开文件失败时返回空指针

* @author  作者
* @version版本
* @date    日期
* @warning 
*         1）每个后缀名长度为3，以 空格为结束符，
*               例如const char ExtMusic[ ] = "MP3 WMA ASF WMV WAV ACT OGG M4A AA  ";
*         2）当后缀名长度不足3 时，以空格补足；
*         3）后缀名为3 个空格时，文件选择器将显示所有文件。
* @code	
*	pFsel  = fsel_open( ROOT_DISK,"JPG BMP JPEG TIFF",10);
*/
Fsel_t * fsel_open( char* disk, char  *pExt, unsigned char property );


/**
* @brief   用于关闭一个已经打开的文件选择器
*
* @param[in]  pFsel 文件选择器结构体
* @return   是否关闭成功
* @retval   <0	 关闭文件选择器失败
* @retval   0	 关闭文件选择器成功
* @author  作者
* @version版本
* @date    日期
* @warning 
*/
int fsel_exit(Fsel_t *pFsel);


/**
* @brief   用于获取有效文件/目录总数。
*
* @param[in]  pFsel 文件选择器结构体，存放文件选择器需要用到的相关参数
* @param[in]  cmd 命令
* -GET_TOTAL_FILE_NUM 	获取工作目录下（包含子目录）所有有效文件数目，返回值<0，表示获取失败
* -GET_FILE_NUM 			获取工作目录下（不包含子目录）有效文件总数，返回值<0，表示获取失败。
* -GET_DIR_NUM 			获取工作目录下目录总数， 返回值<0，表示获取失败。
* -START_SCAN_DIR 		开始目录扫描任务，建立文件信息，返回值-2表示任务启动失败，其他值表示任务建立成功，直接返回文件总数   
* -DEL_SCAN_TASK 		删除目录扫描任务,返回值<0,表示删除失败
* -GET_SCANED_FILE_NUM 	当前扫描到的文件总数，返回值<0，表示扫描失败
* -GET_TASK_STATUS 		获取目录扫描任务的状态，返回-1表示当前任务不存在，返回-2表示任务已经停止
* -REFRESH_FSEL			刷新文件选择器，总是返回0
* @return   与cmd相关
* @author  作者
* @version版本
* @date    日期
* @warning 
*/
int fsel_getFileNum (Fsel_t *pFsel, unsigned char cmd);


/**
* @brief   用于获取工作目录下文件的名称列表(不包含子目录)
*
* @param[in]  file_name 文件名称的结构体数组指针。
* @param[in] fsel 文件选择器结构体
* @param[in] startNO 获取列表的开始位置,从1开始计数
* @param[in] num  调用者希望获取的文件个数。
* @return  实际获取到的文件个数，若实际存在的表项总数小于num数值，则此返回值为实际的表项个数。
* @retval   0 表示查询的目录下StartNO后面没有有效文件，需要重新设置开始序号。
* @retval   <0 表示获取失败。
* @author  作者
* @version版本
* @date    日期
* @warning 
*        1.第一次使用前请设置工作目录
*        2.应用需要根据获取的文件数目，来声明file_name空间大小。
*        例如：获取5 个文件；所以file_name的空间定义应为：filename_t file_name [5]。
*/
int fsel_getFileList (filename_t *file_name, Fsel_t *pFsel, int startNO,int num);


/**
* @brief   用于获取工作目录下(包含子目录)文件的名称
*
* @param[in]  file_name 文件名称的结构体数组指针。
* @param[in] fsel 文件选择器结构体
* @param[in] startNO 获取列表的开始位置,从1开始计数
* @param[in] num  调用者希望获取的文件个数。
* @return  实际获取到的文件个数，若实际存在的表项总数小于num数值，则此返回值为实际的表项个数。
* @retval   0 表示查询的目录下StartNO后面没有有效文件。
* @retval   <0 表示获取失败。
* @author  作者
* @version版本
* @date    日期
* @warning 
*        1.第一次使用前请设置工作目录
*        2.应用需要根据获取的文件数目，来声明file_name空间大小。
*        例如：获取5 个文件；所以file_name的空间定义应为：filename_t file_name [5]。
*/
int fsel_getFileListAll (filename_t *file_name, Fsel_t *pFsel, int startNO,int num);


/**
* @brief   用于获取工作目录下子目录的名称,不包括子目录下的子目录
*
* @param[in]  file_name 文件名称的结构体数组指针。
* @param[in] fsel 文件选择器结构体
* @param[in] startNO 获取列表的开始位置,从1开始计数
* @param[in] num  调用者希望获取的文件个数。
* @return  实际获取到的目录名个数，若实际存在的表项总数小于num数值，则此返回值为实际的表项个数。
* @retval   0 表示查询的目录下StartNO后面没有有效文件。
* @retval   <0 表示获取失败。
* @author  作者
* @version版本
* @date    日期
* @warning 
*        1.第一次使用前请设置工作目录
*        2.应用需要根据获取的文件数目，来声明file_name空间大小。
*        例如：获取5 个文件；所以file_name的空间定义应为：filename_t file_name [5]。
*/
int fsel_getDirList (filename_t *file_name, Fsel_t *pFsel, int startNO,int num);


/**
* @brief   用于获取工作目录下子目录和文件的名称
*
* @param[in]  file_name 文件名称的结构体数组指针。
* @param[in] fsel 文件选择器结构体
* @param[in] startNO 获取列表的开始位置,从1开始计数
* @param[in] num  调用者希望获取的文件个数。
* @return  实际获取到的个数，若实际存在的表项总数小于num数值，则此返回值为实际的表项个数。
* @retval   0 表示查询的目录下没有有效文件。
* @retval   <0 表示获取失败。
* @author  作者
* @version版本
* @date    日期
* @warning 
*        1.先目录名，后文件名
*        2.应用需要根据获取的文件数目，来声明file_name空间大小。
*        例如：获取包含5个文件；所以file_name的空间定义应为：filename _t file_name [5]。
*/
int fsel_getList (filename_t *file_name, Fsel_t *pFsel, int startNO,int num );


/**
* @brief  获取文件的路径信息。
*
* @param[in]  path 存放路径信息buffer，调用前必须好分配足够空间
* @param[in] fsel 文件选择器结构体
* @param[in] start_NO 指定的文件序号
* @return  是否获取成功
* @retval   1 		表示获取成功
* @retval   <=0 表示获取失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*/
int fsel_getFullpath (char * path, Fsel_t *pFsel, int start_NO);


/**
* @brief  设置文件选择器的工作目录，必须是当前工作目录下的路径
*
* @param[in] fsel 文件选择器结构体
* @param[in] pDirName  工作路径下完整的合法的目录名(绝对路径)

* @return  是否设置成功
* @retval   >=0 设置工作目录成功
* @retval  <0设置工作目录失败
* @author  作者
* @version版本
* @date    日期
* @warning 
* 	完整的合法的目录名为：[device][DirPathList]DirectoryName，分别为设备名、目录路径和目录名
* @code 	fsel_serWorkDir(pFsel,"/mnt/udisk/test");
*/
int fsel_setWorkDir (Fsel_t *pFsel, char *pDirName);


/**
* @brief  设置文件选择器的过滤类型
*
* @param[in] fsel 文件选择器结构体
* @param[in] Ext  文件选择器的工作类型。文件选择器通过设定的后缀名过滤文件

* @return  是否设置成功
* @retval   TRUE 设置成功
* @retval  FALSE 设置失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*         1）每个后缀名长度为3，以空格为结束符，
*               例如const char ExtMusic[ ] = "MP3 WMA ASF WMV WAV ACT OGG M4A AA  ";
*         2）当后缀名长度不足3 时，以空格补足；
*         3）后缀名为3 个空格时，文件选择器将显示所有文件。 
*/
bool fsel_setExt (Fsel_t *pFsel, char* Ext);


/**
* @brief  回退到上一级目录，并将其设置为当前工作目录，更新文件选择器；
			(根目录为fsel_setRootDir()设置的目录)
*
* @param[in] fsel 文件选择器结构体
* @param[in] index  上一级目录的序号

* @return  是否成功
* @retval   >=0 成功
* @retval  <0 失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*/
int fsel_setparent(Fsel_t *pFsel,int *index);


/**
* @brief   从当前目录进入到下一级目录，并设置其为工作目录，且更新文件选择器
*
* @param[in] fsel 文件选择器结构体
* @param[in] index  所要进入的目录的序号
* @param[in] pDirName 所要进入的目录名
* @return  是否成功
* @retval   >=0 成功
* @retval  <0 失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*/
int fsel_CD (Fsel_t *pFsel,char *pDirName,int index);


/**
* @brief  根据 建立的文件信息，删除指定序号的文件
*
* @param[in] fsel 文件选择器结构体
* @param[in] index  所要删除的文件序号

* @return  是否成功
* @retval   0 删除成功
* @retval  <0 删除失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*/
int  fsel_del_file(Fsel_t *pFsel,int index);


/**
* @brief  设置文件选择器的根目录
*
* @param[in] fsel 文件选择器结构体
* @param[in] pDirName  根目录所在路径完整的合法的目录名

* @return  是否成功
* @retval   TRUE 	成功
* @retval  FALSE	失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*说明：完整的合法的目录名为：[device][DirPathList]DirectoryName，分别为设备名、目录路径和目录名
*/
bool fsel_setRootDir (Fsel_t *pFsel,char *pDirName);


/**
* @brief  当前目录下指定序号文件夹下包含的有效文件数量
*
* @param[in] fsel 文件选择器结构体
* @param[in] index  文件夹序号

* @return  文件数量

* @author  作者
* @version版本
* @date    日期
* @warning 
*
*/
int  fsel_dir_total_fileNum(Fsel_t *pFsel,int index);


/**
* @brief  打开当前指定全路径的文件
*  后台文件信息还没有建立起来的时候，利用此函数同步打开的文件
* @param[in] fsel 文件选择器结构体
* @param[in] pDirName  文件全路径
* - NULL 同步文件
* @param[in] index  保存打开的文件序号
* @return  是否成功
* @retval   1 成功
* @retval   0 正在同步
* @retval  -1 失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*/
int  fsel_OpenFile (Fsel_t *pFsel,char *pDirName,int *index);

#if 0
/**
* @brief  建立文件信息。
*
* @param[in] fsel 文件选择器结构体
* @param[in] addr  buffer地址

* @return  是否成功

* @retval   NULL 失败

* @author  作者
* @version版本
* @date    日期
* @warning 
*
*/
int fsel_search_dir(Fsel_t *pFsel,char * path,dir_info_t **addr_array,char ** ext_array,int num);


/**
* @brief  设置或者查询保存文件信息的buffer地址。
*
* @param[in] fsel 文件选择器结构体
* @param[in] addr  buffer地址

* @return  是否成功
* @retval   NULL 失败

* @author  作者
* @version版本
* @date    日期
* @warning 
*	暂时没用，后续可能会取消
*/
dir_info_t *  fsel_ListBuffer (Fsel_t *pFsel, dir_info_t ** addr,char ** ext,unsigned int num,unsigned char op);

#endif
/** 
* @brief   查找一个字符串中是否包含有指定的扩展名
*
* @param[in] exts 扩展名字符串
* @param[in] ext  要查找的字符串

* @return  是否成功

* @retval   1 	成功
		  0	失败 
* @author  作者
* @version版本
* @date    日期
* @warning 
*	mnavi 内部函数
*/
int fsel_find_ext(char * exts,char *ext );


/**
* @brief   从文件名中获取文件的后缀名
*
* @param[in] name 文件名
* @param[in] ext  保存扩展名的buf

* @return  是否成功

* @retval   1 成功

* @author  作者
* @version版本
* @date    日期
* @warning
*	mnavi 内部函数
*/
unsigned char fsel_get_ext(char *name,char*ext);


/**
* @brief   打开目录，并设置为当前工作路径
*
* @param[in] name 相对目录名
* @param[in] pdir 	目录句柄

* @return  是否成功
* @DIR*   非空则 成功

* @author  作者
* @version版本
* @date    日期
* @warning 
*	mnavi 内部函数
*/
DIR *xcddir(DIR *pdir,const char *const name);


/**mnavi 内部函数*/
int  fsel_getFileListSub (filename_t *file_name, Fsel_t *pFsel,	int  startNO ,	int  num);


/**
* @brief  回退到上一级目录
*
* @param[in] fsel 文件选择器结构体

* @return  是否成功
* @retval   0 返回成功
* @retval  -1 返回失败
* @author  作者
* @version版本
* @date    日期
* @warning 	
*	与fsel_setparent区分;
*	mnavi 内部函数
*/
int fsel_back_parent(Fsel_t *pFsel);


/**
* @brief   从当前目录进入到下一级目录
*
* @param[in] fsel 文件选择器结构体
* @param[in] name 所要进入的目录名
* @return  是否成功
* @retval   0 	成功
* @retval  -1	失败
* @author  作者
* @version版本
* @date    日期
* @warning 
*	与fsel_CD不同，该函数只是简单地进入到下一级目录;	
*	mnavi 内部函数
*/
int fsel_docd(Fsel_t *pFsel,char* name);

#endif
/**
 * @}
 */
#endif

