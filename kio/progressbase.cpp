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
  : QWidget( parent ) {
  m_bOnlyClean = false;
  m_bStopOnClose = true;
}


void ProgressBase::Connect() {
  connect( m_pJob, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( percent( KIO::Job*, unsigned int ) ),
	   SLOT( slotPercent( KIO::Job*, unsigned int ) ) );

  connect( m_pJob, SIGNAL( copying( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotCopying( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( m_pJob, SIGNAL( moving( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotMoving( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( m_pJob, SIGNAL( deleting( KIO::Job*, const KURL& ) ),
	   SLOT( slotDeleting( KIO::Job*, const KURL& ) ) );
  connect( m_pJob, SIGNAL( creatingDir( KIO::Job*, const KURL& ) ),
 	   SLOT( slotCreatingDir( KIO::Job*, const KURL& ) ) );

  connect( m_pJob, SIGNAL( canResume( KIO::Job*, bool ) ),
 	   SLOT( slotCanResume( KIO::Job*, bool ) ) );
}


void ProgressBase::setJob( KIO::Job *job ) {
  if ( m_pJob ) {
    disconnect( m_pJob ); // completely forget about that job
  }

  m_pJob = job;
  Connect();
}


void ProgressBase::closeEvent( QCloseEvent* ) {
  if ( m_bStopOnClose ) {
    stop();
  } else { // we have to do the cleaning ourselves
    if ( m_bOnlyClean ) {
      clean();
    } else {
      delete this;
    }
  }
}

// this will kill job and subsequently also delete or clean this dialog
void ProgressBase::stop() {
  if ( m_pJob ) {
    m_pJob->kill();
  }
}

#include "progressbase.moc"

