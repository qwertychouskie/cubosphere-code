/**
Copyright (C) 2010 Chriddo
Copyright (C) 2018 Valeri Ochinski <v19930312@gmail.com>

This program is free software;
you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, see <http://www.gnu.org/licenses/>.
**/

#pragma once

#include <vector>
#include <iostream>
#include <string>
#include "vectors.hpp"

extern bool BeginsWith(const std::string &str, const std::string &with);
extern void Tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters = " ");

extern void TokenizeFull(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters = " ",
		const int& first = 0);

extern void TrimSpaces( std::string& str);

inline std::string streamToString(std::istream &in, size_t reserve = 0) {
	std::string ret;
	if (reserve > 0) { ret.reserve(reserve); }
	char buffer[4096];
	while (in.read(buffer, sizeof(buffer))) {
			ret.append(buffer, sizeof(buffer));
			}
	ret.append(buffer, in.gcount());
	return ret;
	}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
