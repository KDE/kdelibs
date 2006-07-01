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

class KSpell;
class KSpellConfig;

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

/**
 * \brief Syntax sensitive text highlighter with spelling support.
 */
class KDEUI_EXPORT KSpellingHighlighter : public KSyntaxHighlighter
{
  public:
    /**
     * Creates a new spelling highlighter.
     *
     * @param textEdit The QTextEdit the spelling highlighter should work on.
     * @param colorQuoting If true ... ( ?!? )
     * @param quoteColor0 The color used for standard text.
     * @param quoteColor1 The color used for indention of first level.
     * @param quoteColor2 The color used for indention of second level.
     * @param quoteColor3 The color used for indention of third level.
     */
    KSpellingHighlighter( QTextEdit *textEdit,
                          const QColor& spellColor = Qt::red,
                          bool colorQuoting = false,
                          const QColor& quoteColor0 = Qt::black,
                          const QColor& quoteColor1 = QColor( 0x00, 0x80, 0x00 ),
                          const QColor& quoteColor2 = QColor( 0x00, 0x80, 0x00 ),
                          const QColor& quoteColor3 = QColor( 0x00, 0x80, 0x00 ) );

    /**
     * Destroys the syntax highlighter.
     */
    ~KSpellingHighlighter();

    /**
     * Highlights the block in the QTextEdit which matches the given
     * @param text.
     */
    virtual void highlightBlock( const QString &text );

    /**
     * Returns the static list of personal words.
     */
    static QStringList personalWords();

    /**
     * Returns whether the given @param word is misspelled.
     */
    virtual bool isMisspelled( const QString& word ) = 0;

    /**
     * Sets the intra word editing enabled or disabled.
     */
    void setIntraWordEditing( bool editing );

    /**
     * Returns whether intra word editing is enabled.
     */
    bool intraWordEditing() const;

  private:
    class Private;
    Private *const d;
};

/**
 * \brief Dictionary sensitive text highlighter
 */
class KDEUI_EXPORT KDictSpellingHighlighter : public KSpellingHighlighter
{
  Q_OBJECT

  public:
    /**
     * Creates a new dictionary spelling highlighter.
     *
     * @param textEdit The QTextEdit the dictionary spelling highlighter should work on.
     * @param spellCheckingActive If true, spellchecking support is enabled.
     * @param spellColor The color used for misspelled words.
     * @param colorQuoting If true ... ( ?!? )
     * @param quoteColor0 The color used for standard text.
     * @param quoteColor1 The color used for indention of first level.
     * @param quoteColor2 The color used for indention of second level.
     * @param quoteColor3 The color used for indention of third level.
     * @param spellConfig The configuration object of the spell checker.
     */
    KDictSpellingHighlighter( QTextEdit *textEdit,
                              bool spellCheckingActive = true,
                              bool autoEnable = true,
                              const QColor& spellColor = Qt::red,
                              bool colorQuoting = false,
                              const QColor& QuoteColor0 = Qt::black,
                              const QColor& QuoteColor1 = QColor( 0x00, 0x80, 0x00 ),
                              const QColor& QuoteColor2 = QColor( 0x00, 0x70, 0x00 ),
                              const QColor& QuoteColor3 = QColor( 0x00, 0x60, 0x00 ),
                              KSpellConfig *spellConfig = 0 );

    /**
     * Destroys the dictionary spelling highlighter.
     */
    ~KDictSpellingHighlighter();

    /**
     * Returns whether the given @param word is misspelled.
     */
    virtual bool isMisspelled( const QString &word );

    /**
     * Reinitializes the dictionary.
     */
    static void dictionaryChanged();

    /**
     * Restarts the background spellchecking process.
     */
    void restartBackgroundSpellCheck();

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

    /**
     * @short En-/Disable automatic (de)activation in case of too many errors.
     *
     * If @p automatic is true then spell checking will be deactivated if
     * too many words were misspelled and spell checking will be activated
     * again if the amount of misspelled words drop below a certain threshold.
     *
     * @param automatic if true, then automatic (de)activation is enabled
     *
     * @see automatic()
     */
    void setAutomatic( bool automatic );

    /**
     * Returns the state of automatic (de)activation.
     *
     * @return true if automatic (de)activation is enabled
     *
     * @see setAutomatic()
     */
    bool automatic() const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the 'as-you-type' spellchecking is enabled
     * or disabled.
     *
     * @param description A i18n'ed description of the state change.
     */
    void activeChanged( const QString &description );

    /**
     * This signal is emitted whenever a misspelled word is detected.
     *
     * @param originalWord The misspelled word.
     * @param suggestions A list of possible replacement words.
     * @param position The position of the misspelled word in the text.
     */
    void newSuggestions( const QString &originalWord, const QStringList &suggestions,
                         unsigned int position );

  protected:
    QString spellKey();
    bool eventFilter( QObject*, QEvent* );

  protected Q_SLOTS:
    void slotMisspelling( const QString&, const QStringList&, unsigned int );
    void slotCorrected( const QString&, const QString&, unsigned int );
    void slotRehighlight();
    void slotDictionaryChanged();
    void slotSpellReady( KSpell* );
    void slotAutoDetection();
    void slotLocalSpellConfigChanged();
    void slotKSpellNotResponding();

  private:
    class Private;
    Private *const d;
};

#endif
