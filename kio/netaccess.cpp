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
 
#include <kurl.h> 
#include <kio_job.h> 

#include "kio_netaccess.h"

bool KIONetAccess::download(const QString src, QString & target, KIOJob *job)
{
  KURL u (src);
  if (u.isLocalFile()){
    // file protocol. We do not need the network
    target = u.path();
    return true;
  }

  KIONetAccess kioNet;
  bool result = kioNet.downloadInternal( src, target, job );
  return result;
}

bool KIONetAccess::upload(const QString src, QString target, KIOJob *job)
{
  if (target.isEmpty())
    return false;

  // If !target.isEmpty(), then downloadInternal just
  // copies src to target.  Great!
  KIONetAccess kioNet;
  bool result = kioNet.downloadInternal( src, target, job );
  return result;
}

QStringList* KIONetAccess::tmpfiles = 0L;
 
void KIONetAccess::removeTempFile(const QString name)
{
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name))
  {
    unlink(name.ascii());
    tmpfiles->remove(name);
  }
}
 
bool KIONetAccess::downloadInternal(const QString src, QString & target,
                                    KIOJob *job)
{
  if (target.isEmpty())
  {
    target = tmpnam(0);
    if (!tmpfiles)
      tmpfiles = new QStringList;
    tmpfiles->append(target);
  }
  bDownloadOk = true; // success unless further error occurs
 
  if (!job) job = new KIOJob;
  connect( job, SIGNAL( sigFinished (int) ), this, SLOT( slotFinished (int) ) );
  connect( job, SIGNAL( sigError( int, int, const char * ) ), 
           this, SLOT( slotError ( int, int, const char * ) ) );
  job->copy(src.ascii(), target.ascii());

  qApp->enter_loop();
  return bDownloadOk;
}

void KIONetAccess::slotError( int , int , const char * )
{
  bDownloadOk = false;
  qApp->exit_loop();
} 

void KIONetAccess::slotFinished( int )
{
    qApp->exit_loop();
}

#include "kio_netaccess.moc"
