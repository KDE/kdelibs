/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <david@mandrakesoft.com>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kfileshare.h"
#include <qdir.h>
#include <kprocess.h>
#include <kprocio.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <kdirnotify_stub.h>

KFileShare::Authorization KFileShare::s_authorization = NotInitialized;
QStringList* KFileShare::s_shareList = 0L;
static KStaticDeleter<QStringList> sdShareList;


KFileSharePrivate::KFileSharePrivate()
{
  m_watchFile=new KDirWatch();
  m_watchFile->addFile("/etc/security/fileshare.conf");
  m_watchFile->startScan();
  connect(m_watchFile, SIGNAL(dirty (const QString&)),this,
	  SLOT(slotFileChange(const QString &)));
}

KFileSharePrivate::~KFileSharePrivate()
{
  delete m_watchFile;
}

KFileSharePrivate *KFileSharePrivate::_self=0L;

static KStaticDeleter<KFileSharePrivate> kstFileShare;

KFileSharePrivate* KFileSharePrivate::self()
{
   if (!_self)
      _self = kstFileShare.setObject(new KFileSharePrivate());
   return _self;
}

void KFileSharePrivate::slotFileChange(const QString &file)
{
  if(file=="/etc/security/fileshare.conf")
    KFileShare::readConfig();
}

void KFileShare::readConfig() // static
{
  KFileSharePrivate::self();
    s_authorization = UserNotAllowed;
    if ( !s_shareList )
        sdShareList.setObject( s_shareList, new QStringList );
    else
        s_shareList->clear();

    // /usr/sbin on Mandrake, $PATH allows flexibility for other distributions
    QString exe = findExe( "filesharelist" );
    if (exe.isEmpty()) {
        s_authorization = ErrorNotFound;
        return;
    }
    KProcIO proc;
    proc << exe;
    if ( !proc.start( KProcess::Block ) ) {
        kdError() << "Can't run " << exe << endl;
        s_authorization = ErrorNotFound;
        return;
    }

    // Reading code shamelessly stolen from khostname.cpp ;)
    QString line;
    int length;
    do {
        length = proc.fgets(line, true);
	if ( length > 0 )
	{
            if ( line[length-1] != '/' )
                line += '/';
            s_shareList->append(line);
            kdDebug() << "Shared dir:" << line << endl;
        }
    } while (length > -1);

    //kdDebug(7000) << "KFileShare: normalExit()=" << proc.normalExit() << " exitStatus()=" << proc.exitStatus() << endl;
    if ( proc.normalExit() )
        switch (proc.exitStatus())
        {
        case 0:
            s_authorization = Authorized;
            kdDebug(7000) << "KFileShare::readConfig: s_authorization = Authorized" << endl;
	    // move while loop here
            return;
        case 1:
            s_authorization = UserNotAllowed;
            kdDebug(7000) << "KFileShare::readConfig: s_authorization = UserNotAllowed" << endl;
            return;
        default:
            break;
        }
    s_authorization = UserNotAllowed;
}

bool KFileShare::isDirectoryShared( const QString& _path )
{
    // The app should do this on startup, but if it doesn't, let's do here.
    if ( s_authorization == NotInitialized )
        readConfig();

    QString path( _path );
    if ( path[path.length()-1] != '/' )
        path += '/';
    return s_shareList && s_shareList->contains( path );
}

KFileShare::Authorization KFileShare::authorization()
{
    // The app should do this on startup, but if it doesn't, let's do here.
    if ( s_authorization == NotInitialized )
        readConfig();
    return s_authorization;
}

QString KFileShare::findExe( const char* exeName )
{
   // /usr/sbin on Mandrake, $PATH allows flexibility for other distributions
   QString path = QString::fromLocal8Bit(getenv("PATH")) + QString::fromLatin1(":/usr/sbin");
   QString exe = KStandardDirs::findExe( exeName, path );
   if (exe.isEmpty())
       kdError() << exeName << " not found in " << path << endl;
   return exe;
}

bool KFileShare::setShared( const QString& path, bool shared )
{
    kdDebug(7000) << "KFileShare::setShared " << path << "," << shared << endl;
    QString exe = KFileShare::findExe( "fileshareset" );
    if (!exe.isEmpty())
    {
        KProcess proc;
        proc << exe;
        if ( shared )
            proc << "--add";
        else
            proc << "--remove";
        proc << path;
        proc.start( KProcess::Block ); // should be ok, the perl script terminates fast
        bool ok = proc.normalExit() && (proc.exitStatus() == 0);
        kdDebug(7000) << "KFileSharePropsPlugin::setShared ok=" << ok << endl;
        if ( proc.normalExit() )
          switch( proc.exitStatus() )
          case 1:
          {
                  // TODO KMessageBox
          }
        return ok;
    }
    return false;
}

#include "kfileshare.moc"
