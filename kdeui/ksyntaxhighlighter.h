/*
 ksyntaxhighlighter.cpp

 Copyright (c) 2003 Trolltech AS
 Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>

 This file is part of the KDE libraries

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

#ifndef KSYNTAXHIGHLIGHTER_H
#define KSYNTAXHIGHLIGHTER_H

#include <QtGui/QColor>
#include <QtGui/QSyntaxHighlighter>

#include <kdelibs_export.h>

class QStringList;
class QTextEdit;

/**
 * \brief Syntax sensitive text highlighter
 */
class KDEUI_EXPORT KSyntaxHighlighter : public QSyntaxHighlighter
{
  public:
    /**
     * A enum for the different text modes the syntax highlighter
     * can work on.
     *
     * @li PlainTextMode - Plain text
     * @li RichTextMode  - Rich text
     */
    enum SyntaxMode
    {
      PlainTextMode,
      RichTextMode
    };

    /**
     * Creates a new syntax highlighter.
     *
     * @param textEdit The QTextEdit the syntax highlighter should work on.
     * @param colorQuoting If true ... ( ?!? )
     * @param quoteColor0 The color used for standard text.
     * @param quoteColor1 The color used for indention of first level.
     * @param quoteColor2 The color used for indention of second level.
     * @param quoteColor3 The color used for indention of third level.
     * @param mode The syntax mode of the text (@see SyntaxMode).
     */
    KSyntaxHighlighter( QTextEdit *textEdit,
                        bool colorQuoting = false,
                        const QColor& quoteColor0 = Qt::black,
                        const QColor& quoteColor1 = QColor( 0x00, 0x80, 0x00 ),
                        const QColor& quoteColor2 = QColor( 0x00, 0x80, 0x00 ),
                        const QColor& quoteColor3 = QColor( 0x00, 0x80, 0x00 ),
                        SyntaxMode mode = PlainTextMode );

    /**
     * Destroys the syntax highlighter.
     */
    ~KSyntaxHighlighter();

    /**
     * Highlights the block in the QTextEdit which matches the given
     * @param text.
     */
    virtual void highlightBlock( const QString &text );

  private:
    class Private;
    Private *const d;
};

#endif
