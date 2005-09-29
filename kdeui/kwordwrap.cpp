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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kwordwrap.h"
#include <kdebug.h>
#include <kstringhandler.h>
#include <qpainter.h>

class KWordWrapPrivate {
public:
  QRect m_constrainingRect;
};

KWordWrap::KWordWrap(const QRect & r) {
    d = new KWordWrapPrivate;
    d->m_constrainingRect = r;
}

KWordWrap* KWordWrap::formatText( QFontMetrics &fm, const QRect & r, int /*flags*/, const QString & str, int len )
{
    KWordWrap* kw = new KWordWrap( r );
    // The wordwrap algorithm
    // The variable names and the global shape of the algorithm are inspired
    // from QTextFormatterBreakWords::format().
    //kdDebug() << "KWordWrap::formatText " << str << " r=" << r.x() << "," << r.y() << " " << r.width() << "x" << r.height() << endl;
    int height = fm.height();
    if ( len == -1 )
        kw->m_text = str;
    else
        kw->m_text = str.left( len );
    if ( len == -1 )
        len = str.length();
    int lastBreak = -1;
    int lineWidth = 0;
    int x = 0;
    int y = 0;
    int w = r.width();
    int textwidth = 0;
    bool isBreakable = false;
    bool wasBreakable = false; // value of isBreakable for last char (i-1)
    bool isParens = false; // true if one of ({[
    bool wasParens = false; // value of isParens for last char (i-1)

    for ( int i = 0 ; i < len; ++i )
    {
        QChar c = str[i];
        int ww = fm.charWidth( str, i );

        isParens = ( c == '(' || c == '[' || c == '{' );
        // isBreakable is true when we can break _after_ this character.
        isBreakable = ( c.isSpace() || c.isPunct() || c.isSymbol() ) & !isParens;

        // Special case for '(', '[' and '{': we want to break before them
        if ( !isBreakable && i < len-1 ) {
            QChar nextc = str[i+1]; // look at next char
            isBreakable = ( nextc == '(' || nextc == '[' || nextc == '{' );
        }
        // Special case for '/': after normal chars it's breakable (e.g. inside a path),
        // but after another breakable char it's not (e.g. "mounted at /foo")
        // Same thing after a parenthesis (e.g. "dfaure [/fool]")
        if ( c == '/' && (wasBreakable || wasParens) )
            isBreakable = false;

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
        if ( c == '\n' ) // Forced break here
        {
            if ( breakAt == -1 && lastBreak != -1) // only break if not already breaking
            {
                breakAt = i - 1;
                lastBreak = -1;
            }
            // remove the line feed from the string
            kw->m_text.remove(i, 1);
            len--;
        }
        if ( breakAt != -1 )
        {
            //kdDebug() << "KWordWrap::formatText breaking after " << breakAt << endl;
            kw->m_breakPositions.append( breakAt );
            int thisLineWidth = lastBreak == -1 ? x + ww : lineWidth;
            kw->m_lineWidths.append( thisLineWidth );
            textwidth = QMAX( textwidth, thisLineWidth );
            x = 0;
            y += height;
            wasBreakable = true;
            wasParens = false;
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
        wasBreakable = isBreakable;
        wasParens = isParens;
    }
    textwidth = QMAX( textwidth, x );
    kw->m_lineWidths.append( x );
    y += height;
    //kdDebug() << "KWordWrap::formatText boundingRect:" << r.x() << "," << r.y() << " " << textwidth << "x" << y << endl;
    if ( r.height() >= 0 && y > r.height() )
        textwidth = r.width();
    int realY = y;
    if ( r.height() >= 0 )
    {
        while ( realY > r.height() )
            realY -= height;
        realY = QMAX( realY, 0 );
    }
    kw->m_boundingRect.setRect( 0, 0, textwidth, realY );
    return kw;
}

KWordWrap::~KWordWrap() {
    delete d;
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
    if ( m_breakPositions.isEmpty() )
        return m_text;

    QString ts = m_text.left( m_breakPositions.first() + 1 );
    if ( dots )
        ts += "...";
    return ts;
}

static QColor mixColors(double p1, QColor c1, QColor c2) {
  return QColor(int(c1.red() * p1 + c2.red() * (1.0-p1)),
                int(c1.green() * p1 + c2.green() * (1.0-p1)),
		int(c1.blue() * p1 + c2.blue() * (1.0-p1)));
}

void KWordWrap::drawFadeoutText(QPainter *p, int x, int y, int maxW,
                                   const QString &t) {
    QFontMetrics fm = p->fontMetrics();
    QColor bgColor = p->backgroundColor();
    QColor textColor = p->pen().color();

    if ( ( fm.boundingRect( t ).width() > maxW ) && ( t.length() > 1 ) ) {
        unsigned int tl = 0;
        int w = 0;
        while ( tl < t.length() ) {
            w += fm.charWidth( t, tl );
            if ( w >= maxW )
                break;
            tl++;
        }

        if (tl > 3) {
            p->drawText( x, y, t.left( tl - 3 ) );
            x += fm.width( t.left( tl - 3 ) );
        }
        int n = QMIN( tl, 3);
        for (int i = 0; i < n; i++) {
            p->setPen( mixColors( 0.70 - i * 0.25, textColor, bgColor ) );
            QString s( t.at( tl - n + i ) );
            p->drawText( x, y, s );
            x += fm.width( s );
        }
    }
    else
        p->drawText( x, y, t );
}

void KWordWrap::drawTruncateText(QPainter *p, int x, int y, int maxW,
                                 const QString &t) {
    QString tmpText = KStringHandler::rPixelSqueeze( t, p->fontMetrics(), maxW );
    p->drawText( x, y, tmpText, maxW );
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
        // if this is the last line, leave the loop
        if ( (d->m_constrainingRect.height() >= 0) &&
	     ((y + 2 * height) > d->m_constrainingRect.height()) )
	    break;
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
    if ( (d->m_constrainingRect.height() < 0) ||
         ((y + height) <= d->m_constrainingRect.height()) ) {
	if ( it == m_breakPositions.end() )
            painter->drawText( x, textY + y + ascent, m_text.mid( start ) );
	else if (flags & FadeOut)
	    drawFadeoutText( painter, textX, textY + y + ascent,
	                     d->m_constrainingRect.width(),
			     m_text.mid( start ) );
        else if (flags & Truncate)
            drawTruncateText( painter, textX, textY + y + ascent,
                              d->m_constrainingRect.width(),
			      m_text.mid( start ) );
	else
            painter->drawText( x, textY + y + ascent,
	                       m_text.mid( start, (*it) - start + 1 ) );
    }
}
