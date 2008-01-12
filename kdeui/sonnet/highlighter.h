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
#ifndef SONNET_HIGHLIGHTER_H
#define SONNET_HIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>
#include <QtCore/QStringList>
#include <kdemacros.h>
#include <kdeui_export.h>

class QTextEdit;

/// The sonnet namespace
namespace Sonnet
{
    /// The Sonnet Highlighter
    class KDEUI_EXPORT Highlighter : public QSyntaxHighlighter
    {
	Q_OBJECT
    public:
        explicit Highlighter(QTextEdit *textEdit,
                             const QString &configFile = QString(),
                             const QColor &col=QColor());
        ~Highlighter();

	bool spellCheckerFound() const;

        QString currentLanguage() const;
        void setCurrentLanguage(const QString &lang);

	static QStringList personalWords();

	/**
	 * @short Enable/Disable spell checking.
	 *
	 * If @p active is true then spell checking is enabled; otherwise it
	 * is disabled. Note that you have to disable automatic (de)activation
	 * with @ref setAutomatic() before you change the state of spell
	 * checking if you want to persistently enable/disable spell
	 * checking.
	 *
	 * @param active if true, then spell checking is enabled
	 *
	 * @see isActive(), setAutomatic()
	 */
	void setActive(bool active);

	/**
	 * Returns the state of spell checking.
	 *
	 * @return true if spell checking is active
	 *
	 * @see setActive()
	 */
	bool isActive() const;

	bool automatic() const;

	void setAutomatic(bool automatic);


    Q_SIGNALS:
	/**
	 * Emitted when as-you-type spell checking is enabled or disabled.
	 *
	 * @param description is a i18n description of the new state,
	 *        with an optional reason
	 */
	void activeChanged(const QString &description);

	/**
	 *
	 * @param originalWord missspelled word
	 *
	 * @param suggestions list of word which can replace missspelled word
	 */
	void newSuggestions(const QString &originalWord, const QStringList &suggestions);

    protected:
	virtual void highlightBlock(const QString &text);

        virtual void setMisspelled(int start, int count);
        virtual void unsetMisspelled(int start,  int count);

	bool eventFilter(QObject *o, QEvent *e);
	bool intraWordEditing() const;
	void setIntraWordEditing(bool editing);

    public Q_SLOTS:
        void slotAutoDetection();
        void slotRehighlight();
    private:
        virtual void connectNotify(const char* signal);
        virtual void disconnectNotify(const char* signal);
        class Private;
        Private *const d;
        Q_DISABLE_COPY( Highlighter )
    };

}

#endif
