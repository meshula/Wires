
/*
This parser was written in the distant past by Nick Porcino for the public domain
and is freely available on an as is basis. It is meant for educational
purposes and is not suitable for any particular purpose. No warranty is expressed
or implied. Use at your own risk. Do not operate heavy machinery or governments
while using this code.
*/

#include "TextScanner.h"
#include <math.h>
#include <stdbool.h>

//! @todo replace Assert with custom error reporting mechanism
#include <assert.h>
#define Assert assert

#ifdef _MSC_VER
using std::powf;
#endif

//----------------------------------------------------------------------------

char const* tsScanForQuote(
    char const* pCurr, char const* pEnd,
    char delim, 
	bool recognizeEscapes)
{
	Assert(pCurr && pEnd && pEnd >= pCurr);
	
	while (pCurr < pEnd) {
		if (*pCurr == '\\' && recognizeEscapes)	// not handling multicharacter escapes such as \u23AB
			++pCurr;
		else if (*pCurr == delim)
			break;
		++pCurr;
	}

	return pCurr;
}

char const* tsScanForWhiteSpace(
	char const* pCurr, char const* pEnd)
{
	Assert(pCurr && pEnd && pEnd >= pCurr);
	
	while (pCurr < pEnd && !tsIsWhiteSpace(*pCurr))
		++pCurr;

	return pCurr+1;
}

char const* tsScanForNonWhiteSpace(
   char const* pCurr, char const* pEnd)
{
	Assert(pCurr && pEnd && pEnd >= pCurr);
	
	while (pCurr < pEnd && tsIsWhiteSpace(*pCurr))
		++pCurr;

	return pCurr;
}

char const* tsScanBackwardsForWhiteSpace(
	char const* pCurr, char const* pStart)
{
	Assert(pCurr && pStart && pStart <= pCurr);
	
	while (pCurr >= pStart && !tsIsWhiteSpace(*pCurr))
		--pCurr;

	return pCurr;
}

char const* tsScanForTrailingNonWhiteSpace(
	char const* pCurr, char const* pEnd)
{
	Assert(pCurr && pEnd && pEnd >= pCurr);
	
	while (pCurr < pEnd && tsIsWhiteSpace(*pEnd))
		--pEnd;

	return pEnd;
}

char const* tsScanForCharacter(
	char const* pCurr, char const* pEnd, 
	char delim)
{
	Assert(pCurr && pEnd);

	while (pCurr < pEnd && *pCurr != delim)
		++pCurr;

	return pCurr;
}

char const* tsScanBackwardsForCharacter(
	char const* pCurr, char const* pStart, 
	char delim)
{
	Assert(pCurr && pStart && pStart <= pCurr);
	
	while (pCurr >= pStart && *pCurr != delim)
		--pCurr;

	return pCurr;
}

char const* tsScanForEndOfLine(
	char const* pCurr, char const* pEnd)
{
	while (pCurr < pEnd)
	{
		if (*pCurr == '\r')
		{
			++pCurr;
			if (*pCurr == '\n')
				++pCurr;
			break;
		}
		if (*pCurr == '\n')
		{
			++pCurr;
			if (*pCurr == '\r')
				++pCurr;
			break;
		}

		++pCurr;
	}
	return pCurr;
}

char const* tsScanForLastCharacterOnLine(
	char const* pCurr, char const* pEnd)
{
	while (pCurr < pEnd)
	{
		if (pCurr[1] == '\r' || pCurr[1] == '\n' || pCurr[1] == '\0')
		{
			break;
		}

		++pCurr;
	}
	return pCurr;
}

char const* tsScanForBeginningOfNextLine(
	char const* pCurr, char const* pEnd)
{
	pCurr = tsScanForEndOfLine(pCurr, pEnd);
	return (tsScanForNonWhiteSpace(pCurr, pEnd));
}

char const* tsScanPastCPPComments(
	char const* pCurr, char const* pEnd)
{
	if (*pCurr == '/')
	{
		if (pCurr[1] == '/')
		{
			pCurr = tsScanForEndOfLine(pCurr, pEnd);
		}
		else if (pCurr[1] == '*')
		{
			pCurr = &pCurr[2];
			while (pCurr < pEnd)
			{
				if (pCurr[0] == '*' && pCurr[1] == '/')
				{
					pCurr = &pCurr[2];
					break;
				}
				
				++pCurr;
			}
		}
	}
	
	return pCurr;
}

char const* tsSkipCommentsAndWhitespace(
	char const* curr, char const*const end)
{
	bool moved = true;
	while (moved)
	{
		char const* past = tsScanForNonWhiteSpace(curr, end);
		curr = past;
		
		past = tsScanPastCPPComments(curr, end);
		moved = past != curr;
		curr = past;
	}
	
	return tsScanForNonWhiteSpace(curr, end);
}

char const* tsGetToken(
	char const* pCurr, char const* pEnd, 
	char delim, 
	char const** resultStringBegin, uint32_t* stringLength)
{
	Assert(pCurr && pEnd);
	
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	*resultStringBegin = pCurr;	
	char const* pStringEnd = tsScanForCharacter(pCurr, pEnd, delim);	
	*stringLength = (uint32_t)(pStringEnd - *resultStringBegin);
	return pStringEnd;
}

char const* tsGetTokenWSDelimited(
	char const* pCurr, char const* pEnd, 
	char const** resultStringBegin, uint32_t* stringLength)
{
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	*resultStringBegin = pCurr;
	char const* pStringEnd = tsScanForWhiteSpace(pCurr, pEnd);
	*stringLength = (uint32_t)(pStringEnd - *resultStringBegin);
	return pStringEnd;
}

char const* tsGetTokenAlphaNumeric(
	char const* pCurr, char const* pEnd, 
	char const** resultStringBegin, uint32_t* stringLength)
{
	Assert(pCurr && pEnd);
	
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	*resultStringBegin = pCurr;
	*stringLength = 0;
	
	while (pCurr < pEnd)
	{
		char test = pCurr[0];
		
		if (tsIsWhiteSpace(test))
			break;
		
		_Bool accept = ((test == '_') || tsIsNumeric(test) || tsIsAlpha(test));
		
		if (!accept)
			break;
		
		++pCurr;
		*stringLength += 1;
	}
	
	return pCurr;
}

char const* tsGetNameSpacedTokenAlphaNumeric(
	char const* pCurr, char const* pEnd, 
	char namespaceChar, 
	char const** resultStringBegin, uint32_t* stringLength)
{
	Assert(pCurr && pEnd);
	
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	*resultStringBegin = pCurr;
	*stringLength = 0;
	
	while (pCurr < pEnd)
	{
		char test = pCurr[0];
		
		if (tsIsWhiteSpace(test))
			break;
		
        // should pass in a string of acceptable characters, ie "$^_"
		_Bool accept = ((test == namespaceChar) || (test == '$') || (test == '^') || (test == '_') || tsIsNumeric(test) || tsIsAlpha(test));
		
		if (!accept)
			break;
		
		++pCurr;
		*stringLength += 1;
	}
	
	return pCurr;
}

char const* tsGetString(
	char const* pCurr, char const* pEnd, 
	bool recognizeEscapes,
	char const** resultStringBegin, uint32_t* stringLength)
{
	Assert(pCurr && pEnd && pEnd >= pCurr);
	
	pCurr = tsScanForQuote(pCurr, pEnd, '\"', recognizeEscapes);
	
	if (pCurr < pEnd)
	{
		++pCurr;	// skip past quote
		*resultStringBegin = pCurr;
		
		pCurr = tsScanForQuote(pCurr, pEnd, '\"', recognizeEscapes);
		
		if (pCurr <= pEnd)
			*stringLength = (uint32_t)(pCurr - *resultStringBegin);
		else
			*stringLength = 0;
		
		++pCurr;	// point past closing quote
	}
	else
		*stringLength = 0;
	
	return pCurr;
}

char const* tsGetString2(
                        char const* pCurr, char const* pEnd, 
                        char delim,
                        bool recognizeEscapes,
                        char const** resultStringBegin, uint32_t* stringLength)
{
	Assert(pCurr && pEnd && pEnd >= pCurr);
	
	pCurr = tsScanForQuote(pCurr, pEnd, delim, recognizeEscapes);
	
	if (pCurr < pEnd)
	{
		++pCurr;	// skip past quote
		*resultStringBegin = pCurr;
		
		pCurr = tsScanForQuote(pCurr, pEnd, delim, recognizeEscapes);
		
		if (pCurr <= pEnd)
			*stringLength = (uint32_t)(pCurr - *resultStringBegin);
		else
			*stringLength = 0;
		
		++pCurr;	// point past closing quote
	}
	else
		*stringLength = 0;
	
	return pCurr;
}

// Match pExpect. If pExect is found in the input stream, return pointing
// to the character that follows, otherwise return the start of the input stream

char const* tsExpect(
    char const* pCurr, char const*const pEnd, 
	char const* pExpect)
{
	char const* pScan = pCurr;
	while (pScan != pEnd && *pScan == *pExpect && *pExpect != '\0') {
		++pScan;
		++pExpect;
	}
	return (*pExpect == '\0' ? pScan : pCurr);
}



char const* tsGetInt16(
	char const* pCurr, char const* pEnd, 
	int16_t* result)
{
	int32_t longresult;
	char const* retval = tsGetInt32(pCurr, pEnd, &longresult);
	*result = (int16_t) longresult;
	return retval;
}

char const* tsGetInt32(
	char const* pCurr, char const* pEnd, 
	int32_t* result)
{
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	
	int ret = 0;
	
	_Bool signFlip = false;
	
	if (*pCurr == '+')
	{
		++pCurr;
	}
	else if (*pCurr == '-')
	{
		++pCurr;
		signFlip = true;
	}
	
	while (pCurr < pEnd)
	{
		if (!tsIsNumeric(*pCurr))
		{
			break;
		}
		ret = ret * 10 + *pCurr - '0';
		++pCurr;
	}
	
	if (signFlip)
	{
		ret = -ret;
	}
	*result = ret;
	return pCurr;
}

char const* tsGetUInt32(
	char const* pCurr, char const* pEnd, 
	uint32_t* result)
{
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	
	uint32_t ret = 0;
	
	while (pCurr < pEnd)
	{
		if (!tsIsNumeric(*pCurr))
		{
			break;
		}
		ret = ret * 10 + *pCurr - '0';
		++pCurr;
	}
	*result = ret;
	return pCurr;
}

char const* tsGetFloat(
	char const* pCurr, char const* pEnd, 
	float* result)
{
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	
	float ret = 0.0f;
	
	_Bool signFlip = false;
	
	if (*pCurr == '+')
	{
		++pCurr;
	}
	else if (*pCurr == '-')
	{
		++pCurr;
		signFlip = true;
	}
	
	// get integer part
	int32_t intPart;
	pCurr = tsGetInt32(pCurr, pEnd, &intPart);
	ret = (float) intPart;
	
	// get fractional part
	if (*pCurr == '.')
	{
		++pCurr;
		
		float scaler = 0.1f;
		while (pCurr < pEnd)
		{
			if (!tsIsNumeric(*pCurr))
			{
				break;
			}
			ret = ret + (float)(*pCurr - '0') * scaler;
			++pCurr;
			scaler *= 0.1f;
		}
	}
	
	// get exponent
	if (*pCurr == 'e' || *pCurr == 'E')
	{
		++pCurr;
		
		pCurr = tsGetInt32(pCurr, pEnd, &intPart);
		ret *= powf(10.0f, (float) intPart);
	}
	
	if (signFlip)
	{
		ret = -ret;
	}
	*result = ret;
	return pCurr;
}


char const* tsGetHex(
	char const* pCurr, char const* pEnd, 
	uint32_t* result)
{
	pCurr = tsScanForNonWhiteSpace(pCurr, pEnd);
	
	int ret = 0;
	
	while (pCurr < pEnd)
	{
		if (tsIsNumeric(*pCurr))
		{
			ret = ret * 16 + *pCurr - '0';
		}
		else if (*pCurr >= 'A' && *pCurr <= 'F')
		{
			ret = ret * 16 + *pCurr - 'A' + 10;
		}
		else if (*pCurr >= 'a' && *pCurr <= 'f')
		{
			ret = ret * 16 + *pCurr - 'a' + 10;
		}
		else
		{
			break;
		}
		++pCurr;
	}
	
	*result = ret;
	return pCurr;
}

_Bool tsIsIn(const char* testString, char test)			
{ 
	for (; *testString != '\0'; ++testString) 
		if (*testString == test) 
			return true; 
	return false; 
}

_Bool tsIsWhiteSpace(char test)	
{ 
	return (test == 9 || test == ' ' || test == 13 || test == 10);	
}

_Bool tsIsNumeric(char test)	
{ 
	return (test >= '0' && test <= '9');	
}

_Bool tsIsAlpha(char test)		
{ 
	return ((test >= 'a' && test <= 'z') || (test >= 'A' && test <= 'Z'));	
}

