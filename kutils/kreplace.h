/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KREPLACE_H
#define KREPLACE_H

#include "kfind.h"

class KReplaceNextDialog;

/**
 * @short A generic implementation of the "replace" function.
 *
 * @author S.R.Haque <srhaque@iee.org>, David Faure <faure@kde.org>
 *
 * @sect Detail
 *
 * This class includes prompt handling etc. Also provides some
 * static functions which can be used to create custom behaviour
 * instead of using the class directly.
 *
 * @sect Example
 *
 * To use the class to implement a complete replace feature:
 *
 * In the slot connect to the replace action, after using KReplaceDialog:
 * <pre>
 *
 *  // This creates a replace-on-prompt dialog if needed.
 *  m_replace = new KReplace(pattern, replacement, options);
 *
 *  // Connect signals to code which handles highlighting
 *  // of found text, and on-the-fly replacement.
 *  connect( m_replace, SIGNAL( highlight( const QString &, int, int ) ),
 *          this, SLOT( slotHighlight( const QString &, int, int ) ) );
 *  // Connect findNext signal - called when pressing the button in the dialog
 *  connect( m_replace, SIGNAL( findNext() ),
 *          this, SLOT( slotReplaceNext() ) );
 *  // Connect replace signal - called when doing a replacement
 *  connect( m_replace, SIGNAL( replace(const QString &, int, int, int) ),
 *          this, SLOT( slotReplace(const QString &, int, int, int) ) );
 *
 *  Then initialize the variables determining the "current position"
 *  (to the cursor, if the option FromCursor is set,
 *   to the beginning of the selection if the option SelectedText is set,
 *   and to the beginning of the document otherwise).
 *  Initialize the "end of search" variables as well (end of doc or end of selection).
 *  Swap begin and end if FindBackwards.
 *  Finally, call slotReplaceNext();
 *
 * <pre>
 *  void slotReplaceNext()
 *  {
 *      KFind::Result res = KFind::NoMatch;
 *      while ( res == KFind::NoMatch && <position not at end> ) {
 *          if ( m_replace->needData() )
 *              m_replace->setData( <current text fragment> );
 *
 *          // Let KReplace inspect the text fragment, and display a dialog if a match is found
 *          res = m_replace->res();
 *
 *          if ( res == KFind::NoMatch ) {
 *              <Move to the next non-empty text fragment, honouring the FindBackwards setting for the direction>
 *          }
 *      }
 *
 *      if ( res == KFind::NoMatch ) // i.e. at end
 *          <Call either  m_replace->displayFinalDialog(); delete m_replace; m_replace = 0L;
 *           or           if ( m_replace->shouldRestart() ) { reinit and call slotReplaceNext(); }
 *                        else { delete m_replace; m_replace = 0L; }>
 *  }
 * </pre>
 *
 *  Don't forget delete m_find in the destructor of your class,
 *  unless you gave it a parent widget on construction.
 *
 * </pre>
 */
class KReplace :
    public KFind
{
    Q_OBJECT

public:

    /** Will create a prompt dialog and use it as needed. */
    KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent = 0);
    virtual ~KReplace();

    /**
     * Return the number of replacements made (i.e. the number of times
     * the @ref replace signal was emitted).
     * Can be used in a dialog box to tell the user how many replacements were made.
     * The final dialog does so already, unless you used setDisplayFinalDialog(false).
     */
    int numReplacements() const { return m_replacements; }

    /**
     * Call this to reset the numMatches & numReplacements counts.
     * Can be useful if reusing the same KReplace for different operations,
     * or when restarting from the beginning of the document.
     */
    virtual void resetCounts();

    /**
     * Walk the text fragment (e.g. kwrite line, kspread cell) looking for matches.
     * For each match, if prompt-on-replace is specified, emits the highlight() signal
     * and displays the prompt-for-replace dialog before doing the replace.
     */
    Result replace();

    /**
     * Return (or create) the dialog that shows the "find next?" prompt.
     * Usually you don't need to call this.
     * One case where it can be useful, is when the user selects the "Find"
     * menu item while a find operation is under way. In that case, the
     * program may want to call setActiveWindow() on that dialog.
     */
    KDialogBase* replaceNextDialog( bool create = false );

    /**
     * Close the "replace next?" dialog. The application should do this when
     * the last match was hit. If the application deletes the KReplace, then
     * "find previous" won't be possible anymore.
     */
    void closeReplaceNextDialog();

    /**
     * Search the given string, replaces with the given replacement string,
     * and returns whether a match was found. If one is,
     * the replacement string length is also returned.
     *
     * A performance optimised version of the function is provided for use
     * with regular expressions.
     *
     * @param text The string to search.
     * @param pattern The pattern to look for.
     * @param replacement The replacement string to insert into the text.
     * @param index The starting index into the string.
     * @param options The options to use.
     * @param replacedLength Output parameter, contains the length of the replaced string.
     * Not always the same as replacement.length(), when backreferences are used.
     * @return The index at which a match was found, or -1 if no match was found.
     */
    static int replace( QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength );
    static int replace( QString &text, const QRegExp &pattern, const QString &replacement, int index, long options, int *replacedLength );

    /**
     * Returns true if we should restart the search from scratch.
     * Can ask the user, or return false (if we already searched/replaced the
     * whole document without the PromptOnReplace option).
     *
     * @param forceAsking set to true if the user modified the document during the
     * search. In that case it makes sense to restart the search again.
     *
     * @param showNumMatches set to true if the dialog should show the number of
     * matches. Set to false if the application provides a "find previous" action,
     * in which case the match count will be erroneous when hitting the end,
     * and we could even be hitting the beginning of the document (so not all
     * matches have even been seen).
     */
    virtual bool shouldRestart( bool forceAsking = false, bool showNumMatches = true ) const;

    /**
     * Displays the final dialog telling the user how many replacements were made.
     * Call either this or shouldRestart().
     */
    virtual void displayFinalDialog() const;

signals:

    /**
     * Connect to this slot to implement updating of replaced text during the replace
     * operation.
     *
     * Extra care must be taken to properly implement the "no prompt-on-replace" case.
     * For instance @ref highlight isn't emitted in that case (some code might rely on it),
     * and for performance reasons one should repaint after replace() ONLY if
     * prompt-on-replace was selected.
     *
     * @param text The text, in which the replacement has already been done.
     * @param replacementIndex Starting index of the matched substring
     * @param replacedLength Length of the replacement string
     * @param matchedLength Length of the matched string
     */
    void replace(const QString &text, int replacementIndex, int replacedLength, int matchedLength);

protected slots:

    void slotSkip();
    void slotReplace();
    void slotReplaceAll();

private:
    KReplaceNextDialog* dialog();
    void doReplace();
    static int replace( QString &text, const QString &replacement, int index, int length );

    QString m_replacement;
    unsigned m_replacements;

    // Binary compatible extensibility.
    class KReplacePrivate;
    KReplacePrivate *d;
};
#endif
