/*
 Copyright (c) 2009 NetAllied Systems GmbH
 
 This file is part of Common libftoa.
 
 Licensed under the MIT Open Source License, 
 for details please see LICENSE file or the website
 http://www.opensource.org/licenses/mit-license.php
 */

#include "itoa.h"

/** Writes the string representation value into @a buffer. @a value must be positive, even for signed types. */
size_t uitoa(unsigned int value, char* buffer, unsigned char radix)
{
	typedef unsigned char DigitType;
	char *p = buffer;                
	
	// We convert to a string in reversed order and store the position of the first digit
	
	char *firstDigitPos = p;      
	
	do {
		DigitType digit = (DigitType) (value % radix);
		value /= radix;       
		
		if (digit > 9)
		{
			*p++ = (char) (digit - 10 + 'a'); 
		}
		else
		{
			*p++ = (char) (digit + '0');      
		}
	} while (value > 0);
	
	// determine the bytes written, without null termination
	size_t bytesWritten = p - buffer;
	
	// null termination, and step back to the last digit
	*p-- = 0;
	
	// we need to revert the string
	do {
		char temp = *p;
		*p = *firstDigitPos;
		*firstDigitPos = temp;   
		--p;
		++firstDigitPos;         
	} while (firstDigitPos < p); 
	
	return bytesWritten;
}


size_t itoa(int value, char* buffer, unsigned char radix)
{
	char *p = buffer;
	
	if ( value < 0 )
	{
		*p++ = '-';
		return uitoa( -value, p, radix) + 1;
	}
	else
	{
		return uitoa( value, p, radix);
	}
}
