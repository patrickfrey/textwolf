#include "textwolf.hpp"
#include <iostream>
#include <string>
#include <map>

//compile: g++ -c -o test_XMLScanner.o -g -I../include -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors test_XMLScanner.cpp
//link	  g++ -lc -o test_XMLScanner test_XMLScanner.o

using namespace textwolf;

int main( int, const char**)
{
	static const char* xmlstr = "<?xml charset=isolatin-1?><note id=1 t=2 g=\"zu\"><stag value='500'/> \n<to>Frog</to>\n<from>Bird</from><body>Hello world!</body>\n</note>";
	typedef XMLScanner<char*,charset::IsoLatin1,charset::IsoLatin1,std::string> MyXMLScanner;
	std::string outputbuf;
	char* xmlitr = const_cast<char*>(xmlstr);

	MyXMLScanner xs( xmlitr, outputbuf);

	MyXMLScanner::iterator itr,end;
	for (itr=xs.begin(),end=xs.end(); itr!=end; itr++)
	{
		std::cout << "Element " << itr->name() << ": " << itr->content() << std::endl;
		if (itr->type() == MyXMLScanner::ErrorOccurred) break;
	}
	return 0;
}



