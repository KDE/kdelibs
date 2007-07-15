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

#ifndef KFINDDIALOG_P_H
#define KFINDDIALOG_P_H

#include "kfinddialog.h"
#include "kfind.h"

#include <QtCore/QStringList>

class KFindDialog::KFindDialogPrivate
{
public:
    KFindDialogPrivate() : regexpDialog(0),
        regexpDialogQueryDone(false),
        enabled(KFind::WholeWordsOnly | KFind::FromCursor |  KFind::SelectedText | KFind::CaseSensitive | KFind::FindBackwards | KFind::RegularExpression),
        initialShowDone(false),
        findExtension(0)
        {}
    QDialog *regexpDialog;
    bool regexpDialogQueryDone;
    long enabled; // uses Options to define which search options are enabled
    bool initialShowDone;
    QStringList findStrings;
    QString pattern;
    QWidget *findExtension;

    QGroupBox *findGrp;
    KHistoryComboBox *find;
    QCheckBox *regExp;
    QPushButton *regExpItem;
    QGridLayout *findLayout;

    QCheckBox *wholeWordsOnly;
    QCheckBox *fromCursor;
    QCheckBox *selectedText;
    QCheckBox *caseSensitive;
    QCheckBox *findBackwards;

    QMenu *patterns;

    // for the replace dialog

    QGroupBox *replaceGrp;
    KHistoryComboBox *replace;
    QCheckBox* backRef;
    QPushButton* backRefItem;
    QGridLayout *replaceLayout;

    QCheckBox* promptOnReplace;

    QMenu *placeholders;
};

#endif // KFINDDIALOG_P_H
