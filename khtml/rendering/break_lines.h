#ifndef BREAK_LINES_H
#define BREAK_LINES_H

#include <qstring.h>

namespace khtml {

    /*
      array of unicode codes where breaking shouldn't occur.
      (in sorted order because of using with binary search)
      these are currently for Japanese, though simply adding
      Korean, Chinese ones should work as well
    */
    /*
      dontbreakbefore[] contains characters not covered by QChar::Punctuation_Close that shouldn't be broken before.
      chars included in QChar::Punctuation_Close are listed below.(look at UAX #14)
         - 3001 ideographic comma
         - 3002 ideographic full stop
         - FE50 small comma
         - FF52 small full stop
         - FF0C fullwidth comma
         - FF0E fullwidth full stop
         - FF61 halfwidth ideographic full stop
         - FF64 halfwidth ideographic comma
      these character is commented out.
    */
    const ushort dontbreakbefore[] = {
        //0x3001,   //ideographic comma
        //0x3002,   //ideographic full stop
        0x3005, //ideographic iteration mark
        0x3009, //right angle bracket
        0x300b, //right double angle bracket
        0x300d, //right corner bracket
        0x300f, //right white corner bracket
        0x3011, //right black lenticular bracket
        0x3015, //right tortoise shell bracket
        0x3041, //small a hiragana
        0x3043, //small i hiragana
        0x3045, //small u hiragana
        0x3047, //small e hiragana
        0x3049, //small o hiragana
        0x3063, //small tsu hiragana
        0x3083, //small ya hiragana
        0x3085, //small yu hiragana
        0x3087, //small yo hiragana
        0x308E, //small wa hiragana
        0x309B, //jap voiced sound mark
        0x309C, //jap semi-voiced sound mark
        0x309D, //jap iteration mark hiragana
        0x309E, //jap voiced iteration mark hiragana
        0x30A1, //small a katakana
        0x30A3, //small i katakana
        0x30A5, //small u katakana
        0x30A7, //small e katakana
        0x30A9, //small o katakana
        0x30C3, //small tsu katakana
        0x30E3, //small ya katakana
        0x30E5, //small yu katakana
        0x30E7, //small yo katakana
        0x30EE, //small wa katakana
        0x30F5, //small ka katakana
        0x30F6, //small ke katakana
        0x30FC, //jap prolonged sound mark
        0x30FD, //jap iteration mark katakana
        0x30FE, //jap voiced iteration mark katakana
        //0xFE50,   //small comma
        //0xFF52,   //small full stop
        0xFF01, //fullwidth exclamation mark
        0xFF09, //fullwidth right parenthesis
        //0xFF0C,   //fullwidth comma
        0xFF0D, //fullwidth hypen-minus
        //0xFF0E,   //fullwidth full stop
        0xFF1F, //fullwidth question mark
        0xFF3D, //fullwidth right square bracket
        0xFF5D, //fullwidth right curly bracket
        //0xFF61,   //halfwidth ideographic full stop
        0xFF63, //halfwidth right corner bracket
        //0xFF64,   //halfwidth ideographic comma
        0xFF67, //halfwidth katakana letter small a
        0xFF68, //halfwidth katakana letter small i
        0xFF69, //halfwidth katakana letter small u
        0xFF6a, //halfwidth katakana letter small e
        0xFF6b, //halfwidth katakana letter small o
        0xFF6c, //halfwidth katakana letter small ya
        0xFF6d, //halfwidth katakana letter small yu
        0xFF6e, //halfwidth katakana letter small yo
        0xFF6f, //halfwidth katakana letter small tu
        0xFF70  //halfwidth katakana-hiragana prolonged sound mark
    };

    // characters that aren't covered by QChar::Punctuation_Open
    const ushort dontbreakafter[] = {
        0x3012, //postal mark
        0xFF03, //full width pound mark
        0xFF04, //full width dollar sign
        0xFF20, //full width @
        0xFFE1, //full width british pound sign
        0xFFE5  //full width yen sign
    };

    inline bool break_bsearch( const ushort* arr, const ushort val ) {
        int left = 0;
        int right = (sizeof(arr) / sizeof(ushort)) - 1;

        while (1) {
            if (left == right)
                return val != arr[left];

            int i = (left + right) >> 1;
            if ( val == arr[i] )
                return false;
            if ( val < arr[i] )
                right = i;
            else
                left = i + 1;
        }
    }
    
    bool isBreakableThai( const QChar *string, const int pos, const int len);
    void cleanup_thaibreaks();

    inline bool isBreakable( const QChar *str, const int pos, int len )
    {
	const QChar *c = str+pos;
	unsigned short ch = c->unicode();
	if ( ch > 0xff ) {
	    // not latin1, need to do more sophisticated checks for asian fonts
	    unsigned char row = c->row();
	    if ( row == 0x0e ) {
		// 0e00 - 0e7f == Thai
		if ( c->cell() < 0x80 ) {
		    // consult libthai
		    return isBreakableThai(str, pos, len);
		} else
		    return false;
	    }
	    if ( row > 0x2d && row < 0xfb || row == 0x11 ) {
                /* asian line breaking. */
                if ( pos == 0 )
                    return false; // never break before first character

                // check for simple punctuation cases
                QChar::Category cat = c->category();
                if ( cat == QChar::Punctuation_Close ||
                     cat == QChar::Punctuation_Other ||
                     (str+(pos-1))->category() == QChar::Punctuation_Open )
                    return false;

                // do binary search in dontbreak[]
                return break_bsearch(dontbreakbefore, c->unicode()) &&
                       break_bsearch(dontbreakafter, (str+(pos-1))->unicode());
            } else // no asian font
		return c->isSpace();
	} else {
	    if ( ch == ' ' || ch == '\n' )
		return true;
	}
	return false;
    }

}

#endif
