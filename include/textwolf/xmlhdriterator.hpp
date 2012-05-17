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

///\namespace textwolf
///\brief Toplevel namespace of the library
namespace textwolf {

///\class HdrSrcIterator
///\tparam WrapIterator
///\brief Iterator wrapper that parses the header only
template <class WrapIterator>
class HdrSrcIterator
{
public:
	///\brief Constructor
	HdrSrcIterator( const WrapIterator& src_)
		:m_state(Left0)
		,m_src(src_)
		,m_cnt0(0){}

	///\brief Copy constructor
	///\brief param[in] o iterator to copy
	HdrSrcIterator( const HdrSrcIterator& o)
		:m_state(o.m_state)
		,m_src(o.m_src)
		,m_cnt0(o.m_cnt0){}

	///\brief Element access
	///\return current character
	char operator* ()
	{
		char ch;

		for (;;)
		{
			if (m_cnt0 >= 4) return 0;
			ch = *m_src;
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
						++m_src;
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
						++m_src;
						continue;
					}

				case Src:
					if (ch)
					{
						if (ch == '\n')
						{
							m_state = Rest;
							++m_src;
							complete();
						}
/*[-]*/std::cout << "CHAR '" << ch << "'" << std::endl;
						return ch;
					}
					else
					{
						++m_src;
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
	HdrSrcIterator& operator++()
	{
		if (m_state != End)
		{
			if (m_state == Rest)
			{
				m_state = End;
			}
			++m_src;
		}
		return *this;
	}

	///\brief Initialize a new source iterator while keeping the state
	///\param [in] p_iterator source iterator
	void setSource( const WrapIterator& src_)
	{
		m_src = src;
	}

	const WrapIterator& src() const		{return m_src;}

	bool complete()
	{
		if (m_state < Rest)
		{
			return false;
		}
		while (m_cnt0 > 0)
		{
			--m_cnt0;
			++m_src;
			char ch = *m_src;
			if (!ch) return false;
		}
		m_state = End;
		return true;
	}

private:
	enum State
	{
		Left0,
		Right0,
		Src,
		Rest,
		End
	};
	State m_state;
	WrapIterator m_src;		//< source iterator
	std::size_t m_cnt0;		//< counter of 0
};

}//namespace
#endif

