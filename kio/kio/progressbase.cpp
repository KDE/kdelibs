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

ProgressBase::ProgressBase( QWidget *parent )
  : QWidget( parent )
{
  m_pJob = 0;
}


void ProgressBase::setJob( KIO::Job *job, bool onlyClean, bool stopOnClose )
{
  m_bOnlyClean = onlyClean;
  m_bStopOnClose = stopOnClose;

  // first connect all slots
  connect( job, SIGNAL( percent( KIO::Job*, unsigned long ) ),
	   SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::CopyJob *job, bool onlyClean, bool stopOnClose )
{
  m_bOnlyClean = onlyClean;
  m_bStopOnClose = stopOnClose;

  // first connect all slots
  connect( job, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
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

  connect( job, SIGNAL( renaming( KIO::Job*, const KURL&, const KURL& ) ),
 	   SLOT( slotRenaming( KIO::Job*, const KURL&, const KURL& ) ) );

  connect( job, SIGNAL( canResume( KIO::Job*, bool ) ),
 	   SLOT( slotCanResume( KIO::Job*, bool ) ) );

  // then assign job
  m_pJob = job;
}


void ProgressBase::setJob( KIO::DeleteJob *job, bool onlyClean, bool stopOnClose )
{
  m_bOnlyClean = onlyClean;
  m_bStopOnClose = stopOnClose;

  // first connect all slots
  connect( job, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( job, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
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

  // then assign job
  m_pJob = job;
}


void ProgressBase::closeEvent( QCloseEvent* ) {
  // kill job when desired
  if ( m_bStopOnClose ) {
    stop();
  }

  // clean or delete dialog
  if ( m_bOnlyClean ) {
    clean();
  } else {
    delete this;
  }
}

// this will kill job and subsequently also delete or clean this dialog
void ProgressBase::stop() {
  if ( m_pJob ) {
    m_pJob->kill();
  } else {
    emit stopped();
  }
}

#include "progressbase.moc"

