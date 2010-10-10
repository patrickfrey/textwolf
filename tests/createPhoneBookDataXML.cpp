#include <stdio.h>
#include <string.h>

//compile: g++ -c -o createPhoneBookDataXML.o -g -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Winline -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors createPhoneBookDataXML.cpp
//link     g++ -lc -o createPhoneBookDataXML createPhoneBookDataXML.o

void processFile( FILE* ff)
{
   static const char* XML[] =
   {
      "<?xml charset=isolatin-1?>\n<doc>\n<name>",
      "</name>\n<vorname>",
      "</vorname>\n<strasse>",
      "</strasse>\n<plz>",
      "</plz>\n<gemeinde>",
      "</gemeinde>\n<tel>",
      "</tel>\n<fax>",
      "</fax>\n<titel>",
      "</titel>\n<sparte>",
      "</sparte>\n</doc>\n",
      0      
   };
   unsigned int lp=0;
   int ch;
   enum {bufsize=2048,maxtagsize=256};
   char buf[ bufsize];
   unsigned int bufpos = 0;
   while ((ch = fgetc( ff)) != EOF && bufpos < bufsize-maxtagsize)
   {
      if (ch=='\r') continue;
      if (ch=='\n')
      {
         if (lp == 0) continue;
         buf[ bufpos] = 0;
         while (XML[lp] != 0)
         {
            strcat( buf+bufpos, XML[lp++]);
            bufpos += strlen( buf+bufpos);
         }
         lp = 0;
         if (bufpos != 0) puts( buf);
         bufpos = 0;
      }
      else if (ch == '\t')
      {
         buf[ bufpos] = 0;
         if (XML[lp] != 0)
         {
            strcat( buf+bufpos, XML[lp++]);
            bufpos += strlen( buf+bufpos);
         }
      }
      else
      {
         if (lp == 0)
         {
            if (ch <= 32 && ch >= 0) continue;
            buf[ bufpos] = 0;
            strcat( buf+bufpos, XML[lp++]);
            bufpos += strlen( buf+bufpos);
         }
         buf[ bufpos++] = (char)ch;
      }
   }
   buf[ bufpos] = 0;
   if (lp != 0)
   {
      while (XML[lp] != 0)
      {
         strcat( buf+bufpos, XML[lp++]);
         bufpos += strlen( buf+bufpos);
      }
   }
   if (bufpos != 0) puts( buf);
}

int main( int argc, const char** argv)
{
   if (argc > 1)
   {
      for (int ii=1; ii<argc; ii++)
      {
         FILE* ff = fopen( argv[ii], "r");
         if (ff != 0)
         {
            processFile( ff);
         }
         else
         {
            fprintf( stderr, "could not open file '%s' for reading\n", argv[ii]); 
         }
      }
   }
   else
   {
      processFile( stdin);   
   }
}
