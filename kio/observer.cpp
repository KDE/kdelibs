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
#include <assert.h>

#include "jobclasses.h"
#include "observer.h"

#include "uiserver_stub.h"

using namespace KIO;

Observer * Observer::s_pObserver = 0L;

Observer::Observer() : DCOPObject("KIO::Observer")
{
    // Register app as able to receive DCOP messages
    if (kapp && !kapp->dcopClient()->isAttached())
    {
        kapp->dcopClient()->attach();
    }

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

int Observer::newJob( KIO::Job * job )
{
    // Tell the UI Server about this new job, and give it the application id
    // at the same time
    int progressId = m_uiserver->newJob( kapp->dcopClient()->appId() );

    // Keep the result in a dict
    m_dctJobs.insert( progressId, job );

    return progressId;
}

void Observer::jobFinished( int progressId )
{
    m_uiserver->jobFinished( progressId );
    m_dctJobs.remove( progressId );
}

void Observer::killJob( int progressId )
{
    KIO::Job * job = m_dctJobs[ progressId ];
    assert(job);
    job->kill();
}

void Observer::slotTotalSize( KIO::Job* job, unsigned long size )
{
  kdDebug() << "** Observer::slotTotalSize " << job << " " << size << endl;
  m_uiserver->totalSize( job->progressId(), size );
}

void Observer::slotTotalFiles( KIO::Job* job, unsigned long files )
{
  kdDebug() << "** Observer::slotTotalFiles " << job << " " << files << endl;
  m_uiserver->totalFiles( job->progressId(), files );
}

void Observer::slotTotalDirs( KIO::Job* job, unsigned long dirs )
{
  kdDebug() << "** Observer::slotTotalDirs " << job << " " << dirs << endl;
  m_uiserver->totalDirs( job->progressId(), dirs );
}

void Observer::slotProcessedSize( KIO::Job* job, unsigned long size )
{
  kdDebug() << "** Observer::slotProcessedSize " << job << " " << size << endl;
  m_uiserver->processedSize( job->progressId(), size );
}

void Observer::slotProcessedFiles( KIO::Job* job, unsigned long files )
{
  kdDebug() << "** Observer::slotProcessedFiles " << job << " " << files << endl;
  m_uiserver->processedFiles( job->progressId(), files );
}

void Observer::slotProcessedDirs( KIO::Job* job, unsigned long dirs )
{
  kdDebug() << "** Observer::slotProcessedDirs " << job << " " << dirs << endl;
  m_uiserver->processedDirs( job->progressId(), dirs );
}

void Observer::slotSpeed( KIO::Job* job, unsigned long bytes_per_second )
{
  kdDebug() << "** Observer::slotSpeed " << job << " " << bytes_per_second << endl;
  m_uiserver->speed( job->progressId(), bytes_per_second );
}

void Observer::slotPercent( KIO::Job* job, unsigned long percent )
{
  kdDebug() << "** Observer::slotPercent " << job << " " << percent << endl;
  m_uiserver->percent( job->progressId(), percent );
}

void Observer::slotCopying( KIO::Job* job, const KURL& from, const KURL& to )
{
  kdDebug() << "** Observer::slotCopying " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->copying( job->progressId(), from, to );
}

void Observer::slotMoving( KIO::Job* job, const KURL& from, const KURL& to )
{
  kdDebug() << "** Observer::slotMoving " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->moving( job->progressId(), from, to );
}

void Observer::slotDeleting( KIO::Job* job, const KURL& url )
{
  kdDebug() << "** Observer::slotDeleting " << job << " " << url.url() << endl;
  m_uiserver->deleting( job->progressId(), url );
}

void Observer::slotCreatingDir( KIO::Job* job, const KURL& dir )
{
  kdDebug() << "** Observer::slotCreatingDir " << job << " " << dir.url() << endl;
  m_uiserver->creatingDir( job->progressId(), dir );
}

void Observer::slotCanResume( KIO::Job* job, bool can_resume )
{
  kdDebug() << "** Observer::slotCanResume " << job << " " << can_resume << endl;
  m_uiserver->canResume( job->progressId(), (uint)can_resume );
}





#include "observer.moc"
