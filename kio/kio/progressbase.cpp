/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

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

#include "jobclasses.h"
#include "progressbase.h"

namespace KIO {

ProgressBase::ProgressBase( QWidget *parent )
  : QWidget( parent )
{
  m_pJob = 0;

  // delete dialog after the job is finished / canceled
  m_bOnlyClean = false;

  // stop job on close
  m_bStopOnClose = true;
}


void ProgressBase::setJob( KIO::Job *job )
{
  // first connect all slots
  connect( job, SIGNAL( percent( KJob*, unsigned long ) ),
	   SLOT( slotPercent( KJob*, unsigned long ) ) );

  connect( job, SIGNAL( result( KJob* ) ),
	   SLOT( slotFinished( KJob* ) ) );

  // After all canceled() is supposed to be unused... remove?
  //connect( job, SIGNAL( canceled( KIO::Job* ) ),
  //	   SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::CopyJob *job )
{
  // first connect all slots
  connect( job, SIGNAL( totalSize( KJob*, qulonglong ) ),
	   SLOT( slotTotalSize( KJob*, qulonglong ) ) );
  connect( job, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( processedSize( KJob*, qulonglong ) ),
	   SLOT( slotProcessedSize( KJob*, qulonglong ) ) );
  connect( job, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( percent( KJob*, unsigned long ) ),
	   SLOT( slotPercent( KJob*, unsigned long ) ) );

  connect( job, SIGNAL( copying( KIO::Job*, const KUrl& , const KUrl& ) ),
	   SLOT( slotCopying( KIO::Job*, const KUrl&, const KUrl& ) ) );
  connect( job, SIGNAL( moving( KIO::Job*, const KUrl& , const KUrl& ) ),
	   SLOT( slotMoving( KIO::Job*, const KUrl&, const KUrl& ) ) );
  connect( job, SIGNAL( creatingDir( KIO::Job*, const KUrl& ) ),
 	   SLOT( slotCreatingDir( KIO::Job*, const KUrl& ) ) );

  connect( job, SIGNAL( result( KJob* ) ),
	   SLOT( slotFinished( KJob* ) ) );

  // Should be safe to remove...
  //connect( job, SIGNAL( canceled( KIO::Job* ) ),
  //	   SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::DeleteJob *job )
{
  // first connect all slots
  connect( job, SIGNAL( totalSize( KJob*, qulonglong ) ),
	   SLOT( slotTotalSize( KJob*, qulonglong ) ) );
  connect( job, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( processedSize( KJob*, qulonglong ) ),
	   SLOT( slotProcessedSize( KJob*, qulonglong ) ) );
  connect( job, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( percent( KJob*, unsigned long ) ),
	   SLOT( slotPercent( KJob*, unsigned long ) ) );

  connect( job, SIGNAL( deleting( KIO::Job*, const KUrl& ) ),
	   SLOT( slotDeleting( KIO::Job*, const KUrl& ) ) );

  connect( job, SIGNAL( result( KJob* ) ),
	   SLOT( slotFinished( KJob* ) ) );

  // Safe to remove?
  //connect( job, SIGNAL( canceled( KIO::Job* ) ),
  //	   SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::closeEvent( QCloseEvent* ) {
  // kill job when desired
  if ( m_bStopOnClose ) {
    slotStop();
  } else {
    // clean or delete dialog
    if ( m_bOnlyClean ) {
      slotClean();
    } else {
      delete this;
    }
  }
}

void ProgressBase::finished() {
  // clean or delete dialog
  if ( m_bOnlyClean ) {
    slotClean();
  } else {
    deleteLater();
  }
}

void ProgressBase::slotFinished( KJob* ) {
  finished();
}


void ProgressBase::slotStop() {
  if ( m_pJob ) {
    m_pJob->kill(); // this will call slotFinished
    m_pJob = 0L;
  } else {
    slotFinished( 0 ); // here we call it ourselves
  }

  emit stopped();
}

void ProgressBase::slotPause() {
  if ( m_pJob ) {
    m_pJob->suspend();
  }
  emit suspend();
}

void ProgressBase::slotResume() {
  if ( m_pJob ) {
    m_pJob->resume();
  }
  emit resume();
}

void ProgressBase::slotClean() {
  hide();
}

void ProgressBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

} /* namespace */

#include "progressbase.moc"

