/* This file is part of the KDE libraries
   Copyright (C) 2001 David Faure <david@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kwordwrap.h"
#include <qpainter.h>

KWordWrap* KWordWrap::formatText( QFontMetrics &fm, const QRect & r, int /*flags*/, const QString & str, int len )
{
    // The wordwrap algorithm
    // The variable names and the global shape of the algorithm are inspired
    // from QTextFormatterBreakWords::format().
    //kdDebug() << "KWordWrap::formatText " << str << " r=" << r.x() << "," << r.y() << " " << r.width() << "x" << r.height() << endl;
    KWordWrap* kw = new KWordWrap;
    if ( len == -1 )
        kw->m_text = str;
    else
        kw->m_text = str.left( len );
    int height = fm.height();
    if ( len == -1 )
        len = str.length();
    int lastBreak = -1;
    int lineWidth = 0;
    int x = 0;
    int y = 0;
    int w = r.width();
    int textwidth = 0;
    for ( int i = 0 ; i < len; ++i )
    {
        QChar c = str[i];
        int ww = fm.charWidth( str, i );
        // isBreakable is true when we can break _after_ this character.
        bool isBreakable = ( c.isSpace() || c.isPunct() || c.isSymbol() )
                           && ( c != '(' && c != '[' && c != '{' );
        if ( !isBreakable && i < len-1 ) {
            QChar nextc = str[i+1];
            isBreakable = ( nextc == '(' || nextc == '[' || nextc == '{' );
        }
        /*kdDebug() << "c='" << QString(c) << "' i=" << i << "/" << len
                  << " x=" << x << " ww=" << ww << " w=" << w
                  << " lastBreak=" << lastBreak << " isBreakable=" << isBreakable << endl;*/
        int breakAt = -1;
        if ( x + ww > w && lastBreak != -1 ) // time to break and we know where
            breakAt = lastBreak;
        if ( x + ww > w - 4 && lastBreak == -1 ) // time to break but found nowhere [-> break here]
            breakAt = i;
        if ( i == len - 2 && x + ww + fm.charWidth( str, i+1 ) > w ) // don't leave the last char alone
            breakAt = lastBreak == -1 ? i - 1 : lastBreak;
        if ( breakAt != -1 )
        {
            //kdDebug() << "KWordWrap::formatText breaking after " << breakAt << endl;
            kw->m_breakPositions.append( breakAt );
            int thisLineWidth = lastBreak == -1 ? x + ww : lineWidth;
            kw->m_lineWidths.append( thisLineWidth );
            textwidth = QMAX( textwidth, thisLineWidth );
            x = 0;
            y += height;
            if ( lastBreak != -1 )
            {
                // Breakable char was found, restart from there
                i = lastBreak;
                lastBreak = -1;
                continue;
            }
        } else if ( isBreakable )
        {
            lastBreak = i;
            lineWidth = x + ww;
        }
        x += ww;
    }
    textwidth = QMAX( textwidth, x );
    kw->m_lineWidths.append( x );
    y += height;
    //kdDebug() << "KWordWrap::formatText boundingRect:" << r.x() << "," << r.y() << " " << textwidth << "x" << y << endl;
    kw->m_boundingRect.setRect( 0, 0, textwidth, y );
    return kw;
}

QString KWordWrap::wrappedString() const
{
    // We use the calculated break positions to insert '\n' into the string
    QString ws;
    int start = 0;
    QValueList<int>::ConstIterator it = m_breakPositions.begin();
    for ( ; it != m_breakPositions.end() ; ++it )
    {
        int end = (*it);
        ws += m_text.mid( start, end - start + 1 ) + '\n';
        start = end + 1;
    }
    ws += m_text.mid( start );
    return ws;
}

QString KWordWrap::truncatedString( bool dots ) const
{
    QString ts;
    QValueList<int>::ConstIterator it = m_breakPositions.begin();
    if ( it != m_breakPositions.end() )
    {
        ts = m_text.left( (*it) + 1 );
        if ( dots )
            ts += "...";
    }
    else
        ts = m_text;
    return ts;
}

void KWordWrap::drawText( QPainter *painter, int textX, int textY, int flags ) const
{
    //kdDebug() << "KWordWrap::drawText text=" << wrappedString() << " x=" << textX << " y=" << textY << endl;
    // We use the calculated break positions to draw the text line by line using QPainter
    int start = 0;
    int y = 0;
    QFontMetrics fm = painter->fontMetrics();
    int height = fm.height(); // line height
    int ascent = fm.ascent();
    int maxwidth = m_boundingRect.width();
    QValueList<int>::ConstIterator it = m_breakPositions.begin();
    QValueList<int>::ConstIterator itw = m_lineWidths.begin();
    for ( ; it != m_breakPositions.end() ; ++it, ++itw )
    {
        int end = (*it);
        int x = textX;
        if ( flags & Qt::AlignHCenter )
            x += ( maxwidth - *itw ) / 2;
        else if ( flags & Qt::AlignRight )
            x += maxwidth - *itw;
        painter->drawText( x, textY + y + ascent, m_text.mid( start, end - start + 1 ) );
        y += height;
        start = end + 1;
    }
    // Draw the last line
    int x = textX;
    if ( flags & Qt::AlignHCenter )
        x += ( maxwidth - *itw ) / 2;
    else if ( flags & Qt::AlignRight )
        x += maxwidth - *itw;
    painter->drawText( x, textY + y + ascent, m_text.mid( start ) );
}

