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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
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
  connect( job, SIGNAL( percent( KIO::Job*, unsigned long ) ),
	   SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( result( KIO::Job* ) ),
	   SLOT( slotFinished( KIO::Job* ) ) );

  connect( job, SIGNAL( canceled( KIO::Job* ) ),
	   SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::CopyJob *job )
{
  // first connect all slots
  connect( job, SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
	   SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
	   SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( percent( KIO::Job*, unsigned long ) ),
	   SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( copying( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotCopying( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( job, SIGNAL( moving( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotMoving( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( job, SIGNAL( creatingDir( KIO::Job*, const KURL& ) ),
 	   SLOT( slotCreatingDir( KIO::Job*, const KURL& ) ) );

  connect( job, SIGNAL( result( KIO::Job* ) ),
	   SLOT( slotFinished( KIO::Job* ) ) );

  connect( job, SIGNAL( canceled( KIO::Job* ) ),
	   SLOT( slotFinished( KIO::Job* ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::DeleteJob *job )
{
  // first connect all slots
  connect( job, SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
	   SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
	   SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( percent( KIO::Job*, unsigned long ) ),
	   SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( deleting( KIO::Job*, const KURL& ) ),
	   SLOT( slotDeleting( KIO::Job*, const KURL& ) ) );

  connect( job, SIGNAL( result( KIO::Job* ) ),
	   SLOT( slotFinished( KIO::Job* ) ) );

  connect( job, SIGNAL( canceled( KIO::Job* ) ),
	   SLOT( slotFinished( KIO::Job* ) ) );

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

void ProgressBase::slotFinished( KIO::Job* ) {
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


void ProgressBase::slotClean() {
  hide();
}

void ProgressBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

} /* namespace */

#include "progressbase.moc"

