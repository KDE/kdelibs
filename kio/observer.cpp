/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

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

#include <kdebug.h>
#include <kapp.h>
#include <dcopclient.h>
#include <kurl.h>

#include "observer.h"

#include "uiserver_stub.h"

using namespace KIO;

Observer * Observer::s_pObserver = 0L;

Observer::Observer()
{
    if ( !kapp->dcopClient()->isApplicationRegistered( "kio_uiserver" ) )
    {
        QCString dcopService;
        QString error;
        if ( KApplication::startServiceByDesktopPath( "kio_uiserver.desktop",
               "", dcopService, error ) > 0 )
        {
            kdError() << "Couldn't start kio_uiserver from kio_uiserver.desktop: " << error << endl;
        }
        // What to do with dcopServer ? Isn't it 'kio_uiserver' ? Let's see.
        kdDebug() << "dcopService : " << dcopService << endl;
    }
    m_uiserver = new UIServer_stub( "kio_uiserver", "UIServer" );
}

int Observer::newJob( KIO::Job* )
{
    return m_uiserver->newJob();
}

void Observer::slotTotalSize( KIO::Job* job, unsigned long size )
{
  kdDebug() << "** Observer::slotTotalSize " << job << " " << size << endl;
}

void Observer::slotTotalFiles( KIO::Job* job, unsigned long files )
{
  kdDebug() << "** Observer::slotTotalFiles " << job << " " << files << endl;
}

void Observer::slotTotalDirs( KIO::Job* job, unsigned long dirs )
{
  kdDebug() << "** Observer::slotTotalDirs " << job << " " << dirs << endl;
}

void Observer::slotProcessedSize( KIO::Job* job, unsigned long size )
{
  kdDebug() << "** Observer::slotProcessedSize " << job << " " << size << endl;
}

void Observer::slotProcessedFiles( KIO::Job* job, unsigned long files )
{
  kdDebug() << "** Observer::slotProcessedFiles " << job << " " << files << endl;
}

void Observer::slotProcessedDirs( KIO::Job* job, unsigned long dirs )
{
  kdDebug() << "** Observer::slotProcessedDirs " << job << " " << dirs << endl;
}

void Observer::slotSpeed( KIO::Job* job, unsigned long bytes_per_second )
{
  kdDebug() << "** Observer::slotSpeed " << job << " " << bytes_per_second << endl;
}

void Observer::slotPercent( KIO::Job* job, unsigned long percent )
{
  kdDebug() << "** Observer::slotPercent " << job << " " << percent << endl;
}

void Observer::slotCopying( KIO::Job* job, const KURL& from, const KURL& to )
{
  kdDebug() << "** Observer::slotCopying " << job << " " << from.path() << " " << to.path() << endl;
}

void Observer::slotMoving( KIO::Job* job, const KURL& from, const KURL& to )
{
  kdDebug() << "** Observer::slotMoving " << job << " " << from.path() << " " << to.path() << endl;
}

void Observer::slotDeleting( KIO::Job* job, const KURL& from )
{
  kdDebug() << "** Observer::slotDeleting " << job << " " << from.path() << endl;
}

void Observer::slotCreatingDir( KIO::Job* job, const KURL& dir )
{
  kdDebug() << "** Observer::slotCreatingDir " << job << " " << dir.path() << endl;
}

void Observer::slotCanResume( KIO::Job* job, bool can_resume )
{
  kdDebug() << "** Observer::slotCanResume " << job << " " << can_resume << endl;
}





#include "observer.moc"
