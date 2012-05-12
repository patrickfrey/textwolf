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
///\file textwolf/xmlparser.hpp
///\brief textwolf XML parser interface hiding character encoding properties

#ifndef __TEXTWOLF_XMLPARSER_HPP__
#define __TEXTWOLF_XMLPARSER_HPP__
#include "textwolf.hpp"
#include <cstring>
#include <cstdlib>

///\namespace textwolf
///\brief Toplevel namespace of the library
namespace textwolf {

template <class SrcIterator>
class HdrSrcIterator
{
public:
	///\brief Constructor
	HdrSrcIterator( SrcIterator& src_)
		:m_state(Left0),m_src(src_),m_restc(0),m_cnt0(0),m_error(false){}

	///\brief Copy constructor
	///\brief param[in] o iterator to copy
	HdrSrcIterator( HdrSrcIterator& o)
		:m_state(o.m_state),m_src(o.m_src),m_restc(o.m_restc),m_cnt0(o.m_cnt0),m_error(o.m_error){}

	///\brief Element access
	///\return current character
	char operator* ()
	{
		char ch;
		for (;;)
		{
			ch = *m_src;
			switch (m_state)
			{
				case Left0:
					if (ch)
					{
						m_state = m_cnt0?Src:Right0;
						m_cnt0 = 0;
						return ch;
					}
					break;

				case Right0:
					if (ch)
					{
						m_state = Src;
						m_restc = m_cnt0;
						m_cnt0 = 0;
						return ch;
					}
					break;

				case Src:
					if (ch)
					{
						if (ch == '>') m_state = Rest;
						return ch;
					}

				case Rest:
					while (m_restc > 0)
					{
						char ch = *m_src;
						if (!ch) m_error = true;
						++src;
						--m_restc;
					}
					return 0;
			}
			++m_cnt0;
			if (m_cnt0 == 4) return 0;
			++m_src;
		}

	}

	///\brief Preincrement
	StrIterator& operator++()
	{
		++m_src;
		return *this;
	}

	///\brief Get the error state
	bool error() const			{return m_error;}

	///\brief Initialize a new source iterator while keeping the state
	///\param [in] p_iterator source iterator
	void setSource( const SrcIterator& src_)
	{
		m_src = src;
	}

	const SrcIterator& src() const		{return m_src;}

private:
	enum State
	{
		Left0,
		Right0,
		Src,
		Rest
	};
	State m_state;
	SrcIterator m_src;		//< source iterator
	std::size_t m_restc;		//< rest zero-bytes after header to consume
	std::size_t m_cnt0;		//< counter of 0
	bool m_error;			//< error encountered (0 expected at end of header)
};

template <class SrcIterator>
class XMLParser
{
private:
	typedef XMLScannerBase::ElementType (*GetNextProc)( void* ctx, const char*& elemptr, std::size_t& elemsize);
	typedef void (*deleteCtx)( void* ctx);

	template <class SrcIterator, class IOCharset, class AppCharset>
	struct Context
	{
		static void* createCtx( SrcIterator& src, std::string& buf)
		{
			return new XMLScanner<SrcIterator,IOCharset,AppCharset,std::string>( src, buf);
		}

		static void deleteCtx( void* ctx)
		{
			delete (XMLScanner<SrcIterator,IOCharset,AppCharset,std::string>*)ctx;
		}

		static XMLScannerBase::ElementType getNextProc( void* ctx, const char*& elemptr, std::size_t& elemsize)
		{
			XMLScanner<SrcIterator,IOCharset,AppCharset,std::string>* ctxi = (XMLScanner<SrcIterator,IOCharset,AppCharset,std::string>*)ctx;
			XMLScannerBase::ElementType rt = ctxi->nextItem();
			elemptr = ctxi->getItem();
			elemsize = ctxi->getItemSize();
			return rt;
		}

		static void create( SrcIterator& src, std::string& buf, GetNextProc& proc, deleteCtx del, void* ctx)
		{
			proc = getNextProc;
			del = deleteCtx;
			ctx = createCtx( src, buf);
		}
	};
public:
	XMLParser( SrcIterator& src, std::string& buf)
		:m_state(ParseHeader),m_hdrsrc(src),m_src(src),m_buf(&buf),m_proc(0),m_del(0),m_ctx(0),m_attrEncoding(false)
	{
		Context<HdrSrcIterator,charset::UTF8,charset::UTF8>::create( m_hdrsrc, *m_buf, m_proc, m_del, m_ctx);
	}

	~XMLParser()
	{
		m_del( m_ctx);
	}

	void setSource( const SrcIterator& src_)
	{
		switch (m_state)
		{
			case ParseHeader:
				m_hdrsrc.setSource( src_);
			break;
			case ParseSource:
				m_src.setSource( src_);
			break;
		}
	}

	XMLScannerBase::ElementType getNext( const char*& elemptr, std::size_t& elemsize)
	{
		XMLScannerBase::ElementType elemtype = m_proc( m_ctx, elemptr, elemsize);
		switch (m_state)
		{
			case ParseHeader:
				switch (elemtype)
				{
					case HeaderStart:
					break;
					case HeaderAttribName:
						m_attrEncoding = (elemsize == 8 && std::memcmp( elemptr, "encoding", 8) == 0);
					break;
					case HeaderAttribValue:
						if (m_attrEncoding) parseEncoding( elemptr, elemsize);
					break;
					case HeaderEnd:
						if (m_encoding.size() >= 8 && std::memcmp( m_encoding.c_str(), "isolatin")
						||  m_encoding.size() >= 7 && std::memcmp( m_encoding.c_str(), "iso8859"))
						{
							Context<SrcIterator,charset::IsoLatin1,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						else if (m_encoding.size() == "utf8")
						{
							Context<SrcIterator,charset::UTF8,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						else if (m_encoding.size() == "utf16" || m_encoding.size() == "utf16be")
						{
							Context<SrcIterator,charset::UTF16BE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						else if (m_encoding.size() == "utf16le")
						{
							Context<SrcIterator,charset::UTF16LE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						else if (m_encoding.size() == "ucs2" || m_encoding.size() == "ucs2be")
						{
							Context<SrcIterator,charset::UCS2BE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						else if (m_encoding.size() == "ucs2le")
						{
							Context<SrcIterator,charset::UCS2LE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						else if (m_encoding.size() == "ucs4" || m_encoding.size() == "ucs4be")
						{
							Context<SrcIterator,charset::UCS4BE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						else if (m_encoding.size() == "ucs4le")
						{
							Context<SrcIterator,charset::UCS4LE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_ctx);
						}
						m_src.setSource( m_hdr.src());
						m_state = ParseSource;
					break;
					default:
						elemptr = "unexpected element in xml header";
						elemsize = std::strlen( elemptr);
						elemtype = ErrorOccurred;
				}
			break;
			case ParseSource:
			break;
		}
		return elemtype;
	}

private:
	void parseEncoding( const char* av, std::size_t avsize)
	{
		std::size_t kk;
		for (kk=0; kk<avsize; kk++)
		{
			if (av[kk] <= ' ') continue;
			if (av[kk] == '-') continue;
			if (av[kk] == ' ') continue;
			m_encoding.push_back( ::tolower( av[kk]));
		}
	}
private:
	enum State
	{
		ParseHeader,
		ParseSource
	};
	State m_state;
	HdrSrcIterator m_hdrsrc;
	SrcIterator m_src;
	std::string* m_buf;
	GetNextProc m_proc;
	deleteCtx m_del;
	void* m_ctx;
	bool m_attrEncoding;
	std::string m_encoding;
};

} //namespace
#endif
