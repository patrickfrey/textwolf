#include "textwolf.hpp"
#include <iostream>
#include <map>

//compile: g++ -c -o feedPhoneBookDataXML.o -g -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors feedPhoneBookDataXML.cpp
//link     g++ -lc -o feedPhoneBookDataXML feedPhoneBookDataXML.o

using namespace textwolf;

namespace
{
struct Input
{
   struct eof {};
   struct iterator
   {
      iterator()                          {};
      iterator( const iterator& o)        :ch(o.ch) {};
      iterator( const eof& o)             :ch(EOF) {};
      int ch;
      void skip()                         {ch=getchar();};                                
      iterator& operator++()              {skip(); return *this;};
      iterator operator++(int)            {iterator tmp(*this); skip(); return tmp;};
      char operator*()                    {return (ch==EOF)?0:(char)ch;};            
   };
   iterator begin()                       {iterator rt; rt.skip(); return rt;};
   iterator end()                         {return iterator(eof());};
};
}//anynomous namespace

int main( int, const char**)
{ 
   try 
   {
      //[1] define the source iterator
      Input input;
      Input::iterator src = input.begin();
      
      //[2] creating the automaton
      typedef XMLPathSelectAutomaton<charset::UTF8> Automaton;
      enum ElementType
      {
         Name, Vorname, Strasse, PLZ, Gemeinde, Tel, Fax, Titel, Sparte, Doc
      };      
      Automaton atm;
      (*atm)["doc"]["name"] = Name;
      (*atm)["doc"]["vorname"] = Vorname;
      (*atm)["doc"]["strasse"] = Strasse;
      (*atm)["doc"]["plz"] = PLZ;
      (*atm)["doc"]["gemeinde"] = Gemeinde;
      (*atm)["doc"]["tel"] = Tel;
      (*atm)["doc"]["fax"] = Fax;
      (*atm)["doc"]["titel"] = Titel;
      (*atm)["doc"]["sparte"] = Sparte;
      (*atm) = Doc;

      //[3] define the XML Path selection by the automaton over the source iterator. 
      //    -Input is IsoLatin-1
      //    -Output is UTF-8
            
      typedef XMLPathSelect<Input::iterator,charset::IsoLatin1,charset::UTF8> MyXMLPathSelect; 
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

