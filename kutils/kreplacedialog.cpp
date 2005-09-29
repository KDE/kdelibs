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

#include "kreplacedialog.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>

/**
 * we need to insert the strings after the dialog is set
 * up, otherwise QComboBox will deliver an aweful big sizeHint
 * for long replacement texts.
 */
class KReplaceDialog::KReplaceDialogPrivate {
  public:
    KReplaceDialogPrivate() : m_initialShowDone(false) {}
    QStringList replaceStrings;
    bool m_initialShowDone;
};

KReplaceDialog::KReplaceDialog(QWidget *parent, const char *name, long options, const QStringList &findStrings, const QStringList &replaceStrings, bool hasSelection) :
    KFindDialog(parent, name, true)
{
    d = new KReplaceDialogPrivate;
    d->replaceStrings = replaceStrings;
    init(true, findStrings, hasSelection);
    setOptions(options);
}

KReplaceDialog::~KReplaceDialog()
{
    delete d;
}

void KReplaceDialog::showEvent( QShowEvent *e )
{
    if ( !d->m_initialShowDone )
    {
        d->m_initialShowDone = true; // only once

        if (!d->replaceStrings.isEmpty())
        {
          setReplacementHistory(d->replaceStrings);
          m_replace->lineEdit()->setText( d->replaceStrings[0] );
        }
    }

    KFindDialog::showEvent(e);
}

long KReplaceDialog::options() const
{
    long options = 0;

    options = KFindDialog::options();
    if (m_promptOnReplace->isChecked())
        options |= PromptOnReplace;
    if (m_backRef->isChecked())
        options |= BackReference;
    return options;
}

QWidget *KReplaceDialog::replaceExtension()
{
    if (!m_replaceExtension)
    {
      m_replaceExtension = new QWidget(m_replaceGrp);
      m_replaceLayout->addMultiCellWidget(m_replaceExtension, 3, 3, 0, 1);
    }

    return m_replaceExtension;
}

QString KReplaceDialog::replacement() const
{
    return m_replace->currentText();
}

QStringList KReplaceDialog::replacementHistory() const
{
    return m_replace->historyItems();
}

void KReplaceDialog::setOptions(long options)
{
    KFindDialog::setOptions(options);
    m_promptOnReplace->setChecked(options & PromptOnReplace);
    m_backRef->setChecked(options & BackReference);
}

void KReplaceDialog::setReplacementHistory(const QStringList &strings)
{
    if (strings.count() > 0)
        m_replace->setHistoryItems(strings, true);
    else
        m_replace->clearHistory();
}

void KReplaceDialog::slotOk()
{
    // If regex and backrefs are enabled, do a sanity check.
    if ( m_regExp->isChecked() && m_backRef->isChecked() )
    {
        QRegExp r ( pattern() );
        int caps = r.numCaptures();
        QRegExp check(QString("((?:\\\\)+)(\\d+)"));
        int p = 0;
        QString rep = replacement();
        while ( (p = check.search( rep, p ) ) > -1 )
        {
            if ( check.cap(1).length()%2 && check.cap(2).toInt() > caps )
            {
                KMessageBox::information( this, i18n(
                        "Your replacement string is referencing a capture greater than '\\%1', ").arg( caps ) +
                    ( caps ?
                        i18n("but your pattern only defines 1 capture.",
                             "but your pattern only defines %n captures.", caps ) :
                        i18n("but your pattern defines no captures.") ) +
                    i18n("\nPlease correct.") );
                return; // abort OKing
            }
            p += check.matchedLength();
        }

    }

    KFindDialog::slotOk();
    m_replace->addToHistory(replacement());
}

// kate: space-indent on; indent-width 4; replace-tabs on;
#include "kreplacedialog.moc"
