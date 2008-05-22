/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kshortcutsdialog.h"

#include "kdebug.h"
#include "klocale.h"


/************************************************************************/
/* KShortcutsDialog                                                     */
/*                                                                      */
/* Originally by Nicolas Hadacek <hadacek@via.ecp.fr>                   */
/*                                                                      */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>              */
/*                                                                      */
/* And by Espen Sand <espen@kde.org> 1999-10-19                         */
/* (by using KDialog there is almost no code left ;)                    */
/*                                                                      */
/************************************************************************/

class KShortcutsDialog::KShortcutsDialogPrivate
{
public:

    KShortcutsDialogPrivate(KShortcutsDialog *q): q(q), m_keyChooser(0) 
        {}

    KShortcutsDialog *q;
    KShortcutsEditor* m_keyChooser; // ### move
};


KShortcutsDialog::KShortcutsDialog( KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts, QWidget *parent )
: KDialog( parent ), d(new KShortcutsDialogPrivate(this))
{
    setCaption(i18n("Configure Shortcuts"));
    setButtons(Default|Ok|Cancel|KDialog::User1);
    setButtonText(KDialog::User1, i18n("Print"));
    setButtonIcon(KDialog::User1, KIcon("file_prit"));
    setModal(true);
    d->m_keyChooser = new KShortcutsEditor( this, types, allowLetterShortcuts );
    setMainWidget( d->m_keyChooser );
    connect( this, SIGNAL(defaultClicked()), d->m_keyChooser, SLOT(allDefault()) );
    connect( this, SIGNAL(user1Clicked()), d->m_keyChooser, SLOT(printShortcuts()) );

    KConfigGroup group( KGlobal::config(), "KShortcutsDialog Settings" );
    resize( group.readEntry( "Dialog Size", sizeHint() ) );
}


KShortcutsDialog::~KShortcutsDialog()
{
    KConfigGroup group( KGlobal::config(), "KShortcutsDialog Settings" );
    group.writeEntry( "Dialog Size", size(), KConfigGroup::Global );
    delete d;
}


void KShortcutsDialog::addCollection(KActionCollection *collection, const QString &title)
{
    d->m_keyChooser->addCollection(collection, title);
}


bool KShortcutsDialog::configure(bool saveSettings)
{
    int retcode = exec();
    if (retcode != Accepted)
        d->m_keyChooser->undoChanges();
    else if (saveSettings)
        d->m_keyChooser->save();

    return retcode;
}

QSize KShortcutsDialog::sizeHint() const
{
    return QSize(500, 400);
}

int KShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                          QWidget *parent, bool saveSettings)
{
    kDebug(125) << "KShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )";
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
    dlg.d->m_keyChooser->addCollection(collection);
    return dlg.configure(saveSettings);
}

#include "kshortcutsdialog.moc"
#include "kshortcutsdialog_p.moc"
