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
        kdDebug() << "Starting kio_uiserver" << endl;
        QString error;
        int ret = KApplication::startServiceByDesktopPath( "kio_uiserver.desktop",
                                                             QStringList(), &error );
        if ( ret > 0 )
        {
            kdError() << "Couldn't start kio_uiserver from kio_uiserver.desktop: " << error << endl;
        } else
            kdDebug() << "startServiceByDesktopPath returned " << ret << endl;

    }
    if ( !kapp->dcopClient()->isApplicationRegistered( "kio_uiserver" ) )
        kdDebug() << "The application kio_uiserver is STILL NOT REGISTERED" << endl;
    else
        kdDebug() << "kio_uiserver registered" << endl;

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
//   kdDebug() << "** Observer::slotTotalSize " << job << " " << size << endl;
  m_uiserver->totalSize( job->progressId(), size );
}

void Observer::slotTotalFiles( KIO::Job* job, unsigned long files )
{
//   kdDebug() << "** Observer::slotTotalFiles " << job << " " << files << endl;
  m_uiserver->totalFiles( job->progressId(), files );
}

void Observer::slotTotalDirs( KIO::Job* job, unsigned long dirs )
{
//   kdDebug() << "** Observer::slotTotalDirs " << job << " " << dirs << endl;
  m_uiserver->totalDirs( job->progressId(), dirs );
}

void Observer::slotProcessedSize( KIO::Job* job, unsigned long size )
{
//   kdDebug() << "** Observer::slotProcessedSize " << job << " " << job->progressId() << " " << size << endl;
  m_uiserver->processedSize( job->progressId(), size );
}

void Observer::slotProcessedFiles( KIO::Job* job, unsigned long files )
{
//   kdDebug() << "** Observer::slotProcessedFiles " << job << " " << files << endl;
  m_uiserver->processedFiles( job->progressId(), files );
}

void Observer::slotProcessedDirs( KIO::Job* job, unsigned long dirs )
{
//   kdDebug() << "** Observer::slotProcessedDirs " << job << " " << dirs << endl;
  m_uiserver->processedDirs( job->progressId(), dirs );
}

void Observer::slotSpeed( KIO::Job* job, unsigned long bytes_per_second )
{
//   kdDebug() << "** Observer::slotSpeed " << job << " " << bytes_per_second << endl;
  m_uiserver->speed( job->progressId(), bytes_per_second );
}

void Observer::slotPercent( KIO::Job* job, unsigned long percent )
{
//   kdDebug() << "** Observer::slotPercent " << job << " " << percent << endl;
  m_uiserver->percent( job->progressId(), percent );
}

void Observer::slotInfoMessage( KIO::Job* job, const QString & msg )
{
  m_uiserver->infoMessage( job->progressId(), msg );
}

void Observer::slotCopying( KIO::Job* job, const KURL& from, const KURL& to )
{
//   kdDebug() << "** Observer::slotCopying " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->copying( job->progressId(), from, to );
}

void Observer::slotMoving( KIO::Job* job, const KURL& from, const KURL& to )
{
//   kdDebug() << "** Observer::slotMoving " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->moving( job->progressId(), from, to );
}

void Observer::slotDeleting( KIO::Job* job, const KURL& url )
{
//   kdDebug() << "** Observer::slotDeleting " << job << " " << url.url() << endl;
  m_uiserver->deleting( job->progressId(), url );
}

void Observer::slotCreatingDir( KIO::Job* job, const KURL& dir )
{
//   kdDebug() << "** Observer::slotCreatingDir " << job << " " << dir.url() << endl;
  m_uiserver->creatingDir( job->progressId(), dir );
}

void Observer::slotCanResume( KIO::Job* job, bool can_resume )
{
//   kdDebug() << "** Observer::slotCanResume " << job << " " << can_resume << endl;
  m_uiserver->canResume( job->progressId(), (uint)can_resume );
}

void Observer::stating( KIO::Job* job, const KURL& url )
{
  m_uiserver->stating( job->progressId(), url );
}

void Observer::mounting( KIO::Job* job, const QString & dev, const QString & point )
{
  m_uiserver->mounting( job->progressId(), dev, point );
}

void Observer::unmounting( KIO::Job* job, const QString & point )
{
  m_uiserver->unmounting( job->progressId(), point );
}


bool Observer::authorize( QString& user, QString& pass ,const QString& head, const QString& key )
{
//   kdDebug() << "** Observer::authorize " << endl;
  QByteArray resultArgs = m_uiserver->authorize( user, head, key );
  if ( m_uiserver->ok() )
  {
    kdDebug(7007) << "Call was ok" << endl;
    QDataStream stream( resultArgs, IO_ReadOnly );
    Q_UINT8 authorized;
    QString u, p;	  	
    stream >> authorized >> u >> p;
    if( authorized )
    {
      user = u;
      pass = p;
      return true;
    }
  }
  else
    kdDebug(7007) << "Call was not OK" << endl;
  return false;		
}

RenameDlg_Result Observer::open_RenameDlg( KIO::Job * job,
                                           const QString & caption,
                                           const QString& src, const QString & dest,
                                           RenameDlg_Mode mode, QString& newDest,
                                           unsigned long sizeSrc,
                                           unsigned long sizeDest,
                                           time_t ctimeSrc,
                                           time_t ctimeDest,
                                           time_t mtimeSrc,
                                           time_t mtimeDest
                                           )
{
  kdDebug(7007) << "Observer::open_RenameDlg" << endl;
  QByteArray resultArgs = m_uiserver->open_RenameDlg( job ? job->progressId() : 0, caption, src, dest, mode,
                                                      sizeSrc, sizeDest,
                                                      (unsigned long) ctimeSrc, (unsigned long) ctimeDest,
                                                      (unsigned long) mtimeSrc, (unsigned long) mtimeDest );
  if ( m_uiserver->ok() )
  {
    QDataStream stream( resultArgs, IO_ReadOnly );
    Q_UINT8 result;
    stream >> result >> newDest;
    kdDebug(7007) << "UIServer::open_RenameDlg returned " << result << "," << newDest << endl;
    return (RenameDlg_Result) result;
  }
  kdDebug(7007) << "open_RenameDlg call failed" << endl;
  return R_CANCEL;
}

SkipDlg_Result Observer::open_SkipDlg( KIO::Job * job,
                                       bool _multi,
                                       const QString& _error_text )
{
  kdDebug(7007) << "Observer::open_SkipDlg" << endl;
  int result = m_uiserver->open_SkipDlg( job ? job->progressId() : 0, (int)_multi, _error_text );
  if ( m_uiserver->ok() )
  {
    kdDebug(7007) << "UIServer::open_SkipDlg returned " << result << endl;
    return (SkipDlg_Result) result;
  }
  kdDebug(7007) << "open_SkipDlg call failed" << endl;
  return S_CANCEL;
}


#include "observer.moc"
