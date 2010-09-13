#include "textwolf.hpp"
#include <iostream>
#include <map>

//compile: g++ -c -o test_XMLPathSelect.o -g -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors test_XMLPathSelect.cpp
//link     g++ -lc -o test_XMLPathSelect test_XMLPathSelect.o

using namespace textwolf;

int main( int, const char**)
{ 
   try 
   {
      //[1] define the source iterator
      char* src = const_cast<char*>
      (
         "<?xml charset=isolatin-1?>"
         "<AA>aa 12</AA><z><AA>_aa _12</AA></z>"
         "<BB>bb 13</BB>"
         "<CC>cc 14</CC>"
         "<X><Y><CC>_CC _14</CC></Y></X>" 
      );

      //[2] creating the automaton
      typedef XMLPathSelectAutomaton<charset::UTF8> Automaton;
      Automaton atm;
      (*atm)["AA"] = 12;
      (*atm)["BB"] = 13;
      (*atm)--["CC"] = 14;

      //[3] define the XML Path selection by the automaton over the source iterator
      typedef XMLPathSelect<char*> MyXMLPathSelect; 
      MyXMLPathSelect xs( &atm, src);

      //[4] iterating through the produced elements and printing them
      MyXMLPathSelect::iterator itr=xs.begin(),end=xs.end();
      for (; itr!=end && !itr->error; itr++)
      {         
         std::cout << "Element " << itr->type << ": " << itr->content << std::endl;
      }
      
      //[5] handle a possible error
      if (itr != end && itr->error)
      {
         std::cerr << "FAILED " << itr->content << std::endl;
         exit( 1);
      } 
      else
      {
         std::cerr << "OK" << std::endl;
         exit( 0);
      }
   }
   catch (exception ee)
   {
      std::cerr << "ERROR " << ee.what() << std::endl;
      exit( 1);
   };
}

