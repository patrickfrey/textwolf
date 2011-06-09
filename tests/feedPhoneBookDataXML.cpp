#include "textwolf.hpp"
#include "textwolf/iterator/file.hpp"
#include <iostream>
#include <map>
#include <stdio.h>
#include <errno.h>

//compile: g++ -c -o feedPhoneBookDataXML.o -g -O5 -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors feedPhoneBookDataXML.cpp
//link     g++ -lc -o feedPhoneBookDataXML feedPhoneBookDataXML.o

using namespace textwolf;

typedef textwolf::io::FileInput Input;

int main( int argc, const char** argv)
{ 
   try 
   {
      //[1] define the source iterator
      const char* filename = (argc >= 2)?argv[1]:"-";
      Input input( filename);
      Input::iterator src = input.begin();
      
      //[2] creating the automaton
      typedef XMLPathSelectAutomaton<charset::UTF8> Automaton;
      enum ElementType
      {
         Name, Vorname, Strasse, PLZ, Gemeinde, Tel, Fax, Titel, Sparte, Doc
      };      
      Automaton atm;
      (*atm)["docs"]["doc"]["name"] = Name;
      (*atm)["docs"]["doc"]["vorname"] = Vorname;
      (*atm)["docs"]["doc"]["strasse"] = Strasse;
      (*atm)["docs"]["doc"]["plz"] = PLZ;
      (*atm)["docs"]["doc"]["gemeinde"] = Gemeinde;
      (*atm)["docs"]["doc"]["tel"] = Tel;
      (*atm)["docs"]["doc"]["fax"] = Fax;
      (*atm)["docs"]["doc"]["titel"] = Titel;
      (*atm)["docs"]["doc"]["sparte"] = Sparte;
      (*atm)["docs"] = Doc;
      unsigned int docCnt = 0;

      //[3] define the XML Path selection by the automaton over the source iterator. 
      //    -Input is IsoLatin-1
      //    -Output is UTF-8
            
      typedef XMLPathSelect<Input::iterator,charset::IsoLatin1,charset::UTF8> MyXMLPathSelect; 
      enum {outputbufSize=1024};
      char outputbuf[ outputbufSize];
      MyXMLPathSelect xs( &atm, src, outputbuf, outputbufSize);

      //[4] iterating through the produced elements and printing them
      MyXMLPathSelect::iterator itr=xs.begin(),end=xs.end();
      for (; itr!=end; itr++)
      {
         if (itr->type == Doc)
         {
            docCnt++;
            if ((docCnt & 16383) == 0) {printf( "\r%u", docCnt); fflush(stdout);}
         }
#undef PRINT_RESULT_STDOUT          
#ifdef PRINT_RESULT_STDOUT          
         std::cout << "Element " << itr->type << ": " << itr->content << std::endl;
#endif
      }
      printf( "\r%u\n", docCnt);
      
      //[5] handle a possible error
      if ((int)itr->state != 0)
      {
         std::cerr << "FAILED " << itr->content << std::endl;
         return 1;
      } 
      else
      {
         std::cerr << "OK" << std::endl;
         return 0;
      }
   }
   catch (exception ee)
   {
      std::cerr << "ERROR " << ee.what() << std::endl;
      return 1;
   };
}

