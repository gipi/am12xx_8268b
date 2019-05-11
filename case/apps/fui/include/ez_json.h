/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef JSON__h
#define JSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* JSON Types: */
#define JSON_False 0
#define JSON_True 1
#define JSON_NULL 2
#define JSON_Number 3
#define JSON_String 4
#define JSON_Array 5
#define JSON_Object 6
	
#define JSON_IsReference 256

#if 1 //Obsoleted.
static void *(*JSON_malloc)(size_t sz) = malloc;
static void (*JSON_free)(void *ptr) = free;
#else
#define	JSON_malloc(size)		ezMalloc(size)
#define	JSON_free(ptr)			ezFree(ptr)

__attribute__ ((visibility("default")))
void *ezJSON_malloc(size_t sz)
{
	return ( JSON_malloc(sz) );
}

__attribute__ ((visibility("default")))
void ezJSON_free(void *ptr)
{
	JSON_free(ptr);
}
#endif

/* The JSON structure: */
typedef struct JSON {
	struct JSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct JSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==JSON_String */
	int valueint;				/* The item's number, if type==JSON_Number */
	double valuedouble;			/* The item's number, if type==JSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} JSON;

typedef struct JSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} JSON_Hooks;

/* Supply malloc, realloc and free functions to JSON */
extern void JSON_InitHooks(JSON_Hooks* hooks);


/* Supply a block of JSON, and this returns a JSON object you can interrogate. Call JSON_Delete when finished. */
extern JSON *JSON_Parse(const char *value);
/* Supply a block of JSON, and this returns a JSON object you can interrogate. Call JSON_Delete when finished.
 * end_ptr will point to 1 past the end of the JSON object */
extern JSON *JSON_Parse_Stream(const char *value, char **end_ptr);
/* Render a JSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *JSON_Print(JSON *item);
/* Render a JSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *JSON_PrintUnformatted(JSON *item);
/* Delete a JSON entity and all subentities. */
extern void   JSON_Delete(JSON *c);

/* Returns the number of items in an array (or object). */
extern int	  JSON_GetArraySize(JSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern JSON *JSON_GetArrayItem(JSON *array,int item);
/* Get item "string" from object. Case insensitive. */
extern JSON *JSON_GetObjectItem(JSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when JSON_Parse() returns 0. 0 when JSON_Parse() succeeds. */
extern const char *JSON_GetErrorPtr();

/* These calls create a JSON item of the appropriate type. */
extern JSON *JSON_CreateNull();
extern JSON *JSON_CreateTrue();
extern JSON *JSON_CreateFalse();
extern JSON *JSON_CreateBool(int b);
extern JSON *JSON_CreateNumber(double num);
extern JSON *JSON_CreateString(const char *string);
extern JSON *JSON_CreateArray();
extern JSON *JSON_CreateObject();

/* These utilities create an Array of count items. */
extern JSON *JSON_CreateIntArray(int *numbers,int count);
extern JSON *JSON_CreateFloatArray(float *numbers,int count);
extern JSON *JSON_CreateDoubleArray(double *numbers,int count);
extern JSON *JSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void JSON_AddItemToArray(JSON *array, JSON *item);
extern void	JSON_AddItemToObject(JSON *object,const char *string,JSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing JSON to a new JSON, but don't want to corrupt your existing JSON. */
extern void JSON_AddItemReferenceToArray(JSON *array, JSON *item);
extern void	JSON_AddItemReferenceToObject(JSON *object,const char *string,JSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern JSON *JSON_DetachItemFromArray(JSON *array,int which);
extern void   JSON_DeleteItemFromArray(JSON *array,int which);
extern JSON *JSON_DetachItemFromObject(JSON *object,const char *string);
extern void   JSON_DeleteItemFromObject(JSON *object,const char *string);
	
/* Update array items. */
extern void JSON_ReplaceItemInArray(JSON *array,int which,JSON *newitem);
extern void JSON_ReplaceItemInObject(JSON *object,const char *string,JSON *newitem);

#define JSON_AddNullToObject(object,name)	JSON_AddItemToObject(object, name, JSON_CreateNull())
#define JSON_AddTrueToObject(object,name)	JSON_AddItemToObject(object, name, JSON_CreateTrue())
#define JSON_AddFalseToObject(object,name)		JSON_AddItemToObject(object, name, JSON_CreateFalse())
#define JSON_AddNumberToObject(object,name,n)	JSON_AddItemToObject(object, name, JSON_CreateNumber(n))
#define JSON_AddStringToObject(object,name,s)	JSON_AddItemToObject(object, name, JSON_CreateString(s))

#ifdef __cplusplus
}
#endif

#endif

