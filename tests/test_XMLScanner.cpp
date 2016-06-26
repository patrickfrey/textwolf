#include "textwolf.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <stdexcept>

//build gcc
//compile: g++ -c -o test_XMLScanner.o -g -I../include/ -pedantic -Wall -O4 test_XMLScanner.cpp
//link: g++ -lc -o test_XMLScanner test_XMLScanner.o
//build windows
//compile: cl.exe /wd4996 /Ob2 /O2 /EHsc /MT /W4 /nologo /I..\include /D "WIN32" /D "_WINDOWS" /Fo"test_XMLScanner.obj" test_XMLScanner.cpp 
//link: link.exe /out:.\test_XMLScannertest_XMLScanner.obj


using namespace textwolf;

static std::string encodeString( char const* xi, unsigned int nn)
{
	std::string rt;
	unsigned int ii=0;
	for (; ii<nn; ++ii)
	{
		if ((unsigned char)xi[ii] < 32)
		{
			rt.push_back( '.');
		}
		else
		{
			rt.push_back( xi[ii]);
		}
	}
	return rt;
}

int main( int, const char**)
{
	static const char* xmlstr = "<?xml charset=isolatin-1?>\r\n<note id=1 t=2 g=\"zu\"><stag value='500'/> \n<to>Frog</to>\n<from>Bird</from><body>Hello world!</body>\n</note>";
	typedef XMLScanner<char*,charset::IsoLatin,charset::IsoLatin,std::string> MyXMLScanner;
	char* xmlitr = const_cast<char*>(xmlstr);

	MyXMLScanner xs( xmlitr);
	MyXMLScanner::iterator itr,end;
	try
	{
		char const* xi = xmlstr;
		const char* xe = xi + std::strlen( xi);
		for (int xidx=0; xi < xe; xi+=10,xidx+=10)
		{
			std::cout << xidx << ": [" << encodeString( xi, 10) << "]" << std::endl;
		}
		std::cout << std::endl;

		for (itr=xs.begin(),end=xs.end(); itr!=end; itr++)
		{
			const char* typestr = 0;
			switch (itr->type())
			{
				case MyXMLScanner::None: continue;
				case MyXMLScanner::HeaderStart: continue;
				case MyXMLScanner::ErrorOccurred: throw std::runtime_error( itr->content());
				case MyXMLScanner::HeaderAttribName: typestr = "attribute name"; break;
				case MyXMLScanner::HeaderAttribValue: typestr = "attribute value"; break;
				case MyXMLScanner::HeaderEnd: typestr = "end of header"; break;
				case MyXMLScanner::DocAttribValue: typestr = "document attribute value"; break;
				case MyXMLScanner::DocAttribEnd: typestr = "end of entity declaration"; break;
				case MyXMLScanner::TagAttribName: typestr = "attribute name"; break;
				case MyXMLScanner::TagAttribValue: typestr = "attribute value"; break;
				case MyXMLScanner::OpenTag: typestr = "open tag"; break;
				case MyXMLScanner::CloseTag: typestr = "close tag"; break;
				case MyXMLScanner::CloseTagIm: typestr = "close tag"; break;
				case MyXMLScanner::Content: typestr = "content"; break;
				case MyXMLScanner::Exit: typestr = "end of document"; break;
			}
			std::cout << xs.getTokenPosition() << ":" << (xs.getPosition()-xs.getTokenPosition()) << " element (" << itr->name() << ")" << typestr << ": " << itr->content() << std::endl;
		}
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "Error " << e.what() << std::endl;
	}
	return 0;
}




