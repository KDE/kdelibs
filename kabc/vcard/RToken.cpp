/*
	
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1999 Rik Hemsley rik@kde.org

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <string.h>
#include <stddef.h>
#include <qcstring.h>
#include <qstrlist.h>

namespace VCARD
{

	Q_UINT32
RTokenise(const char * str, const char * delim, QStrList & l)
{
	// FIXME no stderr !
	l.clear();
	
	if (!delim || !str || strlen(delim) == 0 || strlen(str) == 0) return 0;
	
	char * len = (char *)(str + strlen(str));	// End of string.
	
	register char * rstart = new char[strlen(str) + 1];
	register char * r = rstart;
	
	
	register const char * i = str;	// Cursor.
	
	while (i <= len) {

		if (*i == '\\') { // Escaped chars go straight through.
			*r++ = *i++;
			if (i <= len)
				*r++ = *i++;
			continue;
		}
		
		if (strchr(delim, *i) != 0) {
			// We hit a delimiter. If we have some text, make a new token.
			// This has the effect that multiple delimiters are collapsed.
                        // cs: We mustn't collapse multiple delimiters, otherwise we
                        // lose empty fields.
			*r = '\0';
//			if (r != rstart) {
				l.append(rstart);
//			}
			r = rstart;
			++i;
			continue;
		}
		
		*r++ = *i++;
	}

	// Catch last token
//	if (r != rstart) {
		*r = '\0';
		l.append(rstart);
//	}
	
	r = 0;
	
	delete [] rstart;

	return l.count();
}

}
