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

#include "kfind.h"
#include "kfinddialog.h"
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qregexp.h>
#include <kdebug.h>

//#define DEBUG_FIND

class KFindNextDialog : public KDialogBase
{
public:
    KFindNextDialog(const QString &pattern, QWidget *parent);
};

// Create the dialog.
KFindNextDialog::KFindNextDialog(const QString &pattern, QWidget *parent) :
    KDialogBase(parent, 0, false,  // non-modal!
        i18n("Find"),
        User1 | Close,
        User1,
        false,
        i18n("&Yes"))
{
    setMainWidget( new QLabel( i18n("<qt>Find next occurrence of '<b>%1</b>'?</qt>").arg(pattern), this ) );
}

////

KFind::KFind( const QString &pattern, long options, QWidget *parent )
    : QObject( parent )
{
    m_options = options;
    init( pattern );
}

void KFind::init( const QString& pattern )
{
    m_matches = 0;
    m_pattern = pattern;
    m_dialog = 0;
    m_dialogClosed = false;
    m_index = -1;
    if (m_options & KFindDialog::RegularExpression)
        m_regExp = new QRegExp(pattern, m_options & KFindDialog::CaseSensitive);
    else {
        m_regExp = 0;
    }
}

KFind::~KFind()
{
    delete m_dialog;
}

bool KFind::needData() const
{
    // always true when m_text is empty.
    if (m_options & KFindDialog::FindBackwards)
        return m_index < 0;
    else
        return m_index >= (int)m_text.length() || m_index == -1;
}

void KFind::setData( const QString& data, int startPos )
{
    m_text = data;
    if ( startPos != -1 )
        m_index = startPos;
    else if (m_options & KFindDialog::FindBackwards)
        m_index = QMAX( (int)m_text.length() - 1, 0 );
    else
        m_index = 0;
#ifdef DEBUG_FIND
    kdDebug() << "setData: '" << m_text << "' m_index=" << m_index << endl;
#endif
    Q_ASSERT( m_index != -1 );
}

KDialogBase* KFind::findNextDialog( bool create )
{
    if ( !m_dialog && create )
    {
        m_dialog = new KFindNextDialog( m_pattern, parentWidget() );
        connect( m_dialog, SIGNAL( user1Clicked() ), this, SLOT( slotFindNext() ) );
        connect( m_dialog, SIGNAL( finished() ), this, SLOT( slotDialogClosed() ) );
    }
    return m_dialog;
}

KFind::Result KFind::find()
{
    Q_ASSERT( m_index != -1 );
    if ( m_text.isEmpty() ) {
#ifdef DEBUG_FIND
        kdDebug() << k_funcinfo << "Empty -> NoMatch" << endl;
#endif
        m_index = -1;
        return NoMatch;
    }

#ifdef DEBUG_FIND
    kdDebug() << k_funcinfo << "m_index=" << m_index << endl;
#endif
    do
    {
        // Find the next match.
        if (m_options & KFindDialog::RegularExpression)
            m_index = KFind::find(m_text, *m_regExp, m_index, m_options, &m_matchedLength);
        else
            m_index = KFind::find(m_text, m_pattern, m_index, m_options, &m_matchedLength);
        if (m_index != -1)
        {
            // Flexibility: the app can add more rules to validate a possible match
            if ( validateMatch( m_text, m_index, m_matchedLength ) )
            {
                m_matches++;
                // Tell the world about the match we found, in case someone wants to
                // highlight it.
                emit highlight(m_text, m_index, m_matchedLength);

                if ( !m_dialogClosed )
                    findNextDialog(true)->show();

                // Get ready for next match
                if (m_options & KFindDialog::FindBackwards)
                    m_index--;
                else
                    m_index++;
#ifdef DEBUG_FIND
                kdDebug() << k_funcinfo << "Match. Next m_index=" << m_index << endl;
#endif
                return Match;
            }
            else // Skip match
                if (m_options & KFindDialog::FindBackwards)
                    m_index -= m_matchedLength;
                else
                    m_index += m_matchedLength;
        }
    }
    while (m_index != -1);

#ifdef DEBUG_FIND
    kdDebug() << k_funcinfo << "NoMatch. m_index=" << m_index << endl;
#endif
    return NoMatch;
}

// static
int KFind::find(const QString &text, const QString &pattern, int index, long options, int *matchedLength)
{
    // Handle regular expressions in the appropriate way.
    if (options & KFindDialog::RegularExpression)
    {
        QRegExp regExp(pattern, options & KFindDialog::CaseSensitive);

        return find(text, regExp, index, options, matchedLength);
    }

    bool caseSensitive = (options & KFindDialog::CaseSensitive);

    if (options & KFindDialog::WholeWordsOnly)
    {
        if (options & KFindDialog::FindBackwards)
        {
            // Backward search, until the beginning of the line...
            while (index >= 0)
            {
                // ...find the next match.
                index = text.findRev(pattern, index, caseSensitive);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                *matchedLength = pattern.length();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index--;
            }
        }
        else
        {
            // Forward search, until the end of the line...
            while (index < (int)text.length())
            {
                // ...find the next match.
                index = text.find(pattern, index, caseSensitive);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                *matchedLength = pattern.length();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index++;
            }
            if (index >= (int)text.length()) // end of line
                index = -1; // not found
        }
    }
    else
    {
        // Non-whole-word search.
        if (options & KFindDialog::FindBackwards)
        {
            index = text.findRev(pattern, index, caseSensitive);
        }
        else
        {
            index = text.find(pattern, index, caseSensitive);
        }
        if (index != -1)
        {
            *matchedLength = pattern.length();
        }
    }
    return index;
}

// static
int KFind::find(const QString &text, const QRegExp &pattern, int index, long options, int *matchedLength)
{
    if (options & KFindDialog::WholeWordsOnly)
    {
        if (options & KFindDialog::FindBackwards)
        {
            // Backward search, until the beginning of the line...
            while (index >= 0)
            {
                // ...find the next match.
                index = text.findRev(pattern, index);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                //pattern.match(text, index, matchedLength, false);
                /*int pos =*/ pattern.search( text.mid(index) );
                *matchedLength = pattern.matchedLength();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index--;
            }
        }
        else
        {
            // Forward search, until the end of the line...
            while (index < (int)text.length())
            {
                // ...find the next match.
                index = text.find(pattern, index);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                //pattern.match(text, index, matchedLength, false);
                /*int pos =*/ pattern.search( text.mid(index) );
                *matchedLength = pattern.matchedLength();
                if (isWholeWords(text, index, *matchedLength))
                    break;
                index++;
            }
            if (index >= (int)text.length()) // end of line
                index = -1; // not found
        }
    }
    else
    {
        // Non-whole-word search.
        if (options & KFindDialog::FindBackwards)
        {
            index = text.findRev(pattern, index);
        }
        else
        {
            index = text.find(pattern, index);
        }
        if (index != -1)
        {
            //pattern.match(text, index, matchedLength, false);
            /*int pos =*/ pattern.search( text.mid(index) );
            *matchedLength = pattern.matchedLength();
        }
    }
    return index;
}

bool KFind::isInWord(QChar ch)
{
    return ch.isLetter() || ch.isDigit() || ch == '_';
}

bool KFind::isWholeWords(const QString &text, int starts, int matchedLength)
{
    if ((starts == 0) || (!isInWord(text[starts - 1])))
    {
        int ends = starts + matchedLength;

        if ((ends == (int)text.length()) || (!isInWord(text[ends])))
            return true;
    }
    return false;
}

void KFind::slotFindNext()
{
    emit findNext();
}

void KFind::slotDialogClosed()
{
    emit dialogClosed();
    m_dialogClosed = true;
}

void KFind::displayFinalDialog() const
{
    QString message;
    if ( numMatches() )
        message = i18n( "1 match found.", "%n matches found.", numMatches() );
    else
        message = i18n("<qt>No matches found for '<b>%1</b>'.</qt>").arg(m_pattern);
    KMessageBox::information(parentWidget(), message);
}

bool KFind::shouldRestart( bool forceAsking, bool showNumMatches ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (m_options & KFindDialog::FromCursor) == 0 )
    {
        displayFinalDialog();
        return false;
    }
    QString message;
    if ( showNumMatches )
    {
        if ( numMatches() )
            message = i18n( "1 match found.", "%n matches found.", numMatches() );
        else
            message = i18n("No matches found for '<b>%1</b>'.").arg(m_pattern);
    }
    else
    {
        if ( m_options & KFindDialog::FindBackwards )
            message = i18n( "Beginning of document reached." );
        else
            message = i18n( "End of document reached." );
    }

    message += "\n"; // can't be in the i18n() of the first if() because of the plural form.
    // Hope this word puzzle is ok, it's a different sentence
    message += i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionYesNo( parentWidget(), QString("<qt>")+message+QString("</qt>") );
    bool yes = ( ret == KMessageBox::Yes );
    if ( yes )
        const_cast<KFind*>(this)->m_options &= ~KFindDialog::FromCursor; // clear FromCursor option
    return yes;
}

void KFind::setOptions( long options )
{
    // For now always true. But if we ever implement "letting the app say
    // where 'find next' should start from", like kwrite/kate does, then
    // this adjustement must be disabled (by some future method).
    //bool adjust = true;
    if ( /*adjust &&*/ m_options != options )
    {
        bool wasBack = (m_options & KFindDialog::FindBackwards);
        bool isBack = (options & KFindDialog::FindBackwards);
#ifdef DEBUG_FIND
        kdDebug() << k_funcinfo << "wasBack=" << wasBack << " isBack=" << isBack << " m_index=" << m_index << endl;
#endif
        // If we changed direction, the "++ or --" done right after the last match
        // was done in the wrong direction. So undo it, and go the other way, to
        // avoid catching the same match again.
        if ( wasBack && !isBack )
            m_index += 2;
        else if ( !wasBack && isBack ) {
            // If m_index==-1, setData will be called anyway.
            m_index = QMAX( -1, m_index - 2 );
        }
#ifdef DEBUG_FIND
        kdDebug() << "setOptions: m_index now " << m_index << endl;
#endif
    }
    m_options = options;
}

void KFind::closeFindNextDialog()
{
    delete m_dialog;
    m_dialog = 0L;
    m_dialogClosed = true;
}

#include "kfind.moc"
