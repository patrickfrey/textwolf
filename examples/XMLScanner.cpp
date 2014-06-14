#include "textwolf/xmlscanner.hpp"
#include "textwolf/charset.hpp"
#include <iostream>
#include <string>

void output( const std::string& str)
{
	typedef textwolf::charset::UTF8 Encoding;
	typedef textwolf::CStringIterator Iterator;
	typedef textwolf::XMLScanner<Iterator,Encoding,Encoding,std::string> Scanner;

	Scanner itr( Iterator( m_src));
	for (; *itr; ++itr)
	{
		std::string elem = std::string(itr->content(),itr->size());
		std::cout << itr->name() << " " << elem << std::endl;
	}
}
