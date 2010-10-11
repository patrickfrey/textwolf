#include "textwolf.hpp"
#include <iostream>
#include <map>
#include <stdio.h>
#include <errno.h>

//compile: g++ -c -o feedPhoneBookDataXML.o -g -O5 -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors feedPhoneBookDataXML.cpp
//link     g++ -lc -o feedPhoneBookDataXML feedPhoneBookDataXML.o

using namespace textwolf;

namespace
{
struct Input
{
   enum {maxbufsize=8192};
   char buf[ maxbufsize];
   unsigned int bufsize;
   unsigned int bufpos;
   FILE* file;
   
   Input()                                :bufsize(0),bufpos(0),file(stdin) {};
   Input( const char* fname)              :bufsize(0),bufpos(0),file(fopen(fname,"rb")) {};
   ~Input()                               {if (file != stdin) fclose(file);};
   
   char getchar()
   {
      if (bufpos == bufsize)
      {
         bufsize = fread( buf, 1, maxbufsize, file);
         if (bufsize == 0)
         {
            int ee = errno;
            if (ee != 0) fprintf( stderr, "error %d reading input\n", ee);
            return 0;
         }
         bufpos = 0;          
      }
      return buf[ bufpos++];
   };
   
   struct iterator
   {
      Input* input;
      int ch;
      
      iterator( Input* p_input)           :input(p_input),ch(0) {};
      iterator()                          :input(0),ch(0) {};
      iterator( const iterator& o)        :input(o.input),ch(o.ch) {};
      
      void skip()                         {ch=input->getchar();};                                
      iterator& operator++()              {skip(); return *this;};
      iterator operator++(int)            {iterator tmp(*this); skip(); return tmp;};
      char operator*()                    {return ch;};
   };
   iterator begin()                       {iterator rt(this); rt.skip(); return rt;};
   iterator end()                         {return iterator();};
};
}//anynomous namespace

int main( int argc, const char** argv)
{ 
   try 
   {
      //[1] define the source iterator
      Input* input;
      if (argc == 2)
      {
         input = new Input( argv[1]);
      }
      else if (argc > 2)
      {
         fprintf( stderr, "too many arguments");
         return 1;
      }
      else
      {
         input = new Input();
      }
      Input::iterator src = input->begin();
      
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
      MyXMLPathSelect xs( &atm, src);

      //[4] iterating through the produced elements and printing them
      MyXMLPathSelect::iterator itr=xs.begin(),end=xs.end();
      for (; itr!=end && !itr->error; itr++)
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
      if (itr != end && itr->error)
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

