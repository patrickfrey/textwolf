#include "textwolf.hpp"
#include <iostream>
#include <string>
#include <map>

//build gcc
//compile: g++ -c -o test_XMLScanner.o -g -I../include/ -pedantic -Wall -O4 test_XMLScanner.cpp
//link: g++ -lc -o test_XMLScanner test_XMLScanner.o
//build windows
//compile: cl.exe /wd4996 /Ob2 /O2 /EHsc /MT /W4 /nologo /I..\include /D "WIN32" /D "_WINDOWS" /Fo"test_XMLScanner.obj" test_XMLScanner.cpp 
//link: link.exe /out:.\test_XMLScannertest_XMLScanner.obj


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



