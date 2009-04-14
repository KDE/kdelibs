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

#include "kfilesharedialog.h"
#include "kfsprocess.h"
#include <kvbox.h>
#include <QtGui/QLabel>
#include <QtCore/QDir>
#include <QtGui/QRadioButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QLayout>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <kio/kfileshare.h>
#include <kseparator.h>
#include <QtGui/QPushButton>
#include <kapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>

class KFileSharePropsPlugin::Private
{
public:
    KVBox *m_vBox;
    KfsProcess *m_configProc;
    bool m_bAllShared;
    bool m_bAllUnshared;
    QWidget *m_widget;
    QRadioButton *m_rbShare;
    QRadioButton *m_rbUnShare;
    QPushButton *m_pbConfig;
};

KFileSharePropsPlugin::KFileSharePropsPlugin( KPropertiesDialog *_props )
    : KPropertiesDialogPlugin( _props ),d(new Private)
{
    d->m_vBox = new KVBox();
    _props->addPage( d->m_vBox, i18n("&Share") );

    d->m_configProc = 0;
    properties->setFileSharingPage(d->m_vBox);
    d->m_widget = 0L;
    init();
}

KFileSharePropsPlugin::~KFileSharePropsPlugin()
{
    if (d->m_configProc)
        d->m_configProc->detach(); // Detach to prevent that we kill the process
    delete d;
}

bool KFileSharePropsPlugin::supports( const KFileItemList& items )
{
    // Do not show dialog if in advanced mode,
    // because the advanced dialog is shown already.
    if (KFileShare::shareMode() == KFileShare::Advanced) {
        kDebug() << "KFileSharePropsPlugin::supports: false because sharemode is advanced";
        return false;
    }

    KFileItemList::const_iterator kit = items.begin();
    const KFileItemList::const_iterator kend = items.end();
    for ( ; kit != kend; ++kit )
    {
        bool isLocal = (*kit).isLocalFile();
        // We only support local dirs
        if ( !(*kit).isDir() || !isLocal )
            return false;
    }
    return true;
}

void KFileSharePropsPlugin::init()
{
    // We store the main widget, so that it's possible (later) to call init()
    // more than once, to update the page if something changed (e.g. after
    // the user has been authorized)
    delete d->m_widget;
    d->m_rbShare = 0L;
    d->m_rbUnShare = 0L;
    d->m_widget = new QWidget( d->m_vBox );
    QVBoxLayout * vbox = new QVBoxLayout( d->m_widget );

    switch ( KFileShare::authorization() ) {
    case KFileShare::Authorized:
    {
        // Check if all selected dirs are in $HOME
        QString home = QDir::homePath();
        if ( home[home.length()-1] != '/' )
            home += '/';
        bool ok = true;
        const KFileItemList items = properties->items();
        // We have 3 possibilities: all shared, all unshared, or mixed.
        d->m_bAllShared = true;
        d->m_bAllUnshared = true;
        KFileItemList::const_iterator kit = items.begin();
        const KFileItemList::const_iterator kend = items.end();
        for ( ; kit != kend && ok; ++kit )
        {
            // We know it's local, see supports()
            const QString path = (*kit).url().toLocalFile();
            if ( !path.startsWith( home ) )
                ok = false;
            if ( KFileShare::isDirectoryShared( path ) )
                d->m_bAllUnshared = false;
            else
                d->m_bAllShared = false;
        }
        if ( !ok )
        {
            vbox->addWidget( new QLabel( i18n( "Only folders in your home folder can be shared."),
                                         d->m_widget ), 0 );
        }
        else
        {
            // Everything ok, show the share/unshare GUI
            QButtonGroup *rbGroup = new QButtonGroup( d->m_widget );
            d->m_rbUnShare = new QRadioButton( i18n("Not shared"), d->m_widget );
            connect( d->m_rbUnShare, SIGNAL( toggled(bool) ), SIGNAL( changed() ) );
            vbox->addWidget( d->m_rbUnShare, 0 );
            rbGroup->addButton( d->m_rbUnShare );

            d->m_rbShare = new QRadioButton( i18n("Shared"), d->m_widget );
            connect( d->m_rbShare, SIGNAL( toggled(bool) ), SIGNAL( changed() ) );
            vbox->addWidget( d->m_rbShare, 0 );
            rbGroup->addButton( d->m_rbShare );

            // Activate depending on status
            if ( d->m_bAllShared )
                d->m_rbShare->setChecked(true);
            if ( d->m_bAllUnshared )
                d->m_rbUnShare->setChecked(true);

            // Some help text
            QLabel *label = new QLabel( i18n("Sharing this folder makes it available under Linux/UNIX (NFS) and Windows (Samba).") , d->m_widget );
            label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter);
	    label->setWordWrap(true);
            vbox->addWidget( label, 0 );

	    KSeparator* sep=new KSeparator(d->m_widget);
	    vbox->addWidget( sep, 0 );
	    label = new QLabel( i18n("You can also reconfigure file sharing authorization.") , d->m_widget );
            label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter);
	    label->setWordWrap(true);
	    vbox->addWidget( label, 0 );
	    d->m_pbConfig = new QPushButton( i18n("Configure File Sharing..."), d->m_widget );
	    connect( d->m_pbConfig, SIGNAL( clicked() ), SLOT( slotConfigureFileSharing() ) );
	    vbox->addWidget( d->m_pbConfig, 0, Qt::AlignHCenter );

            vbox->addStretch( 10 );
        }
    }
    break;
    case KFileShare::ErrorNotFound:
        vbox->addWidget( new QLabel( i18n("Error running 'filesharelist'. Check if installed and in $PATH or /usr/sbin."),
                    d->m_widget ), 0 );
        break;
    case KFileShare::UserNotAllowed:
    {
        vbox->setSpacing( 10 );
        if (KFileShare::sharingEnabled()) {
          vbox->addWidget( new QLabel( i18n("You need to be authorized to share folders."),
                    d->m_widget ), 0 );
        } else {
          vbox->addWidget( new QLabel( i18n("File sharing is disabled."),
                    d->m_widget ), 0 );
        }
        QHBoxLayout* hBox = new QHBoxLayout( (QWidget *)0L );
        vbox->addLayout( hBox, 0 );
        d->m_pbConfig = new QPushButton( i18n("Configure File Sharing..."), d->m_widget );
        connect( d->m_pbConfig, SIGNAL( clicked() ), SLOT( slotConfigureFileSharing() ) );
        hBox->addWidget( d->m_pbConfig, 0, Qt::AlignHCenter );
        vbox->addStretch( 10 ); // align items on top
        break;
    }
    case KFileShare::NotInitialized:
        kWarning() << "KFileShare Authorization still NotInitialized after calling authorization() - impossible";
        break;
    }
    d->m_widget->show(); // In case the dialog was shown already.
}

void KFileSharePropsPlugin::slotConfigureFileSharing()
{
    if (d->m_configProc) return;

    d->m_configProc = new KfsProcess(this);
    (*d->m_configProc) << KStandardDirs::findExe("kdesu") << "kcmshell4" << "fileshare";
    if (!d->m_configProc->start())
    {
       delete d->m_configProc;
       d->m_configProc = 0;
       return;
    }
    connect(d->m_configProc, SIGNAL(processExited()),
            this, SLOT(slotConfigureFileSharingDone()));
    d->m_pbConfig->setEnabled(false);
}

void KFileSharePropsPlugin::slotConfigureFileSharingDone()
{
    delete d->m_configProc;
    d->m_configProc = 0;
    KFileShare::readConfig();
    KFileShare::readShareList();
    init();
}

void KFileSharePropsPlugin::applyChanges()
{
    kDebug() << "KFileSharePropsPlugin::applyChanges";
    if ( d->m_rbShare && d->m_rbUnShare )
    {
        bool share = d->m_rbShare->isChecked();

        if (share && d->m_bAllShared)
           return; // Nothing to do
        if (!share && d->m_bAllUnshared)
           return; // Nothing to do

        const KFileItemList items = properties->items();
        bool ok = true;
        KFileItemList::const_iterator kit = items.begin();
        const KFileItemList::const_iterator kend = items.end();
        for ( ; kit != kend && ok; ++kit )
        {
             const QString path = (*kit).url().toLocalFile();
             ok = setShared( path, share );
             if (!ok) {
                if (share)
                  KMessageBox::detailedError(properties,
                    i18n("Sharing folder '%1' failed.", path),
                    i18n("An error occurred while trying to share folder '%1'. "
                         "Make sure that the Perl script 'fileshareset' is set suid root.",
                          path));
                else
                  KMessageBox::error(properties,
                    i18n("Unsharing folder '%1' failed.", path),
                    i18n("An error occurred while trying to unshare folder '%1'. "
                         "Make sure that the Perl script 'fileshareset' is set suid root.",
                          path));

                properties->abortApplying();
                break;
             }
        }

        // Get the change back into our cached info
        KFileShare::readShareList();
    }
}

bool KFileSharePropsPlugin::setShared( const QString& path, bool shared )
{
    kDebug() << "KFileSharePropsPlugin::setShared " << path << "," << shared;
    return KFileShare::setShared( path, shared );
}

QWidget* KFileSharePropsPlugin::page() const
{
    return d->m_vBox;
}

#include "kfilesharedialog.moc"

//TODO: do we need to monitor /etc/security/fileshare.conf ?
// if the user is added to the 'fileshare' group, we wouldn't be notified
// Of course the config module can notify us.
// TODO: listen to such notifications ;)
