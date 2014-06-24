#ifndef __TYPES_H__
#define __TYPES_H__

/** @file   types.h
 *  @brief  Portabillity layer for various types
 *
 *          Mainly to support compilers (such as older codevision) which do 
 *          not provide C99 stdint.h or similar.
 *  
 */
 #include <stdint.h>
 

/*
typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef int             int16_t;
typedef unsigned int    uint16_t;
typedef long            int32_t;
typedef unsigned long   uint32_t;
*/

#ifndef  _MSC_VER
#include <stdbool.h>
#else 
#ifndef __cplusplus
typedef unsigned char bool;
#define true  1
#define false 0
#endif
#endif

#endif