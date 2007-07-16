/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <faure@kde.org>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef KFILESHAREDIALOG_H
#define KFILESHAREDIALOG_H

#include <kpropertiesdialog.h>
class QRadioButton;
class QPushButton;

/**
 * This plugin provides a page to KProperiessDialog, showing the "file sharing" options
 * @author David Faure <faure@kde.org>
 */
class KIO_EXPORT KFileSharePropsPlugin : public KPropertiesDialogPlugin
{
    Q_OBJECT
public:
    KFileSharePropsPlugin( KPropertiesDialog *_props );
    virtual ~KFileSharePropsPlugin();

    /**
     * Apply all changes to the file.
     * This function is called when the user presses 'Ok'. The last plugin inserted
     * is called first.
     */
    virtual void applyChanges();

    static bool supports( const KFileItemList& items );

    QWidget* page() const;

protected Q_SLOTS:
    void slotConfigureFileSharing();
    void slotConfigureFileSharingDone();

private:
    void init();
    bool setShared( const QString&path, bool shared );

    class Private;
    Private* const d;
};

#endif
