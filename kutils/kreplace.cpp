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

#include <qlabel.h>
#include <kapplication.h>
#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include "kreplace.h"
#include "kreplacedialog.h"
#include <qregexp.h>

struct KReplace::KReplacePrivate
{
    void setLabel( const QString& pattern, const QString& replacement ) {
        m_mainLabel->setText( i18n("Replace '%1' with '%2'?").arg(pattern).arg(replacement) );
    }
    QLabel* m_mainLabel;
};

// Create the dialog.
KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent) :
    KFind( pattern, replacement, options, parent )
{
    d = new KReplacePrivate;
    d->m_mainLabel = new QLabel( this );
    d->setLabel( pattern, replacement );
    setMainWidget( d->m_mainLabel );

    m_replacements = 0;
    m_replacement = replacement;
}

KReplace::~KReplace()
{
    if (displayFinalDialog() && !m_cancelled)
    {
        if ( !m_replacements )
            KMessageBox::information(parentWidget(), i18n("No text was replaced."));
        else
            KMessageBox::information(parentWidget(), i18n("1 replacement done.\n", "%n replacements done.\n", m_replacements ) );

    }
    setDisplayFinalDialog( false ); // don't display the KFind dialog :)
    delete d;
}

bool KReplace::replace(QString &text, const QRect &expose)
{
    if (m_options & KFindDialog::FindBackwards)
    {
        m_index = text.length();
    }
    else
    {
        m_index = 0;
    }
    m_text = text;
    m_expose = expose;
    do
    {
        // Find the next match.
        if (m_options & KReplaceDialog::RegularExpression)
            m_index = KFind::find(m_text, *m_regExp, m_index, m_options, &m_matchedLength);
        else
            m_index = KFind::find(m_text, m_pattern, m_index, m_options, &m_matchedLength);
        if (m_index != -1)
        {
            if (m_options & KReplaceDialog::PromptOnReplace)
            {
                if ( validateMatch( m_text, m_index, m_matchedLength ))
                {
                    // Display accurate initial string and replacement string, they can vary
                    QString matchedText = m_text.mid( m_index, m_matchedLength );
                    QString rep = matchedText;
                    KReplace::replace(rep, m_replacement, 0, m_matchedLength);
                    d->setLabel( matchedText, rep );

                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    emit highlight(m_text, m_index, m_matchedLength, m_expose);
                    show();
                    kapp->enter_loop();
                }
                else
                    m_index += m_matchedLength;
            }
            else
            {
                doReplace();
            }
        }
    }
    while ((m_index != -1) && !m_cancelled);
    text = m_text;

    // Should the user continue?
    return !m_cancelled;
}

int KReplace::replace(QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replace(text, replacement, index, matchedLength);
        if (options & KReplaceDialog::FindBackwards)
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
        *replacedLength = replace(text, replacement, index, matchedLength);
        if (options & KReplaceDialog::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

int KReplace::replace(QString &text, const QString &replacement, int index, int length)
{
    // Backreferences: replace /0 with the right portion of 'text'
    QString rep = replacement;
    rep.replace( QRegExp("/0"), text.mid( index, length ) );
    // Then replace rep into the text
    text.replace(index, length, rep);
    return rep.length();
}

// All.
void KReplace::slotUser1()
{
    doReplace();
    m_options &= ~KReplaceDialog::PromptOnReplace;
    kapp->exit_loop();
}

// Skip.
void KReplace::slotUser2()
{
    if (m_options & KReplaceDialog::FindBackwards) m_index--;
       else m_index++;
    kapp->exit_loop();
}

// Yes.
void KReplace::slotUser3()
{
    doReplace();
    kapp->exit_loop();
}

void KReplace::doReplace()
{
    int replacedLength = KReplace::replace(m_text, m_replacement, m_index, m_matchedLength);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    emit replace(m_text, m_index, replacedLength, m_matchedLength, m_expose);
    m_replacements++;
    if (m_options & KReplaceDialog::FindBackwards)
        m_index--;
    else
        m_index += replacedLength;
}

void KReplace::resetCounts()
{
    KFind::resetCounts();
    m_replacements = 0;
}

bool KReplace::shouldRestart( bool forceAsking ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // ... Or if the prompt-on-replace option was set.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (m_options & KFindDialog::FromCursor) == 0
         && (m_options & KReplaceDialog::PromptOnReplace) == 0 )
        return false;
    QString message;
    if ( m_replacements )
        message = i18n("1 replacement done.\n", "%n replacements done.\n", m_replacements );
    else
        message = i18n("No replacement done.\n");

    // Hope this word puzzle is ok, it's a different sentence
    message += i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionYesNo( parentWidget(), message );
    return( ret == KMessageBox::Yes );
}

#include "kreplace.moc"
