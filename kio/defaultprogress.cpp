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

#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>

#include <kapp.h>
#include <kdialog.h>
#include <klocale.h>

#include "jobclasses.h"
#include "defaultprogress.h"


DefaultProgress::DefaultProgress()
  : ProgressBase() {

  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
					    KDialog::spacingHint() );
  topLayout->addStrut( 360 );	// makes dlg at least that wide

  QGridLayout *grid = new QGridLayout(3, 3);
  topLayout->addLayout(grid);
  grid->setColStretch(2, 1);
  grid->addColSpacing(1, KDialog::spacingHint());
  // filenames or action name
  grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);

  sourceLabel = new QLabel(this);
  grid->addWidget(sourceLabel, 0, 2);

  grid->addWidget(new QLabel(i18n("Destination:"), this), 1, 0);

  destLabel = new QLabel(this);
  grid->addWidget(destLabel, 1, 2);

  topLayout->addSpacing( 10 );

  progressLabel = new QLabel(this);
  grid->addWidget(progressLabel, 2, 2);

  m_pProgressBar = new KProgress(0, 100, 0, KProgress::Horizontal, this);
  topLayout->addWidget( m_pProgressBar );

  // processed info
  QHBoxLayout *hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  speedLabel = new QLabel(this);
  hBox->addWidget(speedLabel, 1);

  sizeLabel = new QLabel(this);
  hBox->addWidget(sizeLabel);

  resumeLabel = new QLabel(this);
  hBox->addWidget(resumeLabel);

  hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  hBox->addStretch(1);

  QPushButton *pb = new QPushButton( i18n("Cancel"), this );
  connect( pb, SIGNAL( clicked() ), SLOT( stop() ) );
  hBox->addWidget( pb );

  resize( sizeHint() );
}


void DefaultProgress::slotTotalSize( KIO::Job*, unsigned long _bytes )
{
  m_iTotalSize = _bytes;
}


void DefaultProgress::slotTotalFiles( KIO::Job*, unsigned long _files )
{
  m_iTotalFiles = _files;
}


void DefaultProgress::slotTotalDirs( KIO::Job*, unsigned long _dirs )
{
  m_iTotalDirs = _dirs;
}


void DefaultProgress::slotPercent( KIO::Job*, unsigned int _percent )
{
  QString tmp(i18n( "%1% of %2 ").arg( _percent ).arg( KIO::convertSize(m_iTotalSize)));
  m_pProgressBar->setValue( _percent );
  switch(mode) {
  case Copy:
    tmp.append(i18n(" (Copying)"));
    break;
  case Move:
    tmp.append(i18n(" (Moving)"));
    break;
  case Delete:
    tmp.append(i18n(" (Deleting)"));
    break;
  case Create:
    tmp.append(i18n(" (Creating)"));
    break;
  }

  setCaption( tmp );
}


void DefaultProgress::slotProcessedSize( KIO::Job*, unsigned long _bytes ) {
  QString tmp;

  tmp = i18n( "%1 of %2 ").arg( KIO::convertSize(_bytes) ).arg( KIO::convertSize(m_iTotalSize));
  sizeLabel->setText( tmp );
}


void DefaultProgress::slotProcessedDirs( KIO::Job*, unsigned long _dirs )
{
  m_iProcessedDirs = _dirs;

  QString tmps;
  tmps = i18n("%1 / %2 directories  ").arg( m_iProcessedDirs ).arg( m_iTotalDirs );
  tmps += i18n("%1 / %2 files").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotProcessedFiles( KIO::Job*, unsigned long _files )
{
  m_iProcessedFiles = _files;

  QString tmps;
  if ( m_iTotalDirs > 1 ) {
    tmps = i18n("%1 / %2 directories  ").arg( m_iProcessedDirs ).arg( m_iTotalDirs );
  }
  tmps += i18n("%1 / %2 files").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotSpeed( KIO::Job*, unsigned long _bytes_per_second )
{
  if ( _bytes_per_second == 0 ) {
    speedLabel->setText( i18n( "Stalled") );
  } else {
    speedLabel->setText( i18n( "%1/s").arg( KIO::convertSize( _bytes_per_second )) );
  }
}


void DefaultProgress::slotCopyingFile( KIO::Job*, const KURL& _from,
					    const KURL& _to )
{
  setCaption(i18n("Copy file(s) progress"));
  mode = Copy;
  sourceLabel->setText( _from.path() );
  destLabel->setText( _to.path() );
}


void DefaultProgress::slotMovingFile( KIO::Job*, const KURL& _from,
					    const KURL& _to )
{
  setCaption(i18n("Move file(s) progress"));
  mode = Move;
  sourceLabel->setText( _from.path() );
  destLabel->setText( _to.path() );
}


void DefaultProgress::slotCreatingDir( KIO::Job*, const KURL& _dir )
{
  setCaption(i18n("Creating directory"));
  mode = Create;
  sourceLabel->setText( _dir.path() );
}


void DefaultProgress::slotDeletingFile( KIO::Job*, const KURL& _url )
{
  setCaption(i18n("Delete file(s) progress"));
  mode = Delete;
  sourceLabel->setText( _url.path() );
}


void DefaultProgress::slotCanResume( KIO::Job*, bool _resume )
{
  if ( _resume ) {
    resumeLabel->setText( i18n("Resumable") );
  } else {
    resumeLabel->setText( i18n("Not resumable") );
  }
}

#include "defaultprogress.moc"
