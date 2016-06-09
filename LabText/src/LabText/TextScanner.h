
/*
 __  __           _           _       
|  \/  | ___  ___| |__  _   _| | __ _ 
| |\/| |/ _ \/ __| '_ \| | | | |/ _` |
| |  | |  __/\__ \ | | | |_| | | (_| |
|_|  |_|\___||___/_| |_|\__,_|_|\__,_|
 _____         _   ____                                  
|_   _|____  _| |_/ ___|  ___ __ _ _ __  _ __   ___ _ __ 
  | |/ _ \ \/ / __\___ \ / __/ _` | '_ \| '_ \ / _ \ '__|
  | |  __/>  <| |_ ___) | (_| (_| | | | | | | |  __/ |   
  |_|\___/_/\_\\__|____/ \___\__,_|_| |_|_| |_|\___|_|   
                                                         
This parser was written in the distant past by Nick Porcino for the public 
domain and is freely available on an as is basis. It is meant for educational
purposes and is not suitable for any particular purpose. No warranty is 
expressed or implied. Use at your own risk. Do not operate heavy machinery or 
governments while using this code.

The types referenced are compatible with the C99 stdint.h types.
*/

#ifndef TEXTSCANNER_H
#define TEXTSCANNER_H

#ifdef _MSC_VER
	// provide some C99 types
	typedef unsigned int uint32_t;
	typedef int int32_t;
	typedef unsigned short uint16_t;
	typedef short int16_t;
	typedef bool _Bool;
#else
	#include <stdint.h>
	#include <stdbool.h>
    #if defined(__cplusplus) && !defined(_Bool)
        #define _Bool bool
    #endif
#endif

#ifndef EXTERNC
	#ifdef __cplusplus
		#define EXTERNC extern "C"
	#else
		#define EXTERNC
	#endif
#endif

// Get Token
EXTERNC char const* tsGetToken						(char const* pCurr, char const* pEnd, char delim, char const** resultStringBegin, uint32_t* stringLength);
EXTERNC char const* tsGetTokenWSDelimited			(char const* pCurr, char const* pEnd, char const** resultStringBegin, uint32_t* stringLength);
EXTERNC char const* tsGetTokenAlphaNumeric			(char const* pCurr, char const* pEnd, char const** resultStringBegin, uint32_t* stringLength);
EXTERNC char const* tsGetNameSpacedTokenAlphaNumeric(char const* pCurr, char const* pEnd, char namespaceChar, char const** resultStringBegin, uint32_t* stringLength);

// Get Value
EXTERNC char const* tsGetString						(char const* pCurr, char const* pEnd, bool recognizeEscapes, char const** resultStringBegin, uint32_t* stringLength);
EXTERNC char const* tsGetString2					(char const* pCurr, char const* pEnd, char strDelim, bool recognizeEscapes, char const** resultStringBegin, uint32_t* stringLength);
EXTERNC char const* tsGetInt16						(char const* pCurr, char const* pEnd, int16_t* result);
EXTERNC char const* tsGetInt32						(char const* pCurr, char const* pEnd, int32_t* result);
EXTERNC char const* tsGetUInt32						(char const* pCurr, char const* pEnd, uint32_t* result);
EXTERNC char const* tsGetHex						(char const* pCurr, char const* pEnd, uint32_t* result);
EXTERNC char const* tsGetFloat						(char const* pcurr, char const* pEnd, float* result);

EXTERNC char const* tsScanForCharacter				(char const* pCurr, char const* pEnd, char delim);
EXTERNC char const* tsScanBackwardsForCharacter		(char const* pCurr, char const* pEnd, char delim);
EXTERNC char const* tsScanForWhiteSpace				(char const* pCurr, char const* pEnd);
EXTERNC char const* tsScanBackwardsForWhiteSpace	(char const* pCurr, char const* pStart);
EXTERNC char const* tsScanForNonWhiteSpace			(char const* pCurr, char const* pEnd);
EXTERNC char const* tsScanForTrailingNonWhiteSpace	(char const* pCurr, char const* pEnd);
EXTERNC char const* tsScanForQuote					(char const* pCurr, char const* pEnd, char delim, bool recognizeEscapes);
EXTERNC char const* tsScanForEndOfLine				(char const* pCurr, char const* pEnd);
EXTERNC char const* tsScanForLastCharacterOnLine	(char const* pCurr, char const* pEnd);
EXTERNC char const* tsScanForBeginningOfNextLine	(char const* pCurr, char const* pEnd);
EXTERNC char const* tsScanPastCPPComments			(char const* pCurr, char const* pEnd);
EXTERNC char const* tsSkipCommentsAndWhitespace		(char const* pCurr, char const*const pEnd);

EXTERNC char const* tsExpect                        (char const* pCurr, char const*const pEnd, char const* pExpect);

EXTERNC _Bool tsIsWhiteSpace(char test);
EXTERNC _Bool tsIsNumeric	(char test);
EXTERNC _Bool tsIsAlpha		(char test);
EXTERNC _Bool tsIsIn		(const char* testString, char test);

#undef EXTERNC
#endif // TEXTSCANNER_H
