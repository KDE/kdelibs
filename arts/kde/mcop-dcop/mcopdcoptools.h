/*
   Copyright (c) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef MCOPDCOPTOOLS_H
#define MCOPDCOPTOOLS_H

#include <string>

#include <qdatastream.h>

class MCOPDCOPTools
{
public:
	static long getLong(QDataStream &stream)
	{
		long l;
		stream << l;

		return l;
	}

	static std::string getString(QDataStream &stream)
	{
		Q3CString s;
		stream << s;

		return std::string(s);
	}
};

#endif
