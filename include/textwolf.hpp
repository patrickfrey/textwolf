#ifndef __TEXTWOLF_HPP__
#define __TEXTWOLF_HPP__
#include <iterator>
#include <vector>
#include <stack>
#include <map>
#include <exception>
#undef LOWLEVEL_DEBUG

namespace textwolf {

struct Exception                                          {const char* msg; Exception( const char* p_msg):msg(p_msg){};};
struct ExceptionDimOutOfRange:public Exception            {ExceptionDimOutOfRange():Exception("DimOutOfRange"){};};
struct ExceptionStateNumbersNotAscending:public Exception {ExceptionStateNumbersNotAscending():Exception("StateNumbersNotAscending"){};};
struct ExceptionInvalidParam:public Exception             {ExceptionInvalidParam():Exception("InvalidParam"){};};
struct ExceptionInvalidState:public Exception             {ExceptionInvalidState():Exception("InvalidState"){};};
struct ExceptionIllegalParam:public Exception             {ExceptionIllegalParam():Exception("IllegalParam"){};};
struct ExceptionIllegalAttributeName:public Exception     {ExceptionIllegalAttributeName():Exception("IllegalAttributeName"){};};
struct ExceptionOutOfMem:public Exception                 {ExceptionOutOfMem():Exception("OutOfMem"){};};
struct ExceptionUnknown:public Exception                  {ExceptionUnknown():Exception("Unknown"){};};

/**
* character map for fast typing of a character byte
*/
template <typename RESTYPE, RESTYPE nullvalue_>
class CharMap
{
public:
   typedef RESTYPE valuetype;
   enum Constant {nullvalue=nullvalue_};
   
private:
   RESTYPE ar[ 256];
public:
   CharMap()                                                                         {for (unsigned int ii=0; ii<256; ii++) ar[ii]=(valuetype)nullvalue;};
   CharMap& operator()( unsigned char from, unsigned char to, valuetype value)       {for (unsigned int ii=from; ii<=to; ii++) ar[ii]=value; return *this;};
   CharMap& operator()( unsigned char at, valuetype value)                           {ar[at] = value; return *this;};
   valuetype operator []( unsigned char ii) const                                    {return ar[ii];};
};

/*
* unicode character range type used for processing
*/
typedef unsigned int UChar;

namespace charset {
/**
* Default character set definitions: 
* 1) Iso-Latin-1
* 2) UCS2  (little and big endian, not very efficient implementation)
* 3) UCS4  (little and big endian, not very efficient implementation)
* 4) UTF-8 (see http://de.wikipedia.org/wiki/UTF-8 for algorithms)
*/
struct IsoLatin1
{
   enum {HeadSize=1,Size=1,MaxChar=0xFF};
   
   static unsigned int asize()                                                 {return HeadSize;};
   static unsigned int size( const char*)                                      {return Size;};  
   static char achar( const char* buf)                                         {return buf[0];};
   static UChar value( const char* buf)                                        {return (unsigned char)buf[0];};
   static unsigned int print( UChar chr, char* buf, unsigned int bufsize)      {if (bufsize < 1) return 0; buf[0] = (chr <= 255)?(char)(unsigned char)chr:-1; return 1;};
};

struct ByteOrder
{
   enum {LE=1,BE=2,Machine=1};
};

template <int encoding=ByteOrder::Machine>
struct UCS2
{
   enum {LSB=(encoding==ByteOrder::BE),MSB=(encoding==ByteOrder::LE),HeadSize=2,Size=2,MaxChar=0xFFFF};

   static unsigned int asize()                                                 {return HeadSize;};
   static unsigned int size( const char*)                                      {return Size;};  
   static char achar( const char* buf)                                         {return (buf[MSB])?(char)-1:buf[LSB];};
   static UChar value( const char* buf)                                        {UChar res = (unsigned char)buf[MSB]; return (res << 8) + (unsigned char)buf[LSB];};
   static unsigned int print( UChar chr, char* buf, unsigned int bufsize)      {if (bufsize<2) return 0; if (chr>0xFFFF) {buf[0]=(char)0xFF; buf[1]=(char)0xFF;} else {buf[LSB]=(char)chr; buf[MSB]=(char)(chr>>8);} return 2;};
};

template <int encoding=ByteOrder::Machine>
struct UCS4
{
   enum {B0=(encoding==ByteOrder::BE)?3:0,B1=(encoding==ByteOrder::BE)?2:1,B2=(encoding==ByteOrder::BE)?1:2,B3=(encoding==ByteOrder::BE)?0:3,HeadSize=4,Size=4,MaxChar=0xFFFFFFFF};

   static unsigned int asize()                                                 {return HeadSize;};
   static unsigned int size( const char*)                                      {return Size;};  
   static char achar( const char* buf)                                         {return (buf[B3]|buf[B2]|buf[B1])?(char)-1:buf[B0];};
   static UChar value( const char* buf)                                        {UChar res = (unsigned char)buf[B3]; res = (res << 8) + (unsigned char)buf[B2]; res = (res << 8) + (unsigned char)buf[B1]; return (res << 8) + (unsigned char)buf[B0];};
   static unsigned int print( UChar chr, char* buf, unsigned int bufsize)      {buf[B0]=(char)chr; chr>>=8; buf[B1]=(char)chr; chr>>=8; buf[B2]=(char)chr; chr>>=8; buf[B3]=(char)chr; chr>>=8; return 4;};
};

struct UCS2LE :public UCS2<ByteOrder::LE> {};
struct UCS2BE :public UCS2<ByteOrder::BE> {};
struct UCS4LE :public UCS4<ByteOrder::LE> {};
struct UCS4BE :public UCS4<ByteOrder::BE> {};


struct UTF8
{   
   enum {MaxChar=0xFFFFFFFF};
   enum {B11111111=0xFF,
         B01111111=0x7F,
         B00111111=0x3F,
         B00011111=0x1F,
         B00001111=0x0F,
         B00000111=0x07,
         B00000011=0x03,
         B00000001=0x01,
         B00000000=0x00,
         B10000000=0x80,
         B11000000=0xC0,
         B11100000=0xE0,
         B11110000=0xF0,
         B11111000=0xF8,
         B11111100=0xFC,
         B11111110=0xFE,
         
         B11011111=B11000000|B00011111,
         B11101111=B11100000|B00001111,
         B11110111=B11110000|B00000111,
         B11111011=B11111000|B00000011,
         B11111101=B11111100|B00000001
        };

   enum {HeadSize=1};

   struct CharLengthTab   :public CharMap<unsigned char, 0>
   {
      CharLengthTab()
      {
         (*this)
         (B00000000,B01111111,1)
         (B11000000,B11011111,2)
         (B11100000,B11101111,3)
         (B11110000,B11110111,4)
         (B11111000,B11111011,5)
         (B11111100,B11111101,6)
         (B11111110,B11111110,7)
         (B11111111,B11111111,8);
      };
   };

   static unsigned int asize()                                                 {return HeadSize;};
   static char achar( const char* buf)                                         {return buf[0];};
   static unsigned int size( const char* buf)                                  {static CharLengthTab charLengthTab; return charLengthTab[ (unsigned char)buf[ 0]];};

   static UChar value( const char* buf)
   {
      const UChar invalid = (UChar)0-1;
      UChar res;
      int gg;
      int ii;
      unsigned char ch = (unsigned char)*buf;

      if (ch < 128) return ch;

      gg = size(buf)-2;
      if (gg < 0) return invalid;
      
      res = (ch)&(B00011111>>gg);
      for (ii=0; ii<=gg; ii++)
      {
         unsigned char xx = (unsigned char)buf[ii+1]; 
         res = (res<<6) | (xx & B00111111);
         if ((unsigned char)(xx & B11000000) != B10000000)
         {
            return invalid;
         }
      }
      return res;
   };
   
   static unsigned int print( UChar chr, char* buf, unsigned int bufsize)
   {
      unsigned int rt;
      if (bufsize < 8) return 0;
      if (chr <= 127) {
         buf[0] = chr;
         return 1;
      }
      unsigned int pp,sf;
      for (pp=1,sf=5; pp<5; pp++,sf+=5)
      {
         if (chr < (unsigned int)((1<<6)<<sf)) {
            rt = pp+1;
            while (pp > 0)
            {
               buf[pp--] = (chr & B00111111) | B10000000;
               chr >>= 6;
            }
            unsigned char HB = (unsigned char)(B11111111 << (8-rt));
            buf[0] = (char)(((unsigned char)chr & (~HB >> 1)) | HB); 
            return rt;
         }
      }
      rt = pp+1;
      while (pp > 0)
      {
         buf[pp--] = (chr & B00111111) | B10000000;
         chr >>= 6;
      }
      unsigned char HB = (unsigned char)(B11111111 << (8-rt));
      buf[0] = (char)(((unsigned char)chr & (~HB >> 1)) | HB); 
      return rt;
   };
};
}//namespace charset


/**
* control characters needed for XML scanner statemachine
*/
enum ControlCharacter 
{ 
   Undef=0, EndOfText, EndOfLine, Cntrl, Space, Amp, Lt, Equal, Gt, Slash, Exclam, Questm, Sq, Dq, Osb, Csb, Any,
   NofControlCharacter=17
};
const char* controlCharacterName( ControlCharacter c)
{
   static const char* name[ NofControlCharacter] = {"Undef", "EndOfText", "EndOfLine", "Cntrl", "Space", "Amp", "Lt", "Equal", "Gt", "Slash", "Exclam", "Questm", "Sq", "Dq", "Osb", "Csb", "Any"};
   return name[ (unsigned int)c];   
}


/**
* reads the input and provides the items to control the parsing:
*   control characters, ascii characters, unicode characters
* @TODO Implement better skip for iterator += i if available
*/
template <class Iterator, class CharSet>
class TextScanner
{
private:
   Iterator input;
   
   char buf[8];
   UChar val;
   char cur;
   unsigned char state;
   
public:
   struct ControlCharMap  :public CharMap<ControlCharacter,Undef>
   {
      ControlCharMap()
      {
         (*this)  (0,EndOfText)  (1,31,Cntrl)      (5,Undef)         (33,127,Any)   (128,255,Undef) 
                  ('\t',Space)   ('\r',EndOfLine)  ('\n',EndOfLine)  (' ',Space)    ('&',Amp)
                  ('<',Lt)       ('=',Equal)       ('>',Gt)          ('/',Slash)    ('!',Exclam)
                  ('?',Questm)   ('\'',Sq)         ('\"',Dq)         ('[',Osb)      (']',Osb);         
      };
   };
   
   TextScanner( const Iterator& p_iterator)    :input(p_iterator),val(0),cur(0),state(0)
   {
      skip();
   };
   
   UChar chr()
   {
      if (state == 1) 
      {
         for (int ii=0,nn=CharSet::size(buf)-CharSet::asize(); ii<nn; ii++)
         {
            buf[ii+CharSet::asize()] = *input;
            input++;
         }
         state = 0;
         val = CharSet::value(buf);          
      }
      else if (val == 0)
      {
         val = CharSet::value(buf);          
      }
      return val;
   };
   
   ControlCharacter control()
   {
      static ControlCharMap controlCharMap;
      return controlCharMap[ (unsigned char)cur];
   };
   char ascii()
   {
      return cur>=0?cur:0;
   };
   void skip()
   {
      if (state == 1)
      {
         int nn = CharSet::size(buf)-CharSet::asize();
         while (nn-- > 0) input++;
      }
      else
      {
         state = 1;
      }
      for (unsigned int ii=0; ii<CharSet::asize(); ii++)
      {
         buf[ii] = *input;
         input++;
      }
      cur = CharSet::achar(buf); 
   };
   
   TextScanner& operator ++()     {skip(); return *this;}   
   TextScanner& operator ++(int)  {skip(); return *this;}   
};


/**
* with this class we build up the XML element scanner state machine in a descriptive way
*/ 
class ScannerStatemachine
{
public:
   enum Constant   {MaxNofStates=128};
   struct Element
   {
      int fallbackState;
      int missError;
      struct 
      {
         int op;
         int arg;
      } action;
      char next[ NofControlCharacter];

      Element()
            :fallbackState(-1),missError(-1)
      {
         action.op = -1;
         action.arg = 0;
         for (unsigned int ii=0; ii<NofControlCharacter; ii++) next[ii] = -1;
      };
   };
   Element* get( int stateIdx)
   {
      if ((unsigned int)stateIdx>size) throw( ExceptionInvalidState()); 
      return tab + stateIdx;
   };

private:
   Element tab[ MaxNofStates];
   unsigned int size;

   void newState( int stateIdx)
   {
      if (size != (unsigned int)stateIdx) throw( ExceptionStateNumbersNotAscending());
      if (size >= MaxNofStates) throw( ExceptionDimOutOfRange());
      size++;
   };
   void addOtherTransition( int nextState)
   {
      if (size == 0) throw (ExceptionInvalidState());
      if (nextState < 0 || nextState > MaxNofStates) throw (ExceptionInvalidParam());
      for (unsigned int inputchr=0; inputchr<NofControlCharacter; inputchr++)
      {
         if (tab[ size-1].next[ inputchr] == -1) tab[ size-1].next[ inputchr] = (unsigned char)nextState;
      }
   };
   void addTransition( ControlCharacter inputchr, int nextState)
   {
      if (size == 0) throw (ExceptionInvalidState());
      if ((unsigned int)inputchr >= (unsigned int)NofControlCharacter) throw (ExceptionInvalidParam());
      if (nextState < 0 || nextState > MaxNofStates) throw (ExceptionInvalidParam());
      if (tab[ size-1].next[ inputchr] != -1) throw (ExceptionInvalidParam());
      if (size == 0) throw (ExceptionInvalidState());
      tab[ size-1].next[ inputchr] = (unsigned char)nextState;
   };
   void addTransition( ControlCharacter inputchr)
   {
      addTransition( inputchr, size-1);
   };
   void addAction( int action_op, int action_arg=0)
   {
      if (size == 0) throw (ExceptionInvalidState());
      if (tab[ size-1].action.op != -1) throw (ExceptionInvalidState());
      tab[ size-1].action.op = action_op;
      tab[ size-1].action.arg = action_arg;
   };
   void addMiss( int error)
   {
      if (size == 0) throw (ExceptionInvalidState());
      if (tab[ size-1].missError != -1) throw (ExceptionInvalidState());
      tab[ size-1].missError = error;
   };
   void addFallback( int stateIdx)
   {
      if (size == 0) throw (ExceptionInvalidState());
      if (tab[ size-1].fallbackState != -1) throw (ExceptionInvalidState());
      if (stateIdx < 0 || stateIdx > MaxNofStates) throw (ExceptionInvalidParam());
      tab[ size-1].fallbackState = stateIdx;      
   };
public:
   ScannerStatemachine()  
      :size(0)
   {};

   ScannerStatemachine& operator[]( int stateIdx)                           {newState(stateIdx); return *this;};   
   ScannerStatemachine& operator()( ControlCharacter inputchr, int ns)      {addTransition(inputchr,ns); return *this;};
   ScannerStatemachine& operator()( ControlCharacter inputchr1, 
                                    ControlCharacter inputchr2, int ns)     {addTransition(inputchr1,ns); addTransition(inputchr2,ns); return *this;};
   ScannerStatemachine& operator()( ControlCharacter inputchr1, 
                                    ControlCharacter inputchr2, 
                                    ControlCharacter inputchr3, int ns)     {addTransition(inputchr1,ns); addTransition(inputchr2,ns); addTransition(inputchr3,ns); return *this;};
   ScannerStatemachine& operator()( ControlCharacter inputchr)              {addTransition(inputchr); return *this;};
   ScannerStatemachine& action( int aa, int arg=0)                          {addAction(aa,arg); return *this;};
   ScannerStatemachine& miss( int ee)                                       {addMiss(ee); return *this;};
   ScannerStatemachine& fallback( int stateIdx)                             {addFallback(stateIdx); return *this;};
   ScannerStatemachine& other( int stateIdx)                                {addOtherTransition(stateIdx); return *this;};
};

/**
* the template XMLScanner provides you the XML elements like tags, attributes, etc. with an STL conform input iterator 
* with XMLScannerBase we define the common elements
*/
class XMLScannerBase
{
public:
   enum ElementType
   {
      None, ErrorOccurred, HeaderAttribName, HeaderAttribValue, TagAttribName, TagAttribValue, OpenTag, CloseTag, CloseTagIm, Content, Exit
   };
   static const char* getElementTypeName( ElementType ee)
   {
      static const char* names[ (unsigned int)Exit+1] = {0,"ErrorOccurred","HeaderAttribName","HeaderAttribValue","TagAttribName","TagAttribValue","OpenTag","CloseTag","CloseTagIm","Content","Exit"};
      return names[ (unsigned int)ee];
   };
   enum Error
   {
      Ok,ErrMemblockTooSmall,ErrExpectedOpenTag,ErrUnexpectedState,
      ErrExpectedXMLTag,ErrSyntaxString,ErrUnexpectedEndOfText,ErrOutputBufferTooSmall,
      ErrSyntaxToken,ErrStringExceedsLine,ErrEntityEncodesCntrlChar,ErrExpectedIdentifier,
      ErrUndefinedCharacterEntity,ErrInternalErrorSTM,ErrExpectedTagEnd,ErrExpectedEqual,
      ErrExpectedTagAttribute,ErrExpectedCDATATag,ErrInternal,
      ErrDimOutOfRange, ErrStateNumbersNotAscending, ErrInvalidParam, ErrInvalidState
   };
   static const char* getErrorString( Error ee)
   {
      enum Constant {NofErrors=19};
      static const char* sError[NofErrors]
      = {0,"MemblockTooSmall","ExpectedOpenTag","UnexpectedState",
         "ExpectedXMLTag","SyntaxString","UnexpectedEndOfText","OutputBufferTooSmall",
         "SyntaxToken","StringExceedsLine","EntityEncodesCntrlChar","ExpectedIdentifier",
         "UndefinedCharacterEntity","InternalErrorSTM","ExpectedTagEnd","ExpectedEqual",
         "ExpectedTagAttribute","ExpectedCDATATag","Internal"
      };
      return sError[(unsigned int)ee];
   };
   enum STMState 
   {
      START,  STARTTAG, XTAG, XTAGEND, XTAGAISK, XTAGANAM, XTAGAESK, XTAGAVSK, XTAGAVID, XTAGAVSQ, XTAGAVDQ, CONTENT, 
      TOKEN, XMLTAG, OPENTAG, CLOSETAG, TAGCLSK, TAGAISK, TAGANAM, TAGAESK, TAGAVSK, TAGAVID, TAGAVSQ, TAGAVDQ,
      TAGCLIM, ENTITYSL, ENTITY, CDATA, CDATA1, CDATA2, CDATA3, EXIT
   };
   static const char* getStateString( STMState s)
   {
      enum Constant {NofStates=32};
      static const char* sState[NofStates]
      = {
         "START", "STARTTAG", "XTAG", "XTAGEND", "XTAGAISK", "XTAGANAM", "XTAGAESK", "XTAGAVSK", "XTAGAVID", "XTAGAVSQ", "XTAGAVDQ", "CONTENT",
         "TOKEN", "XMLTAG", "OPENTAG", "CLOSETAG", "TAGCLSK", "TAGAISK", "TAGANAM", "TAGAESK", "TAGAVSK", "TAGAVID", "TAGAVSQ", "TAGAVDQ",
         "TAGCLIM", "ENTITYSL", "ENTITY", "CDATA", "CDATA1", "CDATA2", "CDATA3", "EXIT"
      };      
      return sState[(unsigned int)s];      
   };
   
   enum STMAction 
   { 
      Return, ReturnToken, ReturnIdentifier, ReturnSQString, ReturnDQString, ExpectIdentifierXML, ExpectIdentifierCDATA, ReturnEOF
   };
   static const char* getActionString( STMAction a)
   {
      static const char* name[ 8] = {"Return", "ReturnToken", "ReturnIdentifier", "ReturnSQString", "ReturnDQString", "ExpectIdentifierXML", "ExpectIdentifierCDATA", "ReturnEOF"};
      return name[ (unsigned int)a];
   };

   //@TODO return entity definitions as events back
   struct Statemachine :public ScannerStatemachine
   {
      Statemachine()
      {
         (*this)
         [ START    ](EndOfLine)(Cntrl)(Space)(Lt,STARTTAG).miss(ErrExpectedOpenTag)
         [ STARTTAG ](EndOfLine)(Cntrl)(Space)(Questm,XTAG )(Exclam,ENTITYSL).fallback(OPENTAG)
         [ XTAG     ].action(ExpectIdentifierXML)(EndOfLine,Cntrl,Space,XTAGAISK)(Questm,XTAGEND).miss(ErrExpectedXMLTag)
         [ XTAGEND  ](Gt,CONTENT)(EndOfLine)(Cntrl)(Space).miss(ErrExpectedTagEnd)
         [ XTAGAISK ](EndOfLine)(Cntrl)(Space)(Questm,XTAGEND).fallback(XTAGANAM)
         [ XTAGANAM ].action(ReturnIdentifier,HeaderAttribName)(EndOfLine,Cntrl,Space,XTAGAESK)(Equal,XTAGAVSK).miss(ErrExpectedEqual)
         [ XTAGAESK ](EndOfLine)(Cntrl)(Space)(Equal,XTAGAVSK).miss(ErrExpectedEqual)
         [ XTAGAVSK ](EndOfLine)(Cntrl)(Space)(Sq,XTAGAVSQ)(Dq,XTAGAVDQ).fallback(XTAGAVID)
         [ XTAGAVID ].action(ReturnIdentifier,HeaderAttribValue)(EndOfLine,Cntrl,Space,XTAGAISK)(Questm,XTAGEND).miss(ErrExpectedTagAttribute)
         [ XTAGAVSQ ].action(ReturnSQString,HeaderAttribValue)(EndOfLine,Cntrl,Space,XTAGAISK)(Questm,XTAGEND).miss(ErrExpectedTagAttribute)
         [ XTAGAVDQ ].action(ReturnDQString,HeaderAttribValue)(EndOfLine,Cntrl,Space,XTAGAISK)(Questm,XTAGEND).miss(ErrExpectedTagAttribute)
         [ CONTENT  ](EndOfText,EXIT)(EndOfLine)(Cntrl)(Space)(Lt,XMLTAG).fallback(TOKEN)
         [ TOKEN    ].action(ReturnToken,Content)(EndOfLine)(EndOfText,EXIT)(Cntrl)(Space)(Lt,XMLTAG).fallback(TOKEN)
         [ XMLTAG   ](EndOfLine)(Cntrl)(Space)(Slash,CLOSETAG).fallback(OPENTAG)
         [ OPENTAG  ].action(ReturnIdentifier,OpenTag)(EndOfLine,Cntrl,Space,TAGAISK)(Slash,TAGCLIM)(Gt,CONTENT).miss(ErrExpectedTagAttribute)
         [ CLOSETAG ].action(ReturnIdentifier,CloseTag)(EndOfLine,Cntrl,Space,TAGCLSK)(Gt,CONTENT).miss(ErrExpectedTagEnd)
         [ TAGCLSK  ](EndOfLine)(Cntrl)(Space)(Gt,CONTENT).miss(ErrExpectedTagEnd)
         [ TAGAISK  ](EndOfLine)(Cntrl)(Space)(Gt,CONTENT)(Slash,TAGCLIM).fallback(TAGANAM)
         [ TAGANAM  ].action(ReturnIdentifier,TagAttribName)(EndOfLine,Cntrl,Space,TAGAESK)(Equal,TAGAVSK).miss(ErrExpectedEqual)
         [ TAGAESK  ](EndOfLine)(Cntrl)(Space)(Equal,TAGAVSK).miss(ErrExpectedEqual)
         [ TAGAVSK  ](EndOfLine)(Cntrl)(Space)(Sq,TAGAVSQ)(Dq,TAGAVDQ).fallback(TAGAVID)
         [ TAGAVID  ].action(ReturnIdentifier,TagAttribValue)(EndOfLine,Cntrl,Space,TAGAISK)(Slash,TAGCLIM)(Gt,CONTENT).miss(ErrExpectedTagAttribute)
         [ TAGAVSQ  ].action(ReturnSQString,TagAttribValue)(EndOfLine,Cntrl,Space,TAGAISK)(Slash,TAGCLIM)(Gt,CONTENT).miss(ErrExpectedTagAttribute)
         [ TAGAVDQ  ].action(ReturnDQString,TagAttribValue)(EndOfLine,Cntrl,Space,TAGAISK)(Slash,TAGCLIM)(Gt,CONTENT).miss(ErrExpectedTagAttribute)
         [ TAGCLIM  ].action(Return,CloseTagIm)(EndOfLine)(Cntrl)(Space)(Gt,CONTENT).miss(ErrExpectedTagEnd)
         [ ENTITYSL ](Osb,CDATA).fallback(ENTITY)
         [ ENTITY   ](Exclam,TAGCLSK).other( ENTITY)
         [ CDATA    ].action(ExpectIdentifierCDATA)(Osb,CDATA1).miss(ErrExpectedCDATATag)
         [ CDATA1   ](Csb,CDATA2).other(CDATA1)
         [ CDATA2   ](Csb,CDATA3).other(CDATA1)
         [ CDATA3   ](Gt,CONTENT).other(CDATA1)        
         [ EXIT     ].action(ReturnEOF);
      };
   };
};


template <
      class InputIterator,                         //< STL conform input iterator with ++ and read only * returning 0 als last character of the input
      class InputCharSet_=charset::UTF8,           //Character set encoding of the input, read as stream of bytes
      class OutputCharSet_=charset::UTF8,          //Character set encoding of the output, printed as string of the item type of the character set
      class EntityMap=std::map<const char*,UChar>  //< STL like map from ASCII const char* to UChar
>
class XMLScanner :public XMLScannerBase
{
public:
   typedef InputCharSet_ InputCharSet;
   typedef OutputCharSet_ OutputCharSet;
   typedef unsigned int size_type;
   class iterator;
   
public:
   typedef TextScanner<InputIterator,InputCharSet_> InputReader;
   typedef XMLScanner<InputIterator,InputCharSet_,OutputCharSet_,EntityMap> ThisXMLScanner;
   typedef typename EntityMap::iterator EntityMapIterator;

   unsigned int print( UChar ch)
   {
      unsigned int nn = OutputCharSet::print( ch, outputBuf+outputSize, outputBufSize-outputSize);
      if (nn == 0) error = ErrOutputBufferTooSmall;
      return nn;     
   };
   bool push( UChar ch)
   {
      unsigned int nn = print( ch);
      outputSize += nn; 
      return (nn != 0);     
   };
   bool parseEntity()
   {
      struct HexCharMap :public CharMap<unsigned char, 0xFF>
      {
         HexCharMap()
         {
            (*this)
               ('0',0) ('1', 1)('2', 2)('3', 3)('4', 4)('5', 5)('6', 6)('7', 7)('8', 8)('9', 9)
               ('A',10)('B',11)('C',12)('D',13)('E',14)('F',15)('a',10)('b',11)('c',12)('d',13)('e',14)('f',15);
         };
      };
      static HexCharMap hexCharMap;
      
      unsigned char ch;
      unsigned char chval;
      int base;
      char ebuf[ 256];
      unsigned int ee = 0;
      signed long long value = 0;
      
      if (src.ascii() != '&')
      {
         error = ErrUnexpectedState; return false;
      }
      src.skip();
      ch = src.ascii();
      switch (ch)
      {
         case '#':
            src.skip();
            ch = src.ascii();
            if (ch == 'x')
            {
               src.skip();
               ch = src.ascii();
               chval = hexCharMap[ ch];
               if (chval == HexCharMap::nullvalue)
               {
                  ebuf[ ee++] = 'x'; base = -1; break;
               }
               else
               {
                  base = 16;
               }
            }
            else
            {
               chval = hexCharMap[ ch];
               base = 10;
               if (chval == HexCharMap::nullvalue || chval >= base)
               {
                  base = -1;
               }
            }
            if (base == -1)
            {
               if (!push('&')) return false;
               if (!push('#')) return false;
               return true;
            }
            else
            {
               while (ee < 16)
               {
                  ch = src.ascii();
                  chval = hexCharMap[ ch];
                  if (chval == HexCharMap::nullvalue || value >= base) break;
                  value = value * base + chval;
                  ebuf[ ee++] = ch;
                  src.skip();
               }
               if (ee == 0 || value >= (1LL << (8*sizeof(UChar))) || ch != ';')
               {
                  if (!push('&')) return false;
                  if (!push('#')) return false;
                  for (unsigned int ii=0; ii<ee; ii++) if (!push(ebuf[ii])) return false;
                  return true;
               }
               src.skip();
               UChar chr = (UChar)value;
               if (chr < 32)
               {
                  error = ErrEntityEncodesCntrlChar; return false;
               }
               if (!push( chr)) return false;
               return true;
            }
         default:
            while (ee < sizeof(ebuf) && ch != ';' && src.control() == Any)
            {               
               ebuf[ ee++] = ch;
               src.skip();
               ch = src.ascii();
            }
            if (ch == ';')
            {
               ebuf[ ee] = '\0';
               return pushEntity( ebuf);
            }
            else
            {
               if (!push('&')) return false;
               if (!push('#')) return false;
               for (unsigned int ii=0; ii<ee; ii++) if (!push(ebuf[ii])) return false;
               return true;
            }
         break;
      }
      if (!push('&')) return false;
      return true;
   };
   bool parseToken()
   {
      for (;;)
      {
         ControlCharacter ch;
         while ((ch=src.control()) == Any || ch==Equal || ch==Slash || ch==Exclam || ch==Questm || ch==Sq || ch==Dq || ch==Osb || ch==Csb) 
         {
            push( src.chr());
            src.skip();
         }
         if (src.control() == Amp) 
         {
            if (!parseEntity()) return false;
         }
         else
         {
            if (outputSize == 0)
            {
               error = ErrExpectedIdentifier; return false;
            }
            return true;
         }
      }
   };
   bool skipToken()
   {
      for (;;)
      {
         ControlCharacter ch;
         while ((ch=src.control()) == Any || ch==Equal || ch==Slash || ch==Exclam || ch==Questm || ch==Sq || ch==Dq || ch==Osb || ch==Csb || ch==Amp) src.skip();

         if (outputSize == 0)
         {
            error = ErrExpectedIdentifier; return false;
         }
         return true;
      }
   };
   bool parseIdentifier()
   {
      for (;;)
      {
         ControlCharacter ch;
         while ((ch=src.control()) == Any) 
         {
            push( src.chr());
            src.skip();
         }
         if (src.control() == Amp) 
         {
            if (!parseEntity()) return false;
         }
         else
         {
            if (outputSize == 0)
            {
               error = ErrExpectedIdentifier; return false;
            }
            return true;
         }
      }
   };
   bool skipIdentifier()
   {
      for (;;)
      {
         ControlCharacter ch;
         while ((ch=src.control()) == Any || ch == Amp) src.skip();
         if (outputSize == 0)
         {
            error = ErrExpectedIdentifier; return false;
         }
         return true;
      }
   };   
   bool parseString( ControlCharacter eb)
   {
      ControlCharacter ch;
      while ((ch=src.control()) != EndOfText && ch != EndOfLine && ch != Cntrl && ch != eb)
      {
         if (ch == Amp)
         {
            if (!parseEntity()) return false;
         }
         else
         {
            push( src.chr());
            src.skip();
         }         
      }
      if (ch != eb)
      {
         if (ch == EndOfText) {error = ErrUnexpectedEndOfText; return false;}
         if (ch == EndOfLine) {error = ErrStringExceedsLine; return false;}
         if (ch == Cntrl)     {error = ErrSyntaxString; return false;}
         error = ErrUnexpectedState; return false;         
      }
      src.skip();
      return true;
   }; 
   bool skipString( ControlCharacter eb)
   {
      ControlCharacter ch;
      while ((ch=src.control()) != EndOfText && ch != EndOfLine && ch != Cntrl && ch != eb) src.skip();

      if (ch != eb)
      {
         if (ch == EndOfText) {error = ErrUnexpectedEndOfText; return false;}
         if (ch == EndOfLine) {error = ErrStringExceedsLine; return false;}
         if (ch == Cntrl)     {error = ErrSyntaxString; return false;}
         error = ErrUnexpectedState; return false;         
      }
      src.skip();
      return true;
   }; 
   char nextNonSpace()
   {
      ControlCharacter ch;
      while ((ch=src.control()) == Space || ch == EndOfLine || ch == Cntrl) src.skip();
      return ch;
   };   
   bool expectSpace()
   {
      switch (src.control())
      {
         case Space:
         case EndOfLine:
         case Cntrl:
            src.skip();
            break;
            
         case EndOfText: error = ErrUnexpectedEndOfText; return false;
         default: error = ErrSyntaxToken; return false;
      }
      return true;
   };
   bool expectStr( const char* str)
   {
      unsigned int ii;
      for (ii=0; str[ii] != '\0'; ii++,src.skip())
      {
         if (src.ascii() == str[ii]) continue;
         ControlCharacter ch = src.control();
         if (ch == EndOfText) {error = ErrUnexpectedEndOfText; return false;};
         error = ErrSyntaxToken; return false;
      }
      return true;
   }; 
   bool pushPredefinedEntity( const char* str)
   {
      if (strcmp( str, "quot") == 0) push( '"');
      else if (strcmp( str, "amp") == 0) push( '&');
      else if (strcmp( str, "apos") == 0) push( '\'');
      else if (strcmp( str, "lt") == 0) push( '<');
      else if (strcmp( str, "gt") == 0) push( '>');
      else
      {
         error = ErrUndefinedCharacterEntity;
         return false;
      }
      return true;
   };
   bool pushEntity( const char* str)
   {
      if (entityMap == 0) return pushPredefinedEntity( str);
      EntityMapIterator itr = entityMap->find( str);
      if (itr == entityMap->end()) return pushPredefinedEntity( str);

      UChar ch = itr->second;
      if (ch < 32)
      {
         error = ErrEntityEncodesCntrlChar; return false;
         return false;
      }
      return push( ch);
   };
   
private:
   STMState state;
   Error error;      
   InputReader src;
   EntityMap* entityMap;
   char* outputBuf;
   size_type outputBufSize;
   size_type outputSize;
   
public:
   XMLScanner( InputIterator& p_src, char* p_outputBuf, size_type p_outputBufSize, EntityMap* p_entityMap=0)
         :state(START),error(Ok),src(p_src),entityMap(p_entityMap),outputBuf(p_outputBuf),outputBufSize(p_outputBufSize),outputSize(0)
   {};
   XMLScanner( XMLScanner& o)
         :state(o.state),error(o.error),src(o.src),entityMap(o.entityMap),outputBuf(o.outputBuf),outputBufSize(o.outputBufSize),outputSize(o.outputSize)
   {};

   template <class CharSet>
   static bool getTagName( const char* src, char* p_outputBuf, size_type p_outputBufSize, size_type* p_outputSize)
   {
      bool rt;
      XMLScanner<const char*, charset::UTF8, CharSet> scanner( src, p_outputBuf, p_outputBufSize);
      rt = scanner.parseToken();
      *p_outputSize = scanner.getItemSize();
      return rt;
   }
   
   char* getItem() const {return outputBuf;};
   size_type getItemSize() const {return outputSize;};
   ScannerStatemachine::Element* getState()
   {
      static Statemachine STM;
      return STM.get( state);
   }
   
   Error getError( const char** str=0)
   {
      Error rt = error; 
      error = Ok; 
      if (str) *str=getErrorString(rt); 
      return rt;
   };

   ElementType nextItem( unsigned short mask=0xFFFF)
   {
      outputSize = 0;
      outputBuf[0] = 0;
      ElementType rt = None;
      
      do
      {
         #ifdef LOWLEVEL_DEBUG
         printf( "STATE %s\n", getStateString( state));
         #endif
      
         ScannerStatemachine::Element* sd = getState();
         if (sd->action.op != -1)
         {
            #ifdef LOWLEVEL_DEBUG
            printf( "ACTION %s\n", getActionString( (STMAction)sd->action.op));
            #endif
            switch ((STMAction)(sd->action.op))
            {
               case Return:
                  rt = (ElementType)sd->action.arg;
                  break;

               case ReturnToken:
                  if ((mask&(1<<sd->action.arg)) != 0)
                  {
                     if (!parseToken()) return ErrorOccurred;
                  }
                  else
                  {
                     if (!skipToken()) return ErrorOccurred;
                  }
                  if (!print(0)) return ErrorOccurred;
                  rt = (ElementType)sd->action.arg;
                  break;

               case ReturnIdentifier:
                  if ((mask&(1<<sd->action.arg)) != 0)
                  {
                     if (!parseIdentifier()) return ErrorOccurred;
                  }
                  else
                  {
                     if (!skipIdentifier()) return ErrorOccurred;
                  }
                  if (!print(0)) return ErrorOccurred;
                  rt = (ElementType)sd->action.arg;
                  break;

               case ReturnSQString:
                  if ((mask&(1<<sd->action.arg)) != 0)
                  {
                     if (!parseString( Sq)) return ErrorOccurred;
                  }
                  else
                  {
                     if (!skipString( Sq)) return ErrorOccurred;
                  }
                  if (!print(0)) return ErrorOccurred;
                  rt = (ElementType)sd->action.arg;
                  break;

               case ReturnDQString:
                  if ((mask&(1<<sd->action.arg)) != 0)
                  {
                     if (!parseString( Dq)) return ErrorOccurred;
                  }
                  else
                  {
                     if (!skipString( Dq)) return ErrorOccurred;
                  }
                  if (!print(0)) return ErrorOccurred;
                  rt = (ElementType)sd->action.arg;
                  break;
               
               case ExpectIdentifierXML:
                  if (!expectStr( "xml")) return ErrorOccurred;
                  break;
                  
               case ExpectIdentifierCDATA:
                  if (!expectStr( "CDATA")) return ErrorOccurred;
                  break;

               case ReturnEOF:
                  return Exit;

               default:
                  error = ErrInternal;
                  return ErrorOccurred;   
            }
         }
         ControlCharacter ch = src.control();
         #ifdef LOWLEVEL_DEBUG
         printf( "GET %s\n", controlCharacterName(ch));
         #endif

         if (sd->next[ ch] != -1) 
         {
            state = (STMState)sd->next[ ch];
            src.skip();
         }
         else if (sd->fallbackState != -1)
         {
            state = (STMState)sd->fallbackState;            
         }
         else if (sd->missError != -1)
         {
            print(0);
            error = (Error)sd->missError;
            return ErrorOccurred;
         }
         else if (ch == EndOfText)
         {
            print(0);
            error = ErrUnexpectedEndOfText;
            return ErrorOccurred;
         }
         else
         {
            print(0);
            error = ErrInternal;
            return ErrorOccurred;
         }
      }
      while (rt == None);
      return rt;
   };

   //STL conform input iterator for the output of this XMLScanner:   
   struct End {};
   class iterator
   {
   public:
      struct Element
      {
         ElementType type;
         const char* name() const      {return getElementTypeName( type);};
         char* content;
         size_type size;
         
         Element()                     :type(None),content(0),size(0)
         {};
         Element( const End&)          :type(Exit),content(0),size(0)
         {};
      };
      typedef Element value_type;
      typedef size_type difference_type;
      typedef Element* pointer;
      typedef Element& reference;
      typedef std::input_iterator_tag iterator_category;

   private:
      Element element;
      ThisXMLScanner* input;

      void skip( unsigned short mask=0xFFFF)
      {
         if (input != 0)
         {
            element.type = input->nextItem(mask);
            element.content = input->getItem();
            element.size = input->getItemSize();
         }
      };
      bool compare( const iterator& iter) const
      {
         if (element.type == iter.element.type)
         {
            if (element.type == Exit || element.type == None) return true;  //equal only at beginning and end
         }
         return false;              
      };
   public:
      void assign( const iterator& orig)
      {
         input = orig.input;
         element.type = orig.element.type;
         element.content = orig.element.content;
         element.size = orig.element.size;
      };
      iterator( const iterator& orig) 
      {
         assign( orig);
      };
      iterator( ThisXMLScanner& p_input)
             :input( &p_input)
      {
         element.type = input->nextItem();
         element.content = input->getItem();
         element.size = input->getItemSize();
      };
      iterator( const End& et)  :element(et),input(0) {};
      iterator()  :input(0) {};
      iterator& operator = (const iterator& orig)
      {
         assign( orig);
         return *this;
      }      
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
      return iterator( End());
   };
}; 

template <class CharSet_=charset::UTF8>
class XMLPathSelectAutomaton
{
public:
   XMLPathSelectAutomaton() {};

   typedef int Hash;
   typedef XMLPathSelectAutomaton<CharSet_> ThisXMLPathSelectAutomaton;
   
   static Hash hash( const char* key, unsigned int keysize)
   {
      static Hash crc32table[ 256] = {
         0x0u,0x77073096u,0xee0e612cu,0x990951bau,0x76dc419u,0x706af48fu,0xe963a535u,0x9e6495a3u,
         0xedb8832u,0x79dcb8a4u,0xe0d5e91eu,0x97d2d988u,0x9b64c2bu,0x7eb17cbdu,0xe7b82d07u,0x90bf1d91u,
         0x1db71064u,0x6ab020f2u,0xf3b97148u,0x84be41deu,0x1adad47du,0x6ddde4ebu,0xf4d4b551u,0x83d385c7u,
         0x136c9856u,0x646ba8c0u,0xfd62f97au,0x8a65c9ecu,0x14015c4fu,0x63066cd9u,0xfa0f3d63u,0x8d080df5u,
         0x3b6e20c8u,0x4c69105eu,0xd56041e4u,0xa2677172u,0x3c03e4d1u,0x4b04d447u,0xd20d85fdu,0xa50ab56bu,
         0x35b5a8fau,0x42b2986cu,0xdbbbc9d6u,0xacbcf940u,0x32d86ce3u,0x45df5c75u,0xdcd60dcfu,0xabd13d59u,
         0x26d930acu,0x51de003au,0xc8d75180u,0xbfd06116u,0x21b4f4b5u,0x56b3c423u,0xcfba9599u,0xb8bda50fu,
         0x2802b89eu,0x5f058808u,0xc60cd9b2u,0xb10be924u,0x2f6f7c87u,0x58684c11u,0xc1611dabu,0xb6662d3du,
         0x76dc4190u,0x1db7106u,0x98d220bcu,0xefd5102au,0x71b18589u,0x6b6b51fu,0x9fbfe4a5u,0xe8b8d433u,
         0x7807c9a2u,0xf00f934u,0x9609a88eu,0xe10e9818u,0x7f6a0dbbu,0x86d3d2du,0x91646c97u,0xe6635c01u,
         0x6b6b51f4u,0x1c6c6162u,0x856530d8u,0xf262004eu,0x6c0695edu,0x1b01a57bu,0x8208f4c1u,0xf50fc457u,
         0x65b0d9c6u,0x12b7e950u,0x8bbeb8eau,0xfcb9887cu,0x62dd1ddfu,0x15da2d49u,0x8cd37cf3u,0xfbd44c65u,
         0x4db26158u,0x3ab551ceu,0xa3bc0074u,0xd4bb30e2u,0x4adfa541u,0x3dd895d7u,0xa4d1c46du,0xd3d6f4fbu,
         0x4369e96au,0x346ed9fcu,0xad678846u,0xda60b8d0u,0x44042d73u,0x33031de5u,0xaa0a4c5fu,0xdd0d7cc9u,
         0x5005713cu,0x270241aau,0xbe0b1010u,0xc90c2086u,0x5768b525u,0x206f85b3u,0xb966d409u,0xce61e49fu,
         0x5edef90eu,0x29d9c998u,0xb0d09822u,0xc7d7a8b4u,0x59b33d17u,0x2eb40d81u,0xb7bd5c3bu,0xc0ba6cadu,
         0xedb88320u,0x9abfb3b6u,0x3b6e20cu,0x74b1d29au,0xead54739u,0x9dd277afu,0x4db2615u,0x73dc1683u,
         0xe3630b12u,0x94643b84u,0xd6d6a3eu,0x7a6a5aa8u,0xe40ecf0bu,0x9309ff9du,0xa00ae27u,0x7d079eb1u,
         0xf00f9344u,0x8708a3d2u,0x1e01f268u,0x6906c2feu,0xf762575du,0x806567cbu,0x196c3671u,0x6e6b06e7u,
         0xfed41b76u,0x89d32be0u,0x10da7a5au,0x67dd4accu,0xf9b9df6fu,0x8ebeeff9u,0x17b7be43u,0x60b08ed5u,
         0xd6d6a3e8u,0xa1d1937eu,0x38d8c2c4u,0x4fdff252u,0xd1bb67f1u,0xa6bc5767u,0x3fb506ddu,0x48b2364bu,
         0xd80d2bdau,0xaf0a1b4cu,0x36034af6u,0x41047a60u,0xdf60efc3u,0xa867df55u,0x316e8eefu,0x4669be79u,
         0xcb61b38cu,0xbc66831au,0x256fd2a0u,0x5268e236u,0xcc0c7795u,0xbb0b4703u,0x220216b9u,0x5505262fu,
         0xc5ba3bbeu,0xb2bd0b28u,0x2bb45a92u,0x5cb36a04u,0xc2d7ffa7u,0xb5d0cf31u,0x2cd99e8bu,0x5bdeae1du,
         0x9b64c2b0u,0xec63f226u,0x756aa39cu,0x26d930au,0x9c0906a9u,0xeb0e363fu,0x72076785u,0x5005713u,
         0x95bf4a82u,0xe2b87a14u,0x7bb12baeu,0xcb61b38u,0x92d28e9bu,0xe5d5be0du,0x7cdcefb7u,0xbdbdf21u,
         0x86d3d2d4u,0xf1d4e242u,0x68ddb3f8u,0x1fda836eu,0x81be16cdu,0xf6b9265bu,0x6fb077e1u,0x18b74777u,
         0x88085ae6u,0xff0f6a70u,0x66063bcau,0x11010b5cu,0x8f659effu,0xf862ae69u,0x616bffd3u,0x166ccf45u,
         0xa00ae278u,0xd70dd2eeu,0x4e048354u,0x3903b3c2u,0xa7672661u,0xd06016f7u,0x4969474du,0x3e6e77dbu,
         0xaed16a4au,0xd9d65adcu,0x40df0b66u,0x37d83bf0u,0xa9bcae53u,0xdebb9ec5u,0x47b2cf7fu,0x30b5ffe9u,
         0xbdbdf21cu,0xcabac28au,0x53b39330u,0x24b4a3a6u,0xbad03605u,0xcdd70693u,0x54de5729u,0x23d967bfu,
         0xb3667a2eu,0xc4614ab8u,0x5d681b02u,0x2a6f2b94u,0xb40bbe37u,0xc30c8ea1u,0x5a05df1bu,0x2d02ef8du
      };
      unsigned int crc = 0xFFFFFFFF;
      for (unsigned int ii=0; ii<keysize; ii++) crc = (crc >> 8) ^ crc32table[(crc ^ key[ii]) & 0xff];
      return (int)(~crc & 0x7FFFFFFF);
   };

public:
   enum Operation 
   {
      Content, Tag, Attribute, ThisAttributeValue, AttributeValue
   };
   static const char* operationName( Operation op)
   {
      static const char* name[ 5] = {"Content", "Tag", "Attribute", "ThisAttributeValue", "AttributeValue"};
      return name[ (unsigned int)op];
   };

   struct Mask
   {
      unsigned short pos;
      unsigned short neg;
      Mask( unsigned short p_pos=0, unsigned short p_neg=0):pos(p_pos),neg(p_neg) {};
      Mask( const Mask& orig)                              :pos(orig.pos),neg(orig.neg) {};
      Mask( Operation op)                                  :pos(0),neg(0) {this->match(op);};
      void reset()                                         {pos=0; neg=0;};
      void reject( XMLScannerBase::ElementType e)          {neg |= (1<<(unsigned short)e);};
      void match( XMLScannerBase::ElementType e)           {pos |= (1<<(unsigned short)e);};
      void seekop( Operation op)
      {
         switch (op)
         {
            case Tag:                this->match( XMLScannerBase::OpenTag); break;
            case Attribute:          this->match( XMLScannerBase::TagAttribName); break;               
            case ThisAttributeValue: this->match( XMLScannerBase::TagAttribValue); 
                                     this->reject( XMLScannerBase::TagAttribName); 
                                     this->reject( XMLScannerBase::Content); 
                                     this->reject( XMLScannerBase::OpenTag); break;
            case AttributeValue:     this->match( XMLScannerBase::TagAttribValue); 
                                     this->reject( XMLScannerBase::Content); 
                                     break;
            case Content:            this->match( XMLScannerBase::Content); break; 
         }         
      };
      void join( const Mask& mask)                         {pos |= mask.pos; neg |= mask.neg;};
      bool matches( XMLScannerBase::ElementType e) const   {return (0 != (pos & (1<<(unsigned short)e)));};
      bool rejects( XMLScannerBase::ElementType e) const   {return (0 != (neg & (1<<(unsigned short)e)));};
   };

   struct Core
   {
      Mask mask;
      bool follow;
      Hash x;   
      int typeidx;
      int cnt;

      Core()                      :follow(false),x(-1),typeidx(-1),cnt(-1) {};
      Core( const Core& orig)     :mask(orig.mask),follow(orig.follow),x(orig.x),typeidx(orig.typeidx),cnt(orig.cnt) {};
   };

   struct State
   {
      Core core;
      unsigned int keysize;
      char* key;
      int next;
      int link;
            
      State()                        :keysize(0),key(0),next(-1),link(-1) {};
      State( const State& orig)      :core(orig.core),keysize(orig.keysize),key(0),next(orig.next),link(orig.link)
      {
         if (orig.key && orig.keysize)
         {
            key = new (std::nothrow) char[ keysize];
            if (!key) throw( ExceptionOutOfMem());
            for (unsigned int ii=0; ii<keysize; ii++) key[ii]=orig.key[ii];
            key[ keysize] = 0;
         }
      };
      ~State()
      {
         if (key) delete [] key;
      };
      bool isempty()                 {return key==0&&core.typeidx==0;};
      
      void defNext( Operation op, unsigned int p_keysize, const char* p_key, int p_next, bool p_follow=false) 
      {
         if (p_next < 0) throw(ExceptionIllegalParam());
         core.mask.seekop( op);
         if (p_key)
         {
            core.x = hash(p_key,keysize=p_keysize);
            key = new (std::nothrow) char[ keysize];
            if (!key) throw( ExceptionOutOfMem());
            for (unsigned int ii=0; ii<keysize; ii++) key[ii]=p_key[ii];
         }
         next = p_next;
         core.follow = p_follow; 
      };      
      void defOutput( const Mask& mask, int p_typeidx, bool p_follow=false, int p_cnt=-1)
      {
         if (p_typeidx < 0) throw(ExceptionIllegalParam());
         core.mask.join( mask);
         core.typeidx = p_typeidx;
         if (p_cnt >= 0) core.cnt = p_cnt;
         core.follow = p_follow; 
      };
      void defLink( int p_link)
      {
         if (p_link < 0) throw(ExceptionIllegalParam());
         link = p_link;
      };
   };
         
   struct Token
   {
      Core core;
      int stateidx;
      
      Token()                                       :stateidx(-1) {};
      Token( const Token& orig)                     :core(orig.core),stateidx(orig.stateidx) {};
      Token( const State& state, int p_stateidx)    :core(state.core),stateidx(p_stateidx) {};
   };
   
private:      
   int defNext( int stateidx, Operation op, unsigned int keysize, const char* key, bool follow=false)
   {
      try
      {
         State state;
         if (states.size() == 0)
         {
            stateidx = states.size();
            states.push_back( state);
         }
         if ((unsigned int)stateidx >= states.size()) throw ExceptionIllegalParam();

         Hash x = hash( key, keysize);
         for (int ee=stateidx; ee != -1; stateidx=ee,ee=states[ee].link)
         {
            if (states[ee].core.x == x && states[ee].key != 0 && keysize == states[ee].keysize && states[ee].core.follow == follow && memcmp(states[ee].key,key,keysize)==0)
            {
               return states[ee].next;
            };
         };
         if (!states[stateidx].isempty())
         {
            stateidx = states[stateidx].link = states.size();
            states.push_back( state);
         }
         {
            states.push_back( state);
            unsigned int lastidx = states.size()-1;
            states[ stateidx].defNext( op, keysize, key, lastidx, follow); 
            return stateidx=lastidx;
         }
      }
      catch (std::bad_alloc)
      {
         throw( ExceptionOutOfMem());
      }
      catch (...)
      {
         throw( ExceptionUnknown()); 
      };
   };   
   int defOutput( int stateidx, unsigned short pushOpMask, int typeidx, bool follow=false, int cnt=-1)
   {
      try
      {
         State state;
         if (states.size() == 0)
         {
            stateidx = states.size();
            states.push_back( state);
         }
         if ((unsigned int)stateidx >= states.size()) throw ExceptionIllegalParam();
         
         if (!states[stateidx].isempty())
         {
            stateidx = states[stateidx].link = states.size();
            states.push_back( state);
         }
         states[ stateidx].defOutput( pushOpMask, typeidx, follow, cnt);
         return stateidx;
      }
      catch (std::bad_alloc)
      {
         throw( ExceptionOutOfMem());
      }
      catch (...)
      {
         throw( ExceptionUnknown()); 
      };
   };
   
public:
   struct PathElement
   {
   private:
      enum {MaxSize=1024};
      
      XMLPathSelectAutomaton* xs;
      int stateidx;
      int count;
      bool follow;
      Mask pushOpMask; 

   private:
      PathElement& doSelect( Operation op)
      {
         pushOpMask.reset();
         switch (op)
         {
            case Content:              pushOpMask.match( XMLScannerBase::Content); 
                                       break;
            case Tag:                  pushOpMask.match( XMLScannerBase::Content); 
                                       pushOpMask.match( XMLScannerBase::TagAttribName); 
                                       pushOpMask.match( XMLScannerBase::TagAttribValue); 
                                       break;
            case Attribute:            pushOpMask.match( XMLScannerBase::TagAttribName);
            case ThisAttributeValue:   pushOpMask.match( XMLScannerBase::TagAttribValue);
            case AttributeValue:       pushOpMask.match( XMLScannerBase::TagAttribValue);
         };
         return *this;
      }
      PathElement& doSelect( Operation op, const char* value)
      {
         if (xs != 0)
         {
            char buf[ 1024];
            XMLScanner<char*>::size_type size;
            if (!XMLScanner<char*>::getTagName<CharSet_>( value, buf, sizeof(buf), &size)) throw(ExceptionIllegalAttributeName());
            stateidx = xs->defNext( stateidx, op, size, buf, follow);
         }
         return doSelect( op);
      };
      PathElement& doFollow()
      {
         follow = true;
         return *this;  
      };
      PathElement& doCount( int p_count)
      {
         if (count == -1)
         {
            count = p_count;
         }
         else if (p_count < count)
         {
            count = p_count;
         }
         return *this;
      };
      PathElement& push( int typeidx)
      {
         if (xs != 0) stateidx = xs->defOutput( stateidx, pushOpMask, typeidx, follow, count);
         return *this;
      };

   public:
      PathElement()                                                  :xs(0),stateidx(0),count(-1),follow(false),pushOpMask(0) {}; 
      PathElement( XMLPathSelectAutomaton* p_xs, int p_stateidx=0)   :xs(p_xs),stateidx(p_stateidx),count(-1),follow(false),pushOpMask(0) {};
      PathElement( const PathElement& orig)                          :xs(orig.xs),stateidx(orig.stateidx),count(orig.count),follow(orig.follow),pushOpMask(orig.pushOpMask) {};

      //corresponds to "//" in abbreviated syntax of XPath
      PathElement& operator --(int)                                     {return doFollow();};
      //find tag
      PathElement& operator []( const char* name)                       {return doSelect( Tag, name);};
      //find tag with one attribute
      PathElement& operator ()( const char* name)                       {return doSelect( Attribute, name);};
      //find tag with one attribute
      PathElement& operator ()( const char* name, const char* value)    {return doSelect( Attribute, name).doSelect( ThisAttributeValue, value);};
      //define maximum element count to push
      PathElement& operator /(int cnt)                                  {doCount(cnt);};
      //define element type to push
      PathElement& operator =(int type)                                 {return push( type);};
      //grab content
      PathElement& operator ()()                                        {return doSelect(Content);};
   };
   PathElement operator*()
   {      
      return PathElement( this);
   };

   std::vector<State> states;
};


template <
      class InputIterator,                         //< STL conform input iterator with ++ and read only * returning 0 als last character of the input
      class InputCharSet_=charset::UTF8,           //Character set encoding of the input, read as stream of bytes
      class OutputCharSet_=charset::UTF8,          //Character set encoding of the output, printed as string of the item type of the character set
      class EntityMap=std::map<const char*,UChar>  //< STL like map from ASCII const char* to UChar
>
class XMLPathSelect
{
public:
   typedef XMLPathSelectAutomaton<OutputCharSet_> Automaton;
   typedef XMLScanner<InputIterator,InputCharSet_,OutputCharSet_,EntityMap> ThisXMLScanner;
   typedef XMLPathSelect<InputIterator,InputCharSet_,OutputCharSet_,EntityMap> ThisXMLPathSelect;
   
private:
   ThisXMLScanner scan;
   Automaton* atm;
   typedef typename Automaton::Mask Mask;
   typedef typename Automaton::Token Token;
   typedef typename Automaton::Hash Hash;
   typedef typename Automaton::State State;
   
   struct Scope
   {
      Mask mask;
      Mask followMask;
      struct Range
      {
         unsigned int tokenidx_from;
         unsigned int tokenidx_to;
         unsigned int followidx;
         
         Range()                            :tokenidx_from(0),tokenidx_to(0),followidx(0) {};
         Range( const Scope& orig)          :tokenidx_from(orig.tokenidx_from),tokenidx_to(orig.tokenidx_to),followidx(orig.followidx) {};
      };
      Range range;
      
      Scope( const Scope& orig)             :mask(orig.mask),followMask(orig.followMask),range(orig.range) {};
      Scope& operator =( const Scope& orig) {mask=orig.mask; followMask=orig.followMask; range=orig.range; return *this;};
      Scope()                               {};  
   };
      
   std::stack<Scope> scopestk;              //stack of scopes opened
   std::vector<unsigned int> follows;       //indices of tokens active in all descendant scopes
   std::vector<Token> tokens;               //list of waiting tokens
   
   struct Context
   {
      XMLScannerBase::ElementType type;     //element type processed
      const char* key;                      //string value of element processed
      unsigned int keysize;                 //sizeof string value in bytes of element processed
      Hash x;                               //-1 or hash of key[0..keysize-1] if used
      Scope scope;                          //active scope
      unsigned int scope_iter;              //position of currently visited token in the active scope
      
      Context()                                                                         :type(XMLScannerBase::Content),key(0),keysize(0),x(-1) {};
      void init( XMLScannerBase::ElementType p_type, const char* p_key, int p_keysize)  {type=p_type;key=p_key;keysize=p_keysize;x=-1;scope_iter=scope.range.tokenidx_from;};
   };
   Context context;

   void expand( int stateidx)
   {
      context.scope.mask.join( atm->states[ stateidx].core.mask);
      if (atm->states[ stateidx].core.follow) 
      {
         context.scope.followMask.join( atm->states[ stateidx].core.mask);
         follows.push_back( tokens.size());
      }
      tokens.push_back( Token(atm->states[ stateidx], stateidx));
   };

   //declares the currently processed element of the XMLScanner input. By calling fetch we get the output elements from it
   void initProcessElement( XMLScannerBase::ElementType type, const char* key, int keysize)
   {
      if (context.type == XMLScannerBase::OpenTag)
      {
         //last step of open scope has to be done after all tokens were visited,
         //e.g. with the next element initialization
         
         context.scope.range.tokenidx_from = context.scope.range.tokenidx_to;
      }
      context.scope.range.tokenidx_to = tokens.size();
      context.scope.range.followidx = follows.size();
      
      context.init( type, key, keysize);
      
      if (type == XMLScannerBase::OpenTag) 
      {
         //first step of open scope saves the context context on stack
         scopestk.push( context.scope);
         context.scope.mask = context.scope.followMask;
         context.scope.mask.match( XMLScannerBase::OpenTag);
         //... we reset the mask but ensure that this 'OpenTag' is processed for sure
      }
      else if (type == XMLScannerBase::CloseTag || type == XMLScannerBase::CloseTagIm)
      {
         if (!scopestk.empty())
         {
            context.scope = scopestk.top();
            scopestk.pop();
            follows.resize( context.scope.range.followidx);
            tokens.resize( context.scope.range.tokenidx_to);
         }
      };
   };
   
   int match( unsigned int tokenidx)
   {
      if (context.key != 0)
      {
         if (tokenidx >= context.scope.range.tokenidx_to) return 0;

         const Token& tk = tokens[ tokenidx];
         if (tk.core.cnt != 0 && tk.core.mask.matches( context.type))
         {
            if (tk.core.x != -1)
            {
               if (context.x == -1) context.x = Automaton::hash( context.key, context.keysize);

               if (context.x == tk.core.x)
               {
                  const State& st = atm->states[ tk.stateidx];
                  if (st.keysize == context.keysize && st.key != 0 && memcmp( st.key, context.key, context.keysize) == 0)
                  {
                     expand( st.next);
                     if (tk.core.cnt > 0) tokens[ tokenidx].core.cnt--;
                  };
               }
            }
            if (tk.core.typeidx != 0)
            {
               if (tk.core.cnt > 0) tokens[ tokenidx].core.cnt--;
               return tk.core.typeidx;
            }
         }
         if (tk.core.mask.rejects( context.type))
         {
            //The token must not match anymore after encountering a reject item
            tokens[ tokenidx].core.mask.reset();
         }
      }
      return 0;
   };
   
   int fetch()
   {
      int type = 0;

      if (context.scope.mask.matches( context.type))
      {
         while (!type)
         {      
            if (context.scope_iter < context.scope.range.tokenidx_to)
            {
               type = match( context.scope_iter++);
            }
            else 
            {
               unsigned int ii = context.scope_iter - context.scope.range.tokenidx_to;
               if (ii < context.scope.range.followidx)
               {
                  type = match( follows[ ii]);
               }
               else
               {
                  context.key = 0;
                  return 0; //end of all candidates
               }
            }
         }
      }
      return type;
   };

public:
   XMLPathSelect( Automaton* p_atm, InputIterator& src, char* outputBuf, unsigned int outputBufSize, EntityMap* entityMap=0)  :scan(src,outputBuf,outputBufSize,entityMap),atm(p_atm) {};
   XMLPathSelect( const XMLPathSelect& o)                                                                                     :scan(o.scan),atm(o.atm) {};

   //STL conform input iterator for the output of this XMLScanner:   
   struct End {};
   class iterator
   {
   public:
      struct Element
      {
         bool error;
         bool eof;
         int type;
         const char* content;
         unsigned int size;
         
         Element()                     :error(0),eof(false),type(0),content(0),size(0) {};
         Element( const End&)          :error(0),eof( true),type(0),content(0),size(0) {};
      };
      typedef Element value_type;
      typedef unsigned int difference_type;
      typedef Element* pointer;
      typedef Element& reference;
      typedef std::input_iterator_tag iterator_category;

   private:
      Element element;
      ThisXMLPathSelect* input;

      void skip()
      {
         try
         {
            if (input != 0)
            {
               do
               {
                  if (!input->context.key)
                  {
                     XMLScannerBase::ElementType et = input->scan.nextItem( input->context.scope.mask.pos);
                     if (et == XMLScannerBase::Exit)
                     {
                        element.eof = true;
                        return;
                     }
                     if (et == XMLScannerBase::ErrorOccurred)
                     {
                        element.error = true;
                        (void)input->scan.getError( &element.content);
                        return;
                     }
                     input->initProcessElement( et, input->scan.getItem(), input->scan.getItemSize());
                  }
                  element.type = input->fetch();

               } while (element.type == 0);

               element.content = input->context.key;
               element.size = input->context.keysize;
            }
         }
         catch (std::bad_alloc)
         {
            throw (ExceptionOutOfMem());
         }
         catch (...)
         {
            throw( ExceptionUnknown()); 
         };
      };
      bool compare( const iterator& iter) const
      {
         return (element.eof && iter.element.eof);
      };
   public:
      void assign( const iterator& orig)
      {
         input = orig.input;
         element.eof = orig.element.eof;
         element.type = orig.element.type;
         element.content = orig.element.content;
         element.size = orig.element.size;
      };
      iterator( const iterator& orig)
      {
         assign( orig);
      };
      iterator( ThisXMLPathSelect& p_input)
             :input( &p_input)
      {
         skip();
      };
      iterator( const End& et)  :element(et),input(0) {};
      iterator()  :input(0) {};
      iterator& operator = (const iterator& orig)
      {
         assign( orig);
         return *this;
      };    
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
      return iterator( End());
   };
};

} //namespace textwolf
#endif 

