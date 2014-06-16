#include "textwolf/xmlscanner.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/cstringiterator.hpp"
#include <iostream>
#include <string>

void output( const std::string& str)
{
	typedef textwolf::charset::UTF8 MyEncoding;
	typedef textwolf::CStringIterator MyIterator;
	typedef textwolf::XMLScanner<MyIterator,MyEncoding,MyEncoding,std::string> MyScanner;

	textwolf::CStringIterator si( str);
	MyScanner scan( si);
	MyScanner::iterator itr = scan.begin(), end = scan.end();

	for (; itr != end; ++itr)
	{
		if (itr->error())
		{
			throw std::runtime_error( std::string("xml error: ") + itr->error());
		}
		std::string elem = std::string(itr->content(),itr->size());
		std::cout << itr->name() << " " << elem << std::endl;
	}
}
