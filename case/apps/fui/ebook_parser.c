#ifdef MODULE_CONFIG_EBOOK
#include "ebook_engine.h"


//extern book_var_t  EBookTaskVars;
INT8U IsTaskCreate=0;
book_var_t  EBookTaskVars;
INT8U IS_CJK = 0;//当前选定语言是否为中日韩
INT8U CharLengthInfo[CHAR_LENGTH_SIZE];//字符宽度信息

// background task stack
char *bg_ebook_stack;

// semphore for decoding
//OS_EVENT 	*ebook_dec_sem;
char 	*ebook_cmd_sem;

eb_show_var_t bg_show_var;
INT32U bg_counting = 0;
INT32U totalnum_char=0;

#define _EBOOK_DBG_ 
#ifdef _EBOOK_DBG_
#define EBookEGdbg(x...)   printf("EBook: " x)
#else
#define  EBookEGdbg(...)     do { } while (0)
#endif
///

void ebook_info_parse(void);

void printf_data(INT8U *buffer,INT32S length,INT32S eachline)
{
	INT32S i;
	INT32S j;
	INT32S totalLine = length/eachline;
	INT32S remain = length%eachline;
	for(j=0;j<totalLine;j++){
		for(i=0;i<eachline;i++){
			printf("%x ", buffer[i+j*eachline]);
		}
		printf("\n");
	}
	for(i=0;i<remain;i++)
		printf("%x ",buffer[i+totalLine*eachline]);
	printf("Length=%d,TotalLine=%d\n",length,totalLine);
}


/**
 * @brief  read a special one sector,the offset is var->sector .
 * @param[in] none.
 * @return the result of read.
 * - READ_SECTOR_FAILE: fail.
 * - READ_SECTOR_OK: success.
 */ 
INT8U  _read_special_sector(eb_show_var_t *var)
{
	INT8U  ret = READ_SECTOR_FAILE;

	//EBookEGdbg(" %s: %u: \"%s\" function.\n", __FILE__, __LINE__, __FUNCTION__);
	//var->sector&=REVERSE512;
	if(fseek(var->fp, var->sector, SEEK_SET) ==0)
	{
		var->bookbuffer_len = fread(var->bookbuffer, 1,ONE_SECTOR_LENGTH, var->fp);  
		//printf_data(var->bookbuffer,var->bookbuffer_len,16);
		if(var->bookbuffer_len > 0)
			ret = READ_SECTOR_OK;
	}
	return ret;
}


/**
 * @brief  read previous sector,the offset of the file is var->sector .
 * @param[in] none.
 * @return the result of read.
 * - READ_SECTOR_FAILE: fail.
 * - TO_THE_HEAD: read to the head.
 * - READ_SECTOR_OK: success.
 */ 
INT8U  _read_prev_sector(eb_show_var_t *var)
{
	INT32S result,offset,i;
	INT8U  ret = READ_SECTOR_FAILE;
	//var->sector &= REVERSE512;
	offset = var->sector-ONE_SECTOR_LENGTH;
	if(offset<0)
	{
		var->sector = 0;
		ret = TO_THE_HEAD;
	}
	else
		var->sector = offset;
	if(fseek(var->fp, var->sector, SEEK_SET)==0)
	{
		var->bookbuffer_len = fread(var->bookbuffer, 1,ONE_SECTOR_LENGTH, var->fp);
		//EBookEGdbg("%s,%d,,bookbuffer_len=%x\n",__FILE__,__LINE__,var->bookbuffer_len);
		if(ret == TO_THE_HEAD)
			return ret;
		else if(var->bookbuffer_len > 0)
		{
			ret = READ_SECTOR_OK;
		}
	}
	return ret;
}


/**
 * @brief  read next sector,the offset of the file is var->sector,the offset will auto increase after S_FS_FRead().
 * @param[in] none.
 * @return the result of read.
 * - READ_SECTOR_FAILE: fail.
 * - TO_THE_END: read to the end.
 * - READ_SECTOR_OK: success.
 */ 
INT8U _read_next_sector(eb_show_var_t *var)
{
	//INT32U offset;
	INT8U  ret = READ_SECTOR_FAILE;
	//printf("\n read next sector: sector=%x\n",var->sector);
	//var->sector &= REVERSE512;
	if(fseek(var->fp, var->sector+var->bookbuffer_len, SEEK_SET) == 0)
	{
		var->sector += (var->bookbuffer_len&REVERSE512);
		var->bookbuffer_len = fread(var->bookbuffer, 1,ONE_SECTOR_LENGTH, var->fp);
		if(var->bookbuffer_len > 0)
		{
			//printf("\n read next sector OK\n");
			ret = READ_SECTOR_OK;
		}
		else
		{
			//printf("\n read next sector: to the end!\n");
			//printf_data(var->bookbuffer,var->bookbuffer_len,16);
			ret = TO_THE_END;
		}
	}
	
	return ret;
}




/**
 * @brief  display one line.
 * @param[in] none.
 * @return the offset of the next line,return 0 when reach the end of file.
 */ 
INT32U get_ebook_oneline(eb_show_var_t *var)
{
	INT32S i=0;
	INT32U length=0,flag=1;
	INT32S char_width=0;
	INT32S save_sector=0,save_offset=0,save_i=0;
	INT8U result = TRUE;
	INT8U *showbuffer;
	INT16U char_uni=0,char_uni_old=0;
	//printf("%s,%d:call show one line\n",__func__,__LINE__);
	showbuffer =  var->showbuf;// + EBookTaskVars.show_vars.line*LINE_MAX_BYTES;        
	totalnum_char = 0;
	if(var->bookbuffer_len <= 0 )//上一次已经达到文件尾
	{
		EBookEGdbg("reach the end of file!  %s: %u: \"%s\" function.\n", __FILE__, __LINE__, __FUNCTION__);
		return FALSE;
	}
	if(var->type  == MBCS)
	{
		INT8U curchar,char_old=0;
		if(IS_CJK)
		{
			INT8S doubleflag;
			//printf("%s,%d:Is_CJK=%d\n",__FILE__,__LINE__,IS_CJK);
			for(i=0,length=0;;)
			{
				doubleflag=0;
				if(var->sector_offset>=var->bookbuffer_len)
				{
					if(_read_next_sector(var)==TO_THE_END)
					{
						EBookEGdbg("to the end\n");
						showbuffer[i++] = SPACE_CHAR;//解决字符串显示的bug
						showbuffer[i] = END_STRING_CHAR;	
						totalnum_char = i;
						//printf("%s,%d:charnum=%d\n",__FILE__,__LINE__,totalnum_char);
						return TRUE;
					}
					var->sector_offset=0;
				}
				curchar = var->bookbuffer[var->sector_offset++];
				//printf("c=%x\n",curchar);
				if(curchar == NEW_LINE_CHAR) //换行符
				{
					
					break;
				}
				else if(curchar<SPACE_CHAR)//防止字符宽度为0，超出数组
				{
					curchar = SPACE_CHAR;
				}
				showbuffer[i++] = curchar;
				//printf("showbuffer[%d]=0x%x\n",i-1,curchar);
				if(curchar > ASCII_CODE)//字符宽度固定
				{
					doubleflag = 1;
					if(var->sector_offset>=var->bookbuffer_len)
					{
						if(_read_next_sector(var)==TO_THE_END)
						{
							EBookEGdbg("to the end\n");
							showbuffer[i++] = 0;
							showbuffer[i] = 0;
							totalnum_char = i;
							//printf("%s,%d:charnum=%d\n",__FILE__,__LINE__,totalnum_char);
							return TRUE;
						}
						var->sector_offset=0;
					}
					curchar = var->bookbuffer[var->sector_offset++];
					showbuffer[i++] = curchar;
					//printf("showbuffer[%d]=0x%x\n",i-1,curchar);
					length += 16;//EB_reading_Tex_F;
				}
				else//字符宽度不固定，需要查表！
				{
					length += *(INT8U*)(CharLengthInfo+curchar);
				}
					
				
				if(length > var->TxtW)
				{
					//智能断词
					//printf("%s,%d:length=%d\n",__FILE__,__LINE__,length); 
					if(flag && (!doubleflag)&&EBOOK_SPECIAL_CHAR(curchar)&&EBOOK_SPECIAL_CHAR(char_old))
					{
						{
							var->sector_offset = save_offset;
							i = save_i;
							if(var->sector!=save_sector)
							{
								var->sector = save_sector;
								_read_special_sector(var);
							}
							else
								var->sector = save_sector;
							break;
						}					
						/*INT32S k,j;
						i-=3;//指向前面可能的非可断符
						for(k=0; k<i; k++)
						{
							if(!EBOOK_SPECIAL_CHAR(showbuffer[i-k]))
								break;
						}
						i-=(k-1);
						if((j=(var->sector_offset-k-2))<0)
						{
							//printf("@ ");
							var->sector_offset = ONE_SECTOR_LENGTH+j;
							_read_prev_sector(var);
						}
						else
						{
							var->sector_offset-=(k+2);
						}*/
					}
					else
					{
						//if(curchar>=ASCII_CODE)
						if(doubleflag)
						{
							if(var->sector_offset==0)
							{
								_read_prev_sector(var);
								var->sector_offset = (ONE_SECTOR_LENGTH-2);
							}
							else if(var->sector_offset==1)
							{
								_read_prev_sector(var);
								var->sector_offset = (ONE_SECTOR_LENGTH-1);
							}
							else
							{
								var->sector_offset-=2;
							}
							i-=2;
						}
						else if(curchar > SPACE_CHAR)
						{
							if(var->sector_offset==0)
							{
								_read_prev_sector(var);
								var->sector_offset = (ONE_SECTOR_LENGTH-1);
							}
							else
							{
								var->sector_offset--;
							}
							i--;
						}
						else
						{
							i--;
						}
					}
					break;
			        }
				
				if(!EBOOK_SPECIAL_CHAR(curchar))
				{
					save_sector = var->sector;
					save_offset = var->sector_offset;
					save_i  = i;
					flag++;
				}
				
				char_old = curchar;
			}
			showbuffer[i++] = 0;
			showbuffer[i] = 0;
			totalnum_char = i;
			//printf("%s,%d:charnum=%d\n",__FILE__,__LINE__,totalnum_char);
			return (var->sector+var->sector_offset);
		}
		else//不是中日韩
		{
			//printf("TxtW=%d\n",var->TxtW);
			for(i=0,length=0;;)
			{
				if(var->sector_offset>=var->bookbuffer_len)
				{
					if(_read_next_sector(var)==TO_THE_END)
					{
						printf("to the end\n");
						#if 1
						showbuffer[i++] = 0x20;//解决字符串显示的bug
						showbuffer[i] = 0;
						#else
						showbuffer[i++] = 0;
						showbuffer[i] = 0;
						#endif
						totalnum_char = i;
						return TRUE;
					}
					var->sector_offset=0;
					//printf("^3");
				}
				curchar = var->bookbuffer[var->sector_offset++];
				if(curchar == NEW_LINE_CHAR) //换行符
				{
					break;
				}
				else if(curchar<SPACE_CHAR)//防止字符宽度为0，超出数组
				{
					curchar = SPACE_CHAR;
				}
				//printf("buffer[%d]=0x%X\n",i,curchar);
				showbuffer[i++] = curchar;			
				//字符宽度不固定，需要查表！
				length += *(INT8U*)(CharLengthInfo+curchar);
				if(length > var->TxtW)
				{
					//printf("curchar=0x%x,char_old=0x%x,flag=%d,i=%d\n",curchar,char_old,flag,i);
					if(flag && EBOOK_SPECIAL_CHAR(curchar)&&EBOOK_SPECIAL_CHAR(char_old))
					{
						{
							//printf("var->sector=0x%x,save_sector=0x%x,save_i=%d\n",var->sector,save_sector,save_i);
							if(save_i!=0){
								var->sector_offset = save_offset;
								i = save_i;
							}
							if(var->sector!=save_sector)
							{
								var->sector = save_sector;
								if(_read_special_sector(var)==READ_SECTOR_FAILE)
									printf("%s,%d:READ_SECTOR_FAILE\n",__FILE__,__LINE__);
							}
							//else
							//	var->sector = save_sector;
							break;
						}
					}
					else if(curchar > SPACE_CHAR){
						if(var->sector_offset==0){
							_read_prev_sector(var);
							var->sector_offset = (ONE_SECTOR_LENGTH-1);
						}
						else{
							var->sector_offset--;
						}
						i--;
					}
					else
						i--;
					break;
			        }
				
				if(!EBOOK_SPECIAL_CHAR(curchar))
				{
					//不是数字或字母，记录下当前的段和段内偏移
					//printf("i=%d.curchar=0x%x\n",i,curchar);
					save_sector = var->sector;
					save_offset = var->sector_offset;
					save_i  = i;
					flag++;
				}
				char_old = curchar;
			}
			//add 0 to the end of showbuffer and return the number of char which the function strncat would be used
			showbuffer[i++] = 0;
			showbuffer[i] = 0;
			totalnum_char = i;
			//printf("totalnum-char=%d,buff=%s\n",totalnum_char,showbuffer);
			
			return (var->sector+var->sector_offset);
		}
	}
	else if(var->type  == UNI16_LIT)	
	{
		for(i=0,length=0;;i+=2,var->sector_offset+=2)
		{
			if(var->sector_offset>=var->bookbuffer_len)
			{
				if(_read_next_sector(var)==TO_THE_END)
				{
					EBookEGdbg("to the end\n");
					*(INT16U*)(showbuffer+i) = SPACE_CHAR;
					*(INT16U*)(showbuffer+i+2) = END_STRING_CHAR;
					/*showbuffer[i++] = 0;
					showbuffer[i] = 0;*/
					totalnum_char = i;
					return TRUE;
				}
				var->sector_offset=0;
			}
			char_uni= *(INT16U*)(var->bookbuffer+var->sector_offset);
			//var->sector_offset+=2;
			if(char_uni == NEW_LINE_CHAR) //换行符
			{
				var->sector_offset+=2;
				break;
			}
			else if(char_uni<SPACE_CHAR)//防止字符宽度为0，超出数组
			{
				char_uni = SPACE_CHAR;
			}
	
			if(char_uni<ASCII_CODE)
				length += *(INT8U*)(CharLengthInfo+char_uni);
			else if(EBOOK_CJK_CHAR(char_uni))
				length += EB_reading_Tex_F;
			else{
				char_width = get_char_width(char_uni);
				if(char_width!=0)
					length += char_width;
				else
					length +=EB_reading_Tex_F;
			}
			//printf("length=%d\n",length);
			if(length > var->TxtW){
				if(flag && EBOOK_SPECIAL_CHAR(char_uni)&&EBOOK_SPECIAL_CHAR(char_uni_old)){
					{
						var->sector_offset = save_offset+2;
						i = save_i+2;
						if(var->sector!=save_sector)
						{
							var->sector = save_sector;
							_read_special_sector(var);
						}
						else
							var->sector = save_sector;
						break;
					}
				}
				else if(char_uni <= SPACE_CHAR){
					var->sector_offset+=2;
				}
				break;
		        }
			if(!EBOOK_SPECIAL_CHAR(char_uni)){
				save_sector = var->sector;
				save_offset = var->sector_offset;
				save_i  = i;
				flag++;
			}
			char_uni_old = char_uni;

			*(INT16U*)(showbuffer+i) = char_uni;
		}
		*(INT16U*)(showbuffer+i) = END_STRING_CHAR;
		totalnum_char = i;
		return (var->sector+var->sector_offset);
	}
	else if( var->type  == UNI16_BIG)
	{
		i=0;
		length=0;
		//printf("%s,%d:i=%d,length=%d,var->TxtW=%d\n",__func__,__LINE__,i,length,var->TxtW);
		for(;;)
		{
			i+=2;
			var->sector_offset+=2;
			if(var->sector_offset>=var->bookbuffer_len){
				if(_read_next_sector(var)==TO_THE_END){
					EBookEGdbg("to the end\n");
					*(INT16U*)(showbuffer+i) = SPACE_CHAR;
					*(INT16U*)(showbuffer+i+2) = END_STRING_CHAR;
					totalnum_char = i;
					return 1;
				}
				var->sector_offset=0;
			}
			char_uni= *(INT16U*)(var->bookbuffer+var->sector_offset);
			char_uni = (char_uni<<8) |(char_uni>>8);
		
			if(char_uni == NEW_LINE_CHAR){ //换行符
				var->sector_offset+=2;
				break;
			}
			else if(char_uni<SPACE_CHAR){//防止字符宽度为0，超出数组
				char_uni = SPACE_CHAR;
			}
			if(char_uni<ASCII_CODE)
				length += *(INT8U*)(CharLengthInfo+char_uni);
			else if(EBOOK_CJK_CHAR(char_uni))
				length += EB_reading_Tex_F;
			else{
				char_width = get_char_width(char_uni);
				if(char_width==0)
					length += EB_reading_Tex_F;
				else
					length += char_width;
			}
			if(length > var->TxtW){
				if(flag && EBOOK_SPECIAL_CHAR(char_uni)&&EBOOK_SPECIAL_CHAR(char_uni_old)){
					var->sector_offset = save_offset+2;
					i = save_i+2;
					if(var->sector!=save_sector){
						var->sector = save_sector;
						_read_special_sector(var);
					}
					else
						var->sector = save_sector;
					break;
				}
				else if(char_uni <= SPACE_CHAR){
					var->sector_offset+=2;
				}
				break;
		        }			
			if(!EBOOK_SPECIAL_CHAR(char_uni)){
				save_sector = var->sector;
				save_offset = var->sector_offset;
				save_i  = i;
				flag++;
			}
			char_uni_old = char_uni;
			
			*(INT16U*)(showbuffer+i) = char_uni;
		}
		*(INT16U*)(showbuffer+i) = END_STRING_CHAR;
		totalnum_char = i;
		//printf("%s,%d:call show one line:char=%d,0x%x,0x%x\n",__func__,__LINE__,totalnum_char,var->sector,var->sector_offset);
		return (var->sector+var->sector_offset);
	}
	else if(var->type  == UTF8)//直接读取操作
	{
		INT8U c1=0,c2=0,c3=0;
		for(i=0,length=0;;i+=2)
		{
			if(var->sector_offset>=var->bookbuffer_len)
			{
				if(_read_next_sector(var)==TO_THE_END)
				{
					EBookEGdbg("to the end\n");
					*(INT16U*)(showbuffer+i) = SPACE_CHAR;
					*(INT16U*)(showbuffer+i+2) = END_STRING_CHAR;
					totalnum_char = i;
					return TRUE;
				}
				var->sector_offset=0;
			}
			c1 = var->bookbuffer[var->sector_offset++];

			
			if((c1 & 0x80 ) == 0)//一字节
			{
				//printf("2\n");
				//*(INT8U*)(des++) = c1;
				//*(INT8U*)(des++) = 0x00;
				//printf("2-\n");
				char_uni = c1;
			}
			else if((c1 & 0xf0 ) == 0xe0)//三字节
			{

				if((var->sector_offset+1)>=var->bookbuffer_len)//少一个字节
				{
					c2 = var->bookbuffer[var->sector_offset];
					_read_next_sector(var);
					var->sector_offset = 0;
					c3 = var->bookbuffer[var->sector_offset++];
				}
				else if(var->sector_offset>=var->bookbuffer_len)//少两个字节
				{
					_read_next_sector(var);
					var->sector_offset = 0;
				}
				else
				{
					c2 = var->bookbuffer[var->sector_offset++];
					c3 = var->bookbuffer[var->sector_offset++];
				}
				char_uni = (INT8U)((INT8U)((c2&0x0003)<<6) |(INT8U)(c3&0x003f))|((INT8U)(((c1&0x000f)<<4 )|((c2&0x003c)>>2))<<8);

			//	*(INT8U*)(des++) = (INT8U)((INT8U)((c2&0x0003)<<6) |(INT8U)(c3&0x003f));
			//	*(INT8U*)(des++) = (INT8U)(((c1&0x000f)<<4 )|((c2&0x003c)>>2));	
				//printf("3-\n");
			}
			else if((c1 & 0xe0 ) == 0xc0)//二字节
			{
				if((var->sector_offset)>=var->bookbuffer_len)//少一个字节
				{
					_read_next_sector(var);
					var->sector_offset = 0;
					c2 = var->bookbuffer[var->sector_offset++];
				}
				else
				{
					c2 = var->bookbuffer[var->sector_offset++];
				}
				char_uni = (INT8U)((c2 & 0x3f) |(c1 & 0x03)<<6)|((INT8U)((c1&0x1c)>>2)<<8);
			}
			
			if(char_uni == NEW_LINE_CHAR) //换行符
			{
				break;
			}
			else if(char_uni<SPACE_CHAR)//防止字符宽度为0，超出数组
			{
				char_uni = SPACE_CHAR;
			}

			if(char_uni<ASCII_CODE)
				length += *(INT8U*)(CharLengthInfo+char_uni);
			else if(EBOOK_CJK_CHAR(char_uni))
				length += EB_reading_Tex_F;			
			else{
				char_width = get_char_width(char_uni);
				if(char_width!=0)
					length +=char_width;
				else
					length +=EB_reading_Tex_F;
			}
		
			//printf("length=%d\n",length);
			if(length > var->TxtW){
				if(flag && EBOOK_SPECIAL_CHAR(char_uni)&&EBOOK_SPECIAL_CHAR(char_uni_old)){
					var->sector_offset = save_offset;
					i = save_i+2;
					if(var->sector!=save_sector){
						var->sector = save_sector;
						_read_special_sector(var);
					}
					else
						var->sector = save_sector;
					break;
				}
				else	if(char_uni > SPACE_CHAR){
					INT8S less_zero=0;
					if((c1 & 0x80 ) == 0)//一字节
					{
						var->sector_offset--;
						less_zero = 1;
					}
					else if((c1 & 0xf0 ) == 0xe0)//三字节
					{
						var->sector_offset-=3;
						less_zero = 3;
					}
					else if((c1 & 0xe0 ) == 0xc0)//二字节
					{
						var->sector_offset-=2;
						less_zero = 2;
					}	

					if(var->sector_offset<0)
					{
						var->sector_offset = ONE_SECTOR_LENGTH-less_zero;
						_read_prev_sector(var);
					}					
					break;
				}
		        }

			if(!EBOOK_SPECIAL_CHAR(char_uni))
			{
				save_sector = var->sector;
				save_offset = var->sector_offset;
				save_i  = i;
				flag++;
			}
			char_uni_old = char_uni;

			*(INT16U*)(showbuffer+i) = char_uni;
		}
		*(INT16U*)(showbuffer+i) = END_STRING_CHAR;
		totalnum_char = i;
		return (var->sector+var->sector_offset);
	}
	else
		return 0;
}


INT32U get_ebook_next_nline(eb_show_var_t *var, INT32U lines)
{
	INT32U i,y;
	
	for(i=0;i<lines;i++)
	{
		if(!get_ebook_oneline(var))	
		{
			break;
		}
	}
	return i;
}


/**
 * @brief  get the file type.
 * @param[in] none.
 * @return none.
 */ 
INT32S get_ebook_filetype(eb_show_var_t *var)
{
	var->sector = 0;
	_read_special_sector(var);
	if(((INT8U)var->bookbuffer[0] == 0xff)&&((INT8U)var->bookbuffer[1] == 0xfe))
     	{
	   	 var->type = UNI16_LIT;
		 printf("File type == little\n");
	}
	 else if(((INT8U)var->bookbuffer[0] == 0xfe)&&((INT8U)var->bookbuffer[1] == 0xff))
 	{
	   	 var->type  = UNI16_BIG;
		printf("File type == big\n");
	 }
	else  if(((INT8U)var->bookbuffer[0] == 0xef)&&((INT8U)var->bookbuffer[1] == 0xbb)\
	 	&& ((INT8U)var->bookbuffer[2] == 0xbf))
 	{
		var->type  = UTF8;
		printf("File type == UTF8\n");
	}
	else
   	{
		var->type  = MBCS;   //Mulitiple Byte Character Set
		printf("File type == MBCS\n");
	}
	return var->type;
}



/**
 * @brief  turn to the current point.
 * @param[in] displaypoint: the offset of file.
 * @return none.
 */ 
void turn_to_point(INT32S point, eb_show_var_t *var)
{
	var->sector = point&REVERSE512;
	var->sector_offset = point%ONE_SECTOR_LENGTH;
	_read_special_sector(var);
}

INT32S goto_the_line(INT32S line, eb_show_var_t *var)
{
	INT32U i,page;
	INT32U offset;
	INT32S pageoffset=0;
	//printf("wjb_PageOffsetSave=%x\n",PageOffsetSave[page/10]);
	//printf("goto_the_line=%d,%d,%d\n",line,var->totalLine,var->MaxLine);
	if(line<=0||line>var->totalLine){
		printf("Ebook BG  error\n");
		return EBOOK_BG_ERROR;
	}

	page = 1+ ((line-1)/var->MaxLine);
	pageoffset = (page-1)/var->LineSaveStep;
	if(pageoffset > EB_PAGESAVE_LENGTH){
		printf("Error! ,page offset larger than EB_PAGESAVE_LANGTH\n");
		pageoffset = 0;	
	}
	offset = *(var->PageOffsetSave +pageoffset);

	
	var->sector = offset&REVERSE512;
	var->sector_offset = offset%ONE_SECTOR_LENGTH;
	if(_read_special_sector(var)==READ_SECTOR_FAILE){
		printf("Read Sector Failed!\n");
		return -1;
	}
	i = 1+((page-1)%var->LineSaveStep);
//printf("$$i=%d\n",i);
	//printf("<<<<line=%d,page=%d,offset=%x,i=%d,sector=0x%x,offset=0x%x\n",line,page,offset,i,var->sector,var->sector_offset);
	for( ; i>1; i--)
	{
		get_ebook_next_nline(var, var->MaxLine);
	}
	i = (line-1)%var->MaxLine;
	get_ebook_next_nline(var,i);
	return 0;
}



INT32S goto_the_page(INT32S page, eb_show_var_t *var)
{
	INT32U i;
	INT32U offset;
	INT32S pageoffset;
	//printf("wjb_PageOffsetSave=%x\n",PageOffsetSave[page/10]);

	if(page<=0||page>var->TotlePage)
		return EBOOK_BG_ERROR;

	pageoffset = (page-1)/var->LineSaveStep;
	if(pageoffset > EB_PAGESAVE_LENGTH){
		printf("Error! ,page offset larger than EB_PAGESAVE_LANGTH\n");
		pageoffset = 0;	
	}
	offset = *(var->PageOffsetSave + pageoffset);
	var->sector = offset&REVERSE512;
	var->sector_offset = offset%ONE_SECTOR_LENGTH;
	_read_special_sector(var);

	i = 1+((page-1)%var->LineSaveStep);
	//printf("p=%d,offset=%x,i=%d\n",page,offset,i);

	for( ; i>1; i--)
	{
		//printf("~\n");
		get_ebook_next_nline(var, var->MaxLine);
	}
	return 0;
}



// task ctrl  function
INT32S   ebook_bg_ioctl(INT32U  cmd, void *param)
{
	INT32S  result = 1;
	//printf("BG_EBookCtrl 0x%x\n",cmd);
	
	switch(cmd){
		
	case COUNT_TOTAL_PAGE:
		bg_show_var = *(eb_show_var_t *)param;
		bg_counting = 1;
		//S_OSSemPost(ebook_cmd_sem);//hikwlu
		ebook_info_parse();
		break;
		
	case GET_CUR_LINE_NUM:
		return bg_show_var.curLine;
		break;

	case GET_CUR_PAGE_NUM:
		bg_show_var.CurPage = 1+ ((bg_show_var.curLine-1)/bg_show_var.MaxLine);
		return bg_show_var.CurPage;
		break;
		
	case GET_TOTAL_PAGE_NUM:
		if(bg_counting)
			return EBOOK_BG_ERROR;
		else
			return bg_show_var.TotlePage;
		break;
		
	case GOTO_SPECIAL_LINE:
		goto_the_line(((eb_show_var_t *)param)->curLine, ((eb_show_var_t *)param));
		((eb_show_var_t *)param)->cur_point = ((eb_show_var_t *)param)->sector_offset +((eb_show_var_t *)param)->sector;
		break;

	case GOTO_SPECIAL_PAGE:
		//printf("page=%d\n",((eb_show_var_t *)param)->CurPage);
		result = goto_the_page(((eb_show_var_t *)param)->CurPage, ((eb_show_var_t *)param));
		((eb_show_var_t *)param)->cur_point = ((eb_show_var_t *)param)->sector_offset +((eb_show_var_t *)param)->sector;
		break;
		
	case GET_SPECIAL_LINE_DATA:
		if(goto_the_line(((eb_show_var_t *)param)->curLine, ((eb_show_var_t *)param))!=-1)
			get_ebook_oneline(((eb_show_var_t *)param));
		else{
			totalnum_char= 0;
		}
		break;

	case GOTO_CUR_POINT:
		//goto_the_point((eb_show_var_t *)param);
		turn_to_point(((eb_show_var_t *)param)->cur_point,(eb_show_var_t *)param);
		break;
		
	case CONTINUE_SHOW_ONE_LINE:
		result = get_ebook_oneline((eb_show_var_t *)param);
		break;
		
	case GET_FILE_TYPE:
		result = get_ebook_filetype(((eb_show_var_t *)param));
		break;
		
	case BG_EBOOK_TASK_STOP:
		bg_counting = 0;
		break;
		
	default:
		break;
		
	}
	return result;
	
}
void ebook_info_parse(void)
{
	INT8U  result;
	INT32U ret=0,page_offset=0,line;
	line = bg_show_var.LineSaveStep*bg_show_var.MaxLine;
	printf("<<<<<<<<<<<ebook eg pend end,line=%d\n",line);
	bg_show_var.sector = 0;
	bg_show_var.sector_offset = 0;
	_read_special_sector(&bg_show_var);
	bg_show_var.curLine = 0;
	while(1){
		if(bg_show_var.curLine%line==0){
			//printf("ret=%x ",ret);
			if(page_offset<EB_PAGESAVE_LENGTH){
				*(bg_show_var.PageOffsetSave+(page_offset++)) = ret;//保存页数偏移信息
			}
			else{
				printf("page save space is not enought!\n");
				break;
			}
		}
		if((ret =get_ebook_oneline(&bg_show_var))==0){
			printf("@@@%s %d::curlin=%d\n",__FILE__,__LINE__,bg_show_var.curLine);
			bg_show_var.TotlePage = 1+ ((bg_show_var.curLine-1)/bg_show_var.MaxLine);
			bg_counting = 0;
			bg_show_var.curLine++; //need to add one more line
			break;
		}
		bg_show_var.curLine++;
	}
}
	

/*
// task main function
static void BG_EBookTask()
{
	INT8U  result;
	INT32U ret=0,page_offset=0,line;
	//printf("<<<<<<<<<<ebook eg pend\n");
	S_OSSemPend(ebook_cmd_sem,0,&result);
	line = bg_show_var.LineSaveStep*bg_show_var.MaxLine;
	printf("<<<<<<<<<<<ebook eg pend end,line=%d\n",line);
	bg_show_var.sector = 0;
	bg_show_var.sector_offset = 0;
	_read_special_sector(&bg_show_var);
	bg_show_var.curLine = 0;
	while(1){
		//printf("@~");
		if(bg_counting){
			if(bg_show_var.curLine%line==0){
				//printf("ret=%x ",ret);
				if(page_offset<EB_PAGESAVE_LENGTH){
					*(bg_show_var.PageOffsetSave+(page_offset++)) = ret;//保存页数偏移信息
				}
				else{
					printf("page save space is not enought!\n");
				}
			}
			if((ret =get_ebook_oneline(&bg_show_var))==0){
				printf("@@@%s %d::curlin=%d\n",__FILE__,__LINE__,bg_show_var.curLine);
				bg_show_var.TotlePage = 1+ ((bg_show_var.curLine-1)/bg_show_var.MaxLine);
				bg_counting = 0;
				bg_show_var.curLine++; //need to add one more line
				//S_OSTaskDel(OS_PRIO_SELF);//delete the task 
			}
			bg_show_var.curLine++;
		}
		else{
			//printf("BG_EBookTask time Dly\n");
			// S_OSTimeDly(5);
		}	
	}
}

// task create
INT32U  BG_CreateEBookTask(void)
{
	INT8U result;
	bg_ebook_stack=(char *)malloc(BG_EBOOK_STACK_SIZE);
	
	if(bg_ebook_stack==NULL){
		printf("%s,%d:malloc failed\n",__FILE__,__LINE__);
		return 0;
	}
	result = S_OSTaskCreate(BG_EBookTask,NULL,(OS_STK *)&bg_ebook_stack[(BG_EBOOK_STACK_SIZE>>2)-2],\
		TASKPRIO_UI_EBookEG,TASKID_UI_RadioEG,bg_ebook_stack);
	
	//printf("%s,%d\n",__FILE__,__LINE__);
	
	if(result!=OS_NO_ERR){
		printf("task create error~\n");
		free(bg_ebook_stack);
		return 0;
	}
	
	printf("create bg task\n");
	ebook_cmd_sem = (OS_EVENT *)S_OSSemCreate(0);
	return 1;

}


INT32U   BG_DeleteEBookTask(void)
{
	INT8U  result;
	S_OSTaskDel(TASKPRIO_UI_EBookEG);
	free(bg_ebook_stack);
	S_OSSemDel(ebook_cmd_sem,OS_DEL_ALWAYS,&result);
	printf("del bg task\n");
	return 0;
}

void EXCEP_EbookEngineClose()
{
	printf("excep Ebook\n");
	if(IsTaskCreate){
		BG_EBookCtrl(BG_EBOOK_TASK_STOP,0);
		S_OSTimeDly(5);
		BG_DeleteEBookTask();
		S_FS_FClose(EBookTaskVars.showvar.fp);
		if(EBookTaskVars.showvar.PageOffsetSave !=NULL){
			SWF_Free(EBookTaskVars.showvar.PageOffsetSave);
		}
		EBookTaskVars.showvar.fp = 0;
		IsTaskCreate = 0;
	}
	printf("excep Ebook ok\n");
}
*/
#endif	/** MODULE_CONFIG_EBOOK */