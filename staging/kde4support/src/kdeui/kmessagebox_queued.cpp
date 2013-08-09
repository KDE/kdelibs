/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kmessagebox_queued.h"

#include <kmessagebox_kiw.h>
#include "kdialogqueue_p.h"

namespace KMessageBox {

extern int KWIDGETSADDONS_EXPORT (*KMessageBox_exec_hook)(QDialog*);

int queued_dialog_exec(QDialog *dialog)
{
    KDialogQueue::queueDialog(dialog);
    return KMessageBox::Cancel; // We have to return something.
}

void queuedDetailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
    return queuedDetailedErrorWId( parent ? parent->effectiveWinId() : 0, text, details, caption );
}

void queuedDetailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
   KMessageBox_exec_hook = &queued_dialog_exec;
   (void) detailedErrorWId(parent_id, text, details, caption);
   KMessageBox_exec_hook = 0;
}

void queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption, Options options )
{
    return queuedMessageBoxWId( parent ? parent->effectiveWinId() : 0, type, text, caption, options );
}

void queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption, Options options )
{
    KMessageBox_exec_hook = &queued_dialog_exec;
    (void) messageBoxWId(parent_id, type, text, caption, KStandardGuiItem::yes(),
                     KStandardGuiItem::no(), KStandardGuiItem::cancel(), QString(), options);
    KMessageBox_exec_hook = 0;
}

void queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption )
{
    return queuedMessageBoxWId( parent ? parent->effectiveWinId() : 0, type, text, caption );
}

void queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption )
{
    KMessageBox_exec_hook = &queued_dialog_exec;
    (void) messageBoxWId(parent_id, type, text, caption);
    KMessageBox_exec_hook = 0;
}


} // KMessageBox
