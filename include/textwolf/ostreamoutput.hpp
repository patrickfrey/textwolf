/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \file textwolf/ostreamoutput.hpp
/// \brief Back insertion sequence needed for textwolf output as redirection to a std::ostream

#ifndef __TEXTWOLF_OSTREAM_OUTPUT_HPP__
#define __TEXTWOLF_OSTREAM_OUTPUT_HPP__
#include "textwolf/exception.hpp"
#include <iostream>

namespace textwolf {

/// \class OstreamOutput
/// \brief Simple back insertion sequence for redirects the outputs of textwolf to a std output stream
class OstreamOutput :public throws_exception
{
public:
	/// \brief Constructor
	explicit OstreamOutput( std::ostream* out_)
		:m_out(out_){}

	/// \brief Copy constructor
	OstreamOutput( const OstreamOutput& o)
		:m_out(o.m_out){}

	/// \brief Destructor
	~OstreamOutput(){}

	/// \brief Append one character
	/// \param[in] ch the character to append
	void push_back( char ch)
	{
		(*m_out) << ch;
	}

	/// \brief Append an array of characters
	/// \param[in] cc the characters to append
	/// \param[in] ccsize the number of characters to append
	void append( const char* cc, std::size_t ccsize)
	{
		std::size_t ci = 0;
		for (; ci != ccsize; ++ci)
		{
			(*m_out) << cc[ ci];
		}
	}

private:
	std::ostream* m_out;			///< pointer to ouptut stream to redirect output to
};

}//namespace
#endif
