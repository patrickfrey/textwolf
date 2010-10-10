#include <map>
#include <stdio.h>
#include "textwolf.hpp"

//compile with gcc -o textwolf -g -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors textwolf.cpp

using namespace textwolf;

/*
* forgot what this is usable for (TODO check)
*/
template <
      class InputIterator,                //< STL conform input iterator with ++ and read only * returning 0 als last character of the input
      class InputCharSet=CharSet_UTF8,    //Character set encoding of the input, read as stream of bytes
      class OutputCharSet=CharSet_UTF8    //Character set encoding of the output, printed as string of the item type of the character set
>
class CharScanner
{
   typedef TextReader<InputIterator,InputCharSet> InputReader;
   typedef unsigned int size_type;
   typedef CharScanner<InputIterator,InputCharSet,OutputCharSet> ThisCharScanner;

   InputReader src;
   enum {outputBufSize=16};
   char outputBuf[ outputBufSize];
   
public:
   CharScanner( InputIterator& p_src)
         :src(p_src)
   {};
   CharScanner( CharScanner& o)
         :src(o.src)
   {};
   bool next( char** item, int* size)
   {
      if (src.control() == EndOfText) 
      {
         *item = 0;
         *size = 0;
         return false;
      }
      *size = OutputCharSet::print( src.chr(), outputBuf, outputBufSize);
      src.skip();
      *item = outputBuf;
      return true;
   };
   //STL conform input iterator for the output of this CharScanner:   
   class iterator
   {
   public:
      struct Element
      {
         char* content;
         size_type size;
         
         Element()                     :content(0),size(0)
         {};
      };
      typedef Element value_type;
      typedef size_type difference_type;
      typedef Element* pointer;
      typedef Element& reference;
      typedef std::input_iterator_tag iterator_category;

   private:
      Element element;
      ThisCharScanner* input;

      void skip()
      {
         if (input != 0) input->next( &element.content, &element.size);
      };
      bool compare( const iterator& iter) const
      {
         return (element.content == element.content);
      };
   public:
      iterator( const iterator& orig) 
         :input( orig.input)
      {
         element.content = orig.element.content;
         element.size = orig.element.size;
      };
      iterator( ThisCharScanner& p_input)
             :input( &p_input)
      {
         input->next( &element.content, &element.size);
      };
      iterator()
             :input(0) {};
      
      const Element& operator*()
      {
         return element;
      };
      const Element* operator->()
      {
         return &element;
      };
      iterator& operator++()     {skip(); return *this;};
      iterator& operator++(int)  {skip(); return *this;};

      bool operator==( const iterator& iter) const   {return compare( iter);};
      bool operator!=( const iterator& iter) const   {return !compare( iter);};
   };
   
   iterator begin()
   {
      return iterator( *this);
   };
   iterator end()
   {
      return iterator();
   };
};

class stdinIterator
{
private:
   int ch;
   void skip()                            {ch=::getchar();};
public:
   stdinIterator()                        {skip();};
   stdinIterator( const stdinIterator& o) {ch=o.ch;};
   char operator *() const                {return ch==EOF?0:(char)ch;};
   stdinIterator& operator ++()           {skip(); return *this;};
   stdinIterator& operator ++(int)        {skip(); return *this;};
};


template <class Scanner>
class Result
{
private:
   Scanner scanner;
   Scanner::iterator end;

public:
   bool skip()                            {scanner++; return !scanner.eof();};
   Char getElement()                      {if (scanner == end) return 0; if (Scanner::OutputCharSet::size( scanner->content) != scanner->size) return 0; return Scanner::OutputCharSet::value( scanner->content);};
   bool eof() const                       {return (scanner == end);};

   Result( Scanner& p_scanner)            :scanner(p_scanner),end(p_scanner.end() {};
   
   class iterator
   {
   private:
      Result* result;
      bool compare( const iterator& p)    {return (p.result == 0 && result == 0);}  
   public:
      iteraror( const iteraror& p)        :result(p.result) {};
      iterator( Result& p_result)         :result(&p_result) {};
      iterator()                          :result(0) {};
      Char operator *()                   {return (result)?result->getElement():0;};
      
      iterator& operator++()              {if (result) if (!result->skip()) return = 0;};
      iterator& operator++(int)           {if (result) if (!result->skip()) return = 0;};

      bool operator==(const iterator& r) const  {return compare(r);};
      bool operator!=(const iterator& r) const  {return compare(r);};
   };
   iterator begin()                       {return iterator (*this);};
   iterator end()                         {return iterator ();};
};

template <class iCharset, class oCharset>
static Char test_CharScanner()
{
   typedef EnumCharIterator< iCharset> Source;
   typedef CharScanner< Source, iCharset, oCharset> Scanner;
   
   Source source;
   Scanner scanner( source);
   Result <Scanner> result( scanner;
   Result <Scanner>::iterator ii = result.begin();
   Result <Scanner>::iterator ee = result.end();
   
   Char expected = 1;
   while (ii != ee)
   {
      if (*ii != expected++) break;
   }
   return expected;
};

template <class iCharset>
static bool test_AllCharScanner( const char* name, unsigned int limit)
{
   bool rt = true;
   Char ch; 
   if (limit <= 0xFF)
   {
      ch = test_CharScanner<iCharset,CharSet_IsoLatin1>();
      if (ch-1 != limit) {printf( stderr, "Test char scanner %s->IsoLatin1 failed (%u != %u)\n", name, ch-1, limit); rt=false;};
   }
   if (limit <= 0xFFFF)
   {
      ch = test_CharScanner<iCharset,CharSet_UCS2BE>();
      if (ch-1 != limit) {printf( stderr, "Test char scanner %s->UCS2BE failed (%u != %u)\n", name, ch-1, limit); rt=false;};
      ch = test_CharScanner<iCharset,CharSet_UCS2LE>();
      if (ch-1 != limit) {printf( stderr, "Test char scanner %s->UCS2LE failed (%u != %u)\n", name, ch-1, limit); rt=false;};
   }
   ch = test_CharScanner<iCharset,CharSet_UCS4BE>();
   if (ch-1 != limit) {printf( stderr, "Test char scanner %s->UCS4BE failed (%u < %u)\n", name, ch-1, limit); rt=false;};
   ch = test_CharScanner<iCharset,CharSet_UCS4LE>();
   if (ch-1 != limit) {printf( stderr, "Test char scanner %s->UCS4LE failed (%u < %u)\n", name, ch-1, limit); rt=false;};
   
   ch = test_CharScanner<iCharset,CharSet_UTF8>();
   if (ch-1 != limit) {printf( stderr, "Test char scanner %s->UTF8 failed (%u < %u)\n", name, ch-1, limit); rt=false;};
};


typedef XMLScanner< stdinIterator, std::map<const char*,UChar>, CharSet_IsoLatin1, CharSet_IsoLatin1> Scanner;

int main( int argc, const char** argv)
{
   Char ch; 
   bool rt = true;

   //Test all possible combination of character set encodings:   
   rt &= test_AllCharScanner<CharSet_IsoLatin1>( "IsoLatin1", 0xFF);
   rt &= test_AllCharScanner<CharSet_UCS2BE>( "UCS2BE", 0xFFFF);
   rt &= test_AllCharScanner<CharSet_UCS2LE>( "UCS2LE", 0xFFFF);
   rt &= test_AllCharScanner<CharSet_UCS4BE>( "UCS4BE", 0xFFFFFFFF);
   rt &= test_AllCharScanner<CharSet_UCS4LE>( "UCS4LE", 0xFFFFFFFF);
   rt &= test_AllCharScanner<CharSet_UTF8>( "UTF8", 0xFFFFFFFF);

   //Test all possible character set encodings with a document string with enumerated entities:   


   enum {bufsize=2048};
   char buf[ bufsize];
   stdinIterator iter;
   
   if (argc >= 1) 
   {
      fprintf( stderr, "too many arguments. no arguments exected. reading from std input\n");
      exit( 1);
   }
   Scanner scanner( iter, buf, sizeof(buf));
   Scanner::iterator itr = scanner.begin();
   Scanner::iterator end = scanner.end();
   while (itr != end)
   {
      
   }
}

