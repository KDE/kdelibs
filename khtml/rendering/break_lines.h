#ifndef BREAK_LINES_H
#define BREAK_LINES_H

#include <qstring.h>

namespace khtml {
    inline bool isBreakable( const QChar *str, int pos, int /*len*/ )
    {
	const QChar *c = str+pos;
	unsigned short ch = c->unicode();
	if ( ch > 0xff ) {
	    // not latin1, need to do more sophisticated checks for asian fonts
	    unsigned char row = c->row();
	    if ( row == 0x0e ) {
		// 0e00 - 0e7f == Thai
		if ( c->cell() < 0x80 ) {
		    // we don't a have a thai line breaking lib at the moment, allow
		    // breaks everywhere except directly before punctuation.
		    return true;
		} else
		    return false;
	    }
	    if ( row > 0x2d && row < 0xfb || row == 0x11 )
		// asian line breaking. Everywhere allowed except directly
		// in front of a punctuation character.
		return true;
	    else // no asian font
		return c->isSpace();
	} else {
	    if ( ch == ' ' || ch == '\n' )
		return true;
	}
	return false;
    }

};

#endif
