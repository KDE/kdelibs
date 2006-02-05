/* This file is part of the KDE libraries
   Copyright (C) 2001 David Faure <faure@kde.org>

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
#include <qvector.h>

class KWordWrapPrivate {
public:
  QRect m_constrainingRect;
  QVector<int> m_breakPositions;
  QVector<int> m_lineWidths;
  QRect m_boundingRect;
  QString m_text;
};

KWordWrap::KWordWrap(const QRect & r)
:	d(new KWordWrapPrivate)
{
    d->m_constrainingRect = r;
}

KWordWrap* KWordWrap::formatText( QFontMetrics &fm, const QRect & r, int /*flags*/, const QString & str, int len )
{
    KWordWrap* kw = new KWordWrap( r );
    // The wordwrap algorithm
    // The variable names and the global shape of the algorithm are inspired
    // from QTextFormatterBreakWords::format().
    //kDebug() << "KWordWrap::formatText " << str << " r=" << r.x() << "," << r.y() << " " << r.width() << "x" << r.height() << endl;
    int height = fm.height();
    if ( len == -1 )
        kw->d->m_text = str;
    else
        kw->d->m_text = str.left( len );
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
        QChar c = str.at(i);
        int ww = fm.charWidth( str, i );

        isParens = ( c == QLatin1Char('(') || c == QLatin1Char('[')
                     || c == QLatin1Char('{') );
        // isBreakable is true when we can break _after_ this character.
        isBreakable = ( c.isSpace() || c.isPunct() || c.isSymbol() ) & !isParens;

        // Special case for '(', '[' and '{': we want to break before them
        if ( !isBreakable && i < len-1 ) {
            QChar nextc = str.at(i + 1); // look at next char
            isBreakable = ( nextc == QLatin1Char('(')
                            || nextc == QLatin1Char('[')
                            || nextc == QLatin1Char('{') );
        }
        // Special case for '/': after normal chars it's breakable (e.g. inside a path),
        // but after another breakable char it's not (e.g. "mounted at /foo")
        // Same thing after a parenthesis (e.g. "dfaure [/fool]")
        if ( c == QLatin1Char('/') && (wasBreakable || wasParens) )
            isBreakable = false;

        /*kDebug() << "c='" << QString(c) << "' i=" << i << "/" << len
                  << " x=" << x << " ww=" << ww << " w=" << w
                  << " lastBreak=" << lastBreak << " isBreakable=" << isBreakable << endl;*/
        int breakAt = -1;
        if ( x + ww > w && lastBreak != -1 ) // time to break and we know where
            breakAt = lastBreak;
        if ( x + ww > w - 4 && lastBreak == -1 ) // time to break but found nowhere [-> break here]
            breakAt = i;
        if ( i == len - 2 && x + ww + fm.charWidth( str, i+1 ) > w ) // don't leave the last char alone
            breakAt = lastBreak == -1 ? i - 1 : lastBreak;
        if ( c == QLatin1Char('\n') ) // Forced break here
        {
            if ( breakAt == -1 && lastBreak != -1) // only break if not already breaking
            {
                breakAt = i - 1;
                lastBreak = -1;
            }
            // remove the line feed from the string
            kw->d->m_text.remove(i, 1);
            len--;
        }
        if ( breakAt != -1 )
        {
            //kDebug() << "KWordWrap::formatText breaking after " << breakAt << endl;
            kw->d->m_breakPositions.append( breakAt );
            int thisLineWidth = lastBreak == -1 ? x + ww : lineWidth;
            kw->d->m_lineWidths.append( thisLineWidth );
            textwidth = qMax( textwidth, thisLineWidth );
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
    textwidth = qMax( textwidth, x );
    kw->d->m_lineWidths.append( x );
    y += height;
    //kDebug() << "KWordWrap::formatText boundingRect:" << r.x() << "," << r.y() << " " << textwidth << "x" << y << endl;
    if ( r.height() >= 0 && y > r.height() )
        textwidth = r.width();
    int realY = y;
    if ( r.height() >= 0 )
    {
        while ( realY > r.height() )
            realY -= height;
        realY = qMax( realY, 0 );
    }
    kw->d->m_boundingRect.setRect( 0, 0, textwidth, realY );
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
    for (int i = 0; i < d->m_breakPositions.count(); ++i) {
        int end = d->m_breakPositions.at(i);
        ws += d->m_text.mid( start, end - start + 1 );
        ws += QLatin1Char('\n');
        start = end + 1;
    }
    ws += d->m_text.mid( start );
    return ws;
}

QString KWordWrap::truncatedString( bool dots ) const
{
    if ( d->m_breakPositions.isEmpty() )
        return d->m_text;

    QString ts = d->m_text.left( d->m_breakPositions.first() + 1 );
    if ( dots )
        ts += QLatin1String("...");
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
    QColor bgColor = p->background().color();
    QColor textColor = p->pen().color();

    if ( ( fm.boundingRect( t ).width() > maxW ) && ( t.length() > 1 ) ) {
        int tl = 0;
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
        int n = qMin( tl, 3);
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
    p->drawText( x, y, tmpText );
}

void KWordWrap::drawText( QPainter *painter, int textX, int textY, int flags ) const
{
    //kDebug() << "KWordWrap::drawText text=" << wrappedString() << " x=" << textX << " y=" << textY << endl;
    // We use the calculated break positions to draw the text line by line using QPainter
    int start = 0;
    int y = 0;
    QFontMetrics fm = painter->fontMetrics();
    int height = fm.height(); // line height
    int ascent = fm.ascent();
    int maxwidth = d->m_boundingRect.width();
    int i;
    int lwidth = 0;
    int end = 0;
    for (i = 0; i < d->m_breakPositions.count() ; ++i )
    {
        // if this is the last line, leave the loop
        if ( (d->m_constrainingRect.height() >= 0) &&
	     ((y + 2 * height) > d->m_constrainingRect.height()) )
	    break;
        end = d->m_breakPositions.at(i);
        lwidth = d->m_lineWidths.at(i);
        int x = textX;
        if ( flags & Qt::AlignHCenter )
            x += ( maxwidth - lwidth ) / 2;
        else if ( flags & Qt::AlignRight )
            x += maxwidth - lwidth;
        painter->drawText( x, textY + y + ascent, d->m_text.mid( start, end - start + 1 ) );
        y += height;
        start = end + 1;
    }

    // Draw the last line
    lwidth = d->m_lineWidths.last();
    int x = textX;
    if ( flags & Qt::AlignHCenter )
        x += ( maxwidth - lwidth ) / 2;
    else if ( flags & Qt::AlignRight )
        x += maxwidth - lwidth;
    if ( (d->m_constrainingRect.height() < 0) ||
         ((y + height) <= d->m_constrainingRect.height()) ) {
	if ( i == d->m_breakPositions.count() )
            painter->drawText( x, textY + y + ascent, d->m_text.mid( start ) );
	else if (flags & FadeOut)
	    drawFadeoutText( painter, textX, textY + y + ascent,
	                     d->m_constrainingRect.width(),
			     d->m_text.mid( start ) );
        else if (flags & Truncate)
            drawTruncateText( painter, textX, textY + y + ascent,
                              d->m_constrainingRect.width(),
			      d->m_text.mid( start ) );
	else
            painter->drawText( x, textY + y + ascent,
	                       d->m_text.mid( start ) );
    }
}

QRect KWordWrap::boundingRect() const
{
    return d->m_boundingRect;
}

