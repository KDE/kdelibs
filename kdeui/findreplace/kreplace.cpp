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
    explicit KReplaceNextDialog( QWidget *parent );
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
    setButtonGuiItem( User1, KGuiItem(i18nc("@action:button Replace all occurrences", "&All")) );
    setButtonGuiItem( User2, KGuiItem(i18n("&Skip")) );
    setButtonGuiItem( User3, KGuiItem(i18n("Replace")) );
    setDefaultButton( User3 );

    m_mainLabel = new QLabel( this );
    setMainWidget( m_mainLabel );
}

void KReplaceNextDialog::setLabel( const QString& pattern, const QString& replacement )
{
    m_mainLabel->setText( i18n("Replace '%1' with '%2'?", pattern, replacement) );
}

////

class KReplacePrivate
{
public:
    KReplacePrivate(KReplace *q, const QString& replacement)
        : q(q)
        , m_replacement( replacement )
        , m_replacements( 0 )
    {}

    KReplaceNextDialog* dialog();
    void doReplace();

    void _k_slotSkip();
    void _k_slotReplace();
    void _k_slotReplaceAll();

    KReplace *q;
    QString m_replacement;
    unsigned m_replacements;
};


////

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent) :
    KFind( pattern, options, parent ),
    d( new KReplacePrivate(this, replacement) )
{
}

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent, QWidget *dlg) :
    KFind( pattern, options, parent, dlg ),
    d( new KReplacePrivate(this, replacement) )
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
        return d->dialog();
    return 0L;
}

KReplaceNextDialog* KReplacePrivate::dialog()
{
    if ( !q->KFind::d->dialog )
    {
        q->KFind::d->dialog = new KReplaceNextDialog( q->parentWidget() );
        q->connect( q->KFind::d->dialog, SIGNAL( user1Clicked() ), q, SLOT( _k_slotReplaceAll() ) );
        q->connect( q->KFind::d->dialog, SIGNAL( user2Clicked() ), q, SLOT( _k_slotSkip() ) );
        q->connect( q->KFind::d->dialog, SIGNAL( user3Clicked() ), q, SLOT( _k_slotReplace() ) );
        q->connect( q->KFind::d->dialog, SIGNAL( finished() ), q, SLOT( _k_slotDialogClosed() ) );
    }
    return static_cast<KReplaceNextDialog *>(q->KFind::d->dialog);
}

void KReplace::displayFinalDialog() const
{
    if ( !d->m_replacements )
        KMessageBox::information(parentWidget(), i18n("No text was replaced."));
    else
        KMessageBox::information(parentWidget(), i18np("1 replacement done.", "%1 replacements done.", d->m_replacements ) );
}

static int replaceHelper(QString &text, const QString &replacement, int index, long options, int length, const QRegExp* regExp)
{
    QString rep(replacement);
    if (options & KReplaceDialog::BackReference) {
        // Backreferences: replace \0 with the right portion of 'text'
        rep.replace( "\\0", text.mid( index, length ) );

        // Other backrefs
        if (regExp) {
            const QStringList caps = regExp->capturedTexts();
            for (int i = 0; i < caps.count(); ++i) {
                rep.replace( "\\" + QString::number(i), caps.at(i) );
            }
        }
    }

    // Then replace rep into the text
    text.replace(index, length, rep);
    return rep.length();
}

KFind::Result KReplace::replace()
{
    KFind::Private* df = KFind::d;
#ifdef DEBUG_REPLACE
    kDebug() << "d->index=" << df->index;
#endif
    if ( df->index == INDEX_NOMATCH && df->lastResult == Match )
    {
        df->lastResult = NoMatch;
        return NoMatch;
    }

    do // this loop is only because validateMatch can fail
    {
#ifdef DEBUG_REPLACE
        kDebug() << "beginning of loop: df->index=" << df->index;
#endif
        // Find the next match.
        if ( df->options & KFind::RegularExpression )
            df->index = KFind::find(df->text, *df->regExp, df->index, df->options, &df->matchedLength);
        else
            df->index = KFind::find(df->text, df->pattern, df->index, df->options, &df->matchedLength);

#ifdef DEBUG_REPLACE
        kDebug() << "KFind::find returned df->index=" << df->index;
#endif
        if ( df->index != -1 )
        {
            // Flexibility: the app can add more rules to validate a possible match
            if ( validateMatch( df->text, df->index, df->matchedLength ) )
            {
                if ( df->options & KReplaceDialog::PromptOnReplace )
                {
#ifdef DEBUG_REPLACE
                    kDebug() << "PromptOnReplace";
#endif
                    // Display accurate initial string and replacement string, they can vary
                    QString matchedText (df->text.mid( df->index, df->matchedLength ));
                    QString rep (matchedText);
                    replaceHelper(rep, d->m_replacement, 0, df->options, df->matchedLength, df->regExp);
                    d->dialog()->setLabel( matchedText, rep );
                    d->dialog()->show(); // TODO kde5: virtual void showReplaceNextDialog(QString,QString), so that kreplacetest can skip the show()

                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    emit highlight(df->text, df->index, df->matchedLength);

                    df->lastResult = Match;
                    return Match;
                }
                else
                {
                    d->doReplace(); // this moves on too
                }
            }
            else
            {
                // not validated -> move on
                if (df->options & KFind::FindBackwards)
                    df->index--;
                else
                    df->index++;
            }
        } else
            df->index = INDEX_NOMATCH; // will exit the loop
    }
    while (df->index != INDEX_NOMATCH);

    df->lastResult = NoMatch;
    return NoMatch;
}

int KReplace::replace(QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replaceHelper(text, replacement, index, options, matchedLength, NULL);
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
        *replacedLength = replaceHelper(text, replacement, index, options, matchedLength, &pattern);
        if (options & KFind::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

void KReplacePrivate::_k_slotReplaceAll()
{
    doReplace();
    q->KFind::d->options &= ~KReplaceDialog::PromptOnReplace;
    emit q->optionsChanged();
    emit q->findNext();
}

void KReplacePrivate::_k_slotSkip()
{
    if (q->KFind::d->options & KFind::FindBackwards)
        q->KFind::d->index--;
    else
        q->KFind::d->index++;
    if ( q->KFind::d->dialogClosed ) {
        q->KFind::d->dialog->deleteLater(); q->KFind::d->dialog = 0L; // hide it again
    } else
        emit q->findNext();
}

void KReplacePrivate::_k_slotReplace()
{
    doReplace();
    if ( q->KFind::d->dialogClosed ) {
        q->KFind::d->dialog->deleteLater(); q->KFind::d->dialog = 0L; // hide it again
    } else
        emit q->findNext();
}

void KReplacePrivate::doReplace()
{
    KFind::Private* df = q->KFind::d;
    Q_ASSERT(df->index >= 0);
    const int replacedLength = replaceHelper(df->text, m_replacement, df->index, df->options, df->matchedLength, df->regExp);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    emit q->replace(df->text, df->index, replacedLength, df->matchedLength);
#ifdef DEBUG_REPLACE
    kDebug() << "after replace() signal: KFind::d->index=" << df->index << " replacedLength=" << replacedLength;
#endif
    m_replacements++;
    if (df->options & KFind::FindBackwards) {
        Q_ASSERT(df->index >= 0);
        df->index--;
    } else {
        df->index += replacedLength;
        // when replacing the empty pattern, move on. See also kjs/regexp.cpp for how this should be done for regexps.
        if ( df->pattern.isEmpty() )
            ++(df->index);
    }
#ifdef DEBUG_REPLACE
    kDebug() << "after adjustement: KFind::d->index=" << df->index;
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

    int ret = KMessageBox::questionYesNo( parentWidget(), message, QString(),
                                          KGuiItem(i18nc("@action:button Restart find & replace", "Restart")),
                                          KGuiItem(i18nc("@action:button Stop find & replace", "Stop")) );
    return( ret == KMessageBox::Yes );
}

void KReplace::closeReplaceNextDialog()
{
    closeFindNextDialog();
}

#include "kreplace.moc"
