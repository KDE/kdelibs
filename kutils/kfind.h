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

#ifndef KFIND_H
#define KFIND_H

#include <kdialogbase.h>
#include <qrect.h>

/**
 * @short A generic implementation of the "find" function.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 * @sect Detail
 *
 * This class includes prompt handling etc. Also provides some
 * static functions which can be used to create custom behaviour
 * instead of using the class directly.
 *
 * @sect Example
 *
 * To use the class to implement a complete find feature:
 *
 * <pre>
 *
 *  // This creates a find-next-prompt dialog if needed.
 *  dialog = new KFind(find, options);
 *
 *  // Connect signals to code which handles highlighting
 *  // of found text.
 *  connect(dialog, SIGNAL( highlight( const QString &, int, int, const QRect & ) ),
 *          this, SLOT( highlight( const QString &, int, int, const QRect & ) ) );
 *
 *  // Loop over all the text fragments of our document or selection
 *  for (text chosen by option SelectedText and in a direction set by FindBackwards)
 *       // don't forget to honour FromCursor too
 *  {
 *      // Let KFind inspect the text fragment, and display a dialog if a match is found
 *      if ( !dialog->find( text_fragment, region_to_expose ) )
 *          break; // if cancelled by user
 *  }
 *  delete dialog;
 *
 * </pre>
 */
class KFind :
    public KDialogBase
{
    Q_OBJECT

public:

    /** Will create a prompt dialog and use it as needed. */
    KFind(const QString &pattern, long options, QWidget *parent = 0);
    virtual ~KFind();

    /**
     * Walk the text fragment (e.g. kwrite line, kspread cell) looking for matches.
     * For each match, emits the expose() signal and displays the find-again dialog
     * proceeding.
     *
     * @param text The text fragment to modify.
     * @param expose The region to expose
     * @return false if the user elected to discontinue the find.
     */
    bool find(const QString &text, const QRect &expose);

    /**
     * Return the current options.
     *
     * Warning: this is usually the same value as the one passed to the constructor,
     * but options might change _during_ the replace operation:
     * e.g. the "All" button resets the PromptOnReplace flag.
     *
     */
    long options() const { return m_options; }

    /**
     * Return the number of matches found (i.e. the number of times
     * the @ref highlight signal was emitted).
     * If 0, can be used in a dialog box to tell the user "no match was found".
     * The final dialog does so already, unless you used setDisplayFinalDialog(false).
     */
    int numMatches() const { return m_matches; }

    /**
     * Call this to reset the numMatches count
     * (and the numReplacements count for a KReplace).
     * Can be useful if reusing the same KReplace for different operations,
     * or when restarting from the beginning of the document.
     */
    virtual void resetCounts() { m_matches = 0; }

    /**
     * Virtual method, which allows applications to add extra checks for
     * validating a candidate match. It's only necessary to reimplement this
     * if the find dialog extension has been used to provide additional
     * criterias.
     *
     * @param text  The current text fragment
     * @param index The starting index where the candidate match was found
     * @param matchedlength The length of the candidate match
     */
    virtual bool validateMatch( const QString &/*text*/, int /*index*/, int /*matchedlength*/ ) { return true; }

    /**
     * Returns true if we should restart the search from scratch.
     * Can ask the user, or return false (if we already searched the whole document).
     *
     * @param forceAsking set to true if the user modified the document during the
     * search. In that case it makes sense to restart the search again.
     */
    virtual bool shouldRestart( bool forceAsking = false ) const;

    /**
     * Search the given string, and returns whether a match was found. If one is,
     * the length of the string matched is also returned.
     *
     * A performance optimised version of the function is provided for use
     * with regular expressions.
     *
     * @param text The string to search.
     * @param pattern The pattern to look for.
     * @param index The starting index into the string.
     * @param options. The options to use.
     * @return The index at which a match was found, or -1 if no match was found.
     */
    static int find( const QString &text, const QString &pattern, int index, long options, int *matchedlength );
    static int find( const QString &text, const QRegExp &pattern, int index, long options, int *matchedlength );

    /**
     * Abort the current find process. Call this when the parent widget
     * is getting destroyed.
     */
    void abort();

    /**
     * Sets whether the final dialog saying "no match was found"
     * or "N replacements were made" should be displayed.
     * This is true by default, but some apps might want to deactivate this
     * to display a dialog that includes "do you want to start again"?
     *
     * The final dialog is displayed by the destructor (when abort() wasn't called
     * and the user didn't press Cancel).
     */
    void setDisplayFinalDialog( bool b ) { m_displayFinalDialog = b; }
    bool displayFinalDialog() const { return m_displayFinalDialog; }

protected:
    /**
     * @internal Constructor for KReplace
     */
    KFind(const QString &pattern, const QString &replacement, long options, QWidget *parent);

signals:

    /**
     * Connect to this signal to implement highlighting of found text during the find
     * operation.
     */
    void highlight(const QString &text, int matchingIndex, int matchedLength, const QRect &expose);

private:
    void init( const QString& pattern );


    QString m_pattern;
    QRegExp *m_regExp;
    long m_options;
    unsigned m_matches;
    QString m_text;
    QRect m_expose;
    int m_index;
    int m_matchedLength;
    bool m_cancelled;
    bool m_displayFinalDialog;

    static bool isInWord( QChar ch );
    static bool isWholeWords( const QString &text, int starts, int matchedLength );

    friend class KReplace;

    // Binary compatible extensibility.
    class KFindPrivate;
    KFindPrivate *d;

private slots:

    virtual void slotUser1();   // Yes
    virtual void slotClose();
};

#endif
