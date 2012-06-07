/*
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
*/
#ifndef __TEXTWOLF_TEXT_SCANNER_HPP__
#define __TEXTWOLF_TEXT_SCANNER_HPP__
#include "textwolf/char.hpp"
#include "textwolf/charset_interface.hpp"
#include "textwolf/exception.hpp"
#include <cstddef>

namespace textwolf {

///\class TextScanner
///\brief Reader for scanning the input character by character
///\tparam Iterator source iterator type (implements preincrement and '*' input byte access indirection)
///\tparam CharSet character set of the source stream
template <class Iterator, class CharSet>
class TextScanner
{
private:
	Iterator start;			//< source iterator start of current chunk
	Iterator input;			//< source iterator
	char buf[8];			//< buffer for one character (the current character parsed)
	UChar val;			//< Unicode character representation of the current character parsed
	char cur;			//< ASCII character representation of the current character parsed
	unsigned int state;		//< current state of the text scanner

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
			('\r',Space)
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
	TextScanner()
		:val(0),cur(0),state(0)
	{
		for (unsigned int ii=0; ii<sizeof(buf); ii++) buf[ii] = 0;
	}

	TextScanner( const Iterator& p_iterator)
		:start(p_iterator),input(p_iterator),val(0),cur(0),state(0)
	{
		for (unsigned int ii=0; ii<sizeof(buf); ii++) buf[ii] = 0;
	}

	///\brief Copy constructor
	///\param [in] orig textscanner to copy
	TextScanner( const TextScanner& orig)
		:start(orig.start)
		,input(orig.input)
		,val(orig.val)
		,cur(orig.cur)
		,state(orig.state)
	{
		for (unsigned int ii=0; ii<sizeof(buf); ii++) buf[ii]=orig.buf[ii];
	}

	///\brief Assign something to the iterator while keeping the state
	///\param [in] a source iterator assignment
	template <class IteratorAssignment>
	void setSource( const IteratorAssignment& a)
	{
		input = a;
		start = a;
	}

	///\brief Get the current source iterator position
	///\return source iterator position in character words (usually bytes)
	std::size_t getPosition() const
	{
		return input - start;
	}

	///\brief Get the unicode character of the current character
	///\return the unicode character
	UChar chr()
	{
		if (val == 0)
		{
			val = CharSet::value( buf, state, input);
		}
		return val;
	}

	///\brief Fill the internal buffer with as many current character bytes needed for reading the ASCII representation
	void getcur()
	{
		cur = CharSet::asciichar( buf, state, input);
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
		CharSet::skip( buf, state, input);
		state = 0;
		cur = 0;
		val = 0;
		return *this;
	}

	///\brief see TextScanner::chr()
	UChar operator*()
	{
		return chr();
	}

	///\brief Preincrement: Skip to the next character of the source
	///\return *this
	TextScanner& operator ++()	{return skip();}

	///\brief Postincrement: Skip to the next character of the source
	///\return *this
	TextScanner operator ++(int)	{TextScanner tmp(*this); skip(); return tmp;}
};

}//namespace
#endif
