/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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

#include <font.h>
#include <qpainter.h>
#include <qfontdatabase.h>
#include <qpaintdevicemetrics.h>

using namespace khtml;

void Font::drawText( QPainter *p, int x, int y, QChar *str, int len, 
        int toAdd, QPainter::TextDirection d, int from, int to, QColor bg ) const
{
    // ### fixme for RTL
    if ( !letterSpacing && !wordSpacing && !toAdd && from==-1 ) {
	// simply draw it
	p->drawText( x, y, QConstString(str, len).string(), len, d );
    } else {
	int numSpaces = 0;
	if ( toAdd ) {
	    for( int i = 0; i < len; i++ )
		if ( str[i].direction() == QChar::DirWS )
		    numSpaces++;
	}
	    
	QConstString cstr( str, len );
	QString s( cstr.string() );
	if ( d == QPainter::RTL ) {
	    x += width( str, len ) + toAdd;
	}
	for( int i = 0; i < len; i++ ) {
	    int chw = fm.charWidth( s, i );
	    if ( letterSpacing )
		chw += letterSpacing;
	    if ( (wordSpacing || toAdd) && str[i].isSpace() ) {
		chw += wordSpacing;
		if ( numSpaces ) {
		    int a = toAdd/numSpaces;
		    chw += a;
		    toAdd -= a;
		    numSpaces--;
		}
	    }
	    if ( d == QPainter::RTL )
		x -= chw;
            if ( to==-1 || (i>=from && i<to) )
            {
                if ( bg.isValid() )
                    p->fillRect( x, y-fm.ascent(), chw, fm.height(), bg );

	        p->drawText( x, y, s, i, 1, d );
            }
	    if ( d != QPainter::RTL )
		x += chw;
	}
    }
}


int Font::width( QChar *chs, int len ) const
{
    int w = fm.width( QConstString( chs, len).string(), len );

    if ( letterSpacing )
	w += len*letterSpacing;

    if ( wordSpacing ) {
	// add amount
	for( int i = 0; i < len; i++ ) {
	    if( chs[i].isSpace() )
		w += wordSpacing;
	}
    }
    return w;
}

int Font::width( QChar ch ) const
{
    int w = fm.width( ch );

    if ( letterSpacing )
	w += letterSpacing;

    if ( wordSpacing && ch.isSpace() )
		w += wordSpacing;
    return w;
}


void Font::update( QPaintDeviceMetrics *devMetrics ) const
{
    f.setFamily( fontDef.family );
    f.setItalic( fontDef.italic );
    f.setWeight( fontDef.weight );

    QFontDatabase db;

    float size = fontDef.size;

    float toPix = devMetrics->logicalDpiY()/72.;

    // ok, now some magic to get a nice unscaled font
    // all other font properties should be set before this one!!!!

    if( !db.isSmoothlyScalable(f.family(), db.styleString(f)) )
    {
        QValueList<int> pointSizes = db.smoothSizes(f.family(), db.styleString(f));
        // lets see if we find a nice looking font, which is not too far away
        // from the requested one.
        //kdDebug(6080) << "khtml::setFontSize family = " << f.family() << " size requested=" << size << endl;

        QValueList<int>::Iterator it;
        float diff = 1; // ### 100% deviation
        float bestSize = 0;
        for( it = pointSizes.begin(); it != pointSizes.end(); ++it )
        {
            float newDiff = ((*it)*toPix - size)/size;
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
            size = bestSize*toPix;
//         else if ( size > 4 && size < 16 )
//             size = float( int( ( size + 1 ) / 2 )*2 );
    } else {
	size = size*toPix;
    }

    //qDebug(" -->>> using %f pixel font", size);

//     qDebug("setting font to %s, italic=%d, weight=%d, size=%f", fontDef.family.latin1(), fontDef.italic,
// 	   fontDef.weight, size );

    f.setPixelSizeFloat( size );

    fm = QFontMetrics( f );
}
