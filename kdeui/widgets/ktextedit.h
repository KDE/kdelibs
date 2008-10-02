/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KTEXTEDIT_H
#define KTEXTEDIT_H

#include <kdeui_export.h>
#include <sonnet/highlighter.h>
#include <QtGui/QTextEdit>

/**
 * @short A KDE'ified QTextEdit
 *
 * This is just a little subclass of QTextEdit, implementing
 * some standard KDE features, like cursor auto-hiding, configurable
 * wheelscrolling (fast-scroll or zoom), spell checking and deleting of entire
 * words with Ctrl-Backspace or Ctrl-Delete.
 *
 * This text edit provides two ways of spell checking: background checking,
 * which will mark incorrectly spelled words red, and a spell check dialog,
 * which lets the user check and correct all incorrectly spelled words.
 *
 * Basic rule: whenever you want to use QTextEdit, use KTextEdit!
 *
 * @see QTextEdit
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KDEUI_EXPORT KTextEdit : public QTextEdit
{
    Q_OBJECT

  public:
    /**
     * Constructs a KTextEdit object. See QTextEdit::QTextEdit
     * for details.
     */
    explicit KTextEdit( const QString& text, QWidget *parent = 0 );

    /**
     * Constructs a KTextEdit object. See QTextEdit::QTextEdit
     * for details.
     */
    explicit KTextEdit( QWidget *parent = 0 );

    /**
     * Destroys the KTextEdit object.
     */
    ~KTextEdit();

    /**
     * Reimplemented to set a proper "deactivated" background color.
     */
    virtual void setReadOnly( bool readOnly );

    /**
     * Turns background spell checking for this text edit on or off.
     * Note that spell checking is only available in read-writable KTextEdits.
     *
     * Enabling spell checking will set back the current highlighter to the one
     * returned by createHighlighter().
     *
     * @see checkSpellingEnabled()
     * @see isReadOnly()
     * @see setReadOnly()
     */
    void setCheckSpellingEnabled( bool check );

    /**
     * Returns true if background spell checking is enabled for this text edit.
     * Note that it even returns true if this is a read-only KTextEdit,
     * where spell checking is actually disabled.
     * By default spell checking is disabled.
     *
     * @see setCheckSpellingEnabled()
     */
    bool checkSpellingEnabled() const;

    /**
     * Selects the characters at the specified position. Any previous
     * selection will be lost. The cursor is moved to the first character
     * of the new selection.
     *
     * @param length The length of the selection, in number of characters
     * @param pos The position of the first character of the selection
     */
    void highlightWord( int length, int pos );

    /**
     * Allows to override the config file where the settings for spell checking,
     * like the current language or encoding, are stored.
     * By default, the global config file (kdeglobals) is used, to share
     * spell check settings between all applications.
     *
     * This has to be called before any spell checking is initiated.
     *
     * @param fileName the URL of the config file which will be used to
     *                 read spell settings
     * @bug this has no effect for the spell dialog, only for the background
     *      check
     */
    void setSpellCheckingConfigFileName(const QString &fileName);

     /**
      * change default language for spell checking dialogbox
      * @since 4.1
      */
    void setSpellCheckingLanguage(const QString &language);

    /**
     * Allows to create a specific highlighter if reimplemented.
     *
     * By default, it creates a normal highlighter, based on the config
     * file given to setSpellCheckingConfigFileName().
     *
     * This highlighter is set each time spell checking is toggled on by
     * calling setCheckSpellingEnabled(), but can later be overridden by calling
     * setHighlighter().
     *
     * @see setHighlighter()
     * @see highlighter()
     * @see setSpellCheckingConfigFileName()
     */
    virtual void createHighlighter();

    /**
     * Returns the current highlighter, which is 0 if spell checking is disabled.
     * The default highlighter is the one created by createHighlighter(), but
     * might be overridden by setHighlighter().
     *
     * @see setHighlighter()
     * @see createHighlighter()
     */
    Sonnet::Highlighter* highlighter() const;

    /**
     * Sets a custom backgound spell highlighter for this text edit.
     * Normally, the highlighter returned by createHighlighter() will be
     * used to detect and highlight incorrectly spelled words, but this
     * function allows to set a custom highlighter.
     *
     * This has to be called after enabling spell checking with
     * setCheckSpellingEnabled(), otherwise it has no effect.
     *
     * @see highlighter()
     * @see createHighlighter()
     * @param highLighter the new highlighter which will be used now
     */
    void setHighlighter(Sonnet::Highlighter *_highLighter);

    /**
     * Return standard KTextEdit popupMenu
     * @since 4.1
     */
    QMenu *mousePopupMenu();

    /**
     * Enable find replace action.
     * @since 4.1
     */
    void enableFindReplace( bool enabled);


  Q_SIGNALS:
    /**
     * emit signal when we activate or not autospellchecking
     *
     * @since 4.1
     */
     void checkSpellingChanged( bool );

     /**
      * Signal sends when spell checking is finished/stopped/completed
      * @since 4.1
      */
     void spellCheckStatus(const QString &);

     /**
      * Emitted when the user changes the language in the spellcheck dialog
      * shown by checkSpelling().
      *
      * @param language the new language the user selected
      * @since 4.1
      */
     void languageChanged(const QString &language);

  public Q_SLOTS:
    /**
     * Create a modal dialog to check the spelling.  This slot will not return
     * until spell checking has been completed.
     */
    void checkSpelling();

    /**
     * Create replace dialogbox
     * @since 4.1
     */
    void replace();

  protected Q_SLOTS:
    /**
     * @since 4.1
     */
    void slotDoReplace();
    void slotReplaceNext();
    void slotDoFind();
    void slotFind();
    void slotFindNext();
    void slotReplace();


  protected:
    /**
     * Reimplemented to catch "delete word" shortcut events.
     */
    virtual bool event(QEvent*);

    /**
     * Reimplemented for internal reasons
     */
    virtual void keyPressEvent( QKeyEvent* );

    /**
     * Reimplemented to instantiate a KDictSpellingHighlighter, if
     * spellchecking is enabled.
     */
    virtual void focusInEvent( QFocusEvent* );

    /**
     * Reimplemented to allow fast-wheelscrolling with Ctrl-Wheel
     * or zoom.
     */
    virtual void wheelEvent( QWheelEvent* );

    /**
     * Deletes a word backwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordBack();

    /**
     * Deletes a word forwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordForward();

    /**
     * Reimplemented from QTextEdit to add spelling related items
     * when appropriate.
     */
    virtual void contextMenuEvent( QContextMenuEvent* );

  private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void slotSpellCheckDone( const QString& ) )

    Q_PRIVATE_SLOT( d, void spellCheckerMisspelling( const QString&, int ) )
    Q_PRIVATE_SLOT( d, void spellCheckerCorrected(const QString&, int,const QString&) )
    Q_PRIVATE_SLOT( d, void spellCheckerCanceled())
    Q_PRIVATE_SLOT( d, void spellCheckerAutoCorrect(const QString&,const QString&) )
    Q_PRIVATE_SLOT( d, void spellCheckerFinished() )
    Q_PRIVATE_SLOT( d, void undoableClear() )
    Q_PRIVATE_SLOT( d, void toggleAutoSpellCheck() )
    Q_PRIVATE_SLOT( d, void slotAllowTab() )
    Q_PRIVATE_SLOT( d, void menuActivated( QAction* ) )
    Q_PRIVATE_SLOT( d, void slotFindHighlight(const QString&, int, int))
    Q_PRIVATE_SLOT( d, void slotReplaceText(const QString &, int, int, int))
};

#endif // KTEXTEDIT_H
