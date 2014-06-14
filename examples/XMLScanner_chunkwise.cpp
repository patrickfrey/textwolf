#include "textwolf/xmlscanner.hpp"
#include "textwolf/charset.hpp"
#include <iostream>
#include <string>

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
	for (; *scan; ++scan)
	{
		std::string elem = std::string(scan->content(),scan->size());
		std::cout << scan->name() << " " << elem << std::endl;
	}
	return true;
}

