/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kdesktopfileactions.h"
#include "krun.h"
#include "kautomount.h"
#include <kmessageboxwrapper.h>
#include <kdirnotify.h>
#include <kmountpoint.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <klocale.h>
#include "kservice.h"

static pid_t runFSDevice( const KUrl& _url, const KDesktopFile &cfg );
static pid_t runApplication( const KUrl& _url, const QString & _serviceFile );
static pid_t runLink( const KUrl& _url, const KDesktopFile &cfg );

pid_t KDesktopFileActions::run( const KUrl& u, bool _is_local )
{
    // It might be a security problem to run external untrusted desktop
    // entry files
    if ( !_is_local )
        return 0;

    KDesktopFile cfg( u.path() );
    QString type = cfg.desktopGroup().readEntry( "Type" );
    if ( type.isEmpty() )
    {
        QString tmp = i18n("The desktop entry file %1 "
                           "has no Type=... entry.", u.path() );
        KMessageBoxWrapper::error( 0, tmp);
        return 0;
    }

    //kDebug(7009) << "TYPE = " << type.data() << endl;

    if ( type == "FSDevice" )
        return runFSDevice( u, cfg );
    else if ( type == "Application" )
        return runApplication( u, u.path() );
    else if ( type == "Link" )
    {
        cfg.setDollarExpansion( true ); // for URL=file:$HOME (Simon)
        return runLink( u, cfg );
    }

    QString tmp = i18n("The desktop entry of type\n%1\nis unknown.",  type );
    KMessageBoxWrapper::error( 0, tmp);

    return 0;
}

static pid_t runFSDevice( const KUrl& _url, const KDesktopFile &cfg )
{
    pid_t retval = 0;

    KConfigGroup cg = cfg.desktopGroup();
    QString dev = cg.readEntry( "Dev" );

    if ( dev.isEmpty() )
    {
        QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  _url.path() );
        KMessageBoxWrapper::error( 0, tmp);
        return retval;
    }

    KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( dev );
    // Is the device already mounted ?
    if (mp) {
        KUrl mpURL(mp->mountPoint());
        // Open a new window
        retval = KRun::runUrl( mpURL, QLatin1String("inode/directory"), 0 /*TODO - window*/ );
    } else {
        bool ro = cg.readEntry("ReadOnly", false);
        QString fstype = cg.readEntry( "FSType" );
        if ( fstype == "Default" ) // KDE-1 thing
            fstype.clear();
        QString point = cg.readEntry( "MountPoint" );
#ifndef Q_WS_WIN
        (void) new KAutoMount( ro, fstype.toLatin1(), dev, point, _url.path() );
#endif
        retval = -1; // we don't want to return 0, but we don't want to return a pid
    }

    return retval;
}

static pid_t runApplication( const KUrl& , const QString & _serviceFile )
{
    KService s( _serviceFile );
    if ( !s.isValid() )
        // The error message was already displayed, so we can just quit here
        // ### KDE4: is this still the case?
        return 0;

    KUrl::List lst;
    return KRun::run( s, lst, 0 /*TODO - window*/ );
}

static pid_t runLink( const KUrl& _url, const KDesktopFile &cfg )
{
    QString u = cfg.readUrl();
    if ( u.isEmpty() )
    {
        QString tmp = i18n("The desktop entry file\n%1\nis of type Link but has no URL=... entry.",  _url.prettyUrl() );
        KMessageBoxWrapper::error( 0, tmp );
        return 0;
    }

    KUrl url ( u );
    KRun* run = new KRun(url,(QWidget*)0);

    // X-KDE-LastOpenedWith holds the service desktop entry name that
    // was should be preferred for opening this URL if possible.
    // This is used by the Recent Documents menu for instance.
    QString lastOpenedWidth = cfg.desktopGroup().readEntry( "X-KDE-LastOpenedWith" );
    if ( !lastOpenedWidth.isEmpty() )
        run->setPreferredService( lastOpenedWidth );

    return -1; // we don't want to return 0, but we don't want to return a pid
}

QList<KDesktopFileActions::Service> KDesktopFileActions::builtinServices( const KUrl& _url )
{
    QList<Service> result;

    if ( !_url.isLocalFile() )
        return result;

    KDesktopFile cfg( _url.path() );
    KConfigGroup group = cfg.desktopGroup();
    QString type = group.readEntry( "Type" );

    if ( type.isEmpty() )
        return result;

    if ( type == "FSDevice" ) {
        QString dev = group.readEntry( "Dev" );
        if ( dev.isEmpty() ) {
            QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  _url.path() );
            KMessageBoxWrapper::error( 0, tmp);
        } else {
            KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( dev );
            // not mounted ?
            if ( !mp ) {
                Service mount;
                mount.m_strName = i18n("Mount");
                mount.m_type = ST_MOUNT;
                result.append( mount );
            } else {
                Service unmount;
#ifdef HAVE_VOLMGT
                /*
                 *  Solaris' volume management can only umount+eject
                 */
                unmount.m_strName = i18n("Eject");
#else
                unmount.m_strName = i18n("Unmount");
#endif
                unmount.m_type = ST_UNMOUNT;
                result.append( unmount );
            }
        }
    }

    return result;
}

QList<KDesktopFileActions::Service> KDesktopFileActions::userDefinedServices( const QString& path, bool bLocalFiles )
{
  KDesktopFile cfg( path );
  return userDefinedServices( path, cfg, bLocalFiles );
}

QList<KDesktopFileActions::Service> KDesktopFileActions::userDefinedServices( const QString& path, const KDesktopFile& cfg, bool bLocalFiles, const KUrl::List & file_list )
{
    QList<Service> result;

    KConfigGroup cg = cfg.desktopGroup();

    if ( !cg.hasKey( "Actions" ) && !cg.hasKey( "X-KDE-GetActionMenu") )
        return result;

    if ( cg.hasKey( "TryExec" ) ) {
        QString tryexec = cg.readPathEntry( "TryExec" );
        QString exe =  KStandardDirs::findExe( tryexec );
        if (exe.isEmpty()) {
            return result;
        }
    }

    QStringList keys;

    if( cg.hasKey( "X-KDE-GetActionMenu" )) {
        QStringList dbuscall = cg.readEntry( "X-KDE-GetActionMenu" ).split( QLatin1Char( ' ' ) );
        const QString& app       = dbuscall.at( 0 );
        const QString& object    = dbuscall.at( 1 );
        const QString& interface = dbuscall.at( 2 );
        const QString& function  = dbuscall.at( 3 );

        QDBusInterface remote( app, object, interface );
        QDBusReply<QStringList> reply = remote.call( QDBus::BlockWithGui,
                                                     function, file_list.toStringList() );
        keys = reply;               // ensures that the reply was a QStringList
    }

    keys += cg.readEntry( "Actions", QStringList(), ';' ); //the desktop standard defines ";" as separator!

    if ( keys.count() == 0 )
        return result;

    QStringList::ConstIterator it = keys.begin();
    QStringList::ConstIterator end = keys.end();
    for ( ; it != end; ++it )
    {
        //kDebug(7009) << "CURRENT KEY = " << (*it) << endl;

        QString group = *it;

        if (group == "_SEPARATOR_")
        {
            Service s;
            result.append(s);
            continue;
        }

        group.prepend( "Desktop Action " );

        bool bInvalidMenu = false;

        if ( cfg.hasGroup( group ) )
        {
            cg.changeGroup( group );

            if ( !cg.hasKey( "Name" ) || !cg.hasKey( "Exec" ) )
                bInvalidMenu = true;
            else
            {
                QString exec = cg.readPathEntry( "Exec" );
                if ( bLocalFiles || exec.contains("%U") || exec.contains("%u") )
                {
                    Service s;
                    s.m_strName = cg.readEntry( "Name" );
                    s.m_strIcon = cg.readEntry( "Icon" );
                    s.m_strExec = exec;
                    s.m_type = ST_USER_DEFINED;
                    s.m_display = !cg.readEntry( "NoDisplay" , false );
                    result.append( s );
                }
            }
        }
        else
            bInvalidMenu = true;

        if ( bInvalidMenu )
        {
            QString tmp = i18n("The desktop entry file\n%1\n has an invalid menu entry\n%2.",  path ,  *it );
            KMessageBoxWrapper::error( 0, tmp );
        }
    }

    return result;
}

void KDesktopFileActions::executeService( const KUrl::List& urls, const KDesktopFileActions::Service& _service )
{
    //kDebug(7009) << "EXECUTING Service " << _service.m_strName << endl;

    if ( _service.m_type == ST_USER_DEFINED ) {
        kDebug() << "KDesktopFileActions::executeService " << _service.m_strName
                 << " first url's path=" << urls.first().path() << " exec=" << _service.m_strExec << endl;
        KRun::run( _service.m_strExec, urls, _service.m_strName, _service.m_strIcon );
        // The action may update the desktop file. Example: eject unmounts (#5129).
        org::kde::KDirNotify::emitFilesChanged( urls.toStringList() );
        return;
    } else if ( _service.m_type == ST_MOUNT || _service.m_type == ST_UNMOUNT ) {
        Q_ASSERT( urls.count() == 1 );
        QString path = urls.first().path();
        //kDebug(7009) << "MOUNT&UNMOUNT" << endl;

        KDesktopFile cfg( path );
        const KConfigGroup group = cfg.desktopGroup();
        QString dev = group.readEntry( "Dev" );
        if ( dev.isEmpty() ) {
            QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  path );
            KMessageBoxWrapper::error( 0, tmp );
            return;
        }
        KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( dev );

        if ( _service.m_type == ST_MOUNT ) {
            // Already mounted? Strange, but who knows ...
            if ( mp ) {
                kDebug(7009) << "ALREADY Mounted" << endl;
                return;
            }

            bool ro = group.readEntry("ReadOnly", false);
            QString fstype = group.readEntry( "FSType" );
            if ( fstype == "Default" ) // KDE-1 thing
                fstype.clear();
            QString point = group.readEntry( "MountPoint" );
#ifndef Q_WS_WIN
            (void)new KAutoMount( ro, fstype.toLatin1(), dev, point, path, false );
#endif
        } else if ( _service.m_type == ST_UNMOUNT ) {
            // Not mounted? Strange, but who knows ...
            if ( !mp )
                return;

#ifndef Q_WS_WIN
            (void)new KAutoUnmount( mp->mountPoint(), path );
#endif
        }
    }
    else
        Q_ASSERT( 0 );
}
