/*  $Id$

    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <qstring.h>
#include <qapplication.h>
#include <qfile.h>

#include <kapplication.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/scheduler.h>

#include "kio/netaccess.h"

using namespace KIO;

QString * NetAccess::lastErrorMsg;
QStringList* NetAccess::tmpfiles;

bool NetAccess::download(const KURL& u, QString & target)
{
  return NetAccess::download (u, target, 0);
}

bool NetAccess::download(const KURL& u, QString & target, QWidget* window)
{
  if (u.isLocalFile()) {
    // file protocol. We do not need the network
    target = u.path();
    bool accessible = checkAccess(target, R_OK);
    if(!accessible)
    {
        if(!lastErrorMsg)
            lastErrorMsg = new QString;
        *lastErrorMsg = i18n("File '%1' is not readable").arg(target);
    }
    return accessible;
  }

  if (target.isEmpty())
  {
      KTempFile tmpFile;
      target = tmpFile.name();
      if (!tmpfiles)
          tmpfiles = new QStringList;
      tmpfiles->append(target);
  }

  NetAccess kioNet;
  KURL dest;
  dest.setPath( target );
  return kioNet.copyInternal( u, dest, true /*overwrite*/, window);
}

bool NetAccess::upload(const QString& src, const KURL& target)
{
  return NetAccess::upload(src, target, 0);
}

bool NetAccess::upload(const QString& src, const KURL& target, QWidget* window)
{
  if (target.isEmpty())
    return false;

  // If target is local... well, just copy. This can be useful
  // when the client code uses a temp file no matter what.
  // Let's make sure it's not the exact same file though
  if (target.isLocalFile() && target.path() == src)
    return true;

  NetAccess kioNet;
  KURL s;
  s.setPath(src);
  return kioNet.copyInternal( s, target, true /*overwrite*/, window );
}

bool NetAccess::copy( const KURL & src, const KURL & target )
{
  return NetAccess::copy( src, target, 0 );
}

bool NetAccess::copy( const KURL & src, const KURL & target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.copyInternal( src, target, false /*not overwrite*/, window );
}

bool NetAccess::dircopy( const KURL & src, const KURL & target )
{
  return NetAccess::dircopy( src, target, 0 );
}

bool NetAccess::dircopy( const KURL & src, const KURL & target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( src, target, window );
}

bool NetAccess::exists( const KURL & url )
{
  return NetAccess::exists( url, true, 0 );
}

bool NetAccess::exists( const KURL & url, QWidget* window )
{
// Is there any reason why this code is duplicated ?
#if 0
  if ( url.isLocalFile() )
    return QFile::exists( url.path() );
  NetAccess kioNet;
  return kioNet.statInternal( url, 0 /*no details*/, true /*source assumed*/, window );
#endif  
  
  return NetAccess::exists( url, true, window );
}

bool NetAccess::exists( const KURL & url, bool source )
{
  return NetAccess::exists( url, source, 0 );
}

bool NetAccess::exists( const KURL & url, bool source, QWidget* window )
{
  if ( url.isLocalFile() )
    return QFile::exists( url.path() );
  NetAccess kioNet;
  return kioNet.statInternal( url, 0 /*no details*/, source, window );
}

bool NetAccess::stat( const KURL & url, KIO::UDSEntry & entry )
{
  return NetAccess::stat( url, entry, 0 );
}

bool NetAccess::stat( const KURL & url, KIO::UDSEntry & entry, QWidget* window )
{
  NetAccess kioNet;
  bool ret = kioNet.statInternal( url, 2 /*all details*/, true /*source*/, window );
  if (ret)
    entry = kioNet.m_entry;
  return ret;
}

bool NetAccess::del( const KURL & url )
{
  return NetAccess::del( url, 0 );
}

bool NetAccess::del( const KURL & url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.delInternal( url, window );
}

bool NetAccess::mkdir( const KURL & url, int permissions )
{
  return NetAccess::mkdir( url, 0, permissions );
}

bool NetAccess::mkdir( const KURL & url, QWidget* window, int permissions )
{
  NetAccess kioNet;
  return kioNet.mkdirInternal( url, permissions, window );
}

QString NetAccess::mimetype( const KURL& url )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, 0 );
}

QString NetAccess::mimetype( const KURL& url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, window );
}

void NetAccess::removeTempFile(const QString& name)
{
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name))
  {
    unlink(QFile::encodeName(name));
    tmpfiles->remove(name);
  }
}

bool NetAccess::copyInternal(const KURL& src, const KURL& target, bool overwrite,
                             QWidget* window)
{
  bJobOK = true; // success unless further error occurs

  KIO::Scheduler::checkSlaveOnHold(true);
  KIO::Job * job = KIO::file_copy( src, target, -1, overwrite );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::dircopyInternal(const KURL& src, const KURL& target, 
                                QWidget* window)
{
  bJobOK = true; // success unless further error occurs

  KIO::Job * job = KIO::copy( src, target );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::statInternal( const KURL & url, int details, bool source,
                              QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::StatJob * job = KIO::stat( url, !url.isLocalFile() );
  job->setWindow (window);
  job->setDetails( details );
  job->setSide( source );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::delInternal( const KURL & url, QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::del( url );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::mkdirInternal( const KURL & url, int permissions, 
                               QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::mkdir( url, permissions );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

QString NetAccess::mimetypeInternal( const KURL & url, QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  m_mimetype = QString::fromLatin1("unknown");
  KIO::Job * job = KIO::mimetype( url );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  connect( job, SIGNAL( mimetype (KIO::Job *, const QString &) ),
           this, SLOT( slotMimetype (KIO::Job *, const QString &) ) );
  enter_loop();
  return m_mimetype;
}

void NetAccess::slotMimetype( KIO::Job *, const QString & type  )
{
  m_mimetype = type;
}

// If a troll sees this, he kills me
void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void NetAccess::enter_loop()
{
  QWidget dummy(0,0,WType_Dialog | WShowModal);
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal(&dummy);
  qApp->enter_loop();
  qt_leave_modal(&dummy);
}

void NetAccess::slotResult( KIO::Job * job )
{
  bJobOK = !job->error();
  if ( !bJobOK )
  {
    if ( !lastErrorMsg )
      lastErrorMsg = new QString;
    *lastErrorMsg = job->errorString();
  }
  if ( job->isA("KIO::StatJob") )
    m_entry = static_cast<KIO::StatJob *>(job)->statResult();
  qApp->exit_loop();
}

#include "netaccess.moc"
