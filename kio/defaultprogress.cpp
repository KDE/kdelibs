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
#include <qdatetime.h>

#include <kapp.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kstringhandler.h>
#include <kglobal.h>
#include <klocale.h>

#include "jobclasses.h"
#include "defaultprogress.h"


DefaultProgress::DefaultProgress( bool showNow )
  : ProgressBase( 0 ) {

  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );
  topLayout->addStrut( 360 );   // makes dlg at least that wide

  QGridLayout *grid = new QGridLayout(3, 3);
  topLayout->addLayout(grid);
  grid->setColStretch(2, 1);
  grid->addColSpacing(1, KDialog::spacingHint());
  // filenames or action name
  grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);

  sourceLabel = new QLabel(this);
  grid->addWidget(sourceLabel, 0, 2);

  destInvite = new QLabel(i18n("Destination:"), this);
  grid->addWidget(destInvite, 1, 0);

  destLabel = new QLabel(this);
  grid->addWidget(destLabel, 1, 2);

// why include this waste of space?
//  topLayout->addSpacing( 10 );

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
  connect( pb, SIGNAL( clicked() ), SLOT( slotStop() ) );
  hBox->addWidget( pb );

  resize( sizeHint() );
  setMaximumSize( sizeHint().width()+50, sizeHint().height()+30 );
  kdDebug() << "DefaultProgress: max size: " << sizeHint().width()+50 << "," << sizeHint().height()+30 << endl;
  kdDebug() << "DefaultProgress: size: " << width() << "," << height() << endl;

  if ( showNow ) {
    show();
  }
}


void DefaultProgress::slotTotalSize( KIO::Job*, unsigned long bytes )
{
  m_iTotalSize = bytes;
}


void DefaultProgress::slotTotalFiles( KIO::Job*, unsigned long files )
{
  m_iTotalFiles = files;
}


void DefaultProgress::slotTotalDirs( KIO::Job*, unsigned long dirs )
{
  m_iTotalDirs = dirs;
}


void DefaultProgress::slotPercent( KIO::Job*, unsigned long percent )
{
  QString tmp(i18n( "%1% of %2 ").arg( percent ).arg( KIO::convertSize(m_iTotalSize)));
  m_pProgressBar->setValue( percent );
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


void DefaultProgress::slotInfoMessage( KIO::Job*, const QString & msg )
{
  speedLabel->setText( msg );
}


void DefaultProgress::slotProcessedSize( KIO::Job*, unsigned long bytes ) {
  m_iProcessedSize = bytes;

  QString tmp;
  tmp = i18n( "%1 of %2 complete").arg( KIO::convertSize(bytes) ).arg( KIO::convertSize(m_iTotalSize));
  sizeLabel->setText( tmp );
}


void DefaultProgress::slotProcessedDirs( KIO::Job*, unsigned long dirs )
{
  m_iProcessedDirs = dirs;

  QString tmps;
  tmps = i18n("%1 / %2 directories  ").arg( m_iProcessedDirs ).arg( m_iTotalDirs );
  tmps += i18n("%1 / %2 files").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotProcessedFiles( KIO::Job*, unsigned long files )
{
  m_iProcessedFiles = files;

  QString tmps;
  if ( m_iTotalDirs > 1 ) {
    tmps = i18n("%1 / %2 directories  ").arg( m_iProcessedDirs ).arg( m_iTotalDirs );
  }
  tmps += i18n("%1 / %2 files").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotSpeed( KIO::Job*, unsigned long bytes_per_second )
{
  if ( bytes_per_second == 0 ) {
    speedLabel->setText( i18n( "Stalled") );
  } else {
    QTime remaining = KIO::calculateRemaining( m_iTotalSize, m_iProcessedSize, bytes_per_second );
    speedLabel->setText( i18n( "%1/s ( %2 remaining )").arg( KIO::convertSize( bytes_per_second )).arg( remaining.toString() ) );
  }
}


void DefaultProgress::slotCopying( KIO::Job*, const KURL& from, const KURL& to )
{
  setCaption(i18n("Copy file(s) progress"));
  mode = Copy;
  sourceLabel->setText( KStringHandler::csqueeze(from.prettyURL()) );
  setDestVisible( true );
  destLabel->setText( KStringHandler::csqueeze(to.prettyURL()) );
}


void DefaultProgress::slotMoving( KIO::Job*, const KURL& from, const KURL& to )
{
  setCaption(i18n("Move file(s) progress"));
  mode = Move;
  sourceLabel->setText( KStringHandler::csqueeze(from.prettyURL()) );
  setDestVisible( true );
  destLabel->setText( KStringHandler::csqueeze(to.prettyURL()) );
}


void DefaultProgress::slotCreatingDir( KIO::Job*, const KURL& dir )
{
  setCaption(i18n("Creating directory"));
  mode = Create;
  sourceLabel->setText( KStringHandler::csqueeze(dir.prettyURL()) );
  setDestVisible( false );
}


void DefaultProgress::slotDeleting( KIO::Job*, const KURL& url )
{
  setCaption(i18n("Delete file(s) progress"));
  mode = Delete;
  sourceLabel->setText( KStringHandler::csqueeze(url.prettyURL()) );
  setDestVisible( false );
}

void DefaultProgress::slotStating( KIO::Job*, const KURL& url )
{
  setCaption(i18n("Examining file progress"));
  sourceLabel->setText( KStringHandler::csqueeze(url.prettyURL()) );
  setDestVisible( false );
}

void DefaultProgress::slotMounting( KIO::Job*, const QString & dev, const QString & point )
{
  setCaption(i18n("Mounting %1").arg(dev));
  sourceLabel->setText( point );
  setDestVisible( false );
}

void DefaultProgress::slotUnmounting( KIO::Job*, const QString & point )
{
  setCaption(i18n("Unmounting"));
  sourceLabel->setText( point );
  setDestVisible( false );
}

void DefaultProgress::slotCanResume( KIO::Job*, bool resume )
{
  if ( resume ) {
    resumeLabel->setText( i18n("Resumable") );
  } else {
    resumeLabel->setText( i18n("Not resumable") );
  }
}

void DefaultProgress::setDestVisible( bool visible )
{
  // We can't hide the destInvite/destLabel labels,
  // because it screws up the QGridLayout.
  if (visible)
  {
    destInvite->setText( i18n("Destination:") );
  }
  else
  {
    destInvite->setText( QString::null );
    destLabel->setText( QString::null );
  }
}

#include "defaultprogress.moc"
