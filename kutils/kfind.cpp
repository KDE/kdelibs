/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    Copyright (C) 2004, Arend van Beelen jr. <arend@auton.nl>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfind.h"
#include "kfinddialog.h"

#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qregexp.h>
#include <qpointer.h>
#include <qhash.h>
#include <QTextDocument>

//#define DEBUG_FIND

#define INDEX_NOMATCH -1

class KFindNextDialog : public KDialogBase
{
public:
    KFindNextDialog(const QString &pattern, QWidget *parent);
};

// Create the dialog.
KFindNextDialog::KFindNextDialog(const QString &pattern, QWidget *parent) :
    KDialogBase(parent, 0, false,  // non-modal!
        i18n("Find Next"),
        User1 | Close,
        User1,
        false,
        KStdGuiItem::find())
{
    setMainWidget( new QLabel( i18n("<qt>Find next occurrence of '<b>%1</b>'?</qt>").arg(pattern), this ) );
}

////

struct KFind::Private
{
    Private() :
      findDialog(0),
      patternChanged(false),
      matchedPattern(""),
      emptyMatch(0),
      currentId(0),
      customIds(false)
    {
    }

    ~Private()
    {
        data.clear();
        delete emptyMatch;
        emptyMatch = 0;
    }

    struct Match
    {
        Match() : dataId(-1), index(-1), matchedLength(-1) {}
        bool isNull() const { return index == -1; }
        Match(int _dataId, int _index, int _matchedLength) :
          dataId(_dataId),
          index(_index),
          matchedLength(_matchedLength)
        {
            Q_ASSERT( index != -1 );
        }

        int dataId;
        int index;
        int matchedLength;
    };

    struct Data
    {
        Data() : id(-1), dirty(false) { }
        Data(int id, const QString &text, bool dirty = false) :
          id(id),
          text(text),
          dirty(dirty)
        { }

        int     id;
        QString text;
        bool    dirty;
    };

    QPointer<QWidget>  findDialog;
    bool                  patternChanged;
    QString               matchedPattern;
    QHash<QString,Match>  incrementalPath;
    Match *               emptyMatch;
    QList<Data>           data; // used like a vector, not like a linked-list
    int                   currentId;
    bool                  customIds;
};

////

KFind::KFind( const QString &pattern, long options, QWidget *parent )
    : QObject( parent ),
	d(new KFind::Private)
{
    m_options = options;
    init( pattern );
}

KFind::KFind( const QString &pattern, long options, QWidget *parent, QWidget *findDialog )
    : QObject( parent ),
	d(new KFind::Private)
{
    d->findDialog = findDialog;
    m_options = options;
    init( pattern );
}

void KFind::init( const QString& pattern )
{
    m_matches = 0;
    m_pattern = pattern;
    m_dialog = 0;
    m_dialogClosed = false;
    m_index = INDEX_NOMATCH;
    m_lastResult = NoMatch;
    m_regExp = 0;
    setOptions( m_options ); // create m_regExp with the right options
}

KFind::~KFind()
{
    delete m_dialog;
    delete d;
}

bool KFind::needData() const
{
    // always true when m_text is empty.
    if (m_options & KFind::FindBackwards)
        // m_index==-1 and m_lastResult==Match means we haven't answered nomatch yet
        // This is important in the "replace with a prompt" case.
        return ( m_index < 0 && m_lastResult != Match );
    else
        // "index over length" test removed: we want to get a nomatch before we set data again
        // This is important in the "replace with a prompt" case.
        return m_index == INDEX_NOMATCH;
}

void KFind::setData( const QString& data, int startPos )
{
    setData( -1, data, startPos );
}

void KFind::setData( int id, const QString& data, int startPos )
{
    // cache the data for incremental find
    if ( m_options & KFind::FindIncremental )
    {
        if ( id != -1 )
            d->customIds = true;
        else
            id = d->currentId + 1;

        Q_ASSERT( id <= d->data.size() );

        if ( id == d->data.size() )
            d->data.append( Private::Data(id, data, true) );
        else
            d->data.replace( id, Private::Data(id, data, true) );
        Q_ASSERT( d->data[id].text == data );
    }

    if ( !(m_options & KFind::FindIncremental) || needData() )
    {
        m_text = data;

        if ( startPos != -1 )
            m_index = startPos;
        else if (m_options & KFind::FindBackwards)
            m_index = m_text.length();
        else
            m_index = 0;
#ifdef DEBUG_FIND
        kDebug() << "setData: '" << m_text << "' m_index=" << m_index << endl;
#endif
        Q_ASSERT( m_index != INDEX_NOMATCH );
        m_lastResult = NoMatch;

        d->currentId = id;
    }
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
    Q_ASSERT( m_index != INDEX_NOMATCH || d->patternChanged );

    if ( m_lastResult == Match && !d->patternChanged )
    {
        // Move on before looking for the next match, _if_ we just found a match
        if (m_options & KFind::FindBackwards) {
            m_index--;
            if ( m_index == -1 ) // don't call KFind::find with -1, it has a special meaning
            {
                m_lastResult = NoMatch;
                return NoMatch;
            }
        } else
            m_index++;
    }
    d->patternChanged = false;

    if ( m_options & KFind::FindIncremental )
    {
        // if the current pattern is shorter than the matchedPattern we can
        // probably look up the match in the incrementalPath
        if ( m_pattern.length() < d->matchedPattern.length() )
        {
            Private::Match match;
            if ( !m_pattern.isEmpty() )
                match = d->incrementalPath.value( m_pattern );
            else if ( d->emptyMatch )
                match = *d->emptyMatch;
            QString previousPattern = d->matchedPattern;
            d->matchedPattern = m_pattern;
            if ( !match.isNull() )
            {
                bool clean = true;

                // find the first result backwards on the path that isn't dirty
                while ( d->data[match.dataId].dirty == true &&
                        !m_pattern.isEmpty() )
                {
                    m_pattern.truncate( m_pattern.length() - 1 );

                    match = d->incrementalPath.value( m_pattern );

                    clean = false;
                }

                // remove all matches that lie after the current match
                while ( m_pattern.length() < previousPattern.length() )
                {
                    d->incrementalPath.remove(previousPattern);
                    previousPattern.truncate(previousPattern.length() - 1);
                }

                // set the current text, index, etc. to the found match
                m_text = d->data[match.dataId].text;
                m_index = match.index;
                m_matchedLength = match.matchedLength;
                d->currentId = match.dataId;

                // if the result is clean we can return it now
                if ( clean )
                {
                    if ( d->customIds )
                        emit highlight(d->currentId, m_index, m_matchedLength);
                    else
                        emit highlight(m_text, m_index, m_matchedLength);

                    m_lastResult = Match;
                    d->matchedPattern = m_pattern;
                    return Match;
                }
            }
            // if we couldn't look up the match, the new pattern isn't a
            // substring of the matchedPattern, so we start a new search
            else
            {
                startNewIncrementalSearch();
            }
        }
        // if the new pattern is longer than the matchedPattern we might be
        // able to proceed from the last search
        else if ( m_pattern.length() > d->matchedPattern.length() )
        {
            // continue from the previous pattern
            if ( m_pattern.startsWith(d->matchedPattern) )
            {
                // we can't proceed from the previous position if the previous
                // position already failed
                if ( m_index == INDEX_NOMATCH )
                    return NoMatch;

                QString temp = m_pattern;
                m_pattern.truncate(d->matchedPattern.length() + 1);
                d->matchedPattern = temp;
            }
            // start a new search
            else
            {
                startNewIncrementalSearch();
            }
        }
        // if the new pattern is as long as the matchedPattern, we reset if
        // they are not equal
        else if ( m_pattern != d->matchedPattern )
        {
             startNewIncrementalSearch();
        }
    }

#ifdef DEBUG_FIND
    kDebug() << k_funcinfo << "m_index=" << m_index << endl;
#endif
    do
    {
        // if we have multiple data blocks in our cache, walk through these
        // blocks till we either searched all blocks or we find a match
        do
        {
            // Find the next candidate match.
            if ( m_options & KFind::RegularExpression )
                m_index = KFind::find(m_text, *m_regExp, m_index, m_options, &m_matchedLength);
            else
                m_index = KFind::find(m_text, m_pattern, m_index, m_options, &m_matchedLength);


            if ( m_options & KFind::FindIncremental )
                d->data[d->currentId].dirty = false;

            if ( m_index == -1 && d->currentId < (int) d->data.count() - 1 )
            {
                m_text = d->data[++d->currentId].text;

                if ( m_options & KFind::FindBackwards )
                    m_index = m_text.length();
                else
                    m_index = 0;
            }
            else
                break;
        } while ( !(m_options & KFind::RegularExpression) );

        if ( m_index != -1 )
        {
            // Flexibility: the app can add more rules to validate a possible match
            if ( validateMatch( m_text, m_index, m_matchedLength ) )
            {
                bool done = true;

                if ( m_options & KFind::FindIncremental )
                {
                    if ( m_pattern.isEmpty() ) {
                        delete d->emptyMatch;
                        d->emptyMatch = new Private::Match( d->currentId, m_index, m_matchedLength );
                    } else
                        d->incrementalPath.insert(m_pattern, Private::Match(d->currentId, m_index, m_matchedLength));

                    if ( m_pattern.length() < d->matchedPattern.length() )
                    {
                        m_pattern += d->matchedPattern.mid(m_pattern.length(), 1);
                        done = false;
                    }
                }

                if ( done )
                {
                    m_matches++;
                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    if ( d->customIds )
                        emit highlight(d->currentId, m_index, m_matchedLength);
                    else
                        emit highlight(m_text, m_index, m_matchedLength);

                    if ( !m_dialogClosed )
                        findNextDialog(true)->show();

#ifdef DEBUG_FIND
                    kDebug() << k_funcinfo << "Match. Next m_index=" << m_index << endl;
#endif
                    m_lastResult = Match;
                    return Match;
                }
            }
            else // Skip match
            {
                if (m_options & KFind::FindBackwards)
                    m_index--;
                else
                    m_index++;
            }
        }
        else
        {
            if ( m_options & KFind::FindIncremental )
            {
                QString temp = m_pattern;
                temp.truncate(temp.length() - 1);
                m_pattern = d->matchedPattern;
                d->matchedPattern = temp;
            }

            m_index = INDEX_NOMATCH;
        }
    }
    while (m_index != INDEX_NOMATCH);

#ifdef DEBUG_FIND
    kDebug() << k_funcinfo << "NoMatch. m_index=" << m_index << endl;
#endif
    m_lastResult = NoMatch;
    return NoMatch;
}

void KFind::startNewIncrementalSearch()
{
    Private::Match *match = d->emptyMatch;
    if(match == 0)
    {
        m_text.clear();
        m_index = 0;
        d->currentId = 0;
    }
    else
    {
        m_text = d->data[match->dataId].text;
        m_index = match->index;
        d->currentId = match->dataId;
    }
    m_matchedLength = 0;
    d->incrementalPath.clear();
    delete d->emptyMatch;
    d->emptyMatch = 0;
    d->matchedPattern = m_pattern;
    m_pattern.clear();
}

// static
int KFind::find(const QString &text, const QString &pattern, int index, long options, int *matchedLength)
{
    // Handle regular expressions in the appropriate way.
    if (options & KFind::RegularExpression)
    {
        Qt::CaseSensitivity caseSensitive = (options & KFind::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
        QRegExp regExp(pattern, caseSensitive);

        return find(text, regExp, index, options, matchedLength);
    }

    // In Qt4 QString("aaaaaa").lastIndexOf("a",6) returns -1; we need
    // to start at text.length() - pattern.length() to give a valid index to QString.
    if (options & KFind::FindBackwards)
        index = qMin( text.length() - pattern.length(), index );

    Qt::CaseSensitivity caseSensitive = (options & KFind::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (options & KFind::WholeWordsOnly)
    {
        if (options & KFind::FindBackwards)
        {
            // Backward search, until the beginning of the line...
            while (index >= 0)
            {
                // ...find the next match.
                index = text.lastIndexOf(pattern, index, caseSensitive);
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
                index = text.indexOf(pattern, index, caseSensitive);
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
        if (options & KFind::FindBackwards)
        {
            index = text.lastIndexOf(pattern, index, caseSensitive);
        }
        else
        {
            index = text.indexOf(pattern, index, caseSensitive);
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
    if (options & KFind::WholeWordsOnly)
    {
        if (options & KFind::FindBackwards)
        {
            // Backward search, until the beginning of the line...
            while (index >= 0)
            {
                // ...find the next match.
                index = text.lastIndexOf(pattern, index);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                //pattern.match(text, index, matchedLength, false);
                /*int pos =*/ pattern.indexIn( text.mid(index) );
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
                index = text.indexOf(pattern, index);
                if (index == -1)
                    break;

                // Is the match delimited correctly?
                //pattern.match(text, index, matchedLength, false);
                /*int pos =*/ pattern.indexIn( text.mid(index) );
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
        if (options & KFind::FindBackwards)
        {
            index = text.lastIndexOf(pattern, index);
        }
        else
        {
            index = text.indexOf(pattern, index);
        }
        if (index != -1)
        {
            //pattern.match(text, index, matchedLength, false);
            /*int pos =*/ pattern.indexIn( text.mid(index) );
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
        message = i18n("<qt>No matches found for '<b>%1</b>'.</qt>").arg(Qt::escape(m_pattern));
    KMessageBox::information(dialogsParent(), message);
}

bool KFind::shouldRestart( bool forceAsking, bool showNumMatches ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (m_options & KFind::FromCursor) == 0 )
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
            message = i18n("No matches found for '<b>%1</b>'.").arg(Qt::escape(m_pattern));
    }
    else
    {
        if ( m_options & KFind::FindBackwards )
            message = i18n( "Beginning of document reached." );
        else
            message = i18n( "End of document reached." );
    }

    message += "<br><br>"; // can't be in the i18n() of the first if() because of the plural form.
    // Hope this word puzzle is ok, it's a different sentence
    message +=
        ( m_options & KFind::FindBackwards ) ?
        i18n("Continue from the end?")
        : i18n("Continue from the beginning?");

    int ret = KMessageBox::questionYesNo( dialogsParent(), QString("<qt>")+message+QString("</qt>"),
                                          QString(), KStdGuiItem::cont(), KStdGuiItem::stop() );
    bool yes = ( ret == KMessageBox::Yes );
    if ( yes )
        const_cast<KFind*>(this)->m_options &= ~KFind::FromCursor; // clear FromCursor option
    return yes;
}

void KFind::setOptions( long options )
{
    m_options = options;

    delete m_regExp;
    if (m_options & KFind::RegularExpression) {
        Qt::CaseSensitivity caseSensitive = (m_options & KFind::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
        m_regExp = new QRegExp(m_pattern, caseSensitive);
    } else
        m_regExp = 0;
}

void KFind::closeFindNextDialog()
{
    delete m_dialog;
    m_dialog = 0L;
    m_dialogClosed = true;
}

int KFind::index() const
{
    return m_index;
}

void KFind::setPattern( const QString& pattern )
{
    if ( m_options & KFind::FindIncremental && m_pattern != pattern )
        d->patternChanged = true;

    m_pattern = pattern;
    setOptions( options() ); // rebuild m_regExp if necessary
}

QWidget* KFind::dialogsParent() const
{
    // If the find dialog is still up, it should get the focus when closing a message box
    // Otherwise, maybe the "find next?" dialog is up
    // Otherwise, the "view" is the parent.
    return d->findDialog ? (QWidget*)d->findDialog : ( m_dialog ? m_dialog : parentWidget() );
}

#include "kfind.moc"
