/*
  Copyright (c) 2009-2017 Dave Gamble and   tacJSON contributors

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

#ifndef   tacJSON__h
#define   tacJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

  tacJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
  tacJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
  tacJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the   tacJSON_API_VISIBILITY flag to "export" the same symbols the way   tacJSON_EXPORT_SYMBOLS does

*/

#define   tacJSON_CDECL __cdecl
#define   tacJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(  tacJSON_HIDE_SYMBOLS) && !defined(  tacJSON_IMPORT_SYMBOLS) && !defined(  tacJSON_EXPORT_SYMBOLS)
#define   tacJSON_EXPORT_SYMBOLS
#endif

#if defined(  tacJSON_HIDE_SYMBOLS)
#define   tacJSON_PUBLIC(type)   type   tacJSON_STDCALL
#elif defined(  tacJSON_EXPORT_SYMBOLS)
#define   tacJSON_PUBLIC(type)   __declspec(dllexport) type   tacJSON_STDCALL
#elif defined(  tacJSON_IMPORT_SYMBOLS)
#define   tacJSON_PUBLIC(type)   __declspec(dllimport) type   tacJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define   tacJSON_CDECL
#define   tacJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(  tacJSON_API_VISIBILITY)
#define   tacJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define   tacJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define   tacJSON_VERSION_MAJOR 1
#define   tacJSON_VERSION_MINOR 7
#define   tacJSON_VERSION_PATCH 15

#include <stddef.h>

/*   tacJSON Types: */
#define   tacJSON_Invalid (0)
#define   tacJSON_False  (1 << 0)
#define   tacJSON_True   (1 << 1)
#define   tacJSON_NULL   (1 << 2)
#define   tacJSON_Number (1 << 3)
#define   tacJSON_String (1 << 4)
#define   tacJSON_Array  (1 << 5)
#define   tacJSON_Object (1 << 6)
#define   tacJSON_Raw    (1 << 7) /* raw json */

#define   tacJSON_IsReference 256
#define   tacJSON_StringIsConst 512

/* The   tacJSON structure: */
typedef struct   tacJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct   tacJSON *next;
    struct   tacJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct   tacJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==  tacJSON_String  and type ==   tacJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use   tacJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==  tacJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
}   tacJSON;

typedef struct   tacJSON_Hooks
{
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(  tacJSON_CDECL *malloc_fn)(size_t sz);
      void (  tacJSON_CDECL *free_fn)(void *ptr);
}   tacJSON_Hooks;

typedef int   tacJSON_bool;

/* Limits how deeply nested arrays/objects can be before   tacJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef   tacJSON_NESTING_LIMIT
#define   tacJSON_NESTING_LIMIT 1000
#endif

/* returns the version of   tacJSON as a string */
  tacJSON_PUBLIC(const char*)   tacJSON_Version(void);

/* Supply malloc, realloc and free functions to   tacJSON */
  tacJSON_PUBLIC(void)   tacJSON_InitHooks(  tacJSON_Hooks* hooks);

/* Memory Management: the caller is always responsible to free the results from all variants of   tacJSON_Parse (with   tacJSON_Delete) and   tacJSON_Print (with stdlib free,   tacJSON_Hooks.free_fn, or   tacJSON_free as appropriate). The exception is   tacJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a   tacJSON object you can interrogate. */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_Parse(const char *value);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match   tacJSON_GetErrorPtr(). */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_ParseWithOpts(const char *value, const char **return_parse_end,   tacJSON_bool require_null_terminated);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end,   tacJSON_bool require_null_terminated);

/* Render a   tacJSON entity to text for transfer/storage. */
  tacJSON_PUBLIC(char *)   tacJSON_Print(const   tacJSON *item);
/* Render a   tacJSON entity to text for transfer/storage without any formatting. */
  tacJSON_PUBLIC(char *)   tacJSON_PrintUnformatted(const   tacJSON *item);
/* Render a   tacJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
  tacJSON_PUBLIC(char *)   tacJSON_PrintBuffered(const   tacJSON *item, int prebuffer,   tacJSON_bool fmt);
/* Render a   tacJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE:   tacJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_PrintPreallocated(  tacJSON *item, char *buffer, const int length, const   tacJSON_bool format);
/* Delete a   tacJSON entity and all subentities. */
  tacJSON_PUBLIC(void)   tacJSON_Delete(  tacJSON *item);

/* Returns the number of items in an array (or object). */
  tacJSON_PUBLIC(int)   tacJSON_GetArraySize(const   tacJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_GetArrayItem(const   tacJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_GetObjectItem(const   tacJSON * const object, const char * const string);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_GetObjectItemCaseSensitive(const   tacJSON * const object, const char * const string);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_HasObjectItem(const   tacJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when   tacJSON_Parse() returns 0. 0 when   tacJSON_Parse() succeeds. */
  tacJSON_PUBLIC(const char *)   tacJSON_GetErrorPtr(void);

/* Check item type and return its value */
  tacJSON_PUBLIC(char *)   tacJSON_GetStringValue(const   tacJSON * const item);
  tacJSON_PUBLIC(double)   tacJSON_GetNumberValue(const   tacJSON * const item);

/* These functions check the type of an item */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsInvalid(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsFalse(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsTrue(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsBool(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsNull(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsNumber(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsString(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsArray(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsObject(const   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_IsRaw(const   tacJSON * const item);

/* These calls create a   tacJSON item of the appropriate type. */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateNull(void);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateTrue(void);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateFalse(void);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateBool(  tacJSON_bool boolean);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateNumber(double num);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateString(const char *string);
/* raw json */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateRaw(const char *raw);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateArray(void);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by   tacJSON_Delete */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by   tacJSON_Delete */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateObjectReference(const   tacJSON *child);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateArrayReference(const   tacJSON *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateIntArray(const int *numbers, int count);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateFloatArray(const float *numbers, int count);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateDoubleArray(const double *numbers, int count);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_CreateStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_AddItemToArray(  tacJSON *array,   tacJSON *item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_AddItemToObject(  tacJSON *object, const char *string,   tacJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the   tacJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type &   tacJSON_StringIsConst) is zero before
 * writing to `item->string` */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_AddItemToObjectCS(  tacJSON *object, const char *string,   tacJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing   tacJSON to a new   tacJSON, but don't want to corrupt your existing   tacJSON. */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_AddItemReferenceToArray(  tacJSON *array,   tacJSON *item);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_AddItemReferenceToObject(  tacJSON *object, const char *string,   tacJSON *item);

/* Remove/Detach items from Arrays/Objects. */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_DetachItemViaPointer(  tacJSON *parent,   tacJSON * const item);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_DetachItemFromArray(  tacJSON *array, int which);
  tacJSON_PUBLIC(void)   tacJSON_DeleteItemFromArray(  tacJSON *array, int which);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_DetachItemFromObject(  tacJSON *object, const char *string);
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_DetachItemFromObjectCaseSensitive(  tacJSON *object, const char *string);
  tacJSON_PUBLIC(void)   tacJSON_DeleteItemFromObject(  tacJSON *object, const char *string);
  tacJSON_PUBLIC(void)   tacJSON_DeleteItemFromObjectCaseSensitive(  tacJSON *object, const char *string);

/* Update array items. */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_InsertItemInArray(  tacJSON *array, int which,   tacJSON *newitem); /* Shifts pre-existing items to the right. */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_ReplaceItemViaPointer(  tacJSON * const parent,   tacJSON * const item,   tacJSON * replacement);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_ReplaceItemInArray(  tacJSON *array, int which,   tacJSON *newitem);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_ReplaceItemInObject(  tacJSON *object,const char *string,  tacJSON *newitem);
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_ReplaceItemInObjectCaseSensitive(  tacJSON *object,const char *string,  tacJSON *newitem);

/* Duplicate a   tacJSON item */
  tacJSON_PUBLIC(  tacJSON *)   tacJSON_Duplicate(const   tacJSON *item,   tacJSON_bool recurse);
/* Duplicate will create a new, identical   tacJSON item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two   tacJSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
  tacJSON_PUBLIC(  tacJSON_bool)   tacJSON_Compare(const   tacJSON * const a, const   tacJSON * const b, const   tacJSON_bool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable address area. */
  tacJSON_PUBLIC(void)   tacJSON_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddNullToObject(  tacJSON * const object, const char * const name);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddTrueToObject(  tacJSON * const object, const char * const name);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddFalseToObject(  tacJSON * const object, const char * const name);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddBoolToObject(  tacJSON * const object, const char * const name, const   tacJSON_bool boolean);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddNumberToObject(  tacJSON * const object, const char * const name, const double number);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddStringToObject(  tacJSON * const object, const char * const name, const char * const string);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddRawToObject(  tacJSON * const object, const char * const name, const char * const raw);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddObjectToObject(  tacJSON * const object, const char * const name);
  tacJSON_PUBLIC(  tacJSON*)   tacJSON_AddArrayToObject(  tacJSON * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define   tacJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the   tacJSON_SetNumberValue macro */
  tacJSON_PUBLIC(double)   tacJSON_SetNumberHelper(  tacJSON *object, double number);
#define   tacJSON_SetNumberValue(object, number) ((object != NULL) ?   tacJSON_SetNumberHelper(object, (double)number) : (number))
/* Change the valuestring of a   tacJSON_String object, only takes effect when type of object is   tacJSON_String */
  tacJSON_PUBLIC(char*)   tacJSON_SetValuestring(  tacJSON *object, const char *valuestring);

/* Macro for iterating over an array or object */
#define   tacJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with   tacJSON_InitHooks */
  tacJSON_PUBLIC(void *)   tacJSON_malloc(size_t size);
  tacJSON_PUBLIC(void)   tacJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
