/*
 Copyright (c) 2009 NetAllied Systems GmbH
 
 This file is part of Common libftoa.
 
 Licensed under the MIT Open Source License, 
 for details please see LICENSE file or the website
 http://www.opensource.org/licenses/mit-license.php
 
 Note: I have removed the C++'ness of it, in order to backport it to C
 */

/// @TODO itoa and uitoa should take a buffersize for safety

#ifndef __COMMON_ITOA_H__
#define __COMMON_ITOA_H__

#include <stdlib.h>

#ifndef EXTERNC
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
#endif

EXTERNC size_t uitoa(unsigned int, char* buffer, unsigned char radix);
EXTERNC size_t itoa(signed int, char* buffer, unsigned char radix);

#endif // __COMMON_ITOA_H__
