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

#include "kreplacedialog.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <kcombobox.h>

KReplaceDialog::KReplaceDialog(QWidget *parent, const char *name, long options, const QStringList &findStrings, const QStringList &replaceStrings, bool hasSelection) :
    KFindDialog(parent, name, true)
{
    init(true, findStrings, hasSelection);
    setOptions(options);
    setReplacementHistory(replaceStrings);
}

KReplaceDialog::~KReplaceDialog()
{
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
    return m_replaceExtension;
}

QString KReplaceDialog::replacement() const
{
    return m_replace->currentText();
}

QStringList KReplaceDialog::replacementHistory() const
{
    return m_find->historyItems();
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
    KFindDialog::slotOk();
    m_replace->addToHistory(replacement());
}

#include "kreplacedialog.moc"
