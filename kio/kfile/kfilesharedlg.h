/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <david@mandrakesoft.com>
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kfilesharedlg_h
#define kfilesharedlg_h

#include <kpropertiesdialog.h>
class QVBoxLayout;
class QRadioButton;
class QPushButton;

/**
 * This plugin provides a page to KPropsDlg, showing the "file sharing" options
 * @author David Faure <david@mandrakesoft.com>
 * @since 3.1
 */
class KFileSharePropsPlugin : public KPropsDlgPlugin
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

protected slots:
    void slotConfigureFileSharing();

private:
    void init();
    bool setShared( const QString&path, bool shared );

    QWidget *m_widget;
    QRadioButton *m_rbShare;
    QRadioButton *m_rbUnShare;
    QPushButton *m_pbConfig;
    class Private;
    Private *d;
};

#endif
