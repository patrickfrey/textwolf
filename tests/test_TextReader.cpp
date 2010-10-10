#include "textwolf.hpp"
#include <iostream>

//compile: g++ -c -o test_TextScanner.o -g -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors test_TextScanner.cpp
//link     g++ -lc -o test_TextScanner test_TextScanner.o

using namespace textwolf;

template <class CharSet>
class EnumCharIterator
{
private:
   unsigned int ii;
   char buf[ 16];
   unsigned int bufpos;
   unsigned int size;

public:
   bool eof() const
   {
      return (ii > CharSet::MaxChar && bufpos == size);
   };
   
   bool skip()
   {
      bufpos++;
      if (bufpos >= size)
      {
         if (ii > CharSet::MaxChar)
         {
            bufpos = size; 
            return false;            
         }
         else
         {
            size = CharSet::print( ++ii, buf, sizeof(buf));
            bufpos = 0;
         }
      }
      return true;
   };
   
public:   
   EnumCharIterator( unsigned int start=0)
   {
      ii = start;
      for (unsigned ee=0; ee<sizeof(buf); ee++) buf[ ee]='\0';
      bufpos = 0;
      size = 0;
      skip();
   };
   
   EnumCharIterator( const EnumCharIterator& oo)
   {
      ii = oo.ii;
      for (unsigned ee=0; ee<sizeof(buf); ee++) buf[ee]=oo.buf[ee];
      bufpos = oo.bufpos;
      size = oo.size;
   };
   
   char operator *() const                {return eof()?0:buf[ bufpos];};
   EnumCharIterator& operator ++()        {skip(); return *this;};
   EnumCharIterator& operator ++(int)     {skip(); return *this;};
};

template <class CharSet>
struct TextScannerTest
{
   typedef EnumCharIterator<CharSet> ThisEnumCharIterator;
   typedef TextScanner<ThisEnumCharIterator,CharSet> ThisTextScanner;
   unsigned int start;
   unsigned int rndSeed;
   
   TextScannerTest( unsigned int p_start=0)    :start(p_start),rndSeed(0) {};

   unsigned int rnd()
   {
      rndSeed = (rndSeed+1) * 2654435761u;
      return rndSeed;
   };
   
   unsigned int operator*()
   {
      ThisEnumCharIterator thisEnumCharIterator(start);
      ThisTextScanner tr( thisEnumCharIterator);
      unsigned int ii;
      UChar chr;
      
      for (ii=start; tr.control() != EndOfText; ii++,tr++)
      {
         UChar echr = ii+1;
         unsigned int rr;
         do
         {
            rr = rnd() % 3;
            char aa,ascii;

            switch (rr&1)
            {
               case 0:
                  aa = (echr > 127)?0:(char)(unsigned char)(echr & 0xff);
                  if ((ascii=tr.ascii()) != aa)
                  {
                     printf ("ascii %d != %d expected\n", ascii, aa);
                     return ii;
                  }
               break;
               case 1:
                  if ((chr=tr.chr()) != echr)
                  {
                     printf ("character %d != %d expected\n", chr, echr);
                     return ii;
                  }
               break;
            }
         } while (rr>1);
      }
      if (ii-1 != CharSet::MaxChar) return ii+1;
      return 0;
   };
};

static const char* testAll()
{
   struct Error
   {
      char buf[ 256];
      Error()
      {
         buf[0] = '\0';
      };
      const char* get( const char* testname, unsigned int pos)
      {
         if (pos) snprintf( buf, sizeof(buf), "test %s failed at character pos %u", testname, pos);
         return buf[0]==0?0:buf;
      };
      const char* operator*()
      {
         return buf[0]==0?0:buf;
      };
   };
   static Error error;
   if (error.get( "IsoLatin1", *TextScannerTest< charset::IsoLatin1 >())
   ||  error.get( "UCS2BE",    *TextScannerTest< charset::UCS2BE    >())
   ||  error.get( "UCS2LE",    *TextScannerTest< charset::UCS2LE    >())
   ||  error.get( "UCS4BE",    *TextScannerTest< charset::UCS4BE    >())
   ||  error.get( "UCS4LE",    *TextScannerTest< charset::UCS4LE    >())
   ||  error.get( "UTF8",      *TextScannerTest< charset::UTF8      >())) return *error;
   return 0;
}

int main( int, const char**)
{
   const char* res = testAll();
   if (!res)
   {
      std::cerr << "OK" << std::endl;
      exit( 0);
   }
   else
   {
      std::cerr << res << std::endl;
      exit( 1);
   }
}




