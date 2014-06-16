#include "textwolf/xmlscanner.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include <iostream>
#include <string>
#include <setjmp.h>

typedef textwolf::charset::UTF8 Encoding;
typedef textwolf::SrcIterator Iterator;
typedef textwolf::XMLScanner<Iterator,Encoding,Encoding,std::string> Scanner;

bool output( Scanner& scan, const char* chunk, std::size_t chunksize)
{
	jmp_buf eom;
	scan.setSource( Iterator( chunk, chunksize, &eom));

	if (setjmp(eom) != 0)
	{
		return false; //... do call the function with the next chunk
	}
	Scanner::iterator itr = scan.begin(), end = scan.end();

	for (; itr != end; ++itr)
	{
		if (itr->error())
		{
			throw std::runtime_error( std::string("xml error: ") + itr->error());
		}
		std::string elem = std::string( itr->content(),itr->size());
		std::cout << itr->name() << " " << elem << std::endl;
	}
	return true;
}

