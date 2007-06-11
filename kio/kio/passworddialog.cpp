/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "passworddialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QTextDocument>
#include <QTextLayout>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <khbox.h>

using namespace KIO;

PasswordDialog::PasswordDialog( const QString& prompt, const QString& user,
                                bool enableKeep, bool modal, QWidget* parent )
   :KPasswordDialog( parent , enableKeep ? (ShowUsernameLine | ShowKeepPassword) : ShowUsernameLine )
{
    setModal( modal );
    setPrompt(prompt);
    setUsername( user );
}

PasswordDialog::~PasswordDialog()
{
}


int PasswordDialog::getNameAndPassword( QString& user, QString& pass, bool* keep,
                                        const QString& prompt, bool readOnly,
                                        const QString& caption,
                                        const QString& comment,
                                        const QString& label )
{
    PasswordDialog* dlg;
    dlg = new PasswordDialog( prompt, user, keep );

    if ( !caption.isEmpty() )
        dlg->setCaption( caption );
    else
        dlg->setCaption( i18n("Authorization Dialog") );

    if ( !comment.isEmpty() )
        dlg->addCommentLine( label, comment );

    if ( readOnly )
        dlg->setUsernameReadOnly( readOnly );

    if ( keep )
        dlg->setKeepPassword( *keep );

    int ret = dlg->exec();
    if ( ret == Accepted )
    {
        user = dlg->username();
        pass = dlg->password();
        if ( keep ) { (*keep) = dlg->keepPassword(); }
    }
    delete dlg;
    return ret;
 }


#include "passworddialog.moc"
