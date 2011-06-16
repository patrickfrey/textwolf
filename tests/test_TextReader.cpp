#include "textwolf.hpp"
#include <iostream>
#include <stdio.h>

//build gcc
//compile: g++ -c -o test_TextReader.o -g -I../include/ -pedantic -Wall -O4 test_TextReader.cpp
//link: g++ -lc -o test_TextReader test_TextReader.o
//build windows
//compile: cl.exe /wd4996 /Ob2 /O2 /EHsc /MT /W4 /nologo /I..\include /D "WIN32" /D "_WINDOWS" /Fo"test_TextReader.obj" test_TextReader.cpp 
//link: link.exe /out:.\test_TextReader test_TextReader.obj


using namespace textwolf;

template <class CharSet>
class EnumCharIterator
{
public:
	unsigned int ii;
	unsigned int pos;
	StaticBuffer buf;

public:
	bool eof() const
	{
		return (ii > CharSet::MaxChar && pos == buf.size());
	}

	bool skip()
	{
		pos++;
		if (pos >= buf.size())
		{
			if (ii > CharSet::MaxChar)
			{
				pos = buf.size();
				return false;
			}
			else
			{
				buf.clear();
				CharSet::print( ++ii, buf);
			}
		}
		return true;
	}

public:
	EnumCharIterator( unsigned int start=0)
		:pos(0),buf(16)
	{
		ii = start;
		skip();
	}

	char operator *() const
	{
		if (eof()) return 0;
		return buf[ pos];
	}
	EnumCharIterator& operator ++()		{skip(); return *this;}
};

template <class CharSet>
struct TextScannerTest
{
	typedef EnumCharIterator<CharSet> ThisEnumCharIterator;
	typedef TextScanner<ThisEnumCharIterator,CharSet> ThisTextScanner;
	unsigned int start;
	unsigned int rndSeed;

	TextScannerTest( unsigned int p_start=0)	:start(p_start),rndSeed(0) {};

	unsigned int rnd()
	{
		rndSeed = (rndSeed+1) * 2654435761u;
		return rndSeed;
	};

	unsigned int operator*()
	{
		ThisEnumCharIterator thisEnumCharIterator(start);
		ThisTextScanner tr( thisEnumCharIterator);
		unsigned int ii;
		UChar chr;

		for (ii=start; tr.control() != EndOfText; ++ii,++tr)
		{
			UChar echr = ii+1;
			unsigned int rr;
			do
			{
				rr = rnd() % 3;
				char aa,ascii;

				switch (rr&1)
				{
					case 0:
						aa = (echr > 127)?0:(char)(unsigned char)(echr & 0xff);
						if ((ascii=tr.ascii()) != aa)
						{
							printf ("ascii %d != %d unexpected\n", ascii, aa);
							return ii;
						}
					break;
					case 1:
						if ((chr=tr.chr()) != echr)
						{
							printf ("character %d != %d unexpected\n", chr, echr);
							return ii;
						}
					break;
				}
			} while (rr>1);
		}
		if (ii-1 != CharSet::MaxChar) return ii+1;
		printf ("PASSED Test\n");
		return 0;
	}
};

static const char* testAll()
{
	struct Error
	{
		char buf[ 256];
		Error()
		{
			buf[0] = '\0';
		};
		const char* get( const char* testname, unsigned int pos)
		{
			if (pos) snprintf( buf, sizeof(buf), "test %s failed at character pos %u", testname, pos);
			return buf[0]==0?0:buf;
		};
		const char* operator*()
		{
			return buf[0]==0?0:buf;
		};
	};
	static Error error;
	if (error.get( "UTF8",		*TextScannerTest< charset::UTF8>())
	||  error.get( "IsoLatin1",	*TextScannerTest< charset::IsoLatin1>())
	||  error.get( "UCS2BE",	*TextScannerTest< charset::UCS2BE>())
	||  error.get( "UCS2LE",	*TextScannerTest< charset::UCS2LE>())
	||  error.get( "UCS4BE",	*TextScannerTest< charset::UCS4BE>())
	||  error.get( "UCS4LE",	*TextScannerTest< charset::UCS4LE>())) return *error;
	return 0;
}

int main( int, const char**)
{
	const char* res = testAll();
	if (!res)
	{
		std::cerr << "OK" << std::endl;
		return 0;
	}
	else
	{
		std::cerr << res << std::endl;
		return 1;
	}
}




