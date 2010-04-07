#include "textwolf.hpp"
#include <iostream>
#include <map>

//compile: g++ -c -o test_XMLPathSelect.o -g -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors test_XMLPathSelect.cpp
//link     g++ -lc -o test_XMLPathSelect test_XMLPathSelect.o

using namespace textwolf;

static const char* simpleXmple = "<?xml charset=isolatin-1?><note id=1 t=2 g=\"zu\"><stag value='500'/> \n<to>Frog</to>\n<from>Bird</from><body>Hello world!</body>\n</note>";

int main( int, const char**)
{ 
   try 
   {
      char* src = const_cast<char*>( simpleXmple);
      typedef XMLPathSelectAutomaton<charset::UTF8> Automaton;
      Automaton atm;
      (*atm)["stag"]("value","500")();
      (*atm)["note"]("id");
      (*atm)["note"]("id", "1");
      (*atm)--["stag"]();

      enum {outputBufSize=4096};
      char outputBuf[ outputBufSize];

      typedef XMLPathSelect<char*> MyXMLPathSelect; 
      MyXMLPathSelect xs( &atm, src, outputBuf, outputBufSize);

      MyXMLPathSelect::iterator itr,end;
      for (itr=xs.begin(),end=xs.end(); itr!=end; itr++)
      {         
         if (itr->error)
         {
            std::cerr << "FAILED " << itr->content << std::endl;
            exit( 1);
         }
         std::cout << "Element " << itr->type << ": " << std::string( itr->content, itr->size).c_str();
      }
      
      std::cerr << "OK" << std::endl;
      exit( 0);
   }
   catch (exception ee)
   {
      std::cerr << "ERROR " << ee.what() << std::endl;
      exit( 1);
   };
}

