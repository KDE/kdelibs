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
 * To use the class to implement a complete find feature:
 *
 * In the slot connected to the find action:
 * <pre>
 *
 *  // This creates a find-next-prompt dialog if needed.
 *  m_find = new KFind(pattern, options);
 *
 *  // Connect highlight signal to code which handles highlighting
 *  // of found text.
 *  connect(m_find, SIGNAL( highlight( const QString &, int, int ) ),
 *          this, SLOT( slotHighlight( const QString &, int, int ) ) );
 *  // Connect findNext signal - called when pressing the button in the dialog
 *  connect(m_find, SIGNAL( findNext() ),
 *          this, SLOT( slotFindNext() ) );
 * </pre>
 *
 *  Then initialize the variables determining the "current position"
 *  (to the cursor, if the option FromCursor is set,
 *   to the beginning of the selection if the option SelectedText is set,
 *   and to the beginning of the document otherwise).
 *  Initialize the "end of search" variables as well (end of doc or end of selection).
 *  Finally, call slotFindNext();
 *
 * <pre>
 *  void slotFindNext()
 *  {
 *      KFind::Result res = KFind::NoMatch;
 *      while ( res == KFind::NoMatch && <position not at end> ) {
 *          if ( m_find->needData() )
 *              m_find->setData( <current text fragment> );
 *
 *          // Let KFind inspect the text fragment, and display a dialog if a match is found
 *          res = m_find->find();
 *
 *          if ( res == KFind::NoMatch ) {
 *              <Move to the next non-empty text fragment, honouring the FindBackwards setting for the direction>
 *          }
 *      }
 *
 *      if ( res == KFind::NoMatch ) // i.e. at end
 *          <Call either  m_find->displayFinalDialog(); delete m_find; m_find = 0L;
 *           or           if ( m_find->shouldRestart() ) { reinit and call slotFindNext(); }
                          else { delete m_find; m_find = 0L; }>
 *  }
 * </pre>
 *
 *  Don't forget delete m_find in the destructor of your class,
 *  unless you gave it a parent widget on construction.
 *
 *  This implementation allows to have a "Find Next" action, which resumes the
 *  search, even if the user closed the "Find Next" dialog.
 *
 *  A "Find Previous" action can simply switch temporarily the value of
 *  FindBackwards and call slotFindNext() - and reset the value afterwards.
 */
class KFind :
    public QObject
{
    Q_OBJECT

public:

    KFind(const QString &pattern, long options, QWidget *parent);
    virtual ~KFind();

    enum Result { NoMatch, Match };

    /**
     * @return true if the application must supply a new text fragment
     * It also means the last call returned "NoMatch". But by storing this here
     * the application doesn't have to store it in a member variable (between
     * calls to slotFindNext()).
     */
    bool needData() const;
    /**
     * Call this when needData returns true, before calling @ref find().
     * @param data the text fragment (line)
     * @param startPos if set, the index at which the search should start.
     * This is only necessary for the very first call to setData usually,
     * for the 'find in selection' feature. A value of -1 (the default value)
     * means "process all the data", i.e. either 0 or data.length()-1 depending
     * on FindBackwards.
     */
    void setData( const QString& data, int startPos = -1 );

    /**
     * Walk the text fragment (e.g. text-processor line, kspread cell) looking for matches.
     * For each match, emits the highlight() signal and displays the find-again dialog
     * proceeding.
     */
    Result find();

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
     * Displays the final dialog saying "no match was found", if that was the case.
     * Call either this or shouldRestart().
     */
    virtual void displayFinalDialog() const;

signals:

    /**
     * Connect to this signal to implement highlighting of found text during the find
     * operation.
     */
    void highlight(const QString &text, int matchingIndex, int matchedLength);

    // ## TODO docu
    // findprevious will also emit findNext, after temporarily switching the value
    // of FindBackwards
    void findNext();

protected:
    /**
     * @internal Constructor for KReplace
     */
    KFind(const QString &pattern, const QString &replacement, long options, QWidget *parent);

    QWidget* parentWidget() const { return (QWidget *)parent(); }

protected slots:

    void slotFindNext();
    void slotDialogClosed() { m_dialogClosed = true; }

private:
    void init( const QString& pattern );
    KDialogBase* dialog();

    static bool isInWord( QChar ch );
    static bool isWholeWords( const QString &text, int starts, int matchedLength );

    friend class KReplace;


    QString m_pattern;
    QRegExp *m_regExp;
    KDialogBase* m_dialog;
    long m_options;
    unsigned m_matches;

    QString m_text; // the text set by setData
    int m_index;
    int m_matchedLength;
    bool m_dialogClosed;

    // Binary compatible extensibility.
    class KFindPrivate;
    KFindPrivate *d;
};

#endif
