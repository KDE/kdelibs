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
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
*/

#ifndef KSYNTAXHIGHLIGHTER_H
#define KSYNTAXHIGHLIGHTER_H

#include <qtextedit.h>
#include <qsyntaxhighlighter.h>
#include <qcolor.h>
#include <qstringlist.h>

class QAccel;
class QTimer;
class KSpell;

class KSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    enum SyntaxMode {
	PlainTextMode,
	RichTextMode
    };
    KSyntaxHighlighter( QTextEdit *textEdit,
			 bool colorQuoting = false,
			 const QColor& QuoteColor0 = black,
			 const QColor& QuoteColor1 = QColor( 0x00, 0x80, 0x00 ),
			 const QColor& QuoteColor2 = QColor( 0x00, 0x80, 0x00 ),
			 const QColor& QuoteColor3 = QColor( 0x00, 0x80, 0x00 ),
			 SyntaxMode mode = PlainTextMode );
    ~KSyntaxHighlighter();

    int highlightParagraph( const QString& text, int endStateOfLastPara );

private:
    class KSyntaxHighlighterPrivate;
    KSyntaxHighlighterPrivate *d;
};

class KSpellingHighlighter : public KSyntaxHighlighter
{
public:
    KSpellingHighlighter( QTextEdit *textEdit,
			  const QColor& spellColor = red,
			  bool colorQuoting = false,
			  const QColor& QuoteColor0 = black,
			  const QColor& QuoteColor1 = QColor( 0x00, 0x80, 0x00 ),
			  const QColor& QuoteColor2 = QColor( 0x00, 0x80, 0x00 ),
			  const QColor& QuoteColor3 = QColor( 0x00, 0x80, 0x00 ) );
    ~KSpellingHighlighter();

    virtual int highlightParagraph( const QString &text,
				    int endStateOfLastPara );
    virtual bool isMisspelled( const QString& word ) = 0;
    bool intraWordEditing() const;
    void setIntraWordEditing( bool editing );
    static QStringList personalWords();

private:
    void flushCurrentWord();

    class KSpellingHighlighterPrivate;
    KSpellingHighlighterPrivate *d;
};

class KDictSpellingHighlighter : public QObject, public KSpellingHighlighter
{
Q_OBJECT

public:
    KDictSpellingHighlighter( QTextEdit *textEdit,
			      bool spellCheckingActive = true,
			      bool autoEnable = true,
			      const QColor& spellColor = red,
			      bool colorQuoting = false,
			      const QColor& QuoteColor0 = black,
			      const QColor& QuoteColor1 = QColor( 0x00, 0x80, 0x00 ),
			      const QColor& QuoteColor2 = QColor( 0x00, 0x70, 0x00 ),
			      const QColor& QuoteColor3 = QColor( 0x00, 0x60, 0x00 ) );
    ~KDictSpellingHighlighter();

    virtual bool isMisspelled( const QString &word );
    static void dictionaryChanged();

signals:
    void activeChanged(const QString &);

protected:
    QString spellKey();
    bool eventFilter(QObject *o, QEvent *e);

protected slots:
    void slotMisspelling( const QString &originalWord, const QStringList &suggestions, unsigned int pos );
    void slotRehighlight();
    void slotDictionaryChanged();
    void slotSpellReady( KSpell *spell );
    void slotAutoDetection();

private:
    class KDictSpellingHighlighterPrivate;
    KDictSpellingHighlighterPrivate *d;
};

#endif
