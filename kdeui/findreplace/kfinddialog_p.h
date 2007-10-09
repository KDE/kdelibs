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

class KHistoryComboBox;

class QCheckBox;
class QGridLayout;
class QGroupBox;
class QMenu;
class QPushButton;

class KFindDialog::KFindDialogPrivate
{
public:
    KFindDialogPrivate(KFindDialog *q)
        : q(q),
        regexpDialog(0),
        regexpDialogQueryDone(false),
        initialShowDone(false),
        enabled(KFind::WholeWordsOnly | KFind::FromCursor |  KFind::SelectedText | KFind::CaseSensitive | KFind::FindBackwards | KFind::RegularExpression),
        findExtension(0)
        {}

    void init( bool forReplace, const QStringList &findStrings, bool hasSelection );

    void _k_slotPlaceholdersAboutToShow();
    void _k_slotOk();
    void _k_slotSelectedTextToggled(bool);
    void _k_showPatterns();
    void _k_showPlaceholders();
    void _k_textSearchChanged(const QString&);

    KFindDialog *q;
    QDialog *regexpDialog;
    bool regexpDialogQueryDone : 1;
    bool initialShowDone : 1;
    long enabled; // uses Options to define which search options are enabled
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
