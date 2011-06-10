/**
---------------------------------------------------------------------
    The template library textwolf implements an input iterator on
    a set of XML path expressions without backward references on an
    STL conforming input iterator as source. It does no buffering
    or read ahead and is dedicated for stream processing of XML
    for a small set of XML queries.
    Stream processing in this context refers to processing the
    document without buffering anything but the current result token
    processed with its tag hierarchy information.

    Copyright (C) 2010 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of textwolf can be found at 'http://github.com/patrickfrey/textwolf'
	For documentation see 'http://patrickfrey.github.com/textwolf'

--------------------------------------------------------------------
**/

#ifndef __TEXTWOLF_HPP__
#define __TEXTWOLF_HPP__
#include <iterator>
#include <vector>
#include <stack>
#include <map>
#include <exception>
#include <iostream>
#include <limits>
#include <boost/cstdint.hpp>

///\namespace textwolf
///\brief Toplevel namespace of the library
namespace textwolf {

///\defgroup Exceptions
///\brief Exception classes and structures for error handling in the initialization phase

///\class throws_exception
///\brief Base class for structures that can throw exceptions for non recoverable errors in the automata definition
struct throws_exception
{
	///\enum Cause
	///\brief Enumeration of error cases
	enum Cause
	{
		Unknown,							///< uknown error
		DimOutOfRange,					///< memory reserved for statically allocated table or memory block is too small. Increase the size of memory block passed to the XML path select automaton. Usage error !
		StateNumbersNotAscending,	///< XML scanner automaton definition check failed. Labels of states must be equal to their indices. Internal textwold error !
		InvalidParam,					///< parameter check in automaton definition failed. Internal textwold error !
		InvalidState,					///< invalied state definition in automaton. Internal textwold error !
		IllegalParam,					///< parameter check in automaton definition failed. Internal textwold error !
		IllegalAttributeName,		///< invalid string for a tag or attribute in the automaton definition. Usage error !
		OutOfMem,						///< out of memory in the automaton definition. System error (std::bad_alloc) !
		ArrayBoundsReadWrite,		///< invalid array access. Internal textwold error !
		NotAllowedOperation			///< defining an operation in an automaton definition that is not allowed there. Usage error !
	};
};

///\class exception
///\brief textwolf exception class
///\remark textwolf is after the initialization phase of the automata exception free. Classes throwing exception in the initilazation phase are derived from 'textwolf::throws_exception'
struct exception	:public std::exception
{
	typedef throws_exception::Cause Cause;
	Cause cause;										///< exception cause tag

	///\brief Constructor
	///\param[in] p_cause exception cause tag
	exception (Cause p_cause) throw()
			:cause(p_cause) {}
	///\brief Copy constructor
	///\param[in] orig exception to copy
	exception (const exception& orig) throw()
			:cause(orig.cause) {}
	///\brief Destructor
	virtual ~exception() throw() {}

	///\brief Assignement
	///\param[in] orig exception to copy
	///\return *this
	exception& operator= (const exception& orig) throw()
			{cause=orig.cause; return *this;}

	///\brief Exception message
	///\return exception cause as string
	virtual const char* what() const throw()
	{
		// enumeration of exception causes as strings
		static const char* nameCause[ 10] = {
			"Unknown","DimOutOfRange","StateNumbersNotAscending","InvalidParam",
			"InvalidState","IllegalParam","IllegalAttributeName","OutOfMem",
			"ArrayBoundsReadWrite","NotAllowedOperation"
		};
		return nameCause[ (unsigned int) cause];
	}
};

///\defgroup Charactersets
///\brief Character set encodings and character parsing tables

///\class CharMap
///\brief Character map for fast typing of a character byte
///\tparam RESTYPE result type of the map
///\tparam nullvalue_ default intitialization value of the map
///\tparam RANGE domain of the input values of the map
template <typename RESTYPE, RESTYPE nullvalue_, int RANGE=256>
class CharMap
{
public:
	typedef RESTYPE valuetype;
	enum Constant {nullvalue=nullvalue_};

private:
	RESTYPE ar[ RANGE];						///< the map elements
public:
	///\brief Constructor
	CharMap()									{for (unsigned int ii=0; ii<RANGE; ii++) ar[ii]=(valuetype)nullvalue;}
	///\brief Define the values of the elements in the interval [from,to]
	///\param[in] from start of the input intervall (belongs also to the input)
	///\param[in] to end of the input intervall (belongs also to the input)
	///\param[in] value value assigned to all elements in  [from,to]
	CharMap& operator()( unsigned char from, unsigned char to, valuetype value)	{for (unsigned int ii=from; ii<=to; ii++) ar[ii]=value; return *this;}
	///\brief Define the values of the single element at 'at'
	///\param[in] at the input element
	///\param[in] value value assigned to the element 'at'
	CharMap& operator()( unsigned char at, valuetype value)				{ar[at] = value; return *this;}
	///\brief Read the element assigned to 'ii'
	///\param[in] ii the input element queried
	///\return the element at 'ii'
	valuetype operator []( unsigned char ii) const					{return ar[ii];}
};

///\typedef UChar
///\brief Unicode character type
typedef boost::uint32_t UChar;

///\namespace charset
///\brief Predefined character set encodings
///
/// Predefined character set definitions:
/// 1) Iso-Latin-1
/// 2) UCS2  (little and big endian, not very efficient implementation)
/// 3) UCS4  (little and big endian, not very efficient implementation)
/// 4) UTF-8 (see http://de.wikipedia.org/wiki/UTF-8 for algorithms)
///
namespace charset {

///\class IsoLatin1
///\brief Character set IsoLatin-1
struct IsoLatin1
{
	enum {HeadSize=1,Size=1,MaxChar=0xFF};

	static unsigned int asize()							{return HeadSize;}
	static unsigned int size( const char*)						{return Size;}
	static char achar( const char* buf)						{return buf[0];}
	static UChar value( const char* buf)						{return (unsigned char)buf[0];}
	static unsigned int print( UChar chr, char* buf, unsigned int bufsize)		{if (bufsize < 1) return 0; buf[0] = (chr <= 255)?(char)(unsigned char)chr:-1; return 1;}
};

///\class ByteOrder
///\brief Order of bytes for wide char character sets
struct ByteOrder
{
	enum
	{
		LE=1,		///< little endian
		BE=2		///< big endian
	};
};

///\class UCS2
///\brief Character set UCS-2 (little/big endian)
///\tparam encoding ByteOrder::LE or ByteOrder::BE
template <int encoding>
struct UCS2
{
	enum {LSB=(encoding==ByteOrder::BE),MSB=(encoding==ByteOrder::LE),HeadSize=2,Size=2,MaxChar=0xFFFF};

	static unsigned int asize()							{return HeadSize;}
	static unsigned int size( const char*)						{return Size;}
	static char achar( const char* buf)						{return (buf[MSB])?(char)-1:buf[LSB];}
	static UChar value( const char* buf)						{UChar res = (unsigned char)buf[MSB]; return (res << 8) + (unsigned char)buf[LSB];}
	static unsigned int print( UChar chr, char* buf, unsigned int bufsize)		{if (bufsize<2) return 0; if (chr>0xFFFF) {buf[0]=(char)0xFF; buf[1]=(char)0xFF;} else {buf[LSB]=(char)chr; buf[MSB]=(char)(chr>>8);} return 2;}
};

///\class UCS4
///\brief Character set UCS-4 (little/big endian)
///\tparam encoding ByteOrder::LE or ByteOrder::BE
template <int encoding>
struct UCS4
{
	enum {B0=(encoding==ByteOrder::BE)?3:0,B1=(encoding==ByteOrder::BE)?2:1,B2=(encoding==ByteOrder::BE)?1:2,B3=(encoding==ByteOrder::BE)?0:3,HeadSize=4,Size=4,MaxChar=0xFFFFFFFF};

	static unsigned int asize()							{return HeadSize;}
	static unsigned int size( const char*)						{return Size;}
	static char achar( const char* buf)						{return (buf[B3]|buf[B2]|buf[B1])?(char)-1:buf[B0];}
	static UChar value( const char* buf)						{UChar res = (unsigned char)buf[B3]; res = (res << 8) + (unsigned char)buf[B2]; res = (res << 8) + (unsigned char)buf[B1]; return (res << 8) + (unsigned char)buf[B0];}
	static unsigned int print( UChar chr, char* buf, unsigned int bufsize)		{if (bufsize<4) return 0; buf[B0]=(char)chr; chr>>=8; buf[B1]=(char)chr; chr>>=8; buf[B2]=(char)chr; chr>>=8; buf[B3]=(char)chr; chr>>=8; return 4;}
};

struct UCS2LE :public UCS2<ByteOrder::LE> {};
struct UCS2BE :public UCS2<ByteOrder::BE> {};
struct UCS4LE :public UCS4<ByteOrder::LE> {};
struct UCS4BE :public UCS4<ByteOrder::BE> {};

///\class UTF8
///\brief character set encoding UTF-8
struct UTF8
{
	enum {MaxChar=0xFFFFFFFF};
	enum {
		B11111111=0xFF,
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

	struct CharLengthTab	:public CharMap<unsigned char, 0>
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

	static unsigned int asize()			{return HeadSize;}
	static char achar( const char* buf)		{return buf[0];}
	static unsigned int size( const char* buf)	{static CharLengthTab charLengthTab; return charLengthTab[ (unsigned char)buf[ 0]];}

	static UChar value( const char* buf)
	{
		const UChar invalid = std::numeric_limits<UChar>::max();
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
			buf[0] = (char)(unsigned char)chr;
			return 1;
		}
		unsigned int pp,sf;
		for (pp=1,sf=5; pp<5; pp++,sf+=5)
		{
			if (chr < (unsigned int)((1<<6)<<sf))
			{
				rt = pp+1;
				while (pp > 0)
				{
					buf[pp--] = (char)(unsigned char)((chr & B00111111) | B10000000);
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
			buf[pp--] = (char)(unsigned char)((chr & B00111111) | B10000000);
			chr >>= 6;
		}
		unsigned char HB = (unsigned char)(B11111111 << (8-rt));
		buf[0] = (char)(((unsigned char)chr & (~HB >> 1)) | HB);
		return rt;
	};
};
}//namespace charset

///\enum ControlCharacter
///\brief Enumeration of control characters needed as events for XML scanner statemachine
enum ControlCharacter
{
	Undef=0,						///< not defined (beyond ascii)
	EndOfText,					///< end of data (EOF,EOD,.)
	EndOfLine,					///< end of line
	Cntrl,						///< control character
	Space,						///< space, tab, etc..
	Amp,							///< ampersant ('&')
	Lt,							///< lesser than '<'
	Equal,						///< equal '='
	Gt,							///< greater than '>'
	Slash,						///< slash '/'
	Exclam,						///< exclamation mark '!'
	Questm,						///< question mark '?'
	Sq,							///< single quote
	Dq,							///< double quote
	Osb,							///< open square bracket '['
	Csb,							///< close square bracket ']'
	Any,							///< any ascii character with meaning
	NofControlCharacter=17	///< total number of control characters
};

///\class ControlCharacterM
///\brief Map of the enumeration of control characters to their names for debug messages
struct ControlCharacterM
{
	///\brief Get the name of a control character as string
	///\param [in] c the control character to map
	static const char* name( ControlCharacter c)
	{
		static const char* name[ NofControlCharacter] = {"Undef", "EndOfText", "EndOfLine", "Cntrl", "Space", "Amp", "Lt", "Equal", "Gt", "Slash", "Exclam", "Questm", "Sq", "Dq", "Osb", "Csb", "Any"};
		return name[ (unsigned int)c];
	}
};

///\defgroup Textscanner
///\brief Preliminary scanning of the input providing a unified view on the input character stream

///\class TextScanner
///\brief Reader for scanning the input character by character
///\tparam Iterator source iterator type (implements preincrement and '*' input byte access indirection)
///\tparam CharSet character set of the source stream
template <class Iterator, class CharSet>
class TextScanner
{
private:
	Iterator input;				///< source iterator
	char buf[8];					///< buffer for one character (the current character parsed)
	UChar val;						///< Unicode character representation of the current character parsed
	char cur;						///< ASCII character representation of the current character parsed
	unsigned int state;			///< current state of the text scanner

public:
	///\class ControlCharMap
	///\brief Map of ASCII characters to control character identifiers used in the XML scanner automaton
	struct ControlCharMap  :public CharMap<ControlCharacter,Undef>
	{
		ControlCharMap()
		{
			(*this)
			(0,EndOfText)
			(1,31,Cntrl)
			(5,Undef)
			(33,127,Any)
			(128,255,Undef)
			('\t',Space)
			('\r',EndOfLine)
			('\n',EndOfLine)
			(' ',Space)
			('&',Amp)
			('<',Lt)
			('=',Equal)
			('>',Gt)
			('/',Slash)
			('!',Exclam)
			('?',Questm)
			('\'',Sq)
			('\"',Dq)
			('[',Osb)
			(']',Csb);
		};
	};

	///\brief Constructor
	///\param [in] p_iterator source iterator
	TextScanner( const Iterator& p_iterator)
			:input(p_iterator),val(0),cur(0),state(0)
	{
		for (unsigned int ii=0; ii<sizeof(buf); ii++) buf[ii] = 0;
	}
	///\brief Copy constructor
	///\param [in] orig textscanner to copy
	TextScanner( const TextScanner& orig)
			:val(orig.val),cur(orig.cur),state(orig.state)
	{
		for (unsigned int ii=0; ii<sizeof(buf); ii++) buf[ii]=orig.buf[ii];
	}

	///\brief Get the unicode character of the current character
	///\return the unicode character
	UChar chr()
	{
		if (val == 0)
		{
			while (state < CharSet::size(buf))
			{
				buf[state] = *input;
				++input;
				++state;
			}
			val = CharSet::value(buf);
		}
		return val;
	}

	///\brief Fill the internal buffer with as many current character bytes needed for reading the ASCII representation
	void getcur()
	{
		while (state < CharSet::asize())
		{
			buf[state] = *input;
			++input;
			++state;
		}
		cur = CharSet::achar(buf);
	}

	///\brief Get the control character representation of the current character 
	///\return the control character
	ControlCharacter control()
	{
		static ControlCharMap controlCharMap;
		getcur();
		return controlCharMap[ (unsigned char)cur];
	}

	///\brief Get the ASCII character representation of the current character 
	///\return the ASCII character
	char ascii()
	{
		getcur();
		return cur>=0?cur:0;
	}

	///\brief Skip to the next character of the source
	///\return *this
	TextScanner& skip()
	{
		while (state < CharSet::asize())
		{
			++input;
			++state;
		}
		state = 0;
		cur = 0;
		val = 0;
		return *this;
	}

	///\brief Preincrement: Skip to the next character of the source
	///\return *this
	TextScanner& operator ++()	{return skip();}

	///\brief Postincrement: Skip to the next character of the source
	///\return *this
	TextScanner operator ++(int)	{TextScanner tmp(*this); skip(); return tmp;}
};


///\defgroup XMLscanner
///\brief Structures for iterating on the XML elements

///\class ScannerStatemachine
///\brief Class to build up the XML element scanner state machine in a descriptive way
class ScannerStatemachine :public throws_exception
{
public:
	enum
	{
		MaxNofStates=64			///< maximum number of states (fixed allocated array for state machine)
	};
	///\class Element
	///\brief One state in the state machine
	struct Element
	{
		int fallbackState;						///< state transition if the event does not match (it belongs to the next state = fallbackState)
		int missError;								///< error code in case of an event that does not match and there is no fallback

		///\class Action
		///\brief Definition of action fired by the state machine
		struct Action
		{
			int op;									///< action operand
			int arg;									///< action argument
		};
		Action action;								///< action executed after entering this state
		char next[ NofControlCharacter];		///< follow state fired by an event (control character type parsed)

		///\brief Constructor
		Element() :fallbackState(-1),missError(-1)
		{
			action.op = -1;
			action.arg = 0;
			for (unsigned int ii=0; ii<NofControlCharacter; ii++) next[ii] = -1;
		}
	};
	///\brief Get state addressed by its index
	///\param [in] stateIdx index of the state
	///\return state defintion reference
	Element* get( int stateIdx) const throw(exception)
	{
		if ((unsigned int)stateIdx>size) throw exception(InvalidState);
		return tab + stateIdx;
	}

private:
	Element tab[ MaxNofStates];			///< states of the STM
	unsigned int size;						///< number of states defined in the STM

	///\brief Create a new state
	///\param [in] stateIdx index of the state (must be the size of the STM array, so that state identifiers can be named by enumeration constants for better readability)
	void newState( int stateIdx) throw(exception)
	{
		if (size != (unsigned int)stateIdx) throw exception( StateNumbersNotAscending);
		if (size >= MaxNofStates) throw exception( DimOutOfRange);
		size++;
	}

	///\brief Define a transition for all control character types not firing yet in the last state defined
	///\param [in] nextState the follow state index defined for these transitions
	void addOtherTransition( int nextState) throw(exception)
	{
		if (size == 0) throw exception( InvalidState);
		if (nextState < 0 || nextState > MaxNofStates) throw exception( InvalidParam);
		for (unsigned int inputchr=0; inputchr<NofControlCharacter; inputchr++)
		{
			if (tab[ size-1].next[ inputchr] == -1) tab[ size-1].next[ inputchr] = (unsigned char)nextState;
		}
	}

	///\brief Define a transition for inputchr in the last state defined
	///\param [in] inputchr the firing input control character type
	///\param [in] nextState the follow state index defined for this transition
	void addTransition( ControlCharacter inputchr, int nextState) throw(exception)
	{
		if (size == 0) throw exception( InvalidState);
		if ((unsigned int)inputchr >= (unsigned int)NofControlCharacter)  throw exception( InvalidParam);
		if (nextState < 0 || nextState > MaxNofStates)  throw exception( InvalidParam);
		if (tab[ size-1].next[ inputchr] != -1)  throw exception( InvalidParam);
		if (size == 0)  throw exception( InvalidState);
		tab[ size-1].next[ inputchr] = (unsigned char)nextState;
	}

	///\brief Define a self directing transition for inputchr in the last state defined (the state remains the same for this input)
	///\param [in] inputchr the firing input control character type
	void addTransition( ControlCharacter inputchr) throw(exception)
	{
		addTransition( inputchr, size-1);
	}

	///\brief Define an action in the last state defined (to be executed when entering the state)
	///\param [in] action_op action operand
	///\param [in] action_arg action argument
	void addAction( int action_op, int action_arg=0) throw(exception)
	{
		if (size == 0) throw exception( InvalidState);
		if (tab[ size-1].action.op != -1) throw exception( InvalidState);
		tab[ size-1].action.op = action_op;
		tab[ size-1].action.arg = action_arg;
	}

	///\brief Define an error in the last state defined to be reported when no fallback is defined and no firing input character parsed
	///\param [in] error code to be reported
	void addMiss( int error) throw(exception)
	{
		if (size == 0) throw exception( InvalidState);
		if (tab[ size-1].missError != -1) throw exception( InvalidState);
		tab[ size-1].missError = error;
	}

	///\brief Define in the last state defined a fallback state transition that is fired when no firing input character parsed
	///\param [in] stateIdx follow state index
	void addFallback( int stateIdx) throw(exception)
	{
		if (size == 0) throw exception( InvalidState);
		if (tab[ size-1].fallbackState != -1) throw exception( InvalidState);
		if (stateIdx < 0 || stateIdx > MaxNofStates) throw exception( InvalidParam);
		tab[ size-1].fallbackState = stateIdx;
	}
public:
	///\brief Constructor
	ScannerStatemachine() :size(0){}

	///\brief See ScannerStatemachine::newState(int)
	ScannerStatemachine& operator[]( int stateIdx)									{newState(stateIdx); return *this;}
	///\brief See ScannerStatemachine::addTransition(ControlCharacter,int)
	ScannerStatemachine& operator()( ControlCharacter inputchr, int ns)						{addTransition(inputchr,ns); return *this;}
	///\brief See ScannerStatemachine::addTransition(ControlCharacter,int)
	ScannerStatemachine& operator()( ControlCharacter i1, ControlCharacter i2, int ns)				{addTransition(i1,ns); addTransition(i2,ns); return *this;}
	///\brief See ScannerStatemachine::addTransition(ControlCharacter,int)
	ScannerStatemachine& operator()( ControlCharacter i1, ControlCharacter i2, ControlCharacter i3, int ns)		{addTransition(i1,ns); addTransition(i2,ns); addTransition(i3,ns); return *this;}
	///\brief See ScannerStatemachine::addTransition(ControlCharacter)
	ScannerStatemachine& operator()( ControlCharacter inputchr)							{addTransition(inputchr); return *this;}
	///\brief See ScannerStatemachine::addAction(int,int)
	ScannerStatemachine& action( int aa, int arg=0)									{addAction(aa,arg); return *this;}
	///\brief See ScannerStatemachine::addMiss(int)
	ScannerStatemachine& miss( int ee)										{addMiss(ee); return *this;}
	///\brief See ScannerStatemachine::addFallback(int)
	ScannerStatemachine& fallback( int stateIdx)									{addFallback(stateIdx); return *this;}
	///\brief See ScannerStatemachine::addOtherTransition(int)
	ScannerStatemachine& other( int stateIdx)									{addOtherTransition(stateIdx); return *this;}
};

///\class XMLScannerBase
///\brief XML scanner base class for things common for all XML scanners
class XMLScannerBase
{
public:
	///\enum ElementType
	///\brief Enumeration of XML element types returned by an XML scanner
	enum ElementType
	{
		None,								///< empty (NULL)
		ErrorOccurred,					///< XML scanning error error reported
		HeaderAttribName,				///< tag attribute name in the XML header
		HeaderAttribValue,			///< tag attribute value in the XML header
		HeaderEnd,						///< end of XML header event (after parsing '?>')
		TagAttribName,					///< tag attribute name (e.g. "id" in <person id='5'>
		TagAttribValue,				///< tag attribute value (e.g. "5" in <person id='5'>
		OpenTag,							///< open tag (e.g. "bla" for "<bla...")
		CloseTag,						///< close tag (e.g. "bla" for "</bla>")
		CloseTagIm,						///< immediate close tag (e.g. "bla" for "<bla />")
		Content,							///< content element string (separated by spaces or end of line)
		Exit								///< end of document
	};
	enum
	{
		NofElementTypes=Exit+1		///< number of XML element types defined
	};

	///\brief Get the XML element type as string
	///\param [in] XML element type
	///\return XML element type as string
	static const char* getElementTypeName( ElementType ee)
	{
		static const char* names[ NofElementTypes] = {0,"ErrorOccurred","HeaderAttribName","HeaderAttribValue","HeaderEnd","TagAttribName","TagAttribValue","OpenTag","CloseTag","CloseTagIm","Content","Exit"};
		return names[ (unsigned int)ee];
	}

	///\enum Error
	///\brief Enumeration of XML scanner error codes
	enum Error
	{
		Ok,											///< no error, everything is OK
		ErrExpectedOpenTag,						///< expected an open tag in this state
		ErrExpectedXMLTag,						///< expected an <?xml tag in this state
		ErrUnexpectedEndOfText,					///< unexpected end of text in the middle of the XML definition
		ErrOutputBufferTooSmall,				///< scaned element in XML to big to fit in the buffer provided for it
		ErrSyntaxToken,							///< a specific string expected as token in XML but does not match
		ErrStringNotTerminated,					///< single or double quoted string in XML not terminated on the same line
		ErrEntityEncodesCntrlChar,				///< control character < 32 encoded as entity. This is rejected
		ErrUndefinedCharacterEntity,			///< symbolic character entity is not defined in the entity map defined by the XML scanner caller
		ErrExpectedTagEnd,						///< expected end of tag
		ErrExpectedEqual,							///< expected equal in tag attribute definition
		ErrExpectedTagAttribute,				///< expected tag attribute
		ErrExpectedCDATATag,						///< expected CDATA tag definition
		ErrInternal,								///< internal error (textwolf implementation error)
		ErrUnexpectedEndOfInput					///< unexpected end of input stream
	};

	///\brief Get the error code as string
	///\param [in] ee error code
	///\return the error code as string
	static const char* getErrorString( Error ee)
	{
		enum {NofErrors=15};
		static const char* sError[NofErrors]
			= {0,"ExpectedOpenTag", "ExpectedXMLTag","UnexpectedEndOfText",
				"OutputBufferTooSmall","SyntaxToken","StringNotTerminated",
				"EntityEncodesCntrlChar","UndefinedCharacterEntity","ExpectedTagEnd",
				"ExpectedEqual", "ExpectedTagAttribute","ExpectedCDATATag","Internal",
				"UnexpectedEndOfInput"
		};
		return sError[(unsigned int)ee];
	}

	///\enum STMState
	///\brief Enumeration of states of the XML scanner state machine
	enum STMState
	{
		START, STARTTAG, XTAG, XTAGEND, XTAGAISK, XTAGANAM, XTAGAESK, XTAGAVSK, XTAGAVID, XTAGAVSQ, XTAGAVDQ, XTAGAVQE, CONTENT,
		TOKEN, XMLTAG, OPENTAG, CLOSETAG, TAGCLSK, TAGAISK, TAGANAM, TAGAESK, TAGAVSK, TAGAVID, TAGAVSQ, TAGAVDQ, TAGAVQE,
		TAGCLIM, ENTITYSL, ENTITY, CDATA, CDATA1, CDATA2, CDATA3, EXIT
	};

	///\brief Get the scanner state machine state as string
	///\param [in] s the state
	///\return the state as string
	static const char* getStateString( STMState s)
	{
		enum Constant {NofStates=34};
		static const char* sState[NofStates]
		= {
			"START", "STARTTAG", "XTAG", "XTAGEND", "XTAGAISK", "XTAGANAM", "XTAGAESK", "XTAGAVSK", "XTAGAVID", "XTAGAVSQ", "XTAGAVDQ", "XTAGAVQE", "CONTENT",
			"TOKEN", "XMLTAG", "OPENTAG", "CLOSETAG", "TAGCLSK", "TAGAISK", "TAGANAM", "TAGAESK", "TAGAVSK", "TAGAVID", "TAGAVSQ", "TAGAVDQ", "TAGAVQE",
			"TAGCLIM", "ENTITYSL", "ENTITY", "CDATA", "CDATA1", "CDATA2", "CDATA3", "EXIT"
		};
		return sState[(unsigned int)s];
	}

	///\enum STMAction
	///\brief Enumeration of actions in the XML scanner state machine
	enum STMAction
	{
		Return, ReturnToken, ReturnIdentifier, ReturnSQString, ReturnDQString, ExpectIdentifierXML, ExpectIdentifierCDATA, ReturnEOF,
		NofSTMActions = 8
	};

	///\brief Get the scanner state machine action as string
	///\param [in] a the action
	///\return the action as string
	static const char* getActionString( STMAction a)
	{
		static const char* name[ NofSTMActions] = {"Return", "ReturnToken", "ReturnIdentifier", "ReturnSQString", "ReturnDQString", "ExpectIdentifierXML", "ExpectIdentifierCDATA", "ReturnEOF"};
		return name[ (unsigned int)a];
	};

	///\class Statemachine
	///\brief XML scanner state machine implementation
	struct Statemachine :public ScannerStatemachine
	{
		///\brief Constructor (defines the state machine completely)
		Statemachine()
		{
			(*this)
			[ START    ](EndOfLine)(Cntrl)(Space)(Lt,STARTTAG).miss(ErrExpectedOpenTag)
			[ STARTTAG ](EndOfLine)(Cntrl)(Space)(Questm,XTAG )(Exclam,ENTITYSL).fallback(OPENTAG)
			[ XTAG     ].action(ExpectIdentifierXML)(EndOfLine,Cntrl,Space,XTAGAISK)(Questm,XTAGEND).miss(ErrExpectedXMLTag)
			[ XTAGEND  ].action(Return,HeaderEnd)(Gt,CONTENT)(EndOfLine)(Cntrl)(Space).miss(ErrExpectedTagEnd)
			[ XTAGAISK ](EndOfLine)(Cntrl)(Space)(Questm,XTAGEND).fallback(XTAGANAM)
			[ XTAGANAM ].action(ReturnIdentifier,HeaderAttribName)(EndOfLine,Cntrl,Space,XTAGAESK)(Equal,XTAGAVSK).miss(ErrExpectedEqual)
			[ XTAGAESK ](EndOfLine)(Cntrl)(Space)(Equal,XTAGAVSK).miss(ErrExpectedEqual)
			[ XTAGAVSK ](EndOfLine)(Cntrl)(Space)(Sq,XTAGAVSQ)(Dq,XTAGAVDQ).fallback(XTAGAVID)
			[ XTAGAVID ].action(ReturnIdentifier,HeaderAttribValue)(EndOfLine,Cntrl,Space,XTAGAISK)(Questm,XTAGEND).miss(ErrExpectedTagAttribute)
			[ XTAGAVSQ ].action(ReturnSQString,HeaderAttribValue)(Sq,XTAGAVQE).miss(ErrStringNotTerminated)
			[ XTAGAVDQ ].action(ReturnDQString,HeaderAttribValue)(Dq,XTAGAVQE).miss(ErrStringNotTerminated)
			[ XTAGAVQE ](EndOfLine,Cntrl,Space,XTAGAISK)(Questm,XTAGEND).miss(ErrExpectedTagAttribute)
			[ CONTENT  ](EndOfText,EXIT)(EndOfLine)(Cntrl)(Space)(Lt,XMLTAG).fallback(TOKEN)
			[ TOKEN    ].action(ReturnToken,Content)(EndOfText,EXIT)(EndOfLine,Cntrl,Space,CONTENT)(Lt,XMLTAG).fallback(CONTENT)
			[ XMLTAG   ](EndOfLine)(Cntrl)(Space)(Questm,XTAG)(Slash,CLOSETAG).fallback(OPENTAG)
			[ OPENTAG  ].action(ReturnIdentifier,OpenTag)(EndOfLine,Cntrl,Space,TAGAISK)(Slash,TAGCLIM)(Gt,CONTENT).miss(ErrExpectedTagAttribute)
			[ CLOSETAG ].action(ReturnIdentifier,CloseTag)(EndOfLine,Cntrl,Space,TAGCLSK)(Gt,CONTENT).miss(ErrExpectedTagEnd)
			[ TAGCLSK  ](EndOfLine)(Cntrl)(Space)(Gt,CONTENT).miss(ErrExpectedTagEnd)
			[ TAGAISK  ](EndOfLine)(Cntrl)(Space)(Gt,CONTENT)(Slash,TAGCLIM).fallback(TAGANAM)
			[ TAGANAM  ].action(ReturnIdentifier,TagAttribName)(EndOfLine,Cntrl,Space,TAGAESK)(Equal,TAGAVSK).miss(ErrExpectedEqual)
			[ TAGAESK  ](EndOfLine)(Cntrl)(Space)(Equal,TAGAVSK).miss(ErrExpectedEqual)
			[ TAGAVSK  ](EndOfLine)(Cntrl)(Space)(Sq,TAGAVSQ)(Dq,TAGAVDQ).fallback(TAGAVID)
			[ TAGAVID  ].action(ReturnIdentifier,TagAttribValue)(EndOfLine,Cntrl,Space,TAGAISK)(Slash,TAGCLIM)(Gt,CONTENT).miss(ErrExpectedTagAttribute)
			[ TAGAVSQ  ].action(ReturnSQString,TagAttribValue)(Sq,TAGAVQE).miss(ErrStringNotTerminated)
			[ TAGAVDQ  ].action(ReturnDQString,TagAttribValue)(Dq,TAGAVQE).miss(ErrStringNotTerminated)
			[ TAGAVQE  ](EndOfLine,Cntrl,Space,TAGAISK)(Slash,TAGCLIM)(Gt,CONTENT).miss(ErrExpectedTagAttribute)
			[ TAGCLIM  ].action(Return,CloseTagIm)(EndOfLine)(Cntrl)(Space)(Gt,CONTENT).miss(ErrExpectedTagEnd)
			[ ENTITYSL ](Osb,CDATA).fallback(ENTITY)
			[ ENTITY   ](Exclam,TAGCLSK).other( ENTITY)
			[ CDATA    ].action(ExpectIdentifierCDATA)(Osb,CDATA1).miss(ErrExpectedCDATATag)
			[ CDATA1   ](Csb,CDATA2).other(CDATA1)
			[ CDATA2   ](Csb,CDATA3).other(CDATA1)
			[ CDATA3   ](Gt,CONTENT).other(CDATA1)
			[ EXIT     ].action(Return,Exit);
		}
	};
};


///\class XMLScanner
///\brief XML scanner template that adds the functionality to the statemachine base definition
///\tparam InputIterator input iterator with ++ and read only * returning 0 als last character of the input
///\tparam InputCharSet_ character set encoding of the input, read as stream of bytes
///\tparam OutputCharSet_ character set encoding of the output, printed as string of the item type of the character set
///\tparam EntityMap_ STL like map from ASCII const char* to UChar
template
<
		class InputIterator,
		class InputCharSet_=charset::UTF8,
		class OutputCharSet_=charset::UTF8,
		class EntityMap_=std::map<const char*,UChar>
>
class XMLScanner :public XMLScannerBase
{
private:
	///\class TokState
	///\brief Token state variables
	struct TokState
	{
		///\enum Id
		///\brief Enumeration of token parser states.
		///\remark These states define where the scanner has to continue parsing when it was interrupted by an EoD exception and reentered again with more input to process.
		enum Id
		{
			Start,							///< start state (no parsing action performed at the moment)
			ParsingKey,						///< scanner was interrupted when parsing a key
			ParsingEntity,					///< scanner was interrupted when parsing an XML character entity
			ParsingNumericEntity,		///< scanner was interrupted when parsing an XML numeric character entity
			ParsingNumericBaseEntity,	///< scanner was interrupted when parsing an XML basic character entity (apos,amp,etc..)
			ParsingNamedEntity,			///< scanner was interrupted when parsing an XML named character entity
			ParsingToken					///< scanner was interrupted when parsing a token (not in entity cotext)
		};
		Id id;								///< the scanner token parser state
		unsigned int pos;					///< entity buffer position (buf)
		unsigned int base;				///< numeric entity base (10 for decimal/16 for hexadecimal)
		unsigned long long value;		///< parsed entity value
		char buf[ 16];						///< parsed entity buffer
		UChar curchr_saved;				///< save current character parsed for the case we cannot print it (output buffer too small)

		///\brief Constructor
		TokState()				:id(Start),pos(0),base(0),value(0),curchr_saved(0) {}

		///\brief Reset this state variables (after succesful exit with a new token parsed)
		///\param [in] id_ the new entity parse state
		void init(Id id_=Start)			{id=id_;pos=0;base=0;value=0;curchr_saved=0;}
	};
	TokState tokstate;								///< the entity parsing state of this XML scanner

public:
	typedef InputCharSet_ InputCharSet;
	typedef OutputCharSet_ OutputCharSet;
	typedef unsigned int size_type;
	class iterator;

public:
	typedef TextScanner<InputIterator,InputCharSet_> InputReader;
	typedef XMLScanner<InputIterator,InputCharSet_,OutputCharSet_,EntityMap_> ThisXMLScanner;
	typedef EntityMap_ EntityMap;
	typedef typename EntityMap::iterator EntityMapIterator;

	///\brief Print a character to the output token buffer
	///\param [in] ch unicode character to print
	unsigned int print( UChar ch)
	{
		unsigned int nn = OutputCharSet::print( ch, outputBuf+outputSize, outputBufSize-outputSize);
		if (nn == 0)
		{
			error = ErrOutputBufferTooSmall;
			tokstate.curchr_saved = ch;
		}
		return nn;
	}

	///\brief Print a character to the output token buffer with incrementing the buffer used size
	///\param [in] ch unicode character to print
	bool push( UChar ch)
	{
		unsigned int nn = print( ch);
		outputSize += nn;
		return (nn != 0);
	}

	///\brief Map a hexadecimal digit to its value
	///\param [in] ch hexadecimal digit to map to its decimal value
	static unsigned char HEX( unsigned char ch)
	{
		struct HexCharMap :public CharMap<unsigned char, 0xFF>
		{
			HexCharMap()
			{
				(*this)
					('0',0) ('1', 1)('2', 2)('3', 3)('4', 4)('5', 5)('6', 6)('7', 7)('8', 8)('9', 9)
					('A',10)('B',11)('C',12)('D',13)('E',14)('F',15)('a',10)('b',11)('c',12)('d',13)('e',14)('f',15);
			}
		};
		static HexCharMap hexCharMap;
		return hexCharMap[ch];
	}

	///\brief Parse a numeric entity value for a table definition (map it to the target character set)
	///\param [in] ir input reader
	///\return the value of the entity parsed
	static UChar parseStaticNumericEntityValue( InputReader& ir)
	{
		signed long long value = 0;
		unsigned char ch = ir.ascii();
		unsigned int base;
		if (ch != '#') return 0;
		ir.skip();
		ch = ir.ascii();
		if (ch == 'x')
		{
			ir.skip();
			ch = ir.ascii();
			base = 16;
		}
		else
		{
			base = 10;
		}
		while (ch != ';')
		{
			unsigned char chval = HEX(ch);
			if (value >= base) return 0;
			value = value * base + chval;
			if (value >= 0xFFFFFFFF) return 0;
			ir.skip();
			ch = ir.ascii();
		}
		return (UChar)value;
	}

	///\brief Print the characters of a sequence that was thought to form an entity but did not
	///\return true on success
	bool fallbackEntity()
	{
		switch (tokstate.id)
		{
			case TokState::Start:
			case TokState::ParsingKey:
			case TokState::ParsingToken:
				break;

			case TokState::ParsingEntity:
				return push('&');
			case TokState::ParsingNumericEntity:
				return push('&') && push('#');
			case TokState::ParsingNumericBaseEntity:
				if (!push('&') || !push('#')) return false;
				for (unsigned int ii=0; ii<tokstate.pos; ii++) if (!push( tokstate.buf[ii])) return false;
				return true;
			case TokState::ParsingNamedEntity:
				if (!push('&')) return false;
				for (unsigned int ii=0; ii<tokstate.pos; ii++) if (!push( tokstate.buf[ii])) return false;
				return true;
		}
		error = ErrInternal;
		return false;
	}

	///\brief Try to parse an entity (we got '&')
	///\return true on success
	bool parseEntity()
	{
		unsigned char ch;
		tokstate.id = TokState::ParsingEntity;
		ch = src.ascii();
		if (ch == '#')
		{
			src.skip();
			return parseNumericEntity();
		}
		else
		{
			return parseNamedEntity();
		}
	}

	///\brief Try to parse a numeric entity (we got '&#')
	///\return true on success
	bool parseNumericEntity()
	{
		unsigned char ch;
		tokstate.id = TokState::ParsingNumericEntity;
		ch = src.ascii();
		if (ch == 'x')
		{
			tokstate.base = 16;
			src.skip();
			return parseNumericBaseEntity();
		}
		else
		{
			tokstate.base = 10;
			return parseNumericBaseEntity();
		}
	}

	///\brief Try to parse a numeric entity with known base (we got '&#' and we know the base 10/16 of it)
	///\return true on success
	bool parseNumericBaseEntity()
	{
		unsigned char ch;
		tokstate.id = TokState::ParsingNumericBaseEntity;

		while (tokstate.pos < sizeof(tokstate.buf))
		{
			tokstate.buf[tokstate.pos++] = ch = src.ascii();
			if (ch == ';')
			{
				if (tokstate.value > 0xFFFFFFFF) return fallbackEntity();
				if (tokstate.value < 32)
				{
					error = ErrEntityEncodesCntrlChar;
					return false;
				}
				if (!push( (UChar)tokstate.value)) return false;
				tokstate.init( TokState::ParsingToken);
				src.skip();
				return true;
			}
			else
			{
				unsigned char chval = HEX(ch);
				if (tokstate.value >= tokstate.base) return fallbackEntity();
				tokstate.value = tokstate.value * tokstate.base + chval;
				src.skip();
			}
		}
		return fallbackEntity();
	}

	///\brief Try to parse a named entity
	///\return true on success
	bool parseNamedEntity()
	{
		unsigned char ch;
		tokstate.id = TokState::ParsingNamedEntity;
		ch = src.ascii();
		while (tokstate.pos < sizeof(tokstate.buf)-1 && ch != ';' && src.control() == Any)
		{
			tokstate.buf[ tokstate.pos] = ch;
			src.skip();
			tokstate.pos++;
			ch = src.ascii();
		}
		if (ch == ';')
		{
			tokstate.buf[ tokstate.pos] = '\0';
			if (!pushEntity( tokstate.buf)) return false;
			tokstate.init( TokState::ParsingToken);
			src.skip();
			return true;
		}
		else
		{
			return fallbackEntity();
		}
	}

	///\typedef IsTokenCharMap
	///\brief Forms a set of characters by assigning (true/false) to the whole domain
	typedef CharMap<bool,false,NofControlCharacter> IsTokenCharMap;

	///\class IsTagCharMap
	///\brief Defines the set of tag characters
	struct IsTagCharMap :public IsTokenCharMap
	{
		IsTagCharMap()
		{
			(*this)(Undef,true)(Any,true);
		}
	};

	///\class IsContentCharMap
	///\brief Defines the set of content token characters
	struct IsContentCharMap :public IsTokenCharMap
	{
		IsContentCharMap()
		{
			(*this)(Undef,true)(Equal,true)(Gt,true)(Slash,true)(Exclam,true)(Questm,true)(Sq,true)(Dq,true)(Osb,true)(Csb,true)(Any,true);
		}
	};

	///\class IsSQStringCharMap
	///\brief Defines the set characters belonging to a single quoted string
	struct IsSQStringCharMap :public IsContentCharMap
	{
		IsSQStringCharMap()
		{
			(*this)(Sq,false)(Space,true);
		}
	};

	///\class IsDQStringCharMap
	///\brief Defines the set characters belonging to a double quoted string
	struct IsDQStringCharMap :public IsContentCharMap
	{
		IsDQStringCharMap()
		{
			(*this)(Dq,false)(Space,true);
		}
	};

	///\brief Try to recover from an interrupted token parsing state (end of input exception)
	///\return true on success
	bool parseTokenRecover()
	{
		bool rt = false;
		if (tokstate.curchr_saved)
		{
			if (!push( tokstate.curchr_saved)) return false;
			tokstate.curchr_saved = 0;
		}
		switch (tokstate.id)
		{
			case TokState::Start:
			case TokState::ParsingKey:
			case TokState::ParsingToken:
				error = ErrInternal;
				return false;
			case TokState::ParsingEntity: rt = parseEntity(); break;
			case TokState::ParsingNumericEntity: rt = parseNumericEntity(); break;
			case TokState::ParsingNumericBaseEntity: rt = parseNumericBaseEntity(); break;
			case TokState::ParsingNamedEntity: rt = parseNamedEntity(); break;
		}
		tokstate.init( TokState::ParsingToken);
		return rt;
	}

	///\brief Parse a token defined by the set of valid token characters
	///\param [in] isTok set of valid token characters
	///\return true on success
	bool parseToken( const IsTokenCharMap& isTok)
	{
		if (tokstate.id == TokState::Start)
		{
			tokstate.id = TokState::ParsingToken;
		}
		else if (tokstate.id != TokState::ParsingToken)
		{
			if (!parseTokenRecover())
			{
				tokstate.init();
				return false;
			}
		}
		for (;;)
		{
			ControlCharacter ch;
			while (isTok[ (unsigned char)(ch=src.control())])
			{
				if (!push( src.chr()))
				{
					tokstate.curchr_saved = src.chr();
					return false;
				}
				src.skip();
			}
			if (ch == Amp)
			{
				src.skip();
				if (!parseEntity()) break;
				tokstate.init( TokState::ParsingToken);
				continue;
			}
			else
			{
				tokstate.init();
				return true;
			}
		}
		tokstate.init();
		return false;
	}

	///\brief Static version of parse a token for parsing table definition elements
	///\param [in] isTok set of valid token characters
	///\param [in] ir input reader iterator
	///\param [out] buf buffer where to write the result to
	///\param [in] bufsize allocation size of buf in bytes
	///\param [out] p_outputBufSize number of bytes written to buf
	///\return true on success
	static bool parseStaticToken( const IsTokenCharMap& isTok, InputReader ir, char* buf, size_type bufsize, size_type* p_outputBufSize)
	{
		for (;;)
		{
			ControlCharacter ch;
			size_type ii=0;
			*p_outputBufSize = 0;
			for (;;)
			{
				UChar pc;
				if (isTok[ (unsigned char)(ch=ir.control())])
				{
					pc = ir.chr();
				}
				else if (ch == Amp)
				{
					pc = parseStaticNumericEntityValue( ir);
				}
				else
				{
					*p_outputBufSize = ii;
					return true;
				}
				unsigned int chlen = OutputCharSet::print( pc, buf+ii, bufsize-ii);
				if (chlen == 0 || pc == 0)
				{
					*p_outputBufSize = ii;
					return false;
				}
				ii += chlen;
				ir.skip();
			}
		}
	}

	///\brief Skip a token defined by the set of valid token characters (same as parseToken but nothing written to the output buffer)
	///\param [in] isTok set of valid token characters
	///\return true on success
	bool skipToken( const IsTokenCharMap& isTok)
	{
		for (;;)
		{
			ControlCharacter ch;
			while (isTok[ (unsigned char)(ch=src.control())] || ch == Amp)
			{
				src.skip();
			}
			if (src.control() != Any) return true;
		}
	}

	///\brief Parse a token that must be the same as a given string
	///\param [in] str string expected
	///\return true on success
	bool expectStr( const char* str)
	{
		bool rt = true;
		tokstate.id = TokState::ParsingKey;
		for (; str[tokstate.pos] != '\0'; src.skip(),tokstate.pos++)
		{
			if (src.ascii() == str[ tokstate.pos]) continue;
			ControlCharacter ch = src.control();
			if (ch == EndOfText)
			{
				error = ErrUnexpectedEndOfText;
			}
			else
			{
				error = ErrSyntaxToken;
			}
			rt = false;
			break;
		}
		tokstate.init();
		return rt;
	}

	///\brief Parse an entity defined by name (predefined)
	///\param [in] str pointer to the buffer with the entity name
	///\return true on success
	bool pushPredefinedEntity( const char* str)
	{
		switch (str[0])
		{
			case 'q':
				if (str[1] == 'u' && str[2] == 'o' && str[3] == 't' && str[4] == '\0')
				{
					if (!push( '\"')) return false;
					return true;
				}
				break;

			case 'a':
				if (str[1] == 'm')
				{
					if (str[2] == 'p' && str[3] == '\0')
					{
						if (!push( '&')) return false;
						return true;
					}
				}
				else if (str[1] == 'p')
				{
					if (str[2] == 'o' && str[3] == 's' && str[4] == '\0')
					{
						if (!push( '\'')) return false;
						return true;
					}
				}
				break;

			case 'l':
				if (str[1] == 't' && str[2] == '\0')
				{
					if (!push( '<')) return false;
					return true;
				}
				break;

			case 'g':
				if (str[1] == 't' && str[2] == '\0')
				{
					if (!push( '>')) return false;
					return true;
				}
				break;

			case 'n':
				if (str[1] == 'b' && str[2] == 's' && str[3] == 'p' && str[4] == '\0')
				{
					if (!push( ' ')) return false;
					return true;
				}
				break;
		}
		return false;
	}

	///\brief Parse an entity defined by name (predefined or in defined in entity table)
	///\param [in] str pointer to the buffer with the entity name
	///\return true on success
	bool pushEntity( const char* str)
	{
		if (pushPredefinedEntity( str))
		{
			return true;
		}
		else if (entityMap)
		{
			EntityMapIterator itr = entityMap->find( str);
			if (itr == entityMap->end())
			{
				error = ErrUndefinedCharacterEntity;
				return false;
			}
			else
			{
				UChar ch = itr->second;
				if (ch < 32)
				{
					error = ErrEntityEncodesCntrlChar;
					return false;
				}
				return push( ch);
			}
		}
		else
		{
			error = ErrUndefinedCharacterEntity;
			return false;
		}
	}

private:
	STMState state;						///< current state of the XML scanner
	Error error;							///< last error code
	InputReader src;						///< source input iterator
	EntityMap* entityMap;				///< map with entities defined by the caller
	char* outputBuf;						///< buffer to use for output
	size_type outputBufSize;			///< size of buffer to use for output
	size_type outputSize;				///< number of bytes written to output buffer

public:
	///\brief Constructor
	///\param [in] p_src source iterator
	///\param [in] p_outputBuf buffer to use for output
	///\param [in] p_outputBufSize size of buffer to use for output in bytes
	///\param [in] p_entityMap read only map of named entities defined by the user
	XMLScanner( InputIterator& p_src, char* p_outputBuf, size_type p_outputBufSize, EntityMap* p_entityMap=0)
			:state(START),error(Ok),src(p_src),entityMap(p_entityMap),outputBuf(p_outputBuf),outputBufSize(p_outputBufSize),outputSize(0)
	{}

	///\brief Copy constructor
	///\param [in] o scanner to copy
	XMLScanner( XMLScanner& o)
			:state(o.state),error(o.error),src(o.src),entityMap(o.entityMap),outputBuf(o.outputBuf),outputBufSize(o.outputBufSize),outputSize(o.outputSize)
	{}

	///\brief Redefine the buffer to use for output
	///\param [in] p_outputBuf buffer to use for output
	///\param [in] p_outputBufSize size of buffer to use for output in bytes
	void setOutputBuffer( char* p_outputBuf, size_type p_outputBufSize)
	{
		outputBuf = p_outputBuf;
		outputBufSize = p_outputBufSize;
	}

	///\brief Static parse of a tag name for the elements in a table
	///\tparam Character set of the tag written
	///\param [in] src tagname as ASCII with encoded entities for characters beyond ASCII
	///\param [in] p_outputBuf buffer for output
	///\param [in] p_outputBufSize size of buffer for output in bytes
	///\param [out] p_outputSize number of bytes written to output in bytes
	template <class CharSet>
	static bool getTagName( const char* src, char* p_outputBuf, size_type p_outputBufSize, size_type* p_outputSize)
	{
		static IsTagCharMap isTagCharMap;
		typedef XMLScanner<const char*, charset::UTF8, CharSet> Scan;
		char* itr = const_cast<char*>(src);
		return parseStaticToken( isTagCharMap, itr, p_outputBuf, p_outputBufSize, p_outputSize);
	}

	///\brief Get the current parsed YML element string, if it was not masked out, see nextItem(unsigned short)
	///\return the item string
	char* getItem() const {return outputBuf;}

	///\brief Get the size of the current parsed YML element string in bytes
	///\return the item string
	size_type getItemSize() const {return outputSize;}

	///\brief Get the current XML scanner state machine state
	///\return pointer to the state variables
	ScannerStatemachine::Element* getState()
	{
		static Statemachine STM;
		return STM.get( state);
	}

	///\brief Get the last error
	///\param [out] the error as string
	///\return the error code
	Error getError( const char** str=0)
	{
		Error rt = error;
		error = Ok;
		if (str) *str=getErrorString(rt);
		return rt;
	}

	///\brief Scan the next XML element
	///\param [in] mask element types that should be printed to the output buffer (1 -> print, 0 -> mask out, just return the element as event)
	///\return the type of the XML element
	ElementType nextItem( unsigned short mask=0xFFFF)
	{
		static const IsContentCharMap contentC;
		static const IsTagCharMap tagC;
		static const IsSQStringCharMap sqC;
		static const IsDQStringCharMap dqC;
		static const IsTokenCharMap* tokenDefs[ NofSTMActions] = {0,&contentC,&tagC,&sqC,&dqC,0,0,0};
		static const char* stringDefs[ NofSTMActions] = {0,0,0,0,0,"xml","CDATA",0};

		ElementType rt = None;
		if (tokstate.id == TokState::Start)
		{
			outputSize = 0;
			outputBuf[0] = 0;
		}
		do
		{
			ScannerStatemachine::Element* sd = getState();
			if (sd->action.op != -1)
			{
				if (tokenDefs[sd->action.op])
				{
					if ((mask&(1<<sd->action.arg)) != 0)
					{
						if (!parseToken( *tokenDefs[ sd->action.op])) return ErrorOccurred;
					}
					else
					{
						if (!skipToken( *tokenDefs[ sd->action.op])) return ErrorOccurred;
					}
					if (!print(0)) return ErrorOccurred;
					rt = (ElementType)sd->action.arg;
				}
				else if (stringDefs[sd->action.op])
				{
					if (!expectStr( stringDefs[sd->action.op])) return ErrorOccurred;
				}
				else
				{
					rt = (ElementType)sd->action.arg;
					if (rt == Exit) return rt;
				}
			}
			ControlCharacter ch = src.control();

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
	}

	///\class End
	///\brief end of input tag
	struct End {};

	///\class iterator
	///\brief input iterator for iterating on the output of an XML scanner
	class iterator
	{
	public:
		///\class Element
		///\brief Iterator element visited
		class Element
		{
		private:
			friend class iterator;
			ElementType m_type;			///< type of the element
			char* m_content;				///< value string of the element
			size_type m_size;				///< size of the value string in bytes
		public:
			///\brief Type of the current element as string
			const char* name() const	{return getElementTypeName( m_type);}
			///\brief Type of the current element
			ElementType type() const	{return m_type;}
			///\brief Value of the current element
			const char* content() const	{return m_content;}
			///\brief Size of the value of the current element in bytes
			size_type size() const		{return m_size;}
			///\brief Constructor
			Element()			:m_type(None),m_content(0),m_size(0) {}
			///\brief Constructor
			Element( const End&)		:m_type(Exit),m_content(0),m_size(0) {}
			///\brief Copy constructor
			///\param [in] orig element to copy
			Element( const Element& orig)	:m_type(orig.m_type),m_content(orig.m_content),m_size(orig.m_size) {}
		};
		// input iterator traits
		typedef Element value_type;
		typedef size_type difference_type;
		typedef Element* pointer;
		typedef Element& reference;
		typedef std::input_iterator_tag iterator_category;

	private:
		Element element;						///< currently visited element
		ThisXMLScanner* input;				///< XML scanner

		///\brief Skip to the next element
		///\param [in] mask element types that should be printed to the output buffer (1 -> print, 0 -> mask out, just return the element as event)
		///\return iterator pointing to the next element
		iterator& skip( unsigned short mask=0xFFFF)
		{
			if (input != 0)
			{
				element.m_type = input->nextItem(mask);
				element.m_content = input->getItem();
				element.m_size = input->getItemSize();
			}
			return *this;
		}

		///\brief Compare iterator with another
		///\param [in] iter iterator to compare with
		///\return true if they are equal
		bool compare( const iterator& iter) const
		{
			if (element.type() == iter.element.type())
			{
				if (element.type() == Exit || element.type() == None) return true;  //equal only at beginning and end
			}
			return false;
		}
	public:
		///\brief Assign an iterator to another
		///\param [in] orig iterator to copy
		void assign( const iterator& orig)
		{
			input = orig.input;
			element = orig.element;
		}
		///\brief Copy constructor
		///\param [in] orig iterator to copy
		iterator( const iterator& orig)
		{
			assign( orig);
		}
		///\brief Constructor
		///\param [in] p_input XML scanner to use for iteration
		iterator( ThisXMLScanner& p_input)
				:input( &p_input)
		{
			element.m_type = input->nextItem();
			element.m_content = input->getItem();
			element.m_size = input->getItemSize();
		}
		///\brief Constructor
		iterator( const End& et)  :element(et),input(0) {}
		///\brief Constructor
		iterator()  :input(0) {}
		///\brief Assignement operator
		///\param [in] orig iterator to assign to this
		iterator& operator = (const iterator& orig)
		{
			assign( orig);
			return *this;
		}
		///\brief Element dereference operator
		const Element& operator*()
		{
			return element;
		}
		///\brief Element dereference operator
		const Element* operator->()
		{
			return &element;
		}
		///\brief Preincrement
		///\return *this
		iterator& operator++()				{return skip();}
		///\brief Postincrement
		///\return *this
		iterator operator++(int)			{iterator tmp(*this); skip(); return tmp;}

		///\brief Compare to check for equality
		///\return true, if equal
		bool operator==( const iterator& iter) const	{return compare( iter);}
		///\brief Compare to check for unequality
		///\return true, if not equal
		bool operator!=( const iterator& iter) const	{return !compare( iter);}
	};

	///\brief Get begin iterator
	///\return iterator
	iterator begin()
	{
		return iterator( *this);
	}
	///\brief Get the pointer to the end of content
	///\return iterator
	iterator end()
	{
		return iterator( End());
	}
};

///\defgroup XMLpathselect
///\brief Structures for iterating on the elements typed by XML path selections

template <class CharSet_=charset::UTF8>
class XMLPathSelectAutomaton :public throws_exception
{
public:
	enum {defaultMemUsage=3*1024,defaultMaxDepth=32};
	unsigned int memUsage;
	unsigned int maxDepth;
	unsigned int maxScopeStackSize;
	unsigned int maxFollows;
	unsigned int maxTriggers;
	unsigned int maxTokens;

public:
	XMLPathSelectAutomaton()
			:memUsage(defaultMemUsage),maxDepth(defaultMaxDepth),maxScopeStackSize(0),maxFollows(0),maxTriggers(0),maxTokens(0)
	{
		if (!setMemUsage( memUsage, maxDepth)) throw exception( DimOutOfRange);
	}

	typedef int Hash;
	typedef XMLPathSelectAutomaton<CharSet_> ThisXMLPathSelectAutomaton;

public:
	enum Operation
	{
		Content, Tag, Attribute, ThisAttributeValue, AttributeValue, ContentStart
	};
	static const char* operationName( Operation op)
	{
		static const char* name[ 6] = {"Content", "Tag", "Attribute", "ThisAttributeValue", "AttributeValue", "ContentStart"};
		return name[ (unsigned int)op];
	}

	struct Mask
	{
		unsigned short pos;
		unsigned short neg;
		bool empty() const								{return (pos==0);}
		Mask( unsigned short p_pos=0, unsigned short p_neg=0):pos(p_pos),neg(p_neg) {}
		Mask( const Mask& orig)								:pos(orig.pos),neg(orig.neg) {}
		Mask( Operation op)								:pos(0),neg(0) {this->match(op);}
		void reset()									{pos=0; neg=0;}
		void reject( XMLScannerBase::ElementType e)					{neg |= (1<<(unsigned short)e);}
		void match( XMLScannerBase::ElementType e)					{pos |= (1<<(unsigned short)e);}
		void seekop( Operation op)
		{
			switch (op)
			{
				case Tag:
					this->match( XMLScannerBase::OpenTag);
					break;
				case Attribute:
					this->match( XMLScannerBase::TagAttribName);
					this->match( XMLScannerBase::HeaderAttribName);
					this->reject( XMLScannerBase::Content);
					break;
				case ThisAttributeValue:
					this->match( XMLScannerBase::TagAttribValue);
					this->match( XMLScannerBase::HeaderAttribValue);
					this->reject( XMLScannerBase::TagAttribName);
					this->reject( XMLScannerBase::HeaderAttribName);
					this->reject( XMLScannerBase::Content);
					this->reject( XMLScannerBase::OpenTag);
					break;
				case AttributeValue:
					this->match( XMLScannerBase::TagAttribValue);
					this->match( XMLScannerBase::HeaderAttribValue);
					this->reject( XMLScannerBase::Content);
					break;
				case Content:
					this->match( XMLScannerBase::Content);
					break;
				case ContentStart:
					this->match( XMLScannerBase::HeaderEnd);
					break;
			}
		}
		void join( const Mask& mask)				{pos |= mask.pos; neg |= mask.neg;}
		bool matches( XMLScannerBase::ElementType e) const	{return (0 != (pos & (1<<(unsigned short)e)));}
		bool rejects( XMLScannerBase::ElementType e) const	{return (0 != (neg & (1<<(unsigned short)e)));}
	};

	struct Core
	{
		Mask mask;
		bool follow;
		int typeidx;
		int cnt_start;
		int cnt_end;

		Core()			:follow(false),typeidx(0),cnt_start(0),cnt_end(-1) {}
		Core( const Core& o)	:mask(o.mask),follow(o.follow),typeidx(o.typeidx),cnt_start(o.cnt_start),cnt_end(o.cnt_end) {}
	};

	struct State
	{
		Core core;
		unsigned int keysize;
		char* key;
		char* srckey;
		int next;
		int link;

		State()					:keysize(0),key(0),srckey(0),next(-1),link(-1) {}
		State( const State& orig)		:core(orig.core),keysize(orig.keysize),key(0),srckey(0),next(orig.next),link(orig.link)
		{
			defineKey( orig.keysize, orig.key, orig.srckey);
		}
		~State()
		{
			if (key) delete [] key;
		}

		bool isempty()				{return key==0&&core.typeidx==0;}

		void defineKey( unsigned int p_keysize, const char* p_key, const char* p_srckey)
		{
			unsigned int ii;
			if (key)
			{
				delete [] key;
				key = 0;
			}
			if (srckey)
			{
				delete [] srckey;
				srckey = 0;
			}
			if (p_key)
			{
				key = new char[ keysize=p_keysize];
				for (ii=0; ii<keysize; ii++) key[ii]=p_key[ii];
			}
			if (p_srckey)
			{
				for (ii=0; p_srckey[ii]!=0; ii++);
				srckey = new char[ ii+1];
				for (ii=0; p_srckey[ii]!=0; ii++) srckey[ii]=p_srckey[ii];
				srckey[ ii] = 0;
			}
		}

		void defineNext( Operation op, unsigned int p_keysize, const char* p_key, const char* p_srckey, int p_next, bool p_follow=false)
		{
			core.mask.seekop( op);
			defineKey( p_keysize, p_key, p_srckey);
			next = p_next;
			core.follow = p_follow;
		}

		void defineOutput( const Mask& mask, int p_typeidx, bool p_follow, int p_start, int p_end)
		{
			core.mask = mask;
			core.typeidx = p_typeidx;
			core.cnt_end = p_end;
			core.cnt_start = p_start;
			core.follow = p_follow;
		}

		void defLink( int p_link)
		{
			link = p_link;
		}
	};
	std::vector<State> states;

	struct Token
	{
		Core core;
		int stateidx;

		Token()						:stateidx(-1) {}
		Token( const Token& orig)			:core(orig.core),stateidx(orig.stateidx) {}
		Token( const State& state, int p_stateidx)	:core(state.core),stateidx(p_stateidx) {}
	};

	struct Scope
	{
		Mask mask;
		Mask followMask;
		struct Range
		{
			unsigned int tokenidx_from;
			unsigned int tokenidx_to;
			unsigned int followidx;

			Range()				:tokenidx_from(0),tokenidx_to(0),followidx(0) {}
			Range( const Scope& orig)	:tokenidx_from(orig.tokenidx_from),tokenidx_to(orig.tokenidx_to),followidx(orig.followidx) {}
		};
		Range range;

		Scope( const Scope& orig)		:mask(orig.mask),followMask(orig.followMask),range(orig.range) {}
		Scope& operator =( const Scope& orig)	{mask=orig.mask; followMask=orig.followMask; range=orig.range; return *this;}
		Scope()					{}
	};

	bool setMemUsage( unsigned int p_memUsage, unsigned int p_maxDepth)
	{
		memUsage = p_memUsage;
		maxDepth = p_maxDepth;
		maxScopeStackSize = maxDepth;
		if (p_memUsage < maxScopeStackSize * sizeof(Scope))
		{
			maxScopeStackSize = 0;
		}
		else
		{
			p_memUsage -= maxScopeStackSize * sizeof(Scope);
		}
		maxFollows = (p_memUsage / sizeof(unsigned int)) / 32 + 2;
		maxTriggers = (p_memUsage / sizeof(unsigned int)) / 32 + 3;
		p_memUsage -= sizeof(unsigned int) * maxFollows + sizeof(unsigned int) * maxTriggers;
		maxTokens = p_memUsage / sizeof(Token);
		return (maxScopeStackSize != 0 && maxTokens != 0 && maxFollows != 0 && maxTriggers != 0);
	}

private:
	int defineNext( int stateidx, Operation op, unsigned int keysize, const char* key, const char* srckey, bool follow=false) throw(exception)
	{
		try
		{
			State state;
			if (states.size() == 0)
			{
				stateidx = states.size();
				states.push_back( state);
			}
			for (int ee=stateidx; ee != -1; stateidx=ee,ee=states[ee].link)
			{
				if (states[ee].key != 0 && keysize == states[ee].keysize && states[ee].core.follow == follow)
				{
					unsigned int ii;
					for (ii=0; ii<keysize && states[ee].key[ii]==key[ii]; ii++);
					if (ii == keysize) return states[ee].next;
				}
			}
			if (!states[stateidx].isempty())
			{
				stateidx = states[stateidx].link = states.size();
				states.push_back( state);
			}
			states.push_back( state);
			unsigned int lastidx = states.size()-1;
			states[ stateidx].defineNext( op, keysize, key, srckey, lastidx, follow);
			return stateidx=lastidx;
		}
		catch (std::bad_alloc)
		{
			throw exception( OutOfMem);
		}
		catch (...)
		{
			throw exception( Unknown);
		};
	}

	void defineThisOutput( int stateidx, int typeidx)
	{
		if ((unsigned int)stateidx >= states.size()) throw exception( IllegalParam);
		if (states[stateidx].core.typeidx != 0) throw exception( NotAllowedOperation);
		states[stateidx].core.typeidx = typeidx;
	}

	int defineOutput( int stateidx, const Mask& printOpMask, int typeidx, bool follow, int start, int end) throw(exception)
	{
		try
		{
			State state;
			if (states.size() == 0)
			{
				stateidx = states.size();
				states.push_back( state);
			}
			if ((unsigned int)stateidx >= states.size()) throw exception( IllegalParam);

			if (!states[stateidx].isempty())
			{
				stateidx = states[stateidx].link = states.size();
				states.push_back( state);
			}
			states[ stateidx].defineOutput( printOpMask, typeidx, follow, start, end);
			return stateidx;
		}
		catch (std::bad_alloc)
		{
			throw exception( OutOfMem);
		}
		catch (...)
		{
			throw exception( Unknown);
		};
	}

public:
	struct PathElement :throws_exception
	{
	private:
		enum {MaxSize=1024};

		XMLPathSelectAutomaton* xs;
		int stateidx;
		struct Range
		{
			int start;
			int end;

			Range( const Range& o)		:start(o.start),end(o.end){}
			Range( int p_start, int p_end)	:start(p_start),end(p_end){}
			Range( int count)		:start(0),end(count){}
			Range()				:start(0),end(-1){}
		};
		Range range;
		bool follow;
		Mask pushOpMask;
		Mask printOpMask;

	private:
		PathElement& defineOutput( Operation op)
		{
			printOpMask.reset();
			printOpMask.seekop( op);
			return *this;
		}
		PathElement& doSelect( Operation op, const char* value) throw(exception)
		{
			if (xs != 0)
			{
				if (value)
				{
					char buf[ 1024];
					XMLScanner<char*>::size_type size;
					if (!XMLScanner<char*>::getTagName<CharSet_>( value, buf, sizeof(buf), &size))
					{
						throw exception( IllegalAttributeName);
					}
					stateidx = xs->defineNext( stateidx, op, size, buf, value, follow);
				}
				else
				{
					stateidx = xs->defineNext( stateidx, op, 0, 0, 0, follow);
				}
			}
			return *this;
		}
		PathElement& doFollow()
		{
			follow = true;
			return *this;
		}
		PathElement& doRange( int p_start, int p_end)
		{
			if (range.end == -1)
			{
				range = Range( p_start, p_end);
			}
			else if (p_end < range.end)
			{
				range.end = p_end;
			}
			else if (p_start > range.start)
			{
				range.start = p_start;
			}
			return *this;
		}
		PathElement& doCount( int p_count)
		{
			return doRange( 0, p_count);
		}
		PathElement& doStart( int p_start)
		{
			return doRange( p_start, std::numeric_limits<int>::max());
		}

		PathElement& push( int typeidx) throw(exception)
		{
			if (xs != 0) stateidx = xs->defineOutput( stateidx, printOpMask, typeidx, follow, range.start, range.end);
			return *this;
		}

	public:
		PathElement()							:xs(0),stateidx(0),follow(false),pushOpMask(0),printOpMask(0){}
		PathElement( XMLPathSelectAutomaton* p_xs, int p_si=0)		:xs(p_xs),stateidx(p_si),follow(false),pushOpMask(0),printOpMask(0){}
		PathElement( const PathElement& orig)				:xs(orig.xs),stateidx(orig.stateidx),range(orig.range),follow(orig.follow),pushOpMask(orig.pushOpMask),printOpMask(orig.printOpMask) {}

		//corresponds to "//" in abbreviated syntax of XPath
		PathElement& operator --(int)							{return doFollow();}
		//find tag
		PathElement& operator []( const char* name) throw(exception)			{return doSelect( Tag, name);}
		PathElement& selectTag( const char* name) throw(exception)			{return doSelect( Tag, name);}
		//find tag with one attribute
		PathElement& operator ()( const char* name) throw(exception)			{return doSelect( Attribute, name).defineOutput( ThisAttributeValue);}
		PathElement& selectAttribute( const char* name) throw(exception)		{return doSelect( Attribute, name).defineOutput( ThisAttributeValue);}
		//find tag with one attribute
		PathElement& operator ()( const char* name, const char* value) throw(exception)	{return doSelect( Attribute, name).doSelect( ThisAttributeValue, value);}
		PathElement& ifAttribute( const char* name, const char* value) throw(exception)	{return doSelect( Attribute, name).doSelect( ThisAttributeValue, value);}

		//define maximum element index to push
		PathElement& TO(int cnt) throw(exception)					{return doCount((cnt>=0)?(cnt+1):-1);}
		//define minimum element index to push
		PathElement& FROM(int cnt) throw(exception)					{return doStart(cnt); return *this;}
		//define minimum and maximum element index to push
		PathElement& RANGE(int cnt) throw(exception)					{return doRange(cnt,(cnt>=0)?(cnt+1):-1); return *this;}
		//define element type to push
		PathElement& operator =(int type) throw(exception)				{return push( type);}
		PathElement& assignType(int type) throw(exception)				{return push( type);}
		//grab content
		PathElement& operator ()()  throw(exception)					{return defineOutput(Content);}
		PathElement& selectContent()  throw(exception)					{return defineOutput(Content);}
	};

	PathElement operator*()
	{
		return PathElement( this);
	};
};


template <
		class InputIterator,				//< input iterator with ++ and read only * returning 0 als last character of the input
		class InputCharSet_=charset::UTF8,		//< character set encoding of the input, read as stream of bytes
		class OutputCharSet_=charset::UTF8,		//< character set encoding of the output, printed as string of the item type of the character set
		class EntityMap_=std::map<const char*,UChar>	//< STL like map from ASCII const char* to UChar
>
class XMLPathSelect :public throws_exception
{
public:
	typedef XMLPathSelectAutomaton<OutputCharSet_> Automaton;
	typedef XMLScanner<InputIterator,InputCharSet_,OutputCharSet_,EntityMap_> ThisXMLScanner;
	typedef XMLPathSelect<InputIterator,InputCharSet_,OutputCharSet_,EntityMap_> ThisXMLPathSelect;
	typedef EntityMap_ EntityMap;

private:
	ThisXMLScanner scan;
	const Automaton* atm;
	typedef typename Automaton::Mask Mask;
	typedef typename Automaton::Token Token;
	typedef typename Automaton::Hash Hash;
	typedef typename Automaton::State State;
	typedef typename Automaton::Scope Scope;

	//static array of POD types. I decided to implement it on my own
	template <typename Element>
	class Array :public throws_exception
	{
		Element* m_ar;
		unsigned int m_size;
		unsigned int m_maxSize;
	public:
		Array( unsigned int p_maxSize) :m_size(0),m_maxSize(p_maxSize)
		{
			m_ar = new (std::nothrow) Element[ m_maxSize];
			if (m_ar == 0) throw exception( OutOfMem);
		}
		~Array()
		{
			if (m_ar) delete [] m_ar;
		}
		void push_back( const Element& elem)
		{
			if (m_size == m_maxSize) throw exception( OutOfMem);
			m_ar[ m_size++] = elem;
		}
		void pop_back()
		{
			if (m_size == 0) throw exception( NotAllowedOperation);
			m_size--;
		}
		Element& operator[]( unsigned int idx)
		{
			if (idx >= m_size) throw exception( ArrayBoundsReadWrite);
			return m_ar[ idx];
		}
		Element& back()
		{
			if (m_size == 0) throw exception( ArrayBoundsReadWrite);
			return m_ar[ m_size-1];
		}
		void resize( unsigned int p_size)
		{
			if (p_size > m_size) throw exception( ArrayBoundsReadWrite);
			m_size = p_size;
		}
		unsigned int size() const  {return m_size;}
		bool empty() const			{return m_size==0;}
	};

	Array<Scope> scopestk;		//stack of scopes opened
	Array<unsigned int> follows;	//indices of tokens active in all descendant scopes
	Array<int> triggers;		//triggered elements
	Array<Token> tokens;		//list of waiting tokens

	struct Context
	{
		XMLScannerBase::ElementType type;	//element type processed
		const char* key;			//string value of element processed
		unsigned int keysize;			//sizeof string value in bytes of element processed
		Scope scope;				//active scope
		unsigned int scope_iter;		//position of currently visited token in the active scope

		Context()				:type(XMLScannerBase::Content),key(0),keysize(0) {}

		void init( XMLScannerBase::ElementType p_type, const char* p_key, int p_keysize)
		{
			type = p_type;
			key = p_key;
			keysize = p_keysize;
			scope_iter = scope.range.tokenidx_from;
		}
	};
	Context context;

	void expand( int stateidx)
	{
		while (stateidx!=-1)
		{
			const State& st = atm->states[ stateidx];
			context.scope.mask.join( st.core.mask);
			if (st.core.mask.empty() && st.core.typeidx != 0)
			{
				triggers.push_back( st.core.typeidx);
			}
			else
			{
				if (st.core.follow)
				{
					context.scope.followMask.join( st.core.mask);
					follows.push_back( tokens.size());
				}
				tokens.push_back( Token( st, stateidx));
			}
			stateidx = st.link;
		}
	}

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
			scopestk.push_back( context.scope);
			context.scope.mask = context.scope.followMask;
			context.scope.mask.match( XMLScannerBase::OpenTag);
			//... we reset the mask but ensure that this 'OpenTag' is processed for sure
		}
		else if (type == XMLScannerBase::CloseTag || type == XMLScannerBase::CloseTagIm)
		{
			if (!scopestk.empty())
			{
				context.scope = scopestk.back();
				scopestk.pop_back();
				follows.resize( context.scope.range.followidx);
				tokens.resize( context.scope.range.tokenidx_to);
			}
		}
	}

	void produce( unsigned int tokenidx, const State& st)
	{
		const Token& tk = tokens[ tokenidx];
		if (tk.core.cnt_end == -1)
		{
			expand( st.next);
		}
		else
		{
			if (tk.core.cnt_end > 0)
			{
				if (--tokens[ tokenidx].core.cnt_end == 0)
				{
					tokens[ tokenidx].core.mask.reset();
				}
				if (tk.core.cnt_start <= 0)
				{
					expand( st.next);
				}
				else
				{
					--tokens[ tokenidx].core.cnt_start;
				}
			}
		}
	}

	int match( unsigned int tokenidx)
	{
		int rt = 0;
		if (context.key != 0)
		{
			if (tokenidx >= context.scope.range.tokenidx_to) return 0;

			const Token& tk = tokens[ tokenidx];
			if (tk.core.mask.matches( context.type))
			{
				const State& st = atm->states[ tk.stateidx];
				if (st.key)
				{
					if (st.keysize == context.keysize)
					{
						unsigned int ii;
						for (ii=0; ii<context.keysize && st.key[ii] == context.key[ii]; ii++);
						if (ii==context.keysize)
						{
							produce( tokenidx, st);
						}
					}
				}
				else
				{
					produce( tokenidx, st);
				}
				if (tk.core.typeidx != 0)
				{
					if (tk.core.cnt_end == -1)
					{
						rt = tk.core.typeidx;
					}
					else if (tk.core.cnt_end > 0)
					{
						if (--tokens[ tokenidx].core.cnt_end == 0)
						{
							tokens[ tokenidx].core.mask.reset();
						}
						if (tk.core.cnt_start <= 0)
						{
							rt = tk.core.typeidx;
						}
						else
						{
							--tokens[ tokenidx].core.cnt_start;
						}
					}
				}
			}
			if (tk.core.mask.rejects( context.type))
			{
				//The token must not match anymore after encountering a reject item
				tokens[ tokenidx].core.mask.reset();
			}
		}
		return rt;
	}

	int fetch()
	{
		int type = 0;

		if (context.scope.mask.matches( context.type))
		{
			while (!type)
			{
				if (context.scope_iter < context.scope.range.tokenidx_to)
				{
					type = match( context.scope_iter);
					++context.scope_iter;
				}
				else
				{
					unsigned int ii = context.scope_iter - context.scope.range.tokenidx_to;
					//we match all follows that are not yet been checked in the current scope
					if (ii < context.scope.range.followidx && context.scope.range.tokenidx_from > follows[ ii])
					{
						type = match( follows[ ii]);
						++context.scope_iter;
					}
					else if (!triggers.empty())
					{
						type = triggers.back();
						triggers.pop_back();
					}
					else
					{
						context.key = 0;
						context.keysize = 0;
						return 0; //end of all candidates
					}
				}
			}
		}
		else
		{
			context.key = 0;
			context.keysize = 0;
		}
		return type;
	}

public:
	XMLPathSelect( const Automaton* p_atm, InputIterator& src, char* obuf, unsigned int obufsize, EntityMap* entityMap=0)
		:scan(src,obuf,obufsize,entityMap),atm(p_atm),scopestk(p_atm->maxScopeStackSize),follows(p_atm->maxFollows),triggers(p_atm->maxTriggers),tokens(p_atm->maxTokens)
	{
		if (atm->states.size() > 0) expand(0);
	}
	XMLPathSelect( const XMLPathSelect& o)
		:scan(o.scan),atm(o.atm),scopestk(o.maxScopeStackSize),follows(o.maxFollows),follows(o.maxTriggers),tokens(o.maxTokens){}

	void setOutputBuffer( char* outputBuf, unsigned int outputBufSize)
	{
		scan.setOutputBuffer( outputBuf, outputBufSize);
	}

	//input iterator for the output of this XMLScanner:
	struct End {};
	class iterator
	{
	public:
		class Element
		{
		public:
			enum State {Ok,EndOfOutput,EndOfInput,ErrorState};

			Element()				:m_state(Ok),m_type(0),m_content(0),m_size(0) {}
			Element( const End&)			:m_state(EndOfInput),m_type(0),m_content(0),m_size(0) {}
			Element( const Element& orig)		:m_state(orig.m_state),m_type(orig.m_type),m_content(orig.m_content),m_size(orig.m_size) {}
			State state() const			{return m_state;}
			int type() const			{return m_type;}
			const char* content() const		{return m_content;}
			unsigned int size() const		{return m_size;}
		private:
			friend class iterator;
			State m_state;
			int m_type;
			const char* m_content;
			unsigned int m_size;
		};
		typedef Element value_type;
		typedef unsigned int difference_type;
		typedef Element* pointer;
		typedef Element& reference;
		typedef std::input_iterator_tag iterator_category;

	private:
		Element element;
		ThisXMLPathSelect* input;

		iterator& skip() throw(exception)
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
								if (input->scopestk.size() == 0)
								{
									element.m_state = Element::EndOfInput;
								}
								else
								{
									element.m_state = Element::ErrorState;
									element.m_content = XMLScannerBase::getErrorString( XMLScannerBase::ErrUnexpectedEndOfInput);
								}
								return *this;
							}
							if (et == XMLScannerBase::ErrorOccurred)
							{
								XMLScannerBase::Error err = input->scan.getError( &element.m_content);
								if (err == XMLScannerBase::ErrOutputBufferTooSmall)
								{
									element.m_state = Element::EndOfOutput;
								}
								else
								{
									element.m_state = Element::ErrorState;
								}
								return *this;
							}
							input->initProcessElement( et, input->scan.getItem(), input->scan.getItemSize());
						}
						element.m_type = input->fetch();

					} while (element.m_type == 0);

					element.m_content = input->context.key;
					element.m_size = input->context.keysize;
				}
				return *this;
			}
			catch (exception e)
			{
				throw exception( e.cause);
			};
			return *this;
		}
		bool compare( const iterator& iter) const
		{
			return (element.state() != Element::Ok && iter.element.state() != Element::Ok);
		}
	public:
		void assign( const iterator& orig)
		{
			input = orig.input;
			element = orig.element;
		}
		iterator( const iterator& orig)
		{
			assign( orig);
		}
		iterator( ThisXMLPathSelect& p_input)
				:input( &p_input)
		{
			skip();
		}
		iterator( const End& et)	:element(et),input(0) {}
		iterator()			:input(0) {}
		iterator& operator = (const iterator& orig)
		{
			assign( orig);
			return *this;
		}
		const Element& operator*()
		{
			return element;
		}
		const Element* operator->()
		{
			return &element;
		}
		iterator& operator++()	{return skip();}
		iterator operator++(int)	{iterator tmp(*this); skip(); return tmp;}

		bool operator==( const iterator& iter) const	{return compare( iter);}
		bool operator!=( const iterator& iter) const	{return !compare( iter);}
	};

	iterator begin()
	{
		return iterator( *this);
	}
	iterator end()
	{
		return iterator( End());
	}
};

} //namespace textwolf
#endif

