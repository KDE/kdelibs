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
#include <kconfiggroup.h>
#include <klocale.h>
#include "kservice.h"

enum BuiltinServiceType { ST_MOUNT = 0x0E1B05B0, ST_UNMOUNT = 0x0E1B05B1 }; // random numbers

static bool runFSDevice( const KUrl& _url, const KDesktopFile &cfg );
static bool runApplication( const KUrl& _url, const QString & _serviceFile );
static bool runLink( const KUrl& _url, const KDesktopFile &cfg );

bool KDesktopFileActions::run( const KUrl& u, bool _is_local )
{
    // It might be a security problem to run external untrusted desktop
    // entry files
    if ( !_is_local )
        return false;

    KDesktopFile cfg( u.path() );
    if ( !cfg.desktopGroup().hasKey("Type") )
    {
        QString tmp = i18n("The desktop entry file %1 "
                           "has no Type=... entry.", u.path() );
        KMessageBoxWrapper::error( 0, tmp);
        return false;
    }

    //kDebug(7000) << "TYPE = " << type.data();

    if ( cfg.hasDeviceType() )
        return runFSDevice( u, cfg );
    else if ( cfg.hasApplicationType()
              || (cfg.readType() == "Service" && !cfg.desktopGroup().readEntry("Exec").isEmpty())) // for kio_settings
        return runApplication( u, u.path() );
    else if ( cfg.hasLinkType() )
        return runLink( u, cfg );

    QString tmp = i18n("The desktop entry of type\n%1\nis unknown.",  cfg.readType() );
    KMessageBoxWrapper::error( 0, tmp);

    return false;
}

static bool runFSDevice( const KUrl& _url, const KDesktopFile &cfg )
{
    bool retval = false;

    QString dev = cfg.readDevice();

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
        KConfigGroup cg = cfg.desktopGroup();
        bool ro = cg.readEntry("ReadOnly", false);
        QString fstype = cg.readEntry( "FSType" );
        if ( fstype == "Default" ) // KDE-1 thing
            fstype.clear();
        QString point = cg.readEntry( "MountPoint" );
#ifndef Q_WS_WIN
        (void) new KAutoMount( ro, fstype.toLatin1(), dev, point, _url.path() );
#endif
        retval = false;
    }

    return retval;
}

static bool runApplication( const KUrl& , const QString & _serviceFile )
{
    KService s( _serviceFile );
    if ( !s.isValid() )
        // The error message was already displayed, so we can just quit here
        // ### KDE4: is this still the case?
        return false;

    KUrl::List lst;
    return KRun::run( s, lst, 0 /*TODO - window*/ );
}

static bool runLink( const KUrl& _url, const KDesktopFile &cfg )
{
    QString u = cfg.readUrl();
    if ( u.isEmpty() )
    {
        QString tmp = i18n("The desktop entry file\n%1\nis of type Link but has no URL=... entry.",  _url.prettyUrl() );
        KMessageBoxWrapper::error( 0, tmp );
        return false;
    }

    KUrl url ( u );
    KRun* run = new KRun(url,(QWidget*)0);

    // X-KDE-LastOpenedWith holds the service desktop entry name that
    // was should be preferred for opening this URL if possible.
    // This is used by the Recent Documents menu for instance.
    QString lastOpenedWidth = cfg.desktopGroup().readEntry( "X-KDE-LastOpenedWith" );
    if ( !lastOpenedWidth.isEmpty() )
        run->setPreferredService( lastOpenedWidth );

    return false;
}

QList<KServiceAction> KDesktopFileActions::builtinServices( const KUrl& _url )
{
    QList<KServiceAction> result;

    if ( !_url.isLocalFile() )
        return result;

    KDesktopFile cfg( _url.path() );
    QString type = cfg.readType();

    if ( type.isEmpty() )
        return result;

    if ( cfg.hasDeviceType() ) {
        const QString dev = cfg.readDevice();
        if ( dev.isEmpty() ) {
            QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  _url.path() );
            KMessageBoxWrapper::error(0, tmp);
        } else {
            KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( dev );
            // not mounted ?
            if ( !mp ) {
                KServiceAction mount("mount", i18n("Mount"), QString(), QString(), false);
                mount.setData(QVariant(ST_MOUNT));
                result.append(mount);
            } else {
                QString text;
#ifdef HAVE_VOLMGT
                /*
                 *  Solaris' volume management can only umount+eject
                 */
                text = i18n("Eject");
#else
                text = i18n("Unmount");
#endif
                KServiceAction unmount("unmount", text, QString(), QString(), false);
                unmount.setData(QVariant(ST_UNMOUNT));
                result.append(unmount);
            }
        }
    }

    return result;
}

QList<KServiceAction> KDesktopFileActions::userDefinedServices( const QString& path, bool bLocalFiles )
{
    KDesktopFile cfg( path );
    return userDefinedServices( path, cfg, bLocalFiles );
}

QList<KServiceAction> KDesktopFileActions::userDefinedServices( const QString& path, const KDesktopFile& cfg, bool bLocalFiles, const KUrl::List & file_list )
{
    Q_UNUSED(path); // this was just for debugging; we use service.entryPath() now.
    KService service(&cfg);
    return userDefinedServices(service, bLocalFiles, file_list);
}

QList<KServiceAction> KDesktopFileActions::userDefinedServices( const KService& service, bool bLocalFiles, const KUrl::List & file_list )
{
    QList<KServiceAction> result;

    if (!service.isValid()) // e.g. TryExec failed
        return result;

    QStringList keys;
    const QString actionMenu = service.property("X-KDE-GetActionMenu", QVariant::String).toString();
    if (!actionMenu.isEmpty()) {
        const QStringList dbuscall = actionMenu.split(QChar(' '));
        if (dbuscall.count() >= 4) {
            const QString& app       = dbuscall.at( 0 );
            const QString& object    = dbuscall.at( 1 );
            const QString& interface = dbuscall.at( 2 );
            const QString& function  = dbuscall.at( 3 );

            QDBusInterface remote( app, object, interface );
            // Do NOT use QDBus::BlockWithGui here. It runs a nested event loop,
            // in which timers can fire, leading to crashes like #149736.
            QDBusReply<QStringList> reply = remote.call(function, file_list.toStringList());
            keys = reply;               // ensures that the reply was a QStringList
            if (keys.isEmpty())
                return result;
        } else {
            kWarning(7000) << "The desktop file" << service.entryPath()
                           << "has an invalid X-KDE-GetActionMenu entry."
                           << "Syntax is: app object interface function";
        }
    }

    // Now, either keys is empty (all actions) or it's set to the actions we want

    foreach(const KServiceAction& action, service.actions()) {
        if (keys.isEmpty() || keys.contains(action.name())) {
            const QString exec = action.exec();
            if (bLocalFiles || exec.contains("%U") || exec.contains("%u")) {
                result.append( action );
            }
        }
    }

    return result;
}

void KDesktopFileActions::executeService( const KUrl::List& urls, const KServiceAction& action )
{
    //kDebug(7000) << "EXECUTING Service " << action.name();

    int actionData = action.data().toInt();
    if ( actionData == ST_MOUNT || actionData == ST_UNMOUNT ) {
        Q_ASSERT( urls.count() == 1 );
        const QString path = urls.first().path();
        //kDebug(7000) << "MOUNT&UNMOUNT";

        KDesktopFile cfg( path );
        const QString dev = cfg.readDevice();
        if ( dev.isEmpty() ) {
            QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  path );
            KMessageBoxWrapper::error( 0, tmp );
            return;
        }
        KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( dev );

        if ( actionData == ST_MOUNT ) {
            // Already mounted? Strange, but who knows ...
            if ( mp ) {
                kDebug(7000) << "ALREADY Mounted";
                return;
            }

            const KConfigGroup group = cfg.desktopGroup();
            bool ro = group.readEntry("ReadOnly", false);
            QString fstype = group.readEntry( "FSType" );
            if ( fstype == "Default" ) // KDE-1 thing
                fstype.clear();
            QString point = group.readEntry( "MountPoint" );
#ifndef Q_WS_WIN
            (void)new KAutoMount( ro, fstype.toLatin1(), dev, point, path, false );
#endif
        } else if ( actionData == ST_UNMOUNT ) {
            // Not mounted? Strange, but who knows ...
            if ( !mp )
                return;

#ifndef Q_WS_WIN
            (void)new KAutoUnmount( mp->mountPoint(), path );
#endif
        }
    } else {
        kDebug() << action.name() << "first url's path=" << urls.first().path() << "exec=" << action.exec();
        KRun::run( action.exec(), urls, 0, action.text(), action.icon(), "0" /*no startup notification for actions*/ );
        // The action may update the desktop file. Example: eject unmounts (#5129).
        org::kde::KDirNotify::emitFilesChanged( urls.toStringList() );
    }
}

