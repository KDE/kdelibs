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

#include "kfileshare.h"
#include "kfileshare_p.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/Q_PID>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kuser.h>

static KFileShare::Authorization s_authorization = KFileShare::NotInitialized;
K_GLOBAL_STATIC(QStringList, s_shareList)
static KFileShare::ShareMode s_shareMode;
static bool s_sambaEnabled;
static bool s_nfsEnabled;
static bool s_restricted;
static QString s_fileShareGroup;
static bool s_sharingEnabled;

#define FILESHARECONF "/etc/security/fileshare.conf"

static QString findExe( const char* exeName )
{
   // Normally fileshareset and filesharelist are installed in kde4/libexec;
   // allow distributions to move it somewhere else in the PATH or in /usr/sbin.
   QString path = QString::fromLocal8Bit(qgetenv("PATH"));
#ifndef Q_WS_WIN
   path += QLatin1String(":/usr/sbin");
#endif
   QString exe = KStandardDirs::findExe( exeName, path );
   if (exe.isEmpty())
       kError() << exeName << "not found in" << path;
   return exe;
}

KFileSharePrivate::KFileSharePrivate()
{
  KDirWatch::self()->addFile(FILESHARECONF);
  connect(KDirWatch::self(), SIGNAL(dirty (const QString&)),this,
          SLOT(slotFileChange(const QString &)));
  connect(KDirWatch::self(), SIGNAL(created(const QString&)),this,
          SLOT(slotFileChange(const QString &)));
  connect(KDirWatch::self(), SIGNAL(deleted(const QString&)),this,
          SLOT(slotFileChange(const QString &)));
}

KFileSharePrivate::~KFileSharePrivate()
{
  KDirWatch::self()->removeFile(FILESHARECONF);
}

KFileSharePrivate* KFileSharePrivate::self()
{
   K_GLOBAL_STATIC(KFileSharePrivate, _self)
   return _self;
}

void KFileSharePrivate::slotFileChange(const QString &file)
{
  if(file==FILESHARECONF) {
     KFileShare::readConfig();
     KFileShare::readShareList();
  }
}

KFileShare::ShareMode readEntry(const KConfigGroup &cg, const char* key,
	const KFileShare::ShareMode& aDefault)
{
    const QByteArray data=cg.readEntry(key, QByteArray());

    if (!data.isEmpty()) {
        if (data.toLower() == "simple")
            return KFileShare::Simple;
        else if (data.toLower() == "advanced")
            return KFileShare::Advanced;
    }

    return aDefault;
}

void KFileShare::readConfig() // static
{
    // Create KFileSharePrivate instance
    KFileSharePrivate::self();
    KConfig config(QLatin1String(FILESHARECONF));
    KConfigGroup group( &config, QString() );

    s_sharingEnabled = group.readEntry("FILESHARING", true);
    s_restricted = group.readEntry("RESTRICT", true);
    s_fileShareGroup = group.readEntry("FILESHAREGROUP", "fileshare");


    if (!s_sharingEnabled)
        s_authorization = UserNotAllowed;
    else
    if (!s_restricted )
        s_authorization = Authorized;
    else {
        // check if current user is in fileshare group
        KUserGroup shareGroup(s_fileShareGroup);
        if (shareGroup.users().contains(KUser()) )
            s_authorization = Authorized;
        else
            s_authorization = UserNotAllowed;
    }

    s_shareMode = readEntry(group, "SHARINGMODE", Simple);


    s_sambaEnabled = group.readEntry("SAMBA", true);
    s_nfsEnabled = group.readEntry("NFS", true);
}

KFileShare::ShareMode KFileShare::shareMode() {
  if ( s_authorization == NotInitialized )
      readConfig();

  return s_shareMode;
}

bool KFileShare::sharingEnabled() {
  if ( s_authorization == NotInitialized )
      readConfig();

  return s_sharingEnabled;
}

bool KFileShare::isRestricted() {
  if ( s_authorization == NotInitialized )
      readConfig();

  return s_restricted;
}

QString KFileShare::fileShareGroup() {
  if ( s_authorization == NotInitialized )
      readConfig();

  return s_fileShareGroup;
}


bool KFileShare::sambaEnabled() {
  if ( s_authorization == NotInitialized )
      readConfig();

  return s_sambaEnabled;
}

bool KFileShare::nfsEnabled() {
  if ( s_authorization == NotInitialized )
      readConfig();

  return s_nfsEnabled;
}


void KFileShare::readShareList()
{
    KFileSharePrivate::self();
    s_shareList->clear();

    QString exe = ::findExe( "filesharelist" );
    if (exe.isEmpty()) {
        s_authorization = ErrorNotFound;
        return;
    }
    QProcess proc;
    proc.start( exe, QStringList() );
    if ( !proc.waitForFinished() ) {
        kError() << "Can't run" << exe;
        s_authorization = ErrorNotFound;
        return;
    }

    // Reading code shamelessly stolen from khostname.cpp ;)
    while (!proc.atEnd()) {
        QString line = proc.readLine().trimmed();
        int length = line.length();
	if ( length > 0 )
	{
            if ( line[length-1] != '/' )
                line += '/';
            s_shareList->append(line);
            kDebug(7000) << "Shared dir:" << line;
        }
    }
}


bool KFileShare::isDirectoryShared( const QString& _path )
{
    if ( ! s_shareList.exists() )
        readShareList();

    QString path( _path );
    if ( path[path.length()-1] != '/' )
        path += '/';
    return s_shareList->contains( path );
}

KFileShare::Authorization KFileShare::authorization()
{
    // The app should do this on startup, but if it doesn't, let's do here.
    if ( s_authorization == NotInitialized )
        readConfig();
    return s_authorization;
}

bool KFileShare::setShared( const QString& path, bool shared )
{
    if (! KFileShare::sharingEnabled() ||
          KFileShare::shareMode() == Advanced)
       return false;

    kDebug(7000) << path << "," << shared;
    QString exe = ::findExe( "fileshareset" );
    if (exe.isEmpty())
        return false;

    QStringList args;
    if ( shared )
        args << "--add";
    else
        args << "--remove";
    args << path ;
    int ec = QProcess::execute( exe, args ); // should be ok, the perl script terminates fast
    kDebug(7000) << "exitCode=" << ec;
    bool ok = !ec;
    switch (ec) {
        case 1:
          // User is not authorized
          break;
        case 3:
          // Called script with --add, but path was already shared before.
          // Result is nevertheless what the client wanted, so
          // this is alright.
          ok = true;
          break;
        case 4:
          // Invalid mount point
          break;
        case 5:
          // Called script with --remove, but path was not shared before.
          // Result is nevertheless what the client wanted, so
          // this is alright.
          ok = true;
          break;
        case 6:
          // There is no export method
          break;
        case 7:
          // file sharing is disabled
          break;
        case 8:
          // advanced sharing is enabled
          break;
        case 255:
          // Abitrary error
          break;
    }

    return ok;
}

//#include "kfileshare.moc"
#include "kfileshare_p.moc"

