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

#include "kdedesktopmimetype.h"
#include "krun.h"
#include "kautomount.h"
#include <kmessageboxwrapper.h>
#include <kdirnotify_stub.h>
#include <kio/global.h>

#include <kiconloader.h>
#include <kprocess.h>
#include <ktoolinvocation.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kapplication.h>
#include <dcopclient.h>

QString KDEDesktopMimeType::icon( const KUrl& _url ) const
{
  if ( _url.isEmpty() || !_url.isLocalFile() )
    return KMimeType::icon( _url );

  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  QString icon = cfg.readEntry( "Icon" );
  QString type = cfg.readEntry( "Type" );

  if ( type == "FSDevice" || type == "FSDev") // need to provide FSDev for
                                              // backwards compatibility
  {
    QString unmount_icon = cfg.readEntry( "UnmountIcon" );
    QString dev = cfg.readEntry( "Dev" );
    if ( !icon.isEmpty() && !unmount_icon.isEmpty() && !dev.isEmpty() )
    {
      QString mp = KIO::findDeviceMountPoint( dev );
      // Is the device not mounted ?
      if ( mp.isNull() )
        return unmount_icon;
    }
  } else if ( type == "Link" ) {
      const QString emptyIcon = cfg.readEntry( "EmptyIcon" );
      if ( !emptyIcon.isEmpty() ) {
          const QString u = cfg.readPathEntry( "URL" );
          const KUrl url( u );
          if ( url.protocol() == "trash" ) {
              // We need to find if the trash is empty, preferrably without using a KIO job.
              // So instead kio_trash leaves an entry in its config file for us.
              KSimpleConfig trashConfig( "trashrc", true );
              trashConfig.setGroup( "Status" );
              if ( trashConfig.readEntry( "Empty", true ) ) {
                  return emptyIcon;
              }
          }
      }
  }

  if ( icon.isEmpty() )
    return KMimeType::icon( _url );

  return icon;
}

QPixmap KDEDesktopMimeType::pixmap( const KUrl& _url, K3Icon::Group _group, int _force_size,
                                    int _state, QString * _path ) const
{
  QString _icon = icon( _url );
  QPixmap pix = KGlobal::iconLoader()->loadIcon( _icon, _group,
        _force_size, _state, _path, false );
  if ( pix.isNull() )
      pix = KGlobal::iconLoader()->loadIcon( "unknown", _group,
        _force_size, _state, _path, false );
  return pix;
}

/*QString KDEDesktopMimeType::comment( const QString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  KUrl u( _url );
  return comment( u, _is_local );
}*/

QString KDEDesktopMimeType::comment( const KUrl& _url ) const
{
  if ( _url.isEmpty() || !_url.isLocalFile() )
    return KMimeType::comment( _url );

  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  QString comment = cfg.readEntry( "Comment" );
  if ( comment.isEmpty() )
    return KMimeType::comment( _url );

  return comment;
}

pid_t KDEDesktopMimeType::run( const KUrl& u, bool _is_local )
{
  // It might be a security problem to run external untrusted desktop
  // entry files
  if ( !_is_local )
    return 0;

  KSimpleConfig cfg( u.path(), true );
  cfg.setDesktopGroup();
  QString type = cfg.readEntry( "Type" );
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
  else if ( type == "MimeType" )
    return runMimeType( u, cfg );


  QString tmp = i18n("The desktop entry of type\n%1\nis unknown.",  type );
  KMessageBoxWrapper::error( 0, tmp);

  return 0;
}

pid_t KDEDesktopMimeType::runFSDevice( const KUrl& _url, const KSimpleConfig &cfg )
{
  pid_t retval = 0;

  QString dev = cfg.readEntry( "Dev" );

  if ( dev.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  _url.path() );
    KMessageBoxWrapper::error( 0, tmp);
    return retval;
  }

  QString mp = KIO::findDeviceMountPoint( dev );
  // Is the device already mounted ?
  if ( !mp.isNull() )
  {
    KUrl mpURL;
    mpURL.setPath( mp );
    // Open a new window
    retval = KRun::runURL( mpURL, QLatin1String("inode/directory"), 0 /*TODO - window*/ );
  }
  else
  {
    bool ro = cfg.readEntry("ReadOnly", false);
    QString fstype = cfg.readEntry( "FSType" );
    if ( fstype == "Default" ) // KDE-1 thing
      fstype.clear();
    QString point = cfg.readEntry( "MountPoint" );
#ifndef Q_WS_WIN
    (void) new KAutoMount( ro, fstype.toLatin1(), dev, point, _url.path() );
#endif
    retval = -1; // we don't want to return 0, but we don't want to return a pid
  }

  return retval;
}

pid_t KDEDesktopMimeType::runApplication( const KUrl& , const QString & _serviceFile )
{
  KService s( _serviceFile );
  if ( !s.isValid() )
    // The error message was already displayed, so we can just quit here
    return 0;

  KUrl::List lst;
  return KRun::run( s, lst, 0 /*TODO - window*/ );
}

pid_t KDEDesktopMimeType::runLink( const KUrl& _url, const KSimpleConfig &cfg )
{
  QString u = cfg.readPathEntry( "URL" );
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
  QString lastOpenedWidth = cfg.readEntry( "X-KDE-LastOpenedWith" );
  if ( !lastOpenedWidth.isEmpty() )
      run->setPreferredService( lastOpenedWidth );

  return -1; // we don't want to return 0, but we don't want to return a pid
}

pid_t KDEDesktopMimeType::runMimeType( const KUrl& url , const KSimpleConfig & )
{
  // Hmm, can't really use keditfiletype since we might be looking
  // at the global file, or at a file not in share/mimelnk...

  QStringList args;
  args << "openProperties";
  args << url.path();

  int pid;
  if ( !KToolInvocation::kdeinitExec("kfmclient", args, 0, &pid) )
      return pid;

  KProcess p;
  p << "kfmclient" << args;
  p.start(KProcess::DontCare);
  return p.pid();
}

QList<KDEDesktopMimeType::Service> KDEDesktopMimeType::builtinServices( const KUrl& _url )
{
  QList<Service> result;

  if ( !_url.isLocalFile() )
    return result;

  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  QString type = cfg.readEntry( "Type" );

  if ( type.isEmpty() )
    return result;

  if ( type == "FSDevice" )
  {
    QString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  _url.path() );
      KMessageBoxWrapper::error( 0, tmp);
    }
    else
    {
      QString mp = KIO::findDeviceMountPoint( dev );
      // not mounted ?
      if ( mp.isEmpty() )
      {
        Service mount;
        mount.m_strName = i18n("Mount");
        mount.m_type = ST_MOUNT;
        result.append( mount );
      }
      else
      {
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

QList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const QString& path, bool bLocalFiles )
{
  KSimpleConfig cfg( path, true );
  return userDefinedServices( path, cfg, bLocalFiles );
}

QList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const QString& path, KConfig& cfg, bool bLocalFiles )
{
 return userDefinedServices( path, cfg, bLocalFiles, KUrl::List() );
}

QList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const QString& path, KConfig& cfg, bool bLocalFiles, const KUrl::List & file_list )
{
  QList<Service> result;

  cfg.setDesktopGroup();

  if ( !cfg.hasKey( "Actions" ) && !cfg.hasKey( "X-KDE-GetActionMenu") )
    return result;

  if ( cfg.hasKey( "TryExec" ) )
  {
      QString tryexec = cfg.readPathEntry( "TryExec" );
      QString exe =  KStandardDirs::findExe( tryexec );
      if (exe.isEmpty()) {
          return result;
      }
  }

  QStringList keys;

  if( cfg.hasKey( "X-KDE-GetActionMenu" )) {
    QString dcopcall = cfg.readEntry( "X-KDE-GetActionMenu" );
    const DCOPCString app = dcopcall.section(' ', 0,0).toUtf8();

    QByteArray dataToSend;
    QDataStream dataStream(&dataToSend, QIODevice::WriteOnly);
    dataStream.setVersion(QDataStream::Qt_3_1);
    dataStream << file_list;
    DCOPCString replyType;
    QByteArray replyData;
    DCOPCString object   =  dcopcall.section(' ', 1,-2).toUtf8();
    DCOPCString function =  dcopcall.section(' ', -1).toUtf8();
    if(!function.endsWith("(KUrl::List)")) {
      kWarning() << "Desktop file " << path << " contains an invalid X-KDE-ShowIfDcopCall - the function must take the exact parameter (KUrl::List) and must be specified." << endl;
    } else {
      if(KApplication::dcopClient()->call( app, object,
                   function,
                   dataToSend, replyType, replyData, true, 100)
	    && replyType == "QStringList" ) {

        QDataStream dataStreamIn(replyData);
        dataStreamIn.setVersion(QDataStream::Qt_3_1);
        dataStreamIn >> keys;
      }
    }
  }

  keys += cfg.readEntry( "Actions", QStringList(), ';' ); //the desktop standard defines ";" as separator!

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
      cfg.setGroup( group );

      if ( !cfg.hasKey( "Name" ) || !cfg.hasKey( "Exec" ) )
        bInvalidMenu = true;
      else
      {
        QString exec = cfg.readPathEntry( "Exec" );
        if ( bLocalFiles || exec.contains("%U") || exec.contains("%u") )
        {
          Service s;
          s.m_strName = cfg.readEntry( "Name" );
          s.m_strIcon = cfg.readEntry( "Icon" );
          s.m_strExec = exec;
          s.m_type = ST_USER_DEFINED;
          s.m_display = !cfg.readEntry( "NoDisplay" , false );
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

void KDEDesktopMimeType::executeService( const KUrl::List& urls, KDEDesktopMimeType::Service& _service )
{
  //kDebug(7009) << "EXECUTING Service " << _service.m_strName << endl;

  if ( _service.m_type == ST_USER_DEFINED )
  {
    kDebug() << "KDEDesktopMimeType::executeService " << _service.m_strName
              << " first url's path=" << urls.first().path() << " exec=" << _service.m_strExec << endl;
    KRun::run( _service.m_strExec, urls, _service.m_strName, _service.m_strIcon );
    // The action may update the desktop file. Example: eject unmounts (#5129).
    KDirNotify_stub allDirNotify("*", "KDirNotify*");
    allDirNotify.FilesChanged( urls );
    return;
  }
  else if ( _service.m_type == ST_MOUNT || _service.m_type == ST_UNMOUNT )
  {
    Q_ASSERT( urls.count() == 1 );
    QString path = urls.first().path();
    //kDebug(7009) << "MOUNT&UNMOUNT" << endl;

    KSimpleConfig cfg( path, true );
    cfg.setDesktopGroup();
    QString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry.",  path );
      KMessageBoxWrapper::error( 0, tmp );
      return;
    }
    QString mp = KIO::findDeviceMountPoint( dev );

    if ( _service.m_type == ST_MOUNT )
    {
      // Already mounted? Strange, but who knows ...
      if ( !mp.isEmpty() )
      {
        kDebug(7009) << "ALREADY Mounted" << endl;
        return;
      }

      bool ro = cfg.readEntry("ReadOnly", false);
      QString fstype = cfg.readEntry( "FSType" );
      if ( fstype == "Default" ) // KDE-1 thing
          fstype.clear();
      QString point = cfg.readEntry( "MountPoint" );
#ifndef Q_WS_WIN
      (void)new KAutoMount( ro, fstype.toLatin1(), dev, point, path, false );
#endif
    }
    else if ( _service.m_type == ST_UNMOUNT )
    {
      // Not mounted? Strange, but who knows ...
      if ( mp.isEmpty() )
        return;

#ifndef Q_WS_WIN
      (void)new KAutoUnmount( mp, path );
#endif
    }
  }
  else
    Q_ASSERT( 0 );
}

void KDEDesktopMimeType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }
