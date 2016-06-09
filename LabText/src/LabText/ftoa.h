/*
 Copyright (c) 2009 NetAllied Systems GmbH
 
 This file is part of Common libftoa.
 
 Licensed under the MIT Open Source License, 
 for details please see LICENSE file or the website
 http://www.opensource.org/licenses/mit-license.php
 
 Note: I have removed the C++'ness of it, in order to backport it to C
 */

/// @TODO ftoa should take a buffer size for safety

#ifndef __COMMON_FTOA_H__
#define __COMMON_FTOA_H__

#ifndef EXTERNC
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
#endif

#define FTOA_BUFFER_SIZE 30
EXTERNC int ftoa(float f, char* buffer);

#endif // __COMMON_FTOA_H__

