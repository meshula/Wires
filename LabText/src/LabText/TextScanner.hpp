/*
 *  TextScanner.hpp
 *  Lab
 *
 */

#ifndef TEXTSCANNER_HPP
#define TEXTSCANNER_HPP

#include <string>   // for split/join
#include <vector>   // for split/join

namespace TextScanner
{
	// given an input string, split it up wherever the splitter string is found.
	// the resulting strings won't include the splitter string.
	std::vector<std::string>   Split(const std::string& input, const std::string& splitter);
	
	// given a string, split it into components, at the splitter character.
    // if escapes are allowed, an escaped splitter won't split
    // if empties are allowed, empty strings will get pushed, otherwise not
    // ";" yields two empties if empties are allowed, zero otherwise.
	
    std::vector<std::string>	Split(const std::string& input, char splitter, bool escapes, bool empties);
    std::string					Join(const std::vector<std::string>& input, const std::string& join);	

	// Simple Tests
	template <class Type> inline bool IsWhiteSpace(Type test)	{ return (test == 9 || test == ' ' || test == 13 || test == 10);	}
	template <class Type> inline bool IsNumeric(Type test)		{ return (test >= '0' && test <= '9');	}
	template <class Type> inline bool IsAlpha(Type test)		{ return ((test >= 'a' && test <= 'z') || (test >= 'A' && test <= 'Z'));	}

	// a zillion times faster than Split.
	// This template, given a string, creates an array caching the split parts of the string
    // templated on string and type so that wide strings can be split.
	
	template <typename T, typename TChar>
    class Splitter {
    public:
        Splitter(const T& src, TChar splitter)
        {
            count = 0;
            TChar* curr = buffer;
            parts[count++] = curr;
            const TChar* srcPtr = src.c_str();
            while (*srcPtr)
            {
                if (*srcPtr != splitter)
                {
                    *curr++ = *srcPtr++;
                }
                else
                {
                    *curr++ = '\0';
                    parts[count++] = curr;
                    ++srcPtr;
                }
            }
            *curr++ = '\0'; // terminate final string
            parts[count] = 0;
        }
		
        ~Splitter(){}

        TChar*  parts[64];       // max parts
        TChar   buffer[520];     // MAX_PATH
        int     count;
    };

    // and another. http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
    template < class ContainerT >
    void tokenize(const std::string& str, ContainerT& tokens,
                  const std::string& delimiters = " ", bool trimEmpty = false)
    {
        std::string::size_type pos, lastPos = 0;
        while (true) {
            pos = str.find_first_of(delimiters, lastPos);
            if (pos == std::string::npos) {
                pos = str.length();

                if( pos != lastPos || !trimEmpty)
                    tokens.push_back(ContainerT::value_type(str.data()+lastPos,
                                                            (typename ContainerT::value_type::size_type) pos-lastPos ));

                break;
            }
            else {
                if (pos != lastPos || !trimEmpty)
                    tokens.push_back(ContainerT::value_type(str.data()+lastPos,
                                                            (typename ContainerT::value_type::size_type) pos-lastPos ));
            }
            
            lastPos = pos + 1;
        }
    };
	
    // returns everything to the left of the last separator character
	
	std::string		Path(const std::string& filepath);
    std::string		BaseName(const std::string& filepath);
    std::string		Extension(const std::string& filepath);
    std::wstring	Extension(const std::wstring& filepath);
	
	// Unicode <> UTF8 translators
	
    std::wstring	Unicode(const std::string& input);
    std::string		UTF8(const std::wstring& input);
	
	void			Parsef(const std::string& text, float& res);
	void			ParseFloats(const std::string& text, float* output, size_t count);
	
#ifdef RUN_UNITTEST
	void testSplitter();
#endif
	
}

#endif // TEXTSCANNER_HPP
