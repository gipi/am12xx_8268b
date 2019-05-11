#ifndef SYSTEM_RES_H
#define SYSTEM_RES_H

int	gb2312_to_utf8(char   *inbuf,size_t   *inlen, char   *outbuf, size_t   *outlen) ;
int	gb2312_to_utf16le(char   *inbuf, size_t *inlen, char   *outbuf, size_t   *outlen) ;
int	utf16le_to_utf8(char   *inbuf, size_t *inlen, char   *outbuf, size_t   *outlen) ;
int	utf16be_to_utf8(char   *inbuf, size_t *inlen, char   *outbuf, size_t   *outlen) ;
int	get_char_width(unsigned short aChar);

#endif


