/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#include "config.h"

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "font.h"
#include "khtml_factory.h"
#include "khtml_settings.h"

#include <kdebug.h>
#include <kglobal.h>

#include <qpainter.h>
#include <qfontdatabase.h>
#include <qpaintdevicemetrics.h>

using namespace khtml;

/** closes the current word and returns its width in pixels
 * @param fm metrics of font to be used
 * @param str string
 * @param pos zero-indexed position within @p str upon which all other
 *	indices are based
 * @param wordStart relative index pointing to the position where the word started
 * @param wordEnd relative index pointing one position after the word ended
 * @return the width in pixels. May be 0 if @p wordStart and @p wordEnd were
 *	equal.
 */
inline int closeWordAndGetWidth(const QFontMetrics &fm, const QChar *str, int pos,
	int wordStart, int wordEnd)
{
    if (wordEnd <= wordStart) return 0;

    QConstString s(str + pos + wordStart, wordEnd - wordStart);
    return fm.width(s.string());
}

/** closes the current word and draws it
 * @param p painter
 * @param d text direction
 * @param x current x position, will be inc-/decremented correctly according
 *	to text direction
 * @param y baseline of text
 * @param widths list of widths; width of word is expected at position
 *		wordStart
 * @param str string
 * @param pos zero-indexed position within @p str upon which all other
 *	indices are based
 * @param wordStart relative index pointing to the position where the word started,
 *	will be set to wordEnd after function
 * @param wordEnd relative index pointing one position after the word ended
 */
inline void closeAndDrawWord(QPainter *p, QPainter::TextDirection d,
	int &x, int y, const short widths[], const QChar *str, int pos,
	int &wordStart, int wordEnd)
{
    if (wordEnd <= wordStart) return;

    int width = widths[wordStart];
    if (d == QPainter::RTL)
      x -= width;

    QConstString s(str + pos + wordStart, wordEnd - wordStart);
    p->drawText(x, y, s.string(), -1, d);

    if (d != QPainter::RTL)
      x += width;

    wordStart = wordEnd;
}

void Font::drawText( QPainter *p, int x, int y, QChar *str, int slen, int pos, int len,
        int toAdd, QPainter::TextDirection d, int from, int to, QColor bg, int uy, int h, int deco ) const
{
    if (!str) return;
    QConstString cstr = QConstString(str, slen);
    QString qstr = cstr.string();

    // ### fixme for RTL
    if ( !scFont && !letterSpacing && !wordSpacing && !toAdd && from==-1 ) {
	// simply draw it
	p->drawText( x, y, qstr, pos, len, d );
    } else {
	if (from < 0) from = 0;
	if (to < 0) to = len;

	int numSpaces = 0;
	if ( toAdd ) {
	    for( int i = 0; i < len; ++i )
		if ( str[i+pos].category() == QChar::Separator_Space )
		    ++numSpaces;
	}  

	const int totWidth = width( str, slen, pos, len );
	if ( d == QPainter::RTL ) {
	    x += totWidth + toAdd;
	}
	QString upper = qstr;
	QFontMetrics sc_fm = fm;
	if ( scFont ) {
	    // draw in small caps
	    upper = qstr.upper();
	    sc_fm = QFontMetrics( *scFont );
	}

	// ### sc could be optimized by only painting uppercase letters extra,
	// and treat the rest WordWise, but I think it's not worth it.
	// Somebody else may volunteer, and implement this ;-) (LS)

	// The mode determines whether the text is displayed character by
	// character, word by word, or as a whole
	enum { CharacterWise, WordWise, Whole }
	mode = Whole;
	if (!letterSpacing && !scFont && (wordSpacing || toAdd > 0))
	  mode = WordWise;
	else if (letterSpacing || scFont)
	  mode = CharacterWise;

	if (mode == Whole) {	// most likely variant is treated extra

	    if (to < 0) to = len;
	    const QConstString cstr(str + pos, len);
	    const QConstString segStr(str + pos + from, to - from);
	    const int preSegmentWidth = fm.width(cstr.string(), from);
	    const int segmentWidth = fm.width(segStr.string());
	    const int eff_x = d == QPainter::RTL ? x - preSegmentWidth - segmentWidth
					: x + preSegmentWidth;

	    // draw whole string segment, with optional background
	    if ( bg.isValid() )
		p->fillRect( eff_x, uy, segmentWidth, h, bg );
	    p->drawText(eff_x, y, segStr.string(), -1, d);
	    if (deco)
	        drawDecoration(p, eff_x, uy, y - uy, segmentWidth - 1, h, deco);
	    return;
	}

	// We are using two passes. In the first pass, the widths are collected,
	// and stored. In the second, the actual characters are drawn.

	// For each letter in the text box, save the width of the character.
	// When word-wise, only the first letter contains the width, but of the
	// whole word.
        short* const widthList = (short *)alloca(to*sizeof(short));

	// First pass: gather widths
	int preSegmentWidth = 0;
	int segmentWidth = 0;
        int lastWordBegin = 0;
	bool onSegment = from == 0;
	for( int i = 0; i < to; ++i ) {
	    if (i == from) {
                // Also close words on visibility boundary
	        if (mode == WordWise) {
	            const int width = closeWordAndGetWidth(fm, str, pos, lastWordBegin, i);

		    if (lastWordBegin < i) {
		        widthList[lastWordBegin] = (short)width;
		        lastWordBegin = i;
		        preSegmentWidth += width;
		    }
		}
		onSegment = true;
	    }

	    const QChar ch = str[pos+i];
	    bool lowercase = (ch.category() == QChar::Letter_Lowercase);
	    bool is_space = (ch.category() == QChar::Separator_Space);
	    int chw = 0;
	    if ( letterSpacing )
		chw += letterSpacing;
	    if ( (wordSpacing || toAdd) && is_space ) {
	        if (mode == WordWise) {
		    const int width = closeWordAndGetWidth(fm, str, pos, lastWordBegin, i);
		    if (lastWordBegin < i) {
		        widthList[lastWordBegin] = (short)width;
			lastWordBegin = i;
		        (onSegment ? segmentWidth : preSegmentWidth) += width;
		    }
		    ++lastWordBegin;		// ignore this space
		}
		chw += wordSpacing;
		if ( numSpaces ) {
		    const int a = toAdd/numSpaces;
		    chw += a;
		    toAdd -= a;
		    --numSpaces;
		}
	    }
	    if (is_space || mode == CharacterWise) {
	        chw += lowercase ? sc_fm.charWidth( upper, pos+i ) : fm.charWidth( qstr, pos+i );
		widthList[i] = (short)chw;

		(onSegment ? segmentWidth : preSegmentWidth) += chw;
	    }

	}

	// close last word
	Q_ASSERT(onSegment);
	if (mode == WordWise) {
	   segmentWidth += closeWordAndGetWidth(fm, str, pos, lastWordBegin, to);
	}

        if (d == QPainter::RTL) x -= preSegmentWidth;
	else x += preSegmentWidth;

        const int startx = d == QPainter::RTL ? x-segmentWidth : x;
        
	// optionally draw background
	if ( bg.isValid() )
	    p->fillRect( startx, uy, segmentWidth, h, bg );

	// second pass: do the actual drawing
        lastWordBegin = from;
	for( int i = from; i < to; ++i ) {
	    const QChar ch = str[pos+i];
	    bool lowercase = (ch.category() == QChar::Letter_Lowercase);
	    bool is_space = ch.category() == QChar::Separator_Space;
	    if ( is_space ) {
	        if (mode == WordWise) {
		    closeAndDrawWord(p, d, x, y, widthList, str, pos, lastWordBegin, i);
		    ++lastWordBegin;	// jump over space
		}
	    }
	    if (is_space || mode == CharacterWise) {
	        const int chw = widthList[i];
	        if (d == QPainter::RTL)
		    x -= chw;

		if ( scFont )
		    p->setFont( lowercase ? *scFont : f );
		p->drawText( x, y, (lowercase ? upper : qstr), pos+i, 1, d );

	        if (d != QPainter::RTL)
		    x += chw;
	    }

	}

	// don't forget to draw last word
	if (mode == WordWise) {
	    closeAndDrawWord(p, d, x, y, widthList, str, pos, lastWordBegin, to);
	}

	if (deco)
	    drawDecoration(p, startx, uy, y - uy, segmentWidth - 1, h, deco);

	if ( scFont )
	    p->setFont( f );
    }
}


int Font::width( QChar *chs, int, int pos, int len ) const
{
    const QConstString cstr(chs+pos, len);
    int w = 0;

    const QString qstr = cstr.string();
    if ( scFont ) {
	const QString upper = qstr.upper();
	const QChar *uc = qstr.unicode();
	const QFontMetrics sc_fm( *scFont );
	for ( int i = 0; i < len; ++i ) {
	    if ( (uc+i)->category() == QChar::Letter_Lowercase )
		w += sc_fm.charWidth( upper, i );
	    else
		w += fm.charWidth( qstr, i );
	}
    } else {
	// ### might be a little inaccurate
	w = fm.width( qstr );
    }

    if ( letterSpacing )
	w += len*letterSpacing;

    if ( wordSpacing )
	// add amount
	for( int i = 0; i < len; ++i ) {
	    if( chs[i+pos].category() == QChar::Separator_Space )
		w += wordSpacing;
	}

    return w;
}

int Font::width( QChar *chs, int slen, int pos ) const
{
    int w;
	if ( scFont && chs[pos].category() == QChar::Letter_Lowercase ) {
	    QString str( chs, slen );
	    str[pos] = chs[pos].upper();
	    w = QFontMetrics( *scFont ).charWidth( str, pos );
	} else {
	    const QConstString cstr( chs, slen );
	    w = fm.charWidth( cstr.string(), pos );
	}
    if ( letterSpacing )
	w += letterSpacing;

    if ( wordSpacing && (chs+pos)->category() == QChar::Separator_Space )
		w += wordSpacing;
    return w;
}


void Font::update( QPaintDeviceMetrics* devMetrics ) const
{
    f.setFamily( fontDef.family.isEmpty() ? KHTMLFactory::defaultHTMLSettings()->stdFontName() : fontDef.family );
    f.setItalic( fontDef.italic );
    f.setWeight( fontDef.weight );

    QFontDatabase db;

    int size = fontDef.size;
    const int lDpiY = kMax(devMetrics->logicalDpiY(), 96);

    // ok, now some magic to get a nice unscaled font
    // all other font properties should be set before this one!!!!
    if( !db.isSmoothlyScalable(f.family(), db.styleString(f)) )
    {
        const QValueList<int> pointSizes = db.smoothSizes(f.family(), db.styleString(f));
        // lets see if we find a nice looking font, which is not too far away
        // from the requested one.
        // kdDebug(6080) << "khtml::setFontSize family = " << f.family() << " size requested=" << size << endl;


        float diff = 1; // ### 100% deviation
        float bestSize = 0;

        QValueList<int>::ConstIterator it = pointSizes.begin();
        const QValueList<int>::ConstIterator itEnd = pointSizes.end();

        for( ; it != itEnd; ++it )
        {
            float newDiff = ((*it)*(lDpiY/72.) - float(size))/float(size);
            //kdDebug( 6080 ) << "smooth font size: " << *it << " diff=" << newDiff << endl;
            if(newDiff < 0) newDiff = -newDiff;
            if(newDiff < diff)
            {
                diff = newDiff;
                bestSize = *it;
            }
        }
        //kdDebug( 6080 ) << "best smooth font size: " << bestSize << " diff=" << diff << endl;
        if ( bestSize != 0 && diff < 0.2 ) // 20% deviation, otherwise we use a scaled font...
            size = (int)((bestSize*lDpiY) / 72);
    }

    // make sure we don't bust up X11
    size = KMAX(0, KMIN(255, size));

//       qDebug("setting font to %s, italic=%d, weight=%d, size=%d", fontDef.family.latin1(), fontDef.italic,
//    	   fontDef.weight, size );


    f.setPixelSize( size );

    fm = QFontMetrics( f );
    fontDef.hasNbsp = fm.inFont( 0xa0 );

    // small caps
    delete scFont;
    scFont = 0;

    if ( fontDef.smallCaps ) {
	scFont = new QFont( f );
	scFont->setPixelSize( f.pixelSize()*7/10 );
    }
}

void Font::drawDecoration(QPainter *pt, int _tx, int _ty, int baseline, int width, int height, int deco) const
{
    Q_UNUSED(height);
    // thick lines on small fonts look ugly
    const int thickness = fm.height() > 20 ? fm.lineWidth() : 1;
    const QBrush brush = pt->pen().color();
    if (deco & UNDERLINE) {
        int underlineOffset = ( fm.height() + baseline ) / 2;
        if (underlineOffset <= baseline) underlineOffset = baseline+1;

        pt->fillRect(_tx, _ty + underlineOffset, width + 1, thickness, brush );
    }
    if (deco & OVERLINE) {
        pt->fillRect(_tx, _ty, width + 1, thickness, brush );
    }
    if (deco & LINE_THROUGH) {
        pt->fillRect(_tx, _ty + 2*baseline/3, width + 1, thickness, brush );
    }
}

