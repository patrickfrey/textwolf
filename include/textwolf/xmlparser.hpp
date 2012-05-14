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
#include "textwolf/charset_utf8.hpp"
#include "textwolf/charset_utf16.hpp"
#include "textwolf/charset_ucs.hpp"
#include "textwolf/charset_isolatin.hpp"
#include "textwolf/xmlscanner.hpp"
#include "textwolf/xmlhdriterator.hpp"
#include "textwolf/xmlattributes.hpp"
#include <cstring>
#include <cstdlib>

///\namespace textwolf
///\brief Toplevel namespace of the library
namespace textwolf {

struct XMLParserBase
{
	enum Flag
	{
		DoTokenize
	};

	typedef XMLScannerBase::ElementType (*GetNextProc)( void* obj, const char*& elemptr, std::size_t& elemsize);
	typedef void (*DeleteObj)( void* obj);
	typedef void (*SetFlag)( void* obj, Flag f, bool value);

	struct MethodTable
	{
		GetNextProc m_getNext;
		DeleteObj m_del;
		SetFlag m_setflag;

		MethodTable() :m_getNext(0),m_del(0),m_setflag(0){}
	};

	static void parseEncoding( std::string& dest, const std::string& src)
	{
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
};

template <class ObjSrcIterator, class BufferType, class IOCharset, class AppCharset>
struct XMLParserObject
{
	typedef XMLScanner<ObjSrcIterator,IOCharset,AppCharset,BufferType> This;

	static void* createObj( ObjSrcIterator& src, BufferType& buf)
	{
		return new This( src, buf);
	}

	static void* copyObj( void* obj_, ObjSrcIterator& src, BufferType& buf)
	{
		This* obj = (This*)obj_;
		This* rt = new This( *obj);
		rt->setSource( src);
		rt->setOutputBuffer( buf);
		return rt;
	}

	static void deleteObj( void* obj)
	{
		delete (This*)obj;
	}

	static void setFlag( void* obj_, XMLParserBase::Flag flag, bool value)
	{
		This* obj = (This*)obj_;
		switch (flag)
		{
			case XMLParserBase::DoTokenize: obj->doTokenize( value); break;
		}
	}

	static XMLScannerBase::ElementType getNextProc( void* obj_, const char*& elemptr, std::size_t& elemsize)
	{
		This* obj = (This*)obj_;
		XMLScannerBase::ElementType rt = obj->nextItem();
		elemptr = obj->getItem();
		elemsize = obj->getItemSize();
		return rt;
	}

	static void* create( ObjSrcIterator& src, BufferType& buf, XMLParserBase::MethodTable& mt)
	{
		mt.m_getNext = getNextProc;
		mt.m_del = deleteObj;
		mt.m_setflag = setFlag;
		return createObj( src, buf);
	}

	static void* copy( ObjSrcIterator& src, BufferType& buf, XMLParserBase::MethodTable& mt, void* obj)
	{
		mt.m_getNext = getNextProc;
		mt.m_del = deleteObj;
		mt.m_setflag = setFlag;
		return copyObj( obj, src, buf);
	}
};

///\brief Class for XML parsing independent of the character set
///\tparam SrcIterator iterator on the scanned source
///\tparam BufferType type to use as buffer (STL back insertion interface)
///\tparam XMLAttributes setter/getter class for document attributes
template <class SrcIterator, class BufferType, class XMLAttributes=DefaultXMLAttributes>
class XMLParser :public XMLParserBase
{
public:
	XMLParser( const SrcIterator& src, const XMLAttributes& a)
		:m_state(ParseHeader)
		,m_hdrsrc(src)
		,m_src(src)
		,m_obj(0)
		,m_attrEncoding(false)
		,m_withEmpty(true)
		,m_doTokenize(false)
		,m_interrupted(false)
		,m_attributes(a)
	{
		m_obj = XMLParserObject<HdrSrcIterator<SrcIterator>,BufferType,charset::UTF8,charset::UTF8>::create( m_hdrsrc, m_buf, m_mt);
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
		,m_attributes(o.m_attributes)
	{
		m_obj = XMLParserObject<HdrSrcIterator<SrcIterator>,BufferType,charset::UTF8,charset::UTF8>::copy( m_hdrsrc, m_buf, m_mt, o.m_obj);
	}

	~XMLParser()
	{
		m_mt.m_del( m_obj);
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
		if (m_obj) m_mt.m_setflag( m_obj, DoTokenize, m_doTokenize = val);
		return rt;
	}

	XMLScannerBase::ElementType getNext( const char*& elemptr, std::size_t& elemsize)
	{
		if (!m_obj) return XMLScannerBase::ErrorOccurred;
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
			XMLScannerBase::ElementType elemtype = m_mt.m_getNext( m_obj, elemptr, elemsize);
			switch (m_state)
			{
				case ParseHeader:
					switch (elemtype)
					{
						case XMLScannerBase::HeaderStart:
						break;
						case XMLScannerBase::HeaderAttribName:
							m_attrEncoding = (elemsize == 8 && std::memcmp( elemptr, "encoding", 8) == 0);
						break;
						case XMLScannerBase::HeaderAttribValue:
							if (m_attrEncoding)
							{
								std::string enc( elemptr, elemsize);
								m_attributes.setEncoding( enc);
							}
						break;
						case XMLScannerBase::HeaderEnd:
						{
							m_hdrsrc.complete();
							if (m_hdrsrc.hasError())
							{
								elemptr = m_hdrsrc.getError();
								elemsize = std::strlen( elemptr);
								elemtype = XMLScannerBase::ErrorOccurred;
								break;
							}
							if (!createParser())
							{
								elemptr = "unknown charset encoding";
								elemsize = std::strlen( elemptr);
								elemtype = XMLScannerBase::ErrorOccurred;
							}
							m_state = ParseSource;
						}
						break;
						default:
							elemptr = "unexpected element in xml header";
							elemsize = std::strlen( elemptr);
							elemtype = XMLScannerBase::ErrorOccurred;
					}
				break;
				case ParseSource:
					if (elemtype == XMLScannerBase::Content && !m_withEmpty)
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
	bool createParser()
	{
		std::string enc;
		parseEncoding( enc, m_attributes.getEncoding());

		if (m_obj)
		{
			m_mt.m_del( m_obj);
			m_obj = 0;
		}
		if ((enc.size() >= 8 && std::memcmp( enc.c_str(), "isolatin", enc.size())== 0)
		||  (enc.size() >= 7 && std::memcmp( enc.c_str(), "iso8859", enc.size()) == 0))
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::IsoLatin1,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else if (enc.size() == 0 || enc == "utf8")
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::UTF8,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else if (enc == "utf16" || enc == "utf16be")
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::UTF16BE,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else if (enc == "utf16le")
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::UTF16LE,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else if (enc == "ucs2" || enc == "ucs2be")
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::UCS2BE,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else if (enc == "ucs2le")
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::UCS2LE,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else if (enc == "ucs4" || enc == "ucs4be")
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::UCS4BE,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else if (enc == "ucs4le")
		{
			m_obj = XMLParserObject<SrcIterator,BufferType,charset::UCS4LE,charset::UTF8>::create( m_src, m_buf, m_mt);
		}
		else
		{
			return false;
		}
	}
private:
	enum State
	{
		ParseHeader,			//< parsing the XML header section
		ParseSource			//< parsing the XML content section
	};
	State m_state;				//< parser section parsing state
	HdrSrcIterator<SrcIterator> m_hdrsrc;	//< source iterator for header
	SrcIterator m_src;			//< source iterator for content
	BufferType m_buf;			//< element buffer
	MethodTable m_mt;			//< method table of m_obj
	void* m_obj;				//< pointer to parser objecct
	bool m_attrEncoding;			//< flag used in state 'ParseHeader': true, if last atrribute parsed was 'encoding'
	bool m_withEmpty;			//< do produce empty tokens (containing only spaces)
	bool m_doTokenize;			//< do tokenize (whitespace sequences as delimiters)
	bool m_interrupted;			//< true, if getNext hat been interrupted by an end of message last time
	XMLAttributes m_attributes;		//< defines a method to call when the XML encoding has been detected
};

} //namespace
#endif
