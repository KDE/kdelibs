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

#include "kfilesharedlg.h"
#include <qvbox.h>
#include <qlabel.h>
#include <qdir.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <kprocess.h>
#include <kprocio.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <kio/kfileshare.h>
#include <libintl.h>
#include <kseparator.h>
#include <qpushbutton.h>
#include <kapplication.h>

class KFileSharePropsPlugin::Private
{
public:
    QVBox *m_vBox;
};

KFileSharePropsPlugin::KFileSharePropsPlugin( KPropertiesDialog *_props )
    : KPropsDlgPlugin( _props )
{
    d = new Private;
    d->m_vBox = _props->addVBoxPage( i18n("Local Net Sharing") );
    m_widget = 0L;
    init();
}

KFileSharePropsPlugin::~KFileSharePropsPlugin()
{
    delete d;
}

bool KFileSharePropsPlugin::supports( const KFileItemList& items )
{
    KFileItemListIterator it( items );
    for ( ; it.current(); ++it )
    {
        bool isLocal = ( *it )->isLocalFile();
        // We only support local dirs
        if ( !(*it)->isDir() || !isLocal )
            return false;
        // And sharing the trash doesn't make sense
        if ( isLocal && (*it)->url().path( 1 ) == KGlobalSettings::trashPath() )
            return false;
    }
    return true;
}

void KFileSharePropsPlugin::init()
{
    // We store the main widget, so that it's possible (later) to call init()
    // more than once, to update the page if something changed (e.g. after
    // the user has been authorized)
    delete m_widget;
    m_rbShare = 0L;
    m_rbUnShare = 0L;
    m_widget = new QWidget( d->m_vBox );
    QVBoxLayout * vbox = new QVBoxLayout( m_widget );

    switch ( KFileShare::authorization() ) {
    case KFileShare::Authorized:
    {
        // Check if all selected dirs are in $HOME
        QString home = QDir::homeDirPath();
        if ( home[home.length()-1] != '/' )
            home += '/';
        bool ok = true;
        KFileItemList items = properties->items();
        // We have 3 possibilities: all shared, all unshared, or mixed.
        bool allShared = true;
        bool allUnshared = true;
        KFileItemListIterator it( items );
        for ( ; it.current() && ok; ++it ) {
            QString path = (*it)->url().path();
            if ( !path.startsWith( home ) )
                ok = false;
            if ( KFileShare::isDirectoryShared( path ) )
                allUnshared = false;
            else
                allShared = false;
        }
        if ( !ok )
        {
            vbox->addWidget( new QLabel( i18n( "Only directories in your Home can be shared."),
                                         m_widget ), 0 );
        }
        else
        {
            // Everything ok, show the share/unshare GUI
            vbox->setSpacing( 20 );
            vbox->setMargin( 20 );

            QButtonGroup *rbGroup = new QButtonGroup( m_widget );
            rbGroup->hide();
            m_rbUnShare = new QRadioButton( i18n("Not shared"), m_widget );
            connect( m_rbUnShare, SIGNAL( toggled(bool) ), SIGNAL( changed() ) );
            vbox->addWidget( m_rbUnShare, 0 );
            rbGroup->insert( m_rbUnShare );

            m_rbShare = new QRadioButton( i18n("Shared"), m_widget );
            connect( m_rbShare, SIGNAL( toggled(bool) ), SIGNAL( changed() ) );
            vbox->addWidget( m_rbShare, 0 );
            rbGroup->insert( m_rbShare );

            // Activate depending on status
            if ( allShared )
                m_rbShare->setChecked(true);
            if ( allUnshared )
                m_rbUnShare->setChecked(true);

            // Some help text
            QLabel *label = new QLabel( i18n("Sharing this directory makes it available under Linux/Unix (NFS) and Windows (Samba).") , m_widget );
            label->setAlignment( Qt::AlignAuto | Qt::AlignVCenter | Qt::WordBreak );
            vbox->addWidget( label, 0 );

	    KSeparator* sep=new KSeparator(m_widget);
	    vbox->addWidget( sep, 0 );
	    label = new QLabel( i18n("You can also reconfigure file sharing authorization.") , m_widget );
            label->setAlignment( Qt::AlignAuto | Qt::AlignVCenter | Qt::WordBreak );
	    vbox->addWidget( label, 0 );
	    m_pbConfig = new QPushButton( i18n("Configure File Sharing"), m_widget );
	    connect( m_pbConfig, SIGNAL( clicked() ), SLOT( slotConfigureFileSharing() ) );
	    vbox->addWidget( m_pbConfig, 0, Qt::AlignHCenter );

            vbox->addStretch( 10 );
        }
    }
    break;
    case KFileShare::ErrorNotFound:
        vbox->addWidget( new QLabel( i18n("Error running 'filesharelist'. Check if installed and in $PATH or /usr/sbin."),
                    m_widget ), 0 );
        break;
    case KFileShare::UserNotAllowed:
    {
        vbox->setSpacing( 10 );
        vbox->addWidget( new QLabel( i18n("You need to be authorized to share directories."),
                    m_widget ), 0 );
        QHBoxLayout* hBox = new QHBoxLayout( (QWidget *)0L );
        vbox->addLayout( hBox, 0 );
        m_pbConfig = new QPushButton( i18n("Configure File Sharing"), m_widget );
        connect( m_pbConfig, SIGNAL( clicked() ), SLOT( slotConfigureFileSharing() ) );
        hBox->addWidget( m_pbConfig, 0, Qt::AlignHCenter );
        vbox->addStretch( 10 ); // align items on top
        break;
    }
    }
}

void KFileSharePropsPlugin::slotConfigureFileSharing()
{
    KProcess proc;
    proc << KStandardDirs::findExe("kdesu") << "kcmshell" << "fileshare";
    proc.start( KProcess::DontCare );
    m_pbConfig->setEnabled(false);
}

void KFileSharePropsPlugin::applyChanges()
{
    kdDebug() << "KFileSharePropsPlugin::applyChanges" << endl;
    if ( m_rbShare && m_rbUnShare )
    {
        if ( m_rbShare->isChecked() )
        {
            // Share selected directories
            KFileItemList items = properties->items();
            KFileItemListIterator it( items );
            bool ok = true;
            for ( ; it.current() && ok; ++it ) {
                QString path = (*it)->url().path();
                ok = setShared( path, true );
            }
        }
        else if ( m_rbUnShare->isChecked() )
        {
            // Unshare selected directories
            KFileItemList items = properties->items();
            KFileItemListIterator it( items );
            bool ok = true;
            for ( ; it.current() && ok; ++it ) {
                QString path = (*it)->url().path();
                ok = setShared( path, false );
            }
        }
        // Get the change back into our cached info
        KFileShare::readConfig();
    }
}

bool KFileSharePropsPlugin::setShared( const QString& path, bool shared )
{
    kdDebug() << "KFileSharePropsPlugin::setShared " << path << "," << shared << endl;
    return KFileShare::setShared( path, shared );
}

QWidget* KFileSharePropsPlugin::page() const
{
    return d->m_vBox;
}

#include "kfilesharedlg.moc"

//TODO: do we need to monitor /etc/security/fileshare.conf ?
// if the user is added to the 'fileshare' group, we wouldn't be notified
// Of course the config module can notify us.
// TODO: listen to such notifications ;)
