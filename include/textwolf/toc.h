/// \file "toc.h" File used by doxygen to generate the main page
/*! \mainpage textwolf C++ template library for iterating on XML content
 *
 * \section TextScanner Iterating on unicode characters
 *
 * The template class textwolf::TextScanner defines a character by character iterator on a text source.
 * \code
#include "textwolf/xmlscanner.hpp"
#include "textwolf/charset.hpp"
#include <iostream>
#include <string>

void output( const std::string& str)
{
	typedef textwolf::TextScanner<textwolf::CStringIterator,textwolf::charset::UTF8> Scanner;
	Scanner itr(src);
	
	for (; *itr; ++itr)
	{
		std::cout << std::hex << (unsigned int)*itr << std::dec << std::endl;
	}
}
 * \endcode
 *
 * \section XMLScanner Iterating on XML elements
 *
 * The template class textwolf::XMLScanner defines an iterator on XML tokens.
 * \code
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
 * \endcode
 *
 * \section XMLPathSelect Iterating on XML path expression matches
 * \code
#include "textwolf/xmlscanner.hpp"
#include "textwolf/xmlpathselect.hpp"
#include "textwolf/charset.hpp"
#include <iostream>
#include <string>

void output( const std::string& str)
{
	typedef textwolf::charset::UTF8 Encoding;
	typedef textwolf::CStringIterator Iterator;
	typedef textwolf::XMLScanner<Iterator,Encoding,Encoding,std::string> Scanner;
	typedef textwolf::XMLPathSelect<Encoding> Selector;

	textwolf::XMLPathSelectAutomaton<Encoding> atm;
	(*atm)["address"]("name") = 1;	 //... assign 1 to matches of /address/@name
	(*atm)["address"]("street") = 2; //... assign 2 to matches of /address/@street

	Scanner scanner( str);
	Selector selector( &atm);

	// Fetch the input elements, feed them to the selector and iterate on the result dropping out:
	Scanner::iterator ci,ce;
	for (ci=scanner.begin(),ce=scanner.end(); ci!=ce; ci++)
	{
		std::string elem = std::string( itr->content(), itr->size());
		Selector::iterator itr = selector.push( ci->type(), elem), end = selector.end();

		for (; itr!=end; itr++)
		{
			std::cout << *itr << ": " << itr->name() << elem << std::endl;
		}
	}
}
 * \endcode
 *
 * \section XMLScanner_chunkwise Iterating on XML elements in chunkwise feeded source
 *
 * \code
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
 * \endcode
 *
 * The template class textwolf::XMLPathSelect fed with XML tokens lets you iterate on matching XML path expressions
 *
 * \section Interface Adding new character set encodings
 *
 * Besides the supported character set encodings listed in "textwolf/charset.hpp" you can define your own
 * by implementing a class as shown as textwolf::charset::Interface.
 *
 * \section SrcIterator Source iterators
 *
 * - Use 'char*' as iterator type
 * - Use textwolf::CStringIterator as iterator on a complete std::string 
 * - Use textwolf::IStreamIterator as iterator on a std::istream
 * - Use textwolf::SrcIterator as iterator able to do chunk by chunk processing
 *
 */

