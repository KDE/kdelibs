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

QString * NetAccess::lastErrorMsg = 0L;

bool NetAccess::download(const KURL& u, QString & target)
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
  return kioNet.copyInternal( u, dest, true /*overwrite*/);
}

bool NetAccess::upload(const QString& src, const KURL& target)
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
  return kioNet.copyInternal( s, target, true /*overwrite*/ );
}

bool NetAccess::copy( const KURL & src, const KURL & target )
{
  NetAccess kioNet;
  return kioNet.copyInternal( src, target, false /*not overwrite*/ );
}

bool NetAccess::dircopy( const KURL & src, const KURL & target )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( src, target );
}

bool NetAccess::exists( const KURL & url )
{
  if ( url.isLocalFile() )
    return QFile::exists( url.path() );
  NetAccess kioNet;
  return kioNet.statInternal( url );
}

bool NetAccess::stat( const KURL & url, KIO::UDSEntry & entry )
{
  NetAccess kioNet;
  bool ret = kioNet.statInternal( url );
  if (ret)
    entry = kioNet.m_entry;
  return ret;
}

bool NetAccess::del( const KURL & url )
{
  NetAccess kioNet;
  return kioNet.delInternal( url );
}

bool NetAccess::mkdir( const KURL & url, int permissions )
{
  NetAccess kioNet;
  return kioNet.mkdirInternal( url, permissions );
}

QStringList* NetAccess::tmpfiles = 0L;

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

bool NetAccess::copyInternal(const KURL& src, const KURL& target, bool overwrite)
{
  bJobOK = true; // success unless further error occurs

  KIO::Scheduler::checkSlaveOnHold(true);
  KIO::Job * job = KIO::file_copy( src, target, -1, overwrite );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::dircopyInternal(const KURL& src, const KURL& target)
{
  bJobOK = true; // success unless further error occurs

  KIO::Job * job = KIO::copy( src, target );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::statInternal( const KURL & url )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::stat( url, !url.isLocalFile() );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::delInternal( const KURL & url )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::del( url );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::mkdirInternal( const KURL & url, int permissions )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::mkdir( url, permissions );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

QString NetAccess::mimetypeInternal( const KURL & url )
{
  bJobOK = true; // success unless further error occurs
  m_mimetype = QString::fromLatin1("unknown");
  KIO::Job * job = KIO::mimetype( url );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  connect( job, SIGNAL( mimetype (KIO::Job *, const QString &type) ),
           this, SLOT( slotMimetype (KIO::Job *, const QString &type) ) );
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
