/*
 *  linux/lib/kasprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <stdarg.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>

/* Simplified asprintf. */
char *kvasprintf(gfp_t gfp, const char *fmt, va_list ap)
{
	unsigned int len;
	char *p;
	va_list aq;
	
	//am_printf("%s:%d \n",__func__,__LINE__);

	va_copy(aq, ap);
	
	//am_printf("%s:%d \n",__func__,__LINE__);
	len = vsnprintf(NULL, 0, fmt, aq);
	
	//am_printf("%s:%d \n",__func__,__LINE__);
	va_end(aq);
	//am_printf("%s:%d \n",__func__,__LINE__);

	p = kmalloc(len+1, gfp);
	
	//am_printf("%s:%d \n",__func__,__LINE__);
	if (!p)
		return NULL;
	//am_printf("%s:%d \n",__func__,__LINE__);

	vsnprintf(p, len+1, fmt, ap);
	
	//am_printf("%s:%d \n",__func__,__LINE__);

	return p;
}
EXPORT_SYMBOL(kvasprintf);

char *kasprintf(gfp_t gfp, const char *fmt, ...)
{
	va_list ap;
	char *p;
	//am_printf("%s:%d \n",__func__,__LINE__);

	va_start(ap, fmt);
	
	//am_printf("%s:%d \n",__func__,__LINE__);
	p = kvasprintf(gfp, fmt, ap);
	
	//am_printf("%s:%d \n",__func__,__LINE__);
	va_end(ap);

	return p;
}
EXPORT_SYMBOL(kasprintf);
