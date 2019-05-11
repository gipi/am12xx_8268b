/*
 * Copyright (c) 2016 Actionsmicro Inc., ALL RIGHTS RESERVED
 *
 * This source code and any compilation or derivative thereof is the sole
 * property of Actionsmicro and is provided pursuant to a Software License
 * Agreement. This code is the proprietary information of Actionsmicro and is
 * confidential in nature.
 * Its use and dissemination by any party other than Actionsmicro is strictly
 * limited by the confidential information provisions of Software License
 * Agreement referenced above.
 */

 /**
 * @file DIR.c
 *
 * @note
 * airdisk dir module.
 *
 * @author lemon
 *
 * @version v1.0.0.1
 * @par ChangeLog:
 * @verbatim
 *  v1.0.0.1-15/03/2016
 * @endverbatim
 */
#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include <unistd.h>
#include<dirent.h>
#include <string.h>
#include<ctype.h>



#define MAX_NAME_LEN 128
#define MAX_TYPE_LEN 10
#define MAX_PATH_LEN 1024


//For LinuxTool bug!! (i.e.; #if !define xxx)
#ifndef __ACT_MODULE_CONFIG_H__ 
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif

// g_ means  global variables,  p meas a pointer,  ch  means charater ,static  means  used in dir.c only
//the LSDK support list of  Video,Audio,Image,Document format 
static const char *gpch_VideoType[]={"AVI","MP4","3GP","MOV","MPG","MPEG","MOD","VOB","TS","TP","M2T","M2TS","MTS","WMV","MKV","RM","RMVB","DIVX","3G2","M4V","FLV","SWF",""};
static const char *gpch_AudioType[]={"MP3","WMA","OGG","WAV","AAC","FLAC","COOK","APE","AC3","DTS","RA","AMR","M4A",""};
static const char *gpch_ImageType[]={"JPG","BMP","JPEG","TIF","PNG", "GIF",""};
static const char *gpch_DocmType[]={"DOC","DOCX","WPS","TXT","PDF","HTML","C","PPT","XLS","XLSX","TAR","GZ","ZIP",""};

//file type 
enum {
	VIDEO=1,
	AUDIO,
	IMAGE,
	DOCUMENT,
	UNKNOW
};

typedef struct FileInfo{
    char name[MAX_NAME_LEN];      //file name 
    char fullName[MAX_NAME_LEN];  //file path
    int  size;			  //file size
    char type[10];		  //file type
}fileinfo;


//filelist node  use  chain table store fileinfo
typedef struct FileList
{
    fileinfo file;
    struct FileList* nextfile;
}filelist;

void Str_ToUpper(char *str)
{
	int i=0;
	int len=strlen(str);
	for( i=0;i<len;i++)
	{
		//if(str[i]>='a'&&str[i]<='z')
			//str[i]-=('a'-'A');
			
		/*use linux api*/
		if(islower(str[i]))
			str[i]=toupper(str[i]);
			
	}
}

//check file type is supported or not
char * FileTypeComp( const char *str1[], const char *str2)
{
        int i=0;
        while(str1[i]!=(char *)"")
        {
            if(strcmp(str1[i],str2)==0)
            	return(char *)str1[i] ;
       		i++;
        }
        //did not find in filetype array,
        return NULL;         
}     
/*******************************************************************************
*  FUNCTION:       FileTypeGet
*  ____________________________________________________________________________
*
*  DESCRIPTION:
*    
*    Prase file type by  file extensions, like : test.mp4  will  get file  type MP4
*    
*  INPUTS:
*    filename - the filename of current file 
*
*  OUTPUTS:
*    None.
*
*  RETURNS:
*    filetype.
*
********************************************************************************/

int FileTypeGet( char *filename )
{
	char type[10]="";
	char *index;
	char c='.';
	index=strrchr(filename,c);
	if(index&&(strlen(index)<11))
	{
		index+=1;
		char *fp=NULL;
		strcpy(type,index);
		Str_ToUpper(type);
		if((fp=FileTypeComp(gpch_DocmType,type))!=NULL)
			return DOCUMENT;
		else if((fp=FileTypeComp(gpch_VideoType,type))!=NULL)
			return VIDEO;
		else if((fp=FileTypeComp(gpch_AudioType,type))!=NULL)
			return AUDIO;
		else if((fp=FileTypeComp(gpch_ImageType,type))!=NULL)
			return IMAGE;
		else
			return UNKNOW;
	}
	
	return UNKNOW;

}

/*******************************************************************************
*  FUNCTION:       GetFileList
*  ____________________________________________________________________________
*
*  DESCRIPTION:
*    
*    get all the file supported(video ,audio image ,document) in current directory,and restore them in  filelist chain table
*    
*  INPUTS:
*    filepath: the current directory 
*
*  OUTPUTS:
*    None.
*
*  RETURNS:
*    fileList.
*
********************************************************************************/


filelist * GetFileList(char *filepath)
{
    filelist *head=NULL;
    filelist *cur_list=NULL;
    char name_temp[MAX_PATH_LEN];
    DIR * dir;
    struct dirent *dir_env;
    struct stat stat_file;
    head =(filelist*)malloc(sizeof(filelist));
    head->nextfile = NULL;
    if((dir=opendir(filepath))==NULL){
    	
    	fprintf(cgiOut,"dir open Error!!!\n");
  		return NULL ;  
    }
	//opendirectory success ,start read file in directory
    while(dir_env=readdir(dir))
    {
        /* skip .(current dir ) ..(parent dir )*/
        if(strcmp(dir_env->d_name,".")==0  || strcmp(dir_env->d_name,"..")==0)
            continue;
        strcpy(name_temp,filepath);
        strcat(name_temp,dir_env->d_name);//file full name=file path+file name

		//get file infomation >stat_file by file full name 
        stat(name_temp,&stat_file);
		
		//new space for node 
        cur_list=(filelist*)malloc(sizeof(filelist));
        strcpy(cur_list->file.name,dir_env->d_name);
        strcpy(cur_list->file.fullName,name_temp);
        if( S_ISDIR(stat_file.st_mode))
        {
            cur_list->file.size = 0;
            strcpy(cur_list->file.type,"0");
            strcat(cur_list->file.fullName,"/");//cur file is a directroy  add a '/',
        }else//other file not dir
        {
            cur_list->file.size = stat_file.st_size;
            strcpy(cur_list->file.type,"1");
        }

	   	//insert new node behind head node
        if(head->nextfile ==NULL)
        {
            head->nextfile =cur_list;
            cur_list->nextfile = NULL;
        }else{
            cur_list->nextfile = head->nextfile;
            head->nextfile = cur_list;
        }

    }

    if(!head->nextfile)//node lenght is 1
    {
		fprintf(cgiOut,"Dir is Empty!!!\n");
	}
    return head;

}

/*******************************************************************************
*  FUNCTION:	   ShowDirList
*  ____________________________________________________________________________
*
*  DESCRIPTION:
*	 
*	 get  file type of directroy form filelist chain table
*	 printf  out by cgiout ,html will get the result
*  INPUTS:
*	 filename - the filename of current file 
*
*  OUTPUTS:
*	 None.
*
*  RETURNS:
*	 NULL.
*
********************************************************************************/

void ShowDirList(filelist * head)
{
    if(head == NULL)return;
    while(head)
    {	
    	if(!strcmp(head->file.type,"0"))
    	{
    		fprintf(cgiOut,"%s,%s*","0",head->file.name);
       	
      	}
      	head = head->nextfile;
    }   
    return ;
}

void ShowVideoList(filelist * head)
{
	int filetype;
  	if(head == NULL)return;
	while(head)
	{
		//is a normal file ,not directroy 
	   	if(!strcmp(head->file.type,"1"))
	    {
	       // filetype is VIDEO
	   	    filetype=FileTypeGet(head->file.name);
			if(VIDEO==filetype)
			/*VIDEO  left  ,if u use "=" instead of " ==" carelessly,
			make error will occur to warn u  */ 
			
	  		fprintf(cgiOut,"%d,%s*",VIDEO,head->file.name);
	 	}
	    head = head->nextfile;
	 }
	 return ;
}
void ShowAudioList(filelist * head)
{
	int filetype;
	if(head == NULL)return;
    while(head)
    {
    	if(!strcmp(head->file.type,"1"))
    	{
       	    filetype=FileTypeGet(head->file.name);
			if(AUDIO==filetype)
      		fprintf(cgiOut,"%d,%s*",AUDIO,head->file.name);
      	}
      head = head->nextfile;
    }
    return ;
}
void ShowImageList(filelist * head)
{
	int filetype;
  	if(head == NULL)return;
    while(head)
    {
    	if(!strcmp(head->file.type,"1"))
    	{
       	    filetype=FileTypeGet(head->file.name);
			if(IMAGE==filetype)
      			fprintf(cgiOut,"%d,%s*",IMAGE,head->file.name);
      	}
      head = head->nextfile;
    }
    return ;
}

void ShowDocList(filelist * head)
{
	int filetype;
  	if(head == NULL)return;
    while(head)
    {
    	if(!strcmp(head->file.type,"1"))
    	{
       
       	    filetype=FileTypeGet(head->file.name);
			if(DOCUMENT==filetype||UNKNOW==filetype)
      			fprintf(cgiOut,"%d,%s*",DOCUMENT,head->file.name);
      	}
      head = head->nextfile;
   }
 
    return ;
}
void Free_List(filelist *head )
{

	  filelist * temp;
	  while(head!=NULL)
	  {	
	  	temp=head;
		head=head->nextfile;
		free(temp);
	  }
}

int cgiMain(int argc,char*argv[])
{
	
	char fullname1[MAX_PATH_LEN];
	char fullname[MAX_PATH_LEN-MAX_NAME_LEN]="";
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	cgiFormString("fullname", fullname1, MAX_PATH_LEN-MAX_NAME_LEN);
	if(!strcmp("card_sta",fullname1))
	{
		int *card_status=NULL;
		int ret=0;
		htmlsetting_func("airdisk_cmd", (void*)fullname1, strlen(fullname1), (void **)&card_status, NULL);
		if(card_status){
				ret = *card_status;
				fprintf(cgiOut,"card_sta%d\n",ret);	
			}else{
				ret = -1;
				fprintf(cgiOut,"card_sta%d\n",ret);			
			}
			del_return((void **)&card_status);
		//fprintf(cgiOut,"card_sta%d",card_status);
	}
	else if(!strcmp("usb_sta",fullname1))
	{

		
		int *usb_status=NULL;
		int ret=0;
		htmlsetting_func("airdisk_cmd", (void*)fullname1, strlen(fullname1), (void **)&usb_status, NULL);
		//fprintf(cgiOut,"usb_sta%d",usb_status);
		if(usb_status){
				ret = *usb_status;
				fprintf(cgiOut,"usb_sta%d\n",ret);	
			}else{
				ret = -1;
				fprintf(cgiOut,"usb_sta%d\n",ret);		
			}
			del_return((void **)&usb_status);
	}
	else{
		//get file info
		sprintf(fullname,"%s/",fullname1);
		
	    filelist *mylist;
	    mylist =GetFileList(fullname); //storage the dir and file node in current dir;
	    strcpy(mylist->file.fullName,fullname);
		
	    fprintf(cgiOut,"%s","#");
	    ShowDirList(mylist);

	#if MODULE_CONFIG_EZMUSIC_ENABLE
		ShowVideoList(mylist);
		ShowAudioList(mylist);
	#else
		ShowVideoList(mylist);
		ShowAudioList(mylist);
		ShowImageList(mylist);
		ShowDocList(mylist);
	#endif
		Free_List(mylist);
	}
    return 0;
}
