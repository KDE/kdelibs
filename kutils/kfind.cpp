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
    setMainWidget( new QLabel( i18n("Find next '%1'").arg(pattern), this ) );
    resize(minimumSize());
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
    //kdDebug() << k_funcinfo << " m_index=" << m_index << endl;
    Q_ASSERT( m_index != -1 );
}

KDialogBase* KFind::dialog()
{
    if ( !m_dialog )
    {
        m_dialog = new KFindNextDialog( m_pattern, parentWidget() );
        connect( m_dialog, SIGNAL( user1Clicked() ), this, SLOT( slotFindNext() ) );
        connect( m_dialog, SIGNAL( finished() ), this, SLOT( slotDialogClosed() ) );
    }
    return m_dialog;
}

KFind::Result KFind::find()
{
    //kdDebug() << k_funcinfo << "m_index=" << m_index << endl;
    Q_ASSERT( !needData() ); // happens if setData(QString::null) was called -> don't do that
    Q_ASSERT( m_index != -1 );
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
                    dialog()->show();
                // Get ready for next match
                if (m_options & KFindDialog::FindBackwards)
                    m_index--;
                else
                    m_index++;
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

void KFind::displayFinalDialog() const
{
    QString message;
    if ( numMatches() )
        message = i18n( "1 match found.", "%n matches found.", numMatches() );
    else
        message = i18n("No matches found.");
    KMessageBox::information(parentWidget(), message);
}

bool KFind::shouldRestart( bool forceAsking ) const
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
    if ( numMatches() )
        message = i18n( "1 match found.", "%n matches found.", numMatches() );
    else
        message = i18n("No matches found.");
    message += "\n"; // can't be in the i18n() because of the plural form.
    // Hope this word puzzle is ok, it's a different sentence
    message += i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionYesNo( parentWidget(), message );
    bool yes = ( ret == KMessageBox::Yes );
    if ( yes )
        const_cast<KFind*>(this)->m_options &= ~KFindDialog::FromCursor; // clear FromCursor option
    return yes;
}

#include "kfind.moc"
