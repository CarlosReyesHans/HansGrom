/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * Header file for Application types.
 *
 * Part of the application, host declarations of application types mapped against
 * ServiceData Objects and ProcessData Objects.
 */

#ifndef __utypes_h__
#define __utypes_h__

#include <cc.h>

typedef struct
{
   uint16_t temp[15];
   uint16_t status;
   uint16_t event;
   uint16_t error;
} _Rbuffer;

typedef struct
{
   uint16_t command;
   uint16_t testVal0;
   uint16_t testVal1;
   uint16_t testVal2;
} _Wbuffer;

typedef struct
{
   uint32_t reset_counter;
} _Cbuffer;

//extern _Rbuffer Rb;
//extern _Wbuffer Wb;
//extern _Cbuffer Cb;

#endif
