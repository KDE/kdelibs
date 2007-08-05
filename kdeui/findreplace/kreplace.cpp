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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kreplace.h"
#include "kfind_p.h"

#include <QtGui/QLabel>
#include <kapplication.h>
#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include "kreplacedialog.h"
#include <QtCore/QRegExp>

//#define DEBUG_REPLACE
#define INDEX_NOMATCH -1

class KReplaceNextDialog : public KDialog
{
public:
    KReplaceNextDialog( QWidget *parent );
    void setLabel( const QString& pattern, const QString& replacement );
private:
    QLabel* m_mainLabel;
};

KReplaceNextDialog::KReplaceNextDialog(QWidget *parent) :
    KDialog(parent)
{
    setModal( false );
    setCaption( i18n("Replace") );
    setButtons( User3 | User2 | User1 | Close );
    setButtonGuiItem( User1, KGuiItem(i18n("&All")) );
    setButtonGuiItem( User2, KGuiItem(i18n("&Skip")) );
    setButtonGuiItem( User3, KGuiItem(i18n("Replace")) );
    setDefaultButton( User3 );
    showButtonSeparator( false );

    m_mainLabel = new QLabel( this );
    setMainWidget( m_mainLabel );
    resize(minimumSize());
}

void KReplaceNextDialog::setLabel( const QString& pattern, const QString& replacement )
{
    m_mainLabel->setText( i18n("Replace '%1' with '%2'?", pattern, replacement) );
}

////

class KReplace::KReplacePrivate
{
public:
    KReplacePrivate(const QString& replacement)
        : m_replacement( replacement )
        , m_replacements( 0 )
    {}
    QString m_replacement;
    unsigned m_replacements;
};


////

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent) :
    KFind( pattern, options, parent ),
    d( new KReplacePrivate(replacement) )
{
}

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent, QWidget *dlg) :
    KFind( pattern, options, parent, dlg ),
    d( new KReplacePrivate(replacement) )
{
}

KReplace::~KReplace()
{
    delete d;
}

int KReplace::numReplacements() const
{
    return d->m_replacements;
}

KDialog* KReplace::replaceNextDialog( bool create )
{
    if ( KFind::d->dialog || create )
        return dialog();
    return 0L;
}

KReplaceNextDialog* KReplace::dialog()
{
    if ( !KFind::d->dialog )
    {
        KFind::d->dialog = new KReplaceNextDialog( parentWidget() );
        connect( KFind::d->dialog, SIGNAL( user1Clicked() ), this, SLOT( slotReplaceAll() ) );
        connect( KFind::d->dialog, SIGNAL( user2Clicked() ), this, SLOT( slotSkip() ) );
        connect( KFind::d->dialog, SIGNAL( user3Clicked() ), this, SLOT( slotReplace() ) );
        connect( KFind::d->dialog, SIGNAL( finished() ), this, SLOT( slotDialogClosed() ) );
    }
    return static_cast<KReplaceNextDialog *>(KFind::d->dialog);
}

void KReplace::displayFinalDialog() const
{
    if ( !d->m_replacements )
        KMessageBox::information(parentWidget(), i18n("No text was replaced."));
    else
        KMessageBox::information(parentWidget(), i18np("1 replacement done.", "%1 replacements done.", d->m_replacements ) );
}

KFind::Result KReplace::replace()
{
#ifdef DEBUG_REPLACE
    kDebug() << k_funcinfo << "d->index=" << KFind::d->index;
#endif
    if ( KFind::d->index == INDEX_NOMATCH && KFind::d->lastResult == Match )
    {
        KFind::d->lastResult = NoMatch;
        return NoMatch;
    }

    do // this loop is only because validateMatch can fail
    {
#ifdef DEBUG_REPLACE
        kDebug() << k_funcinfo << "beginning of loop: KFind::d->index=" << KFind::d->index;
#endif
        // Find the next match.
        if ( KFind::d->options & KFind::RegularExpression )
            KFind::d->index = KFind::find(KFind::d->text, *KFind::d->regExp, KFind::d->index, KFind::d->options, &KFind::d->matchedLength);
        else
            KFind::d->index = KFind::find(KFind::d->text, KFind::d->pattern, KFind::d->index, KFind::d->options, &KFind::d->matchedLength);
#ifdef DEBUG_REPLACE
        kDebug() << k_funcinfo << "KFind::find returned KFind::d->index=" << KFind::d->index;
#endif
        if ( KFind::d->index != -1 )
        {
            // Flexibility: the app can add more rules to validate a possible match
            if ( validateMatch( KFind::d->text, KFind::d->index, KFind::d->matchedLength ) )
            {
                if ( KFind::d->options & KReplaceDialog::PromptOnReplace )
                {
#ifdef DEBUG_REPLACE
                    kDebug() << k_funcinfo << "PromptOnReplace";
#endif
                    // Display accurate initial string and replacement string, they can vary
                    QString matchedText = KFind::d->text.mid( KFind::d->index, KFind::d->matchedLength );
                    QString rep = matchedText;
                    KReplace::replace(rep, d->m_replacement, 0, KFind::d->options, KFind::d->matchedLength);
                    dialog()->setLabel( matchedText, rep );
                    dialog()->show();

                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    emit highlight(KFind::d->text, KFind::d->index, KFind::d->matchedLength);

                    KFind::d->lastResult = Match;
                    return Match;
                }
                else
                {
                    doReplace(); // this moves on too
                }
            }
            else
            {
                // not validated -> move on
                if (KFind::d->options & KFind::FindBackwards)
                    KFind::d->index--;
                else
                    KFind::d->index++;
            }
        } else
            KFind::d->index = INDEX_NOMATCH; // will exit the loop
    }
    while (KFind::d->index != INDEX_NOMATCH);

    KFind::d->lastResult = NoMatch;
    return NoMatch;
}

int KReplace::replace(QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replace(text, replacement, index, options, matchedLength);
        if (options & KFind::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

int KReplace::replace(QString &text, const QRegExp &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replace(text, replacement, index, options, matchedLength);
        if (options & KFind::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

int KReplace::replace(QString &text, const QString &replacement, int index, long options, int length)
{
    QString rep = replacement;
    // Backreferences: replace \0 with the right portion of 'text'
    if ( options & KReplaceDialog::BackReference )
        rep.replace( "\\0", text.mid( index, length ) );
    // Then replace rep into the text
    text.replace(index, length, rep);
    return rep.length();
}

void KReplace::slotReplaceAll()
{
    doReplace();
    KFind::d->options &= ~KReplaceDialog::PromptOnReplace;
    emit optionsChanged();
    emit findNext();
}

void KReplace::slotSkip()
{
    if (KFind::d->options & KFind::FindBackwards)
        KFind::d->index--;
    else
        KFind::d->index++;
    if ( KFind::d->dialogClosed ) {
        delete KFind::d->dialog; // hide it again
        KFind::d->dialog = 0L;
    } else
        emit findNext();
}

void KReplace::slotReplace()
{
    doReplace();
    if ( KFind::d->dialogClosed ) {
        delete KFind::d->dialog; // hide it again
        KFind::d->dialog = 0L;
    } else
        emit findNext();
}

void KReplace::doReplace()
{
    int replacedLength = KReplace::replace(KFind::d->text, d->m_replacement, KFind::d->index, KFind::d->options, KFind::d->matchedLength);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    emit replace(KFind::d->text, KFind::d->index, replacedLength, KFind::d->matchedLength);
#ifdef DEBUG_REPLACE
    kDebug() << k_funcinfo << "after replace() signal: KFind::d->index=" << KFind::d->index << " replacedLength=" << replacedLength;
#endif
    d->m_replacements++;
    if (KFind::d->options & KFind::FindBackwards)
        KFind::d->index--;
    else {
        KFind::d->index += replacedLength;
        // when replacing the empty pattern, move on. See also kjs/regexp.cpp for how this should be done for regexps.
        if ( KFind::d->pattern.isEmpty() )
            ++KFind::d->index;
    }
#ifdef DEBUG_REPLACE
    kDebug() << k_funcinfo << "after adjustement: KFind::d->index=" << KFind::d->index;
#endif
}

void KReplace::resetCounts()
{
    KFind::resetCounts();
    d->m_replacements = 0;
}

bool KReplace::shouldRestart( bool forceAsking, bool showNumMatches ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // ... Or if the prompt-on-replace option was set.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (KFind::d->options & KFind::FromCursor) == 0
         && (KFind::d->options & KReplaceDialog::PromptOnReplace) == 0 )
    {
        displayFinalDialog();
        return false;
    }
    QString message;
    if ( showNumMatches )
    {
        if ( !d->m_replacements )
            message = i18n("No text was replaced.");
        else
            message = i18np("1 replacement done.", "%1 replacements done.", d->m_replacements );
    }
    else
    {
        if ( KFind::d->options & KFind::FindBackwards )
            message = i18n( "Beginning of document reached." );
        else
            message = i18n( "End of document reached." );
    }

    message += '\n';
    // Hope this word puzzle is ok, it's a different sentence
    message +=
        ( KFind::d->options & KFind::FindBackwards ) ?
        i18n("Do you want to restart search from the end?")
        : i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionYesNo( parentWidget(), message, QString(), KGuiItem(i18n("Restart")), KGuiItem(i18n("Stop")) );
    return( ret == KMessageBox::Yes );
}

void KReplace::closeReplaceNextDialog()
{
    closeFindNextDialog();
}

#include "kreplace.moc"
