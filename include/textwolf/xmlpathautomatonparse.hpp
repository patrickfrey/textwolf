/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \file textwolf/xmlpathautomatonparse.hpp
/// \brief Parser to create a path expression selector automaton from a source (list of path expression in abbreviated syntax of xpath)

#ifndef __TEXTWOLF_XML_PATH_AUTOMATON_PARSE_HPP__
#define __TEXTWOLF_XML_PATH_AUTOMATON_PARSE_HPP__
#include "textwolf/xmlpathautomaton.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/cstringiterator.hpp"
#include <limits>
#include <string>
#include <cstdlib>
#include <list>
#include <vector>
#include <cstring>
#include <cstddef>
#include <stdexcept>

namespace textwolf {

///\class XMLPathSelectAutomatonParser
///\tparam SrcCharSet character set of the automaton definition source
///\tparam AtmCharSet character set of the token defintions of the automaton
///\brief Automaton to define XML path expressions and assign types (int values) to them
template <class SrcCharSet=charset::UTF8, class AtmCharSet=charset::UTF8>
class XMLPathSelectAutomatonParser :public XMLPathSelectAutomaton<AtmCharSet>
{
public:
	typedef XMLPathSelectAutomaton<AtmCharSet> ThisAutomaton;
	typedef typename ThisAutomaton::PathElement PathElement;
	typedef XMLPathSelectAutomatonParser This;
	typedef TextScanner<CStringIterator,SrcCharSet> SrcScanner;

public:
	///\brief Constructor
	XMLPathSelectAutomatonParser(){}
	virtual ~XMLPathSelectAutomatonParser(){}

	int addExpression( int typeidx, const char* esrc, std::size_t esrcsize)
	{
		// Check for namespaces, not supported:
		char const* xx = (char const*)std::memchr( esrc, ':', esrcsize);
		while (xx)
		{
			std::size_t xpos = xx - esrc;
			if (xpos + 1 < esrcsize && xx[1] == ':')
			{
				return xpos+1;
			}
			xx = (char const*)std::memchr( xx+1, ':', esrcsize - (xpos+1));
		}

		// Parse the expression:
		IdentifierBuf idbuf( &m_atmcharset);
		CStringIterator itr( esrc, esrcsize);
		SrcScanner src( m_srccharset, itr);
		ExprState expr( this);
		for (; *src; skipSpaces( src))
		{
			switch (*src)
			{
				case '@':
				{
					++src;
					if (!isIdentifierChar( src)) return src.getPosition()+1;
					expr.selectAttribute( idbuf.parseIdentifier( src));
					continue;
				}
				case '/':
				{
					++src;
					if (*src == '/')
					{
						expr.forAllDescendants();
						++src;
					}
					if (*src == '@')
					{
						++src;
						if (*src == '*')
						{
							++src;
							expr.selectAttribute( 0);
						}
						else if (isIdentifierChar( src))
						{
							expr.selectAttribute( idbuf.parseIdentifier( src));
						}
						else if (*src == '{')
						{
							std::vector<const char*> alt = idbuf.parseIdentifierList( src, '{', '}');
							if (alt.empty()) return src.getPosition()+1;
							expr.selectAttributeAlt( alt);
						}
						else
						{
							return src.getPosition()+1;
						}
					}
					else if (*src == '(')
					{
						continue;
					}
					else
					{
						if (*src == '*')
						{
							++src;
							expr.selectTag( 0);
						}
						else if (isIdentifierChar( src))
						{
							expr.selectTag( idbuf.parseIdentifier( src));
						}
						else if (*src == '{')
						{
							std::vector<const char*> alt = idbuf.parseIdentifierList( src, '{', '}');
							if (alt.empty()) return src.getPosition()+1;
							expr.selectTagAlt( alt);
						}
						else
						{
							return src.getPosition()+1;
						}
					}
					continue;
				}
				case '~':
				{
					++src;
					expr.selectCloseTag();
					continue;
				}
				case '[':
				{
					++src; skipSpaces( src);
					if (*src == '@')
					{
						++src; skipSpaces( src);
						// Attribute condition:
						if (!isIdentifierChar( src)) return src.getPosition()+1;
						const char* attrname = idbuf.parseIdentifier( src);
						skipSpaces( src);
						if (*src != '=') return src.getPosition()+1;
						++src; skipSpaces( src);
						const char* attrval = idbuf.parseValue( src);
						skipSpaces( src);
						if (!attrval || *src != ']') return src.getPosition()+1;
						expr.ifAttribute( attrname, attrval);
						++src;
					}
					else
					{
						// Range
						skipSpaces( src);
						if (!isIdentifierChar( src)) return src.getPosition()+1;
						int range_start = parseNum( src);
						if (range_start < 0) return src.getPosition()+1;
						skipSpaces( src);
						if (*src == ',')
						{
							++src; skipSpaces( src);
							if (*src == ']')
							{
								expr.FROM( range_start);
								++src;
							}
							else
							{
								if (!isIdentifierChar( src)) return src.getPosition()+1;
								int range_end = parseNum( src);
								if (range_end < 0) return src.getPosition()+1;
								skipSpaces( src);
								if (*src != ']') return src.getPosition()+1;
								expr.RANGE( range_start, range_end);
								++src;
							}
						}
						else if (*src == ']')
						{
							expr.INDEX( range_start);
							++src;
						}
						else
						{
							return src.getPosition()+1;
						}
					}
					continue;
				}
				case '(':
					++src;
					skipSpaces( src);
					if (*src != ')') return src.getPosition()+1;
					++src;
					expr.selectContent();
					skipSpaces( src);
					if (*src) return src.getPosition()+1;
					continue;

				default:
					return src.getPosition()+1;
			}
		}
		expr.assignType( typeidx);
		return 0;
	}

private:
#define ExprState_FOREACH( EXPR) {typename std::vector<PathElement>::iterator si = statelist.begin(), se = statelist.end(); for (; si != se; ++si) {si->EXPR;}}
	class ExprState
	{
	public:
		ExprState( XMLPathSelectAutomatonParser* atm)
		{
			statelist.push_back(PathElement(atm));
		}
		ExprState( const ExprState& o)
			:statelist(o.statelist){}

		void selectTagAlt( std::vector<const char*> alt)
		{
			std::vector<PathElement> new_statelist;
			std::vector<const char*>::const_iterator ai = alt.begin(), ae = alt.end();
			for (; ai != ae; ++ai)
			{
				ExprState alt_path( *this);
				alt_path.selectTag( *ai);
				new_statelist.insert( new_statelist.end(), alt_path.statelist.begin(), alt_path.statelist.end());
			}
			statelist = new_statelist;
		}
		void selectAttributeAlt( std::vector<const char*> alt)
		{
			std::vector<PathElement> new_statelist;
			std::vector<const char*>::const_iterator ai = alt.begin(), ae = alt.end();
			for (; ai != ae; ++ai)
			{
				ExprState alt_path( *this);
				alt_path.selectAttribute( *ai);
				new_statelist.insert( new_statelist.end(), alt_path.statelist.begin(), alt_path.statelist.end());
			}
			statelist = new_statelist;
		}

		void TO(int idx)						ExprState_FOREACH(TO(idx))
		void FROM(int idx)						ExprState_FOREACH(FROM(idx))
		void RANGE(int idx1, int idx2)					ExprState_FOREACH(RANGE(idx1,idx2))
		void INDEX(int idx)						ExprState_FOREACH(INDEX(idx))
		void selectAttribute( const char* name)				ExprState_FOREACH(selectAttribute(name))
		void selectTag( const char* name)				ExprState_FOREACH(selectTag(name))
		void assignType(int type)					ExprState_FOREACH(assignType(type))
		void forAllDescendants()					ExprState_FOREACH(forAllDescendants())
		void selectCloseTag()						ExprState_FOREACH(selectCloseTag())
		void ifAttribute( const char* name, const char* value)		ExprState_FOREACH(ifAttribute(name,value))
		void selectContent()						ExprState_FOREACH(selectContent())
	private:
		std::vector<PathElement> statelist;
	};

	static void skipSpaces( SrcScanner& src)
	{
		for (; src.control() == Space; ++src);
	}

	static int parseNum( SrcScanner& src)
	{
		std::string num;
		for (; *src>='0' && *src<='9';++src) num.push_back( *src);
		if (num.size() == 0 || num.size() > 8) return -1;
		return std::atoi( num.c_str());
	}

	static bool isIdentifierChar( SrcScanner& src)
	{
		if (src.control() == Undef || src.control() == Any)
		{
			if (*src == (unsigned char)'*') return false;
			if (*src == (unsigned char)'~') return false;
			if (*src == (unsigned char)'/') return false;
			if (*src == (unsigned char)'(') return false;
			if (*src == (unsigned char)')') return false;
			if (*src == (unsigned char)'@') return false;
			return true;
		}
		return false;
	}

	class IdentifierBuf
	{
	public:
		explicit IdentifierBuf( const AtmCharSet* atmcharset_)
			:atmcharset(atmcharset_){}
		~IdentifierBuf(){}

		const char* parseIdentifier( SrcScanner& src)
		{
			idlist.push_back( std::string());
			for (; isIdentifierChar(src); ++src)
			{
				atmcharset->print( *src, idlist.back());
			}
			return idlist.back().c_str();
		}
		const char* parseValue( SrcScanner& src)
		{
			idlist.push_back( std::string());
			if (*src == '"' || *src == '\'')
			{
				unsigned char eb = *src;
				for (++src; *src && *src != eb; ++src)
				{
					atmcharset->print( *src, idlist.back());
				}
				if (*src) ++src;
				return idlist.back().c_str();
			}
			else if (isIdentifierChar(src))
			{
				return parseIdentifier( src);
			}
			else
			{
				return NULL;
			}
		}
		std::vector<const char*> parseIdentifierList( SrcScanner& src, char startBracket, char endBracket)
		{
			std::vector<const char*> rt;
			if (*src != startBracket) return std::vector<const char*>();
			do
			{
				++src; skipSpaces( src);
				if (!isIdentifierChar( src)) return std::vector<const char*>();
				rt.push_back( parseIdentifier( src));
				skipSpaces( src);
			} while (*src == ',');
			if (*src != endBracket) return std::vector<const char*>();
			++src;
			return rt;
		}

	private:
		const AtmCharSet* atmcharset;
		std::list<std::string> idlist;
	};

private:
	AtmCharSet m_atmcharset;
	SrcCharSet m_srccharset;
};

} //namespace
#endif
