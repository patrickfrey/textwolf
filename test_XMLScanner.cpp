#include "textwolf.hpp"
#include <iostream>
#include <map>

//compile: g++ -c -o test_XMLScanner.o -g -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors test_XMLScanner.cpp
//link     g++ -lc -o test_XMLScanner test_XMLScanner.o

using namespace textwolf;

static const char* simpleXmple = "<?xml charset=isolatin-1?><note id=1 t=2 g=\"zu\"><stag value='500'/> \n<to>Frog</to>\n<from>Bird</from><body>Hello world!</body>\n</note>";

template <class InputCharSet, class OutputCharSet>
struct XMLScannerTest
{
   typedef XMLScanner<char*,InputCharSet,OutputCharSet> MyXMLScanner;
   enum {outputBufSize=2048};
   MyXMLScanner xs;
   
   XMLScannerTest( char* text) :xs(text,outputBufSize) {};

   bool run()
   {
      typename MyXMLScanner::iterator itr;
      typename MyXMLScanner::iterator end;
      for (itr=xs.begin(),end=xs.end(); itr!=end; itr++)
      {
         std::cout << "Element " << itr->name() << ": " << itr->content << std::endl;
         if (itr->type == MyXMLScanner::ErrorOccurred) break;
      }
      return true;
   };
};

static bool simpleTest( const char* txt)
{
   typedef XMLScannerTest<charset::IsoLatin1,charset::IsoLatin1> Test;
   return Test( const_cast<char*>(txt)).run();
}

static bool testAll()
{
   if (!simpleTest( simpleXmple)) return false;
   return true;
}

int main( int, const char**)
{   
   return testAll()?0:1;
}




