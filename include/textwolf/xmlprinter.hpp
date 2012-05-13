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
///\file textwolf/xmlprinter.hpp
///\brief textwolf XML printer interface hiding character encoding properties

#ifndef __TEXTWOLF_XML_PRINTER_HPP__
#define __TEXTWOLF_XML_PRINTER_HPP__
#include "textwolf.hpp"
#include "textwolf/xmlparser.hpp"
#include "textwolf/xmltagstack.hpp"
#include "textwolf/striterator.hpp"
#include <cstring>
#include <cstdlib>

///\namespace textwolf
///\brief Toplevel namespace of the library
namespace textwolf {

///\brief Class for XML printing to a buffer
///\tparam BufferType type to use as buffer (STL back insertion interface)
template <class BufferType>
class XMLPrinter :public CharsetEncodingCallBack
{
private:
	///\class FilterBase
	///\brief Filter base template
	///\tparam IOCharset Character set encoding of input and output
	///\tparam AppCharset Character set encoding of the application processor
	///\tparam BufferType STL back insertion sequence to use for printing output
	template <class IOCharset, class AppCharset, class BufferType>
	struct Base
	{
		Base()
			:m_state(Content){}

		Base( const Base& o)
			:m_state(o.m_state),m_buf(o.m_buf),m_tagstack(o.m_tagstack){}

		///\brief Prints a character string to an STL back insertion sequence buffer in the IO character set encoding
		///\param [in] src pointer to string to print
		///\param [in] srcsize size of src in bytes
		void printToBuffer( const char* src, std::size_t srcsize, BufferType& buf)
		{
			StrIterator itr( src, srcsize);
			textwolf::TextScanner<StrIterator,AppCharset> ts( itr);

			textwolf::UChar ch;
			while ((ch = ts.chr()) != 0)
			{
				IOCharset::print( ch, buf);
				++ts;
			}
		}

		void printHeader( BufferType& buf)
		{
			printToBuffer( "<?xml version=\"1.0\" encoding=\"", 30, buf);
			printToBuffer( m_encoding.c_str(), m_encoding.size(), buf);
			printToBuffer( "\" standalone=\"yes\"?>", 20, buf);
		}

		void printOpenTag( const char* src, std::size_t srcsize, BufferType& buf)
		{
			if (m_pendingOpenTag == true)
			{
				printToBuffer( '>', buf);
			}
			printToBuffer( '<', buf);
			printToBuffer( (const char*)element, elementsize, buf);

			m_tagstack.push( element, elementsize);
			m_pendingOpenTag = true;
		}

		bool printAttribute( const char* src, std::size_t srcsize, BufferType& buf)
		{
			if (m_pendingOpenTag)
			{
				printToBuffer( ' ', buf);
				printToBuffer( (const char*)element, elementsize, buf);
				printToBuffer( '=', buf);
				m_attributeContext = true;
				return true;
			}
			else
			{
				return false;
			}
		 }

		bool printAttributeValue( const char* src, std::size_t srcsize, BufferType& buf)
		{
			if (m_pendingOpenTag)
			{
				printToBuffer( ' ', buf);
				printToBufferAttributeValue( (const char*)element, elementsize, buf);
				m_attributeContext = false;
				return true;
			}
			else
			{
				return false;
			}
		 }

		void printValue( const char* src, std::size_t srcsize, BufferType& buf)
		{
			if (m_pendingOpenTag == true)
			{
				printToBuffer( '>', buf);
			}
			printToBuffer( ' ', buf);
			printToBuffer( (const char*)element, elementsize, buf);
			m_attributeContext = false;
			m_pendingOpenTag = false;
		 }

		bool printCloseTag( BufferType& buf)
		{
			if (topTag( cltag, cltagsize) || !cltagsize)
			{
				return false;
			}
			if (m_pendingOpenTag == true)
			{
				printToBuffer( '/', buf);
				printToBuffer( '>', buf);
			}
			else
			{
				printToBuffer( '<', buf);
				printToBuffer( '/', buf);
				printToBuffer( (const char*)cltag, cltagsize, buf);
				printToBuffer( '>', buf);
			}
			m_pendingOpenTag = false;
			m_attributeContext = false;
			popTag();
			return true;
		}

		///\brief Prints an end of line marker (EOL) to an STL back insertion sequence buffer in the IO character set encoding
		///\param [in,out] buf buffer to print to
		void printToBufferEOL()
		{
			static const char* str =  protocol::EndOfLineMarker::value();
			static unsigned int len = protocol::EndOfLineMarker::size();
			printToBuffer( str, len, m_buf);
		}

		///\brief Prints a character to an STL back insertion sequence buffer in the IO character set encoding
		///\param [in] ch character to print
		///\param [in,out] buf buffer to print to
		void printToBuffer( char ch)
		{
			IOCharset::print( (textwolf::UChar)(unsigned char)ch, m_buf);
		}
	private:
		enum State
		{
			Content,
			TagAttribute,
			TagValue
		};
		State m_state;
		BufferType m_buf;
		TagStack m_tagstack;
	};

	typedef bool (*PrintProc)( void* obj, XMLScannerBase::ElementType elemtype, const char* elemptr, std::size_t elemsize);
	typedef void (*DeleteObj)( void* obj);

	struct MethodTable
	{
		PrintProc m_proc;
		DeleteObj m_del;

		MethodTable() :m_proc(0),m_del(0){}
	};

	template <class IOCharset, class AppCharset>
	struct Object
	{
		typedef Base<IOCharset,AppCharset,BufferType> This;
		static void* createObj()
		{
			return new This();
		}

		static void* copyObj()
		{
			This* obj = (This*)obj_;
			rt = new This( *obj);
			return rt;
		}

		static void* printProc( void* obj_, BufferType& buf)
		{
			This* obj = (This*)obj_;
		}

		static void deleteObj( void* obj)
		{
			delete (This*)obj;
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
			mt.m_proc = printProc;
			mt.m_del = deleteObj;
			obj = createObj( src, buf);
		}

		static void copy( SrcIterator& src, BufferType& buf, MethodTable& mt, void*& obj)
		{
			mt.m_proc = printProc;
			mt.m_del = deleteObj;
			obj = copyObj( obj, src, buf);
		}
	};
public:
	XMLPrinter()
	{
	}

	XMLPrinter( const XMLParser& o)
	{
	}

	~XMLPrinter()
	{
		m_del( m_obj);
	}

	virtual bool setEncoding( const std::string& encoding)
	{
		m_encoding = encoding;
		std::string enc;
		parseEncoding( enc, m_encoding);

		if (enc.size() >= 8 && std::memcmp( enc.c_str(), "isolatin")
		||  enc.size() >= 7 && std::memcmp( enc.c_str(), "iso8859"))
		{
			Object<charset::IsoLatin1,charset::UTF8>::create( m_proc, m_del, m_obj);
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
				return false;
			}
		}
	}

	const std::string& getEncoding() const
	{
		return m_encoding;
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
	bool m_headerPrinted;				//< true, if header has already been printed
	BufferType m_buf;				//< element buffer
	MethodTable m_mt;				//< method table of m_obj
	void* m_obj;					//< pointer to parser objecct
	std::string m_encoding;				//< xml encoding
};

} //namespace
#endif
