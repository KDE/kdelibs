/**
 * highlighter.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef KSPELL_HIGHLIGHTER_H
#define KSPELL_HIGHLIGHTER_H

#include "filter.h"

#include <QSyntaxHighlighter>
#include <QStringList>

class QTextEdit;

namespace KSpell2
{
    class Highlighter : public QSyntaxHighlighter
    {
	Q_OBJECT
    public:
        Highlighter( QTextEdit *textEdit,
                     const QString& configFile = QString(),
                     Filter *filter = Filter::defaultFilter() );
        ~Highlighter();
	
	virtual void highlightBlock ( const QString & text );

        Filter *currentFilter() const;
        void setCurrentFilter( Filter *filter );

        QString currentLanguage() const;
        void setCurrentLanguage( const QString& lang );
	
	static QStringList personalWords();

	/**
	 * @short Enable/Disable spell checking.
	 *
	 * If @p active is true then spell checking is enabled; otherwise it
	 * is disabled. Note that you have to disable automatic (de)activation
	 * with @ref setAutomatic() before you change the state of spell checking
	 * if you want to persistently enable/disable spell checking.
	 *
	 * @param active if true, then spell checking is enabled
	 *
	 * @see isActive(), setAutomatic()
	 */
	void setActive( bool active );
	
	/**
	 * Returns the state of spell checking.
	 *
	 * @return true if spell checking is active
	 *
	 * @see setActive()
	 */
	bool isActive() const;

	void rehighlight();

    
    Q_SIGNALS:
	void activeChanged(const QString &);

    protected:
        virtual void setMisspelled( int start, int count );
        virtual void unsetMisspelled( int start,  int count );
    private:
        class Private;
        Private* const d;
    };
    
    class KEMailQuotingSyntaxHighlighter : public Highlighter
	{
	public:
	    enum SyntaxMode {
		PlainTextMode,
		RichTextMode
	    };
	    KEMailQuotingSyntaxHighlighter( QTextEdit *textEdit,
				const QColor& QuoteColor0 = Qt::black,
				const QColor& QuoteColor1 = QColor( 0x00, 0x80, 0x00 ),
				const QColor& QuoteColor2 = QColor( 0x00, 0x80, 0x00 ),
				const QColor& QuoteColor3 = QColor( 0x00, 0x80, 0x00 ),
				SyntaxMode mode = PlainTextMode );
	    ~KEMailQuotingSyntaxHighlighter();
	    
	    virtual void highlightBlock ( const QString & text );
	    
	private:
	    class KEmailQuotingSyntaxHighlighterPrivate;
	    KEmailQuotingSyntaxHighlighterPrivate *const d;
	};

}

#endif
