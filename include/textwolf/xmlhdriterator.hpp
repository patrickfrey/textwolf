/*
---------------------------------------------------------------------
    The template library textwolf implements an input iterator on
    a set of XML path expressions without backward references on an
    STL conforming input iterator as source. It does no buffering
    or read ahead and is dedicated for stream processing of XML
    for a small set of XML queries.
    Stream processing in this Object refers to processing the
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
///\file textwolf/xmlhdriterator.hpp
///\brief textwolf XML source iterator template for parsing the header

#ifndef __TEXTWOLF_XML_HEADER_ITERATOR_HPP__
#define __TEXTWOLF_XML_HEADER_ITERATOR_HPP__
#include <cstdlib>
#include "textwolf/sourceiterator.hpp"

///\namespace textwolf
///\brief Toplevel namespace of the library
namespace textwolf {

///\class XmlHdrIterator
///\brief Iterator that parses the header character stream without the NUL characters
class XmlHdrSrcIterator :public SrcIterator
{
public:
	///\brief Constructor
	XmlHdrSrcIterator()
		:m_state(Left0)
		,m_cnt0(0){}

	///\brief Copy constructor
	///\brief param[in] o iterator to copy
	XmlHdrSrcIterator( const XmlHdrSrcIterator& o)
		:SrcIterator(o)
		,m_state(o.m_state)
		,m_cnt0(o.m_cnt0){}

	///\brief Constructor
	///\param [in] buf source chunk to iterate on
	///\param [in] size size of source chunk to iterate on in bytes
	///\param [in] eof true, if end of data has been reached (no next chunk anymore)
	XmlHdrSrcIterator( const char* buf, std::size_t size, bool eof)
		:SrcIterator(buf,size,eof){}

	///\brief Element access
	///\return current character
	char operator* ()
	{
		char ch;

		for (;;)
		{
			if (m_cnt0 >= 4) return 0;
			ch = cur();
			switch (m_state)
			{
				case Left0:
					if (ch)
					{
						if (m_cnt0)
						{
							m_state = Src;
							m_cnt0 = 0;
						}
						else
						{
							m_state = Right0;
						}
						return ch;
					}
					else
					{
						++m_cnt0;
						skip();
						continue;
					}

				case Right0:
					if (ch)
					{
						m_state = Src;
						return ch;
					}
					else
					{
						++m_cnt0;
						skip();
						continue;
					}

				case Src:
					if (ch)
					{
						if (ch == '\n')
						{
							m_state = Rest;
							skip();
							complete();
						}
						return ch;
					}
					else
					{
						skip();
						continue;
					}

				case Rest:
					complete();
				case End:
					return 0;
			}
		}
	}

	///\brief Preincrement
	XmlHdrSrcIterator& operator++()
	{
		if (m_state != End)
		{
			if (m_state == Rest)
			{
				m_state = End;
			}
			skip();
		}
		return *this;
	}

	bool complete()
	{
		if (m_state < Rest)
		{
			return false;
		}
		while (m_cnt0 > 0)
		{
			char ch = cur();
			if (ch) return false;
			--m_cnt0;
			skip();
		}
		m_state = End;
		return true;
	}

private:
	char cur()	{return SrcIterator::operator*();}
	void skip()	{SrcIterator::operator++();}

	enum State
	{
		Left0,
		Right0,
		Src,
		Rest,
		End
	};
	State m_state;			//< header parsing state
	std::size_t m_cnt0;		//< counter of 0
};

}//namespace
#endif

