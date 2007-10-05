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
#include "kfinddialog_p.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtCore/QRegExp>
#include <khistorycombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

/**
 * we need to insert the strings after the dialog is set
 * up, otherwise QComboBox will deliver an aweful big sizeHint
 * for long replacement texts.
 */
class KReplaceDialogPrivate
{
  public:
    KReplaceDialogPrivate(KReplaceDialog *q)
    : q(q)
    , initialShowDone(false)
    , replaceExtension (0)
    {
    }

    void _k_slotOk();

    KReplaceDialog *q;
    QStringList replaceStrings;
    bool initialShowDone;
    QWidget *replaceExtension;
};

KReplaceDialog::KReplaceDialog(QWidget *parent, long options, const QStringList &findStrings,
                               const QStringList &replaceStrings, bool hasSelection)
    : KFindDialog(parent, options, findStrings, hasSelection, true /*create replace dialog*/),
      d(new KReplaceDialogPrivate(this))
{
    d->replaceStrings = replaceStrings;
}

KReplaceDialog::~KReplaceDialog()
{
    delete d;
}

void KReplaceDialog::showEvent( QShowEvent *e )
{
    if ( !d->initialShowDone )
    {
        d->initialShowDone = true; // only once

        if (!d->replaceStrings.isEmpty())
        {
          setReplacementHistory(d->replaceStrings);
          KFindDialog::d->replace->lineEdit()->setText( d->replaceStrings[0] );
        }
    }

    KFindDialog::showEvent(e);
}

long KReplaceDialog::options() const
{
    long options = 0;

    options = KFindDialog::options();
    if (KFindDialog::d->promptOnReplace->isChecked())
        options |= PromptOnReplace;
    if (KFindDialog::d->backRef->isChecked())
        options |= BackReference;
    return options;
}

QWidget *KReplaceDialog::replaceExtension() const
{
    if (!d->replaceExtension)
    {
      d->replaceExtension = new QWidget(KFindDialog::d->replaceGrp);
      KFindDialog::d->replaceLayout->addWidget(d->replaceExtension, 3, 0, 1, 2);
    }

    return d->replaceExtension;
}

QString KReplaceDialog::replacement() const
{
    return KFindDialog::d->replace->currentText();
}

QStringList KReplaceDialog::replacementHistory() const
{
    QStringList lst = KFindDialog::d->replace->historyItems();
    // historyItems() doesn't tell us about the case of replacing with an empty string
    if ( KFindDialog::d->replace->lineEdit()->text().isEmpty() )
        lst.prepend( QString() );
    return lst;
}

void KReplaceDialog::setOptions(long options)
{
    KFindDialog::setOptions(options);
    KFindDialog::d->promptOnReplace->setChecked(options & PromptOnReplace);
    KFindDialog::d->backRef->setChecked(options & BackReference);
}

void KReplaceDialog::setReplacementHistory(const QStringList &strings)
{
    if (strings.count() > 0)
        KFindDialog::d->replace->setHistoryItems(strings, true);
    else
        KFindDialog::d->replace->clearHistory();
}

void KReplaceDialogPrivate::_k_slotOk()
{
    // If regex and backrefs are enabled, do a sanity check.
    if ( q->KFindDialog::d->regExp->isChecked() && q->KFindDialog::d->backRef->isChecked() )
    {
        QRegExp r ( q->pattern() );
        int caps = r.numCaptures();
        QRegExp check(QString("((?:\\\\)+)(\\d+)"));
        int p = 0;
        QString rep = q->replacement();
        while ( (p = check.indexIn( rep, p ) ) > -1 )
        {
            if ( check.cap(1).length()%2 && check.cap(2).toInt() > caps )
            {
                KMessageBox::information( q, i18n(
                        "Your replacement string is referencing a capture greater than '\\%1', ",  caps ) +
                    ( caps ?
                        i18np("but your pattern only defines 1 capture.",
                             "but your pattern only defines %1 captures.", caps ) :
                        i18n("but your pattern defines no captures.") ) +
                    i18n("\nPlease correct.") );
                return; // abort OKing
            }
            p += check.matchedLength();
        }

    }

    q->KFindDialog::d->_k_slotOk();
    q->KFindDialog::d->replace->addToHistory(q->replacement());
}

// kate: space-indent on; indent-width 4; replace-tabs on;
#include "kreplacedialog.moc"
