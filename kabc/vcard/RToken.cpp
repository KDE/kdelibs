/*
	
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1999 Rik Hemsley rik@kde.org

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
