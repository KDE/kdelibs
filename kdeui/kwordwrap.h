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

#ifndef kwordwrap_h
#define kwordwrap_h

#include <qfontmetrics.h>
#include <qvaluelist.h>
#include <qrect.h>
#include <qstring.h>

/**
 * Word-wrap algorithm that takes into account beautifulness ;)
 * That means: not letting a letter alone on the last line,
 * breaking at punctuation signs, etc.
 *
 * Usage: call the static method, @ref formatText, with the text to
 * wrap and the constraining rectangle etc., it will return an instance of KWordWrap
 * containing internal data, result of the word-wrapping.
 * From that instance you can retrieve the boundingRect, and invoke drawing.
 *
 * This design allows to call the word-wrap algorithm only when the text changes
 * and not every time we want to know the bounding rect or draw the text.
 */
class KWordWrap
{
public:
    /**
     * Main method for wrapping text.
     *
     * @param fm Font metrics, for the chosen font. Better cache it, creating a QFontMetrics is expensive.
     * @param r Constraining rectangle. Only the width and height matter.
     * @param flags - currently unused.
     * @param str The text to be wrapped.
     * @param len Length of text to wrap (default is -1 for all).
     * @return a KWordWrap instance. The caller is responsible for storing and deleting the result.
     */
    static KWordWrap* formatText( QFontMetrics &fm, const QRect & r, int flags, const QString & str, int len = -1 );

    /**
     * @return the bounding rect, calculated by formatText.
     */
    QRect boundingRect() const { return m_boundingRect; }

    /**
     * @return the original string, with '\n' inserted where
     * the text is broken by the wordwrap algorithm.
     */
    QString wrappedString() const; // gift for Dirk :)

    /**
     * @return the original string, truncated to the first line.
     * If @p dots was set, '...' is appended in case the string was truncated.
     * Bug: Note that the '...' come out of the bounding rect.
     */
    QString truncatedString( bool dots = true ) const;

    /**
     * Draw the text that has been previously wrapped, at position x,y.
     * Flags are for alignment, e.g. AlignHCenter. Default is AlignAuto.
     */
    void drawText( QPainter *painter, int x, int y, int flags = Qt::AlignAuto ) const;

private:
    QValueList<int> m_breakPositions;
    QValueList<int> m_lineWidths;
    QRect m_boundingRect;
    QString m_text;
};

#endif
