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
///\file textwolf/xmlparser.hpp
///\brief textwolf XML parser interface hiding character encoding properties

#ifndef __TEXTWOLF_XML_PARSER_HPP__
#define __TEXTWOLF_XML_PARSER_HPP__
#include "textwolf.hpp"
#include <cstring>
#include <cstdlib>

///\namespace textwolf
///\brief Toplevel namespace of the library
namespace textwolf {

///\class HdrSrcIterator
///\tparam SrcIterator
///\brief Iterator wrapper that parses the header only
template <class SrcIterator>
class HdrSrcIterator
{
public:
	///\brief Constructor
	HdrSrcIterator( const SrcIterator& src_)
		:m_state(Left0)
		,m_src(src_)
		,m_restc(0)
		,m_cnt0(0)
		,m_error(Ok){}

	///\brief Copy constructor
	///\brief param[in] o iterator to copy
	HdrSrcIterator( HdrSrcIterator& o)
		:m_state(o.m_state)
		,m_src(o.m_src)
		,m_restc(o.m_restc)
		,m_cnt0(o.m_cnt0)
		,m_error(o.m_error){}

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
					complete();
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

	enum Error
	{
		Ok,
		ErrIllegalState,
		ErrIllegalCharacterAtEndOfHeader
	};

	const char* getError()
	{
		static const char ar[] = {"", "illegal xml header", "broken character in header"};
		return ar[(int)m_error];
	}

	///\brief Get the error state
	bool hasError() const			{return m_error!=Ok;}

	///\brief Initialize a new source iterator while keeping the state
	///\param [in] p_iterator source iterator
	void setSource( const SrcIterator& src_)
	{
		m_src = src;
	}

	const SrcIterator& src() const		{return m_src;}

	void complete()
	{
		if (m_state != Rest)
		{
			m_error = ErrIllegalState;
		}
		while (m_restc > 0)
		{
			char ch = *m_src;
			if (!ch) m_error = ErrIllegalCharacterAtEndOfHeader;
			++src;
			--m_restc;
		}
	}

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

///\brief Class for XML parsing independent of the character set
///\tparam BufferType type to use as buffer (STL back insertion interface)
///\tparam SrcIterator iterator on the scanned source
template <class SrcIterator, class BufferType>
class XMLParser
{
private:
	typedef XMLScannerBase::ElementType (*GetNextProc)( void* obj, const char*& elemptr, std::size_t& elemsize);
	typedef void (*DeleteObj)( void* obj);
	typedef void (*SetFlag)( void* obj);

	struct MethodTable
	{
		GetNextProc m_proc;
		DeleteObj m_del;
		SetFlag m_setflag;

		MethodTable() :m_proc(0),m_del(0),m_setflag(0){}
	};

	template <class SrcIterator, class IOCharset, class AppCharset>
	struct Object
	{
		static void* createObj( SrcIterator& src, BufferType& buf)
		{
			return new XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>( src, buf);
		}

		static void* copyObj( void* obj_, SrcIterator& src, BufferType& buf)
		{
			XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>* obj = (XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>*)obj_;
			XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>* obj;
			rt = new XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>( *obj);
			rt->seSource( src);
			rt->setOutputBuffer( buf);
			return rt;
		}

		static void deleteObj( void* obj)
		{
			delete (XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>*)obj;
		}

		enum Flag
		{
			doTokenize
		};

		static bool setFlag( void* obj_, Flag flag, bool value)
		{
			XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>* obj = (XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>*)obj_;
			switch (flag)
			{
				obj->doTokenize( value);
			}
		}

		static XMLScannerBase::ElementType getNextProc( void* obj_, const char*& elemptr, std::size_t& elemsize)
		{
			XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>* obj = (XMLScanner<SrcIterator,IOCharset,AppCharset,BufferType>*)obj_;
			XMLScannerBase::ElementType rt = obj->nextItem();
			elemptr = obj->getItem();
			elemsize = obj->getItemSize();
			return rt;
		}

		static void create( SrcIterator& src, BufferType& buf, MethodTable& mt, void*& obj)
		{
			mt.m_proc = getNextProc;
			mt.m_del = deleteObj;
			mt.m_setflag = setFlag;
			obj = createObj( src, buf);
		}

		static void copy( SrcIterator& src, BufferType& buf, MethodTable& mt, void*& obj)
		{
			mt.m_proc = getNextProc;
			mt.m_del = deleteObj;
			mt.m_setflag = setFlag;
			obj = copyObj( obj, src, buf);
		}
	};
public:
	struct CharsetEncodingCallBack
	{
		virtual bool setEncoding( const std::string&){}
	};

public:
	XMLParser( SrcIterator* src, CharsetEncodingCallBack* encodingCallBack=0)
		:m_state(ParseHeader)
		,m_hdrsrc(src)
		,m_src(src)
		,m_obj(0)
		,m_attrEncoding(false)
		,m_withEmpty(true)
		,m_doTokenize(false)
		,m_interrupted(false)
		,m_encodingCallBack(encodingCallBack)
	{
		m_obj = Object<HdrSrcIterator,charset::UTF8,charset::UTF8>::create( m_hdrsrc, m_buf, m_mt);
	}

	XMLParser( const XMLParser& o)
		:m_state(o.m_state)
		,m_hdrsrc(o.m_hdrsrc)
		,m_src(o.m_src)
		,m_buf(o.m_buf)
		,m_obj(0)
		,m_attrEncoding(o.m_attrEncoding)
		,m_withEmpty(o.m_withEmpty)
		,m_doTokenize(o.m_doTokenize)
		,m_interrupted(o.m_interrupted)
		,m_encodingCallBack(o.m_encodingCallBack)
	{
		m_obj = Object<HdrSrcIterator,charset::UTF8,charset::UTF8>::copy( m_hdrsrc, m_buf, m_mt, o.m_obj);
	}

	~XMLParser()
	{
		m_del( m_obj);
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

	const std::string& encoding() const
	{
		return m_encoding;
	}

	bool withEmpty()
	{
		return m_withEmpty;
	}

	bool doTokenize()
	{
		return m_doTokenize;
	}

	bool withEmpty( bool val)
	{
		bool rt = m_withEmpty;
		m_withEmpty = val;
		return rt;
	}

	bool doTokenize( bool val)
	{
		bool rt = m_doTokenize;
		if (m_mt) m_mt->setFlag( m_obj, Object::doTokenize, m_doTokenize = val);
		return rt;
	}

	XMLScannerBase::ElementType getNext( const char*& elemptr, std::size_t& elemsize)
	{
		if (!m_interrupted)
		{
			m_buf.clear();
		}
		else
		{
			m_interrupted = true;
		}
		for(;;)
		{
			XMLScannerBase::ElementType elemtype = m_proc( m_obj, elemptr, elemsize);
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
							m_encoding.clear();
							m_encoding.append( elemptr, elemsize);
						break;
						case HeaderEnd:
						{
							std::string enc;
							parseEncoding( enc, m_encoding);

							m_hdrsrc.complete();

							if (m_hdrsrc.hasError())
							{
								elemptr = m_hdrsrc.getError();
								elemsize = std::strlen( elemptr);
								elemtype = ErrorOccurred;
							}
							if (m_encodingCallBack)
							{
								m_encodingCallBack->setEncoding( m_encoding);
							}
							if (enc.size() >= 8 && std::memcmp( enc.c_str(), "isolatin")
							||  enc.size() >= 7 && std::memcmp( enc.c_str(), "iso8859"))
							{
								Object<SrcIterator,charset::IsoLatin1,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else if (enc.size() == 0 || enc == "utf8")
							{
								Object<SrcIterator,charset::UTF8,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else if (enc == "utf16" || enc == "utf16be")
							{
								Object<SrcIterator,charset::UTF16BE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else if (enc == "utf16le")
							{
								Object<SrcIterator,charset::UTF16LE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else if (enc == "ucs2" || enc == "ucs2be")
							{
								Object<SrcIterator,charset::UCS2BE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else if (enc == "ucs2le")
							{
								Object<SrcIterator,charset::UCS2LE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else if (enc == "ucs4" || enc == "ucs4be")
							{
								Object<SrcIterator,charset::UCS4BE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else if (enc == "ucs4le")
							{
								Object<SrcIterator,charset::UCS4LE,charset::UTF8>::create( m_src, *m_buf, m_proc, m_del, m_obj);
							}
							else
							{
								elemptr = "unknown charset encoding";
								elemsize = std::strlen( elemptr);
								elemtype = ErrorOccurred;
							}
							m_src.setSource( m_hdr.src());
							m_state = ParseSource;
						}
						break;
						default:
							elemptr = "unexpected element in xml header";
							elemsize = std::strlen( elemptr);
							elemtype = ErrorOccurred;
					}
				break;
				case ParseSource:
					if (elemtype == Content && !m_withEmpty)
					{
						std::size_t ii=0;
						const unsigned char* cc = (const unsigned char*)elemptr;
						for (;ii<elemsize && cc[ii] <= ' '; ++ii);
						if (ii==elemsize)
						{
							m_buf.clear();
							continue;
						}
					}
				break;
			}
			m_interrupted = false;
			return elemtype;
		}
	}

private:
	static void parseEncoding( std::string& dest, const std::string& src)
	{
		std::size_t kk;
		dest.clear();
		std::string::const_iterator cc=src.begin();
		for (; cc != src.end(); ++cc)
		{
			if (*cc <= ' ') continue;
			if (*cc == '-') continue;
			if (*cc == ' ') continue;
			dest.push_back( ::tolower( *cc));
		}
	}
private:
	enum State
	{
		ParseHeader,				//< parsing the XML header section
		ParseSource				//< parsing the XML content section
	};
	State m_state;					//< parser section parsing state
	HdrSrcIterator m_hdrsrc;			//< source iterator for header
	SrcIterator m_src;				//< source iterator for content
	BufferType m_buf;				//< element buffer
	MethodTable m_mt;				//< method table of m_obj
	void* m_obj;					//< pointer to parser objecct
	bool m_attrEncoding;				//< flag used in state 'ParseHeader': true, if last atrribute parsed was 'encoding'
	std::string m_encoding;				//< xml encoding
	bool m_withEmpty;				//< do produce empty tokens (containing only spaces)
	bool m_doTokenize;				//< do tokenize (whitespace sequences as delimiters)
	bool m_interrupted;				//< true, if getNext hat been interrupted by an end of message last time
	CharsetEncodingCallBack* m_encodingCallBack;	//< defines a method to call when the XML encoding has been detected
};

} //namespace
#endif
