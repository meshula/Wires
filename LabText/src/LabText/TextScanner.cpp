
/*
This parser was written in the distant past by Nick Porcino for the public domain
and is freely available on an as is basis. It is meant for educational
purposes and is not suitable for any particular purpose. No warranty is expressed
or implied. Use at your own risk. Do not operate heavy machinery or governments
while using this code.
*/

#include "TextScanner.h"
#include "TextScanner.hpp"

#ifdef PLATFORM_WINDOWS
#include <windows.h>    // for unicode conversions
#endif

#ifdef PLATFORM_MACOSX
#include <libkern/OSTypes.h>
#include <stddef.h>
#endif

#include <cmath>

//! @todo replace Assert with proper error reporting mechanism

#include <assert.h>
#define Assert assert

#ifdef _MSC_VER
using std::powf;
#endif

using std::vector;
using std::string;

std::vector<std::string> TextScanner::Split(const std::string& input, const std::string& splitter)
{
	vector<string> output;
	if (input != splitter) {
		const char* c = input.c_str();
		const char* split = strstr(c, splitter.c_str());
		if (!split)
			output.push_back(input);
		else {
			while (split) {
				if (c != split)
					output.push_back(std::string(c, split - c));
				c = split + splitter.length();
				split = strstr(c, splitter.c_str());
			}
			if (strlen(c) > 0)
				output.push_back(std::string(c));
		}
	}
	return output;
}


// given a string, split it into components, at the splitter character.
// if escapes are allowed, an escaped splitter won't split
// if empties are allowed, empty strings will get pushed, otherwise not
// ";" yields two empties if empties are allowed, zero otherwise.

vector<string> TextScanner::Split(const string& input, char splitter, bool escapes, bool empties)
{
    vector<string> output;
    if (input.find(splitter) == string::npos)
    {
        output.push_back(input);
    }
    else
    {
        size_t curr = 0;
        size_t start = 0;
        size_t end = input.length();
        while (curr < end)
        {
            if (escapes && input[curr] == '\\')
            {
                ++curr;
                if (curr != end && input[curr] == splitter)
                    ++curr;
            }
            else
            {
                if (input[curr] == splitter)
                {
                    if (curr>start)
                    {
                        output.push_back(input.substr(start, curr-start));
                    }
                    else if (empties && curr == 0)
                    {
                        output.push_back("");
                    }
                    start = curr+1;
                    if (empties && (input[start] == splitter || start == end))
                    {
                        output.push_back("");
                        ++start;
                        ++curr;
                    }
                }
                ++curr;
            }
        }
        if (curr - start > 0)
            output.push_back(input.substr(start, curr-start));
    }


    return output;
}

string 
TextScanner::Join(const vector<string>& input, const string& join)
{
    string result;
    vector<string>::const_iterator i;
    for (i = input.begin(); i != input.end();)
    {
        result += *i;
        if (++i != input.end())
            result += join;
    }
    return result;
}


void 
TextScanner::Parsef(const std::string& text, float& res)
{
    char const* pCurr = text.c_str();
    char const* pEnd  = pCurr + text.length();
	/*pCurr =*/ tsGetFloat(pCurr, pEnd, &res);
}

void 
TextScanner::ParseFloats(const std::string& text, float* output, size_t count)
{
    char const* pCurr = text.c_str();
    char const* pEnd  = pCurr + text.length();

    for (size_t i = 0; i < count; ++i)
    {
        float temp;
	    pCurr = tsGetFloat(pCurr, pEnd, &temp);
        *output++ = temp;
    }
}


std::string 
TextScanner::Path(const std::string& filepath)
{
    std::string path;
    std::string::size_type sep = filepath.rfind('/');
    if (sep != std::string::npos)
    {
        path = filepath.substr(0, sep);
    }
    return path;
}

std::string 
TextScanner::BaseName(const std::string& filepath)
{
    std::string path;
    std::string::size_type sep = filepath.rfind('/');
    if (sep != std::string::npos)
    {
        path = filepath.substr(sep+1); // default second param for substr is "remaining characters after position"
    }
    else
    {
        path = filepath;
    }
    return path;
}

std::string TextScanner::Extension(const std::string& filepath)
{
    std::string path;
    std::string::size_type sep = filepath.rfind('.');
    if (sep != std::string::npos)
    {
        path = filepath.substr(sep+1); // default second param for substr is "remaining characters after position"
    }
    else
    {
        path = filepath;
    }
    return path;
}

std::wstring TextScanner::Extension(const std::wstring& filepath)
{
    std::wstring path;
    std::wstring::size_type sep = filepath.rfind('.');
    if (sep != std::wstring::npos)
    {
        path = filepath.substr(sep+1); // default second param for substr is "remaining characters after position"
    }
    else
    {
        path = filepath;
    }
    return path;
}

std::wstring TextScanner::Unicode(const std::string& input)
{
    if (input.length() == 0)
        return L"";
    else
    {
#ifdef PLATFORM_WINDOWS
        int len = (int) input.length() * 3 + 1;
        wchar_t* temp = (wchar_t*) alloca(len);
        MultiByteToWideChar( CP_ACP, 0, input.c_str(), (int)input.length() + 1, temp, len);
        return temp;
#elif defined(PLATFORM_MACOSX)
#if 0
		@TODO - how do you get this stuff now? maybe carbon has to be added as a framework?
		static OSStatus status;
		static TECObjectRef converter;
		static TextEncoding utf8Encoding;
		static TextEncoding unicodeEncoding;
		
		static bool init = true;
		if (init)
		{
			utf8Encoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
											  kUnicodeNoSubset, kUnicodeUTF8Format);
			unicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
												 kUnicodeNoSubset, kUnicode16BitFormat);
			status = TECCreateConverter(&converter, utf8Encoding, unicodeEncoding);
			
			init = false;
		}
		
		ByteCount inBufBytesRead;
		ByteCount bufSize = input.length() * 4 + 4;
		wchar_t* outBuf = (wchar_t*) alloca(bufSize);
		ByteCount outBufBytesWritten;
		status = TECConvertText(converter, (const UInt8*) input.c_str(), input.length(), &inBufBytesRead,
								(UInt8*) outBuf, bufSize, &outBufBytesWritten);
		
		return std::wstring(outBuf);
#endif
		return L"";
#endif
    }
	return L"";
}

std::string TextScanner::UTF8(const std::wstring& input)
{
#ifdef PLATFORM_WINDOWS
    char* temp = (char*) alloca(input.length() + 1);
    WideCharToMultiByte( CP_ACP, 0, input.c_str(), -1, temp, (int) input.length() + 1, NULL, NULL );
	return temp;
#elif defined(PLATFORM_MACOSX)
	char outBuf[512];

#if 0
	@TODO - how do you get this stuff now? maybe carbon has to be added as a framework?
	static OSStatus status;
	static TECObjectRef converter;
	static TextEncoding utf8Encoding;
	static TextEncoding unicodeEncoding;
	
	static bool init = true;
	if (init)
	{
		utf8Encoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
										  kUnicodeNoSubset, kUnicodeUTF8Format);
		unicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
											 kUnicodeNoSubset, kUnicode16BitFormat);
		status = TECCreateConverter(&converter, unicodeEncoding, utf8Encoding);
		init = false;
	}
	
	ByteCount inBufBytesRead;
	ByteCount outBufBytesWritten;
	char outBuf[512];
	status = TECConvertText(converter, (const UInt8*) input.c_str(), input.length() * 4 + 4, &inBufBytesRead,
							(UInt8*) outBuf, 512, &outBufBytesWritten);
	// for UTF16 to UTF8 conversion you don't need to call TECFlushText
	
	// 3. Dispose of converter (once at end) dp currently leaking
	//status = TECDisposeConverter(converter);
	return outBuf;
#endif
#endif
	return "";
}

#ifdef RUN_UNITTEST

void printResult(std::vector<std::string>& foo)
{
	for (std::vector<std::string>::const_iterator i = foo.begin(); i != foo.end(); ++i) {
		std::cout << "[" << *i << "]\n";
	}
	std::cout << "------\n";
}


void TextScanner::testSplitter()
{
	std::string test = "abc";
	std::vector<std::string> result = TextScanner::Split(test, "foo");
	printResult(result);
	result = TextScanner::Split(test, "abc");
	printResult(result);
	result = TextScanner::Split(test, "b");
	printResult(result);
	test = "abcabc";
	result = TextScanner::Split(test, "abc");
	printResult(result);
	test = "apples foo bananas foo cherries";
	result = TextScanner::Split(test, "foo");
	printResult(result);
	result = TextScanner::Split(test, "cherries");
	printResult(result);
	result = TextScanner::Split(test, "apples");
	printResult(result);
}

#endif


