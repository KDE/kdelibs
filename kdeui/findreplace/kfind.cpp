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
#include "kfind_p.h"
#include "kfinddialog.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <QtGui/QLabel>
#include <QtCore/QRegExp>
#include <QtCore/QHash>
#include <QTextDocument>

// #define DEBUG_FIND

static const int INDEX_NOMATCH = -1;

class KFindNextDialog : public KDialog
{
public:
    KFindNextDialog(const QString &pattern, QWidget *parent);
};

// Create the dialog.
KFindNextDialog::KFindNextDialog(const QString &pattern, QWidget *parent) :
    KDialog(parent)
{
    setModal( false );
    setCaption( i18n("Find Next") );
    setButtons( User1 | Close );
    setButtonGuiItem( User1, KStandardGuiItem::find() );
    setDefaultButton( User1 );

    setMainWidget( new QLabel( i18n("<qt>Find next occurrence of '<b>%1</b>'?</qt>", pattern), this ) );
}

////


KFind::KFind( const QString &pattern, long options, QWidget *parent )
    : QObject( parent ),
	d(new KFind::Private(this))
{
    d->options = options;
    d->init( pattern );
}

KFind::KFind( const QString &pattern, long options, QWidget *parent, QWidget *findDialog )
    : QObject( parent ),
	d(new KFind::Private(this))
{
    d->findDialog = findDialog;
    d->options = options;
    d->init( pattern );
}

void KFind::Private::init( const QString& _pattern )
{
    matches = 0;
    pattern = _pattern;
    dialog = 0;
    dialogClosed = false;
    index = INDEX_NOMATCH;
    lastResult = NoMatch;
    regExp = 0;
    q->setOptions( options ); // create d->regExp with the right options
}

KFind::~KFind()
{
    delete d;
    kDebug() ;
}

bool KFind::needData() const
{
    // always true when d->text is empty.
    if (d->options & KFind::FindBackwards)
        // d->index==-1 and d->lastResult==Match means we haven't answered nomatch yet
        // This is important in the "replace with a prompt" case.
        return ( d->index < 0 && d->lastResult != Match );
    else
        // "index over length" test removed: we want to get a nomatch before we set data again
        // This is important in the "replace with a prompt" case.
        return d->index == INDEX_NOMATCH;
}

void KFind::setData( const QString& data, int startPos )
{
    setData( -1, data, startPos );
}

void KFind::setData( int id, const QString& data, int startPos )
{
    // cache the data for incremental find
    if ( d->options & KFind::FindIncremental )
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
        Q_ASSERT( d->data.at(id).text == data );
    }

    if ( !(d->options & KFind::FindIncremental) || needData() )
    {
        d->text = data;

        if ( startPos != -1 )
            d->index = startPos;
        else if (d->options & KFind::FindBackwards)
            d->index = d->text.length();
        else
            d->index = 0;
#ifdef DEBUG_FIND
        kDebug() << "setData: '" << d->text << "' d->index=" << d->index;
#endif
        Q_ASSERT( d->index != INDEX_NOMATCH );
        d->lastResult = NoMatch;

        d->currentId = id;
    }
}

KDialog* KFind::findNextDialog( bool create )
{
    if ( !d->dialog && create )
    {
        d->dialog = new KFindNextDialog( d->pattern, parentWidget() );
        connect( d->dialog, SIGNAL( user1Clicked() ), this, SLOT( _k_slotFindNext() ) );
        connect( d->dialog, SIGNAL( finished() ), this, SLOT( _k_slotDialogClosed() ) );
    }
    return d->dialog;
}

KFind::Result KFind::find()
{
    Q_ASSERT( d->index != INDEX_NOMATCH || d->patternChanged );

    if ( d->lastResult == Match && !d->patternChanged )
    {
        // Move on before looking for the next match, _if_ we just found a match
        if (d->options & KFind::FindBackwards) {
            d->index--;
            if ( d->index == -1 ) // don't call KFind::find with -1, it has a special meaning
            {
                d->lastResult = NoMatch;
                return NoMatch;
            }
        } else
            d->index++;
    }
    d->patternChanged = false;

    if ( d->options & KFind::FindIncremental )
    {
        // if the current pattern is shorter than the matchedPattern we can
        // probably look up the match in the incrementalPath
        if ( d->pattern.length() < d->matchedPattern.length() )
        {
            Private::Match match;
            if ( !d->pattern.isEmpty() )
                match = d->incrementalPath.value( d->pattern );
            else if ( d->emptyMatch )
                match = *d->emptyMatch;
            QString previousPattern (d->matchedPattern);
            d->matchedPattern = d->pattern;
            if ( !match.isNull() )
            {
                bool clean = true;

                // find the first result backwards on the path that isn't dirty
                while ( d->data.at(match.dataId).dirty == true &&
                        !d->pattern.isEmpty() )
                {
                    d->pattern.truncate( d->pattern.length() - 1 );

                    match = d->incrementalPath.value( d->pattern );

                    clean = false;
                }

                // remove all matches that lie after the current match
                while ( d->pattern.length() < previousPattern.length() )
                {
                    d->incrementalPath.remove(previousPattern);
                    previousPattern.truncate(previousPattern.length() - 1);
                }

                // set the current text, index, etc. to the found match
                d->text = d->data.at(match.dataId).text;
                d->index = match.index;
                d->matchedLength = match.matchedLength;
                d->currentId = match.dataId;

                // if the result is clean we can return it now
                if ( clean )
                {
                    if ( d->customIds )
                        emit highlight(d->currentId, d->index, d->matchedLength);
                    else
                        emit highlight(d->text, d->index, d->matchedLength);

                    d->lastResult = Match;
                    d->matchedPattern = d->pattern;
                    return Match;
                }
            }
            // if we couldn't look up the match, the new pattern isn't a
            // substring of the matchedPattern, so we start a new search
            else
            {
                d->startNewIncrementalSearch();
            }
        }
        // if the new pattern is longer than the matchedPattern we might be
        // able to proceed from the last search
        else if ( d->pattern.length() > d->matchedPattern.length() )
        {
            // continue from the previous pattern
            if ( d->pattern.startsWith(d->matchedPattern) )
            {
                // we can't proceed from the previous position if the previous
                // position already failed
                if ( d->index == INDEX_NOMATCH )
                    return NoMatch;

                QString temp (d->pattern);
                d->pattern.truncate(d->matchedPattern.length() + 1);
                d->matchedPattern = temp;
            }
            // start a new search
            else
            {
                d->startNewIncrementalSearch();
            }
        }
        // if the new pattern is as long as the matchedPattern, we reset if
        // they are not equal
        else if ( d->pattern != d->matchedPattern )
        {
             d->startNewIncrementalSearch();
        }
    }

#ifdef DEBUG_FIND
    kDebug() << "d->index=" << d->index;
#endif
    do
    {
        // if we have multiple data blocks in our cache, walk through these
        // blocks till we either searched all blocks or we find a match
        do
        {
            // Find the next candidate match.
            if ( d->options & KFind::RegularExpression )
                d->index = KFind::find(d->text, *d->regExp, d->index, d->options, &d->matchedLength);
            else
                d->index = KFind::find(d->text, d->pattern, d->index, d->options, &d->matchedLength);

            if ( d->options & KFind::FindIncremental )
                d->data[d->currentId].dirty = false;

            if (d->index == -1 && d->currentId < d->data.count() - 1) {
                d->text = d->data.at(++d->currentId).text;

                if ( d->options & KFind::FindBackwards )
                    d->index = d->text.length();
                else
                    d->index = 0;
            } else {
                break;
            }
        } while ( !(d->options & KFind::RegularExpression) );

        if ( d->index != -1 )
        {
            // Flexibility: the app can add more rules to validate a possible match
            if ( validateMatch( d->text, d->index, d->matchedLength ) )
            {
                bool done = true;

                if ( d->options & KFind::FindIncremental )
                {
                    if ( d->pattern.isEmpty() ) {
                        delete d->emptyMatch;
                        d->emptyMatch = new Private::Match( d->currentId, d->index, d->matchedLength );
                    } else
                        d->incrementalPath.insert(d->pattern, Private::Match(d->currentId, d->index, d->matchedLength));

                    if ( d->pattern.length() < d->matchedPattern.length() )
                    {
                        d->pattern += d->matchedPattern.mid(d->pattern.length(), 1);
                        done = false;
                    }
                }

                if ( done )
                {
                    d->matches++;
                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    if ( d->customIds )
                        emit highlight(d->currentId, d->index, d->matchedLength);
                    else
                        emit highlight(d->text, d->index, d->matchedLength);

                    if ( !d->dialogClosed )
                        findNextDialog(true)->show();

#ifdef DEBUG_FIND
                    kDebug() << "Match. Next d->index=" << d->index;
#endif
                    d->lastResult = Match;
                    return Match;
                }
            }
            else // Skip match
            {
                if (d->options & KFind::FindBackwards)
                    d->index--;
                else
                    d->index++;
            }
        }
        else
        {
            if ( d->options & KFind::FindIncremental )
            {
                QString temp (d->pattern);
                temp.truncate(temp.length() - 1);
                d->pattern = d->matchedPattern;
                d->matchedPattern = temp;
            }

            d->index = INDEX_NOMATCH;
        }
    }
    while (d->index != INDEX_NOMATCH);

#ifdef DEBUG_FIND
    kDebug() << "NoMatch. d->index=" << d->index;
#endif
    d->lastResult = NoMatch;
    return NoMatch;
}

void KFind::Private::startNewIncrementalSearch()
{
    Private::Match *match = emptyMatch;
    if(match == 0)
    {
        text.clear();
        index = 0;
        currentId = 0;
    }
    else
    {
        text = data.at(match->dataId).text;
        index = match->index;
        currentId = match->dataId;
    }
    matchedLength = 0;
    incrementalPath.clear();
    delete emptyMatch; emptyMatch = 0;
    matchedPattern = pattern;
    pattern.clear();
}

static bool isInWord(QChar ch)
{
    return ch.isLetter() || ch.isDigit() || ch == '_';
}

static bool isWholeWords(const QString &text, int starts, int matchedLength)
{
    if (starts == 0 || !isInWord(text.at(starts-1)))
    {
        const int ends = starts + matchedLength;
        if (ends == text.length() || !isInWord(text.at(ends))) {
            return true;
        }
    }
    return false;
}

static bool matchOk(const QString& text, int index, int matchedLength, long options)
{
    if (options & KFind::WholeWordsOnly) {
        // Is the match delimited correctly?
        if (isWholeWords(text, index, matchedLength))
            return true;
    } else {
        // Non-whole-word search: this match is good
        return true;
    }
    return false;
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
    if (options & KFind::FindBackwards) {
        index = qMin( qMax(0, text.length() - pattern.length()), index );
    }

    Qt::CaseSensitivity caseSensitive = (options & KFind::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (options & KFind::FindBackwards) {
        // Backward search, until the beginning of the line...
        while (index >= 0) {
            // ...find the next match.
            index = text.lastIndexOf(pattern, index, caseSensitive);
            if (index == -1)
                break;

            if (matchOk(text, index, pattern.length(), options))
                break;
            index--;
            kDebug() << "decrementing:" << index;
        }
    } else {
        // Forward search, until the end of the line...
        while (index <= text.length())
        {
            // ...find the next match.
            index = text.indexOf(pattern, index, caseSensitive);
            if (index == -1)
                break;

            if (matchOk(text, index, pattern.length(), options))
                break;
            index++;
        }
        if (index > text.length()) { // end of line
            kDebug() << "at" << index << "-> not found";
            index = -1; // not found
        }
    }
    if (index <= -1)
        *matchedLength = 0;
    else
        *matchedLength = pattern.length();
    return index;
}

// Core method for the regexp-based find
static int doFind(const QString &text, const QRegExp &pattern, int index, long options, int *matchedLength)
{
    if (options & KFind::FindBackwards) {
        // Backward search, until the beginning of the line...
        while (index >= 0) {
            // ...find the next match.
            index = text.lastIndexOf(pattern, index);
            if (index == -1)
                break;

            /*int pos =*/ pattern.indexIn( text.mid(index) );
            *matchedLength = pattern.matchedLength();
            if (matchOk(text, index, *matchedLength, options))
                break;
            index--;
        }
    } else {
        // Forward search, until the end of the line...
        while (index <= text.length()) {
            // ...find the next match.
            index = text.indexOf(pattern, index);
            if (index == -1)
                break;

            /*int pos =*/ pattern.indexIn( text.mid(index) );
            *matchedLength = pattern.matchedLength();
            if (matchOk(text, index, *matchedLength, options))
                break;
            index++;
        }
        if (index > text.length()) { // end of line
            index = -1; // not found
        }
    }
    if (index == -1)
        *matchedLength = 0;
    return index;
}

// Since QRegExp doesn't support multiline searches (the equivalent of perl's /m)
// we have to cut the text into lines if the pattern starts with ^ or ends with $.
static int lineBasedFind(const QString &text, const QRegExp &pattern, int index, long options, int *matchedLength)
{
    const QStringList lines = text.split('\n');
    int offset = 0;
    // Use "index" to find the first line we should start from
    int startLineNumber = 0;
    for (; startLineNumber < lines.count(); ++startLineNumber) {
        const QString line = lines.at(startLineNumber);
        if (index < offset + line.length()) {
            break;
        }
        offset += line.length() + 1 /*newline*/;
    }

    if (options & KFind::FindBackwards) {

        if (startLineNumber == lines.count()) {
            // We went too far, go back to the last line
            --startLineNumber;
            offset -= lines.at(startLineNumber).length() + 1;
        }

        for (int lineNumber = startLineNumber; lineNumber >= 0; --lineNumber) {
            const QString line = lines.at(lineNumber);
            const int ret = doFind(line, pattern, lineNumber == startLineNumber ? index - offset : line.length(), options, matchedLength);
            if (ret > -1)
                return ret + offset;
            offset -= line.length() + 1 /*newline*/;
        }

    } else {
        for (int lineNumber = startLineNumber; lineNumber < lines.count(); ++lineNumber) {
            const QString line = lines.at(lineNumber);
            const int ret = doFind(line, pattern, lineNumber == startLineNumber ? (index - offset) : 0, options, matchedLength);
            if (ret > -1) {
                return ret + offset;
            }
            offset += line.length() + 1 /*newline*/;
        }
    }
    return -1;
}

// static
int KFind::find(const QString &text, const QRegExp &pattern, int index, long options, int *matchedLength)
{
    if (pattern.pattern().startsWith('^') || pattern.pattern().endsWith('$')) {
        return lineBasedFind(text, pattern, index, options, matchedLength);
    }

    return doFind(text, pattern, index, options, matchedLength);
}

void KFind::Private::_k_slotFindNext()
{
    emit q->findNext();
}

void KFind::Private::_k_slotDialogClosed()
{
#ifdef DEBUG_FIND
    kDebug() << " Begin";
#endif
    emit q->dialogClosed();
    dialogClosed = true;
#ifdef DEBUG_FIND
    kDebug() << " End";
#endif

}

void KFind::displayFinalDialog() const
{
    QString message;
    if ( numMatches() )
        message = i18np( "1 match found.", "%1 matches found.", numMatches() );
    else
        message = i18n("<qt>No matches found for '<b>%1</b>'.</qt>", Qt::escape(d->pattern));
    KMessageBox::information(dialogsParent(), message);
}

bool KFind::shouldRestart( bool forceAsking, bool showNumMatches ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (d->options & KFind::FromCursor) == 0 )
    {
        displayFinalDialog();
        return false;
    }
    QString message;
    if ( showNumMatches )
    {
        if ( numMatches() )
            message = i18np( "1 match found.", "%1 matches found.", numMatches() );
        else
            message = i18n("No matches found for '<b>%1</b>'.", Qt::escape(d->pattern));
    }
    else
    {
        if ( d->options & KFind::FindBackwards )
            message = i18n( "Beginning of document reached." );
        else
            message = i18n( "End of document reached." );
    }

    message += "<br><br>"; // can't be in the i18n() of the first if() because of the plural form.
    // Hope this word puzzle is ok, it's a different sentence
    message +=
        ( d->options & KFind::FindBackwards ) ?
        i18n("Continue from the end?")
        : i18n("Continue from the beginning?");

    int ret = KMessageBox::questionYesNo( dialogsParent(), "<qt>"+message+"</qt>",
                                          QString(), KStandardGuiItem::cont(), KStandardGuiItem::stop() );
    bool yes = ( ret == KMessageBox::Yes );
    if ( yes )
        const_cast<KFind*>(this)->d->options &= ~KFind::FromCursor; // clear FromCursor option
    return yes;
}

long KFind::options() const
{
    return d->options;
}

void KFind::setOptions( long options )
{
    d->options = options;

    delete d->regExp;
    if (d->options & KFind::RegularExpression) {
        Qt::CaseSensitivity caseSensitive = (d->options & KFind::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
        d->regExp = new QRegExp(d->pattern, caseSensitive);
    } else
        d->regExp = 0;
}

void KFind::closeFindNextDialog()
{
    if (d->dialog) {
        d->dialog->deleteLater();
        d->dialog = 0;
    }
    d->dialogClosed = true;
}

int KFind::index() const
{
    return d->index;
}

QString KFind::pattern() const
{
    return d->pattern;
}

void KFind::setPattern( const QString& pattern )
{
    if ( d->options & KFind::FindIncremental && d->pattern != pattern )
        d->patternChanged = true;

    d->pattern = pattern;
    setOptions( options() ); // rebuild d->regExp if necessary
}

int KFind::numMatches() const
{
    return d->matches;
}

void KFind::resetCounts()
{
    d->matches = 0;
}

bool KFind::validateMatch( const QString &, int, int )
{
    return true;
}

QWidget* KFind::parentWidget() const
{
    return (QWidget *)parent();
}

QWidget* KFind::dialogsParent() const
{
    // If the find dialog is still up, it should get the focus when closing a message box
    // Otherwise, maybe the "find next?" dialog is up
    // Otherwise, the "view" is the parent.
    return d->findDialog ? (QWidget*)d->findDialog : ( d->dialog ? d->dialog : parentWidget() );
}

#include "kfind.moc"
