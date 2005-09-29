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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfileshare.h"
#include <qdir.h>
#include <qfile.h>
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
#include <ksimpleconfig.h>
#include <kuser.h>

KFileShare::Authorization KFileShare::s_authorization = NotInitialized;
QStringList* KFileShare::s_shareList = 0L;
static KStaticDeleter<QStringList> sdShareList;

KFileShare::ShareMode KFileShare::s_shareMode;
bool KFileShare::s_sambaEnabled;
bool KFileShare::s_nfsEnabled;
bool KFileShare::s_restricted;
QString KFileShare::s_fileShareGroup;
bool KFileShare::s_sharingEnabled;


#define FILESHARECONF "/etc/security/fileshare.conf"

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

KFileSharePrivate *KFileSharePrivate::_self=0L;

static KStaticDeleter<KFileSharePrivate> kstFileShare;

KFileSharePrivate* KFileSharePrivate::self()
{
   if (!_self)
      _self = kstFileShare.setObject(_self, new KFileSharePrivate());
   return _self;
}

void KFileSharePrivate::slotFileChange(const QString &file)
{
  if(file==FILESHARECONF) {
     KFileShare::readConfig();
     KFileShare::readShareList();
  }
}

void KFileShare::readConfig() // static
{    
    // Create KFileSharePrivate instance
    KFileSharePrivate::self();
    KSimpleConfig config(QString::fromLatin1(FILESHARECONF),true);
    
    s_sharingEnabled = config.readEntry("FILESHARING", "yes") == "yes";
    s_restricted = config.readEntry("RESTRICT", "yes") == "yes";
    s_fileShareGroup = config.readEntry("FILESHAREGROUP", "fileshare");
    
    
    if (!s_sharingEnabled) 
        s_authorization = UserNotAllowed;
    else 
    if (!s_restricted )
        s_authorization = Authorized;
    else {
        // check if current user is in fileshare group
        KUserGroup shareGroup(s_fileShareGroup);
        if (shareGroup.users().findIndex(KUser()) > -1 ) 
            s_authorization = Authorized;
        else
            s_authorization = UserNotAllowed;
    }
                
    if (config.readEntry("SHARINGMODE", "simple") == "simple") 
        s_shareMode = Simple;
    else        
        s_shareMode = Advanced;
          
        
    s_sambaEnabled = config.readEntry("SAMBA", "yes") == "yes";
    s_nfsEnabled = config.readEntry("NFS", "yes") == "yes";
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
        length = proc.readln(line, true);
	if ( length > 0 )
	{
            if ( line[length-1] != '/' )
                line += '/';
            s_shareList->append(line);
            kdDebug(7000) << "Shared dir:" << line << endl;
        }
    } while (length > -1);
}


bool KFileShare::isDirectoryShared( const QString& _path )
{
    if ( ! s_shareList )
        readShareList();

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
    if (! KFileShare::sharingEnabled() ||
          KFileShare::shareMode() == Advanced)
       return false;

    kdDebug(7000) << "KFileShare::setShared " << path << "," << shared << endl;
    QString exe = KFileShare::findExe( "fileshareset" );
    if (exe.isEmpty())
        return false;
        
    KProcess proc;
    proc << exe;
    if ( shared )
        proc << "--add";
    else
        proc << "--remove";
    proc << path;
    proc.start( KProcess::Block ); // should be ok, the perl script terminates fast
    bool ok = proc.normalExit() && (proc.exitStatus() == 0);
    kdDebug(7000) << "KFileSharePropsPlugin::setShared normalExit=" 
                  << proc.normalExit() << endl;
    kdDebug(7000) << "KFileSharePropsPlugin::setShared exitStatus=" 
                  << proc.exitStatus() << endl;
    if ( proc.normalExit() ) {
      switch( proc.exitStatus() ) {
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
    } 
    
    return ok;
}

#include "kfileshare.moc"
