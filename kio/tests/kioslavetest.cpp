 /*
  This file is or will be part of KDE desktop environment

  Copyright 1999 Matt Koss <koss@miesto.sk>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <qlayout.h>
#include <qmessagebox.h>

#include <kapp.h>
#include <klocale.h>
#include <kurl.h>

#include "kioslavetest.h"


KioslaveTest *kmain;

void usage() {
  qDebug( "\nkioslavetest - test for checking kioslave features\n");
  qDebug( "usage: kioslavetest [-s source] [-d destination]");
  qDebug( "\t[-o operation]\tValid types are : list, get, copy, move, del");
  qDebug("\t\t\tDefault operation is copy.\n");
  qDebug( "\t[-p progress]\tValid types are : none, simple, list, little");
  qDebug("\t\t\tDefault progress type is simple.\n");
  exit(0);
}


KioslaveTest::KioslaveTest( QString src, QString dest, uint op, uint pr )
  : KTMainWindow("") {
  
  job = 0L;

  main_widget = new QWidget( this, "");
  QBoxLayout *topLayout = new QVBoxLayout( main_widget, 10, 5 );
	
  QGridLayout *grid = new QGridLayout( 2, 2, 10 );
  topLayout->addLayout( grid );

  grid->setRowStretch(0,1);
  grid->setRowStretch(1,1);
    
  grid->setColStretch(0,1);
  grid->setColStretch(1,100);

  lb_from = new QLabel( i18n("From :"), main_widget );
  grid->addWidget( lb_from, 0, 0 );

  le_source = new QLineEdit( main_widget );
  grid->addWidget( le_source, 0, 1 );
  le_source->setText( src );

  lb_to = new QLabel( i18n("To :"), main_widget );
  grid->addWidget( lb_to, 1, 0 );

  le_dest = new QLineEdit( main_widget );
  grid->addWidget( le_dest, 1, 1 );
  le_dest->setText( dest );

  // Operation groupbox & buttons
  opButtons = new QButtonGroup( i18n("Operation"), main_widget );
  topLayout->addWidget( opButtons, 10 );
  connect( opButtons, SIGNAL(clicked(int)), SLOT(changeOperation(int)) );

  QBoxLayout *hbLayout = new QHBoxLayout( opButtons, 15 );

  rbList = new QRadioButton( i18n("List"), opButtons );
  opButtons->insert( rbList, List );
  hbLayout->addWidget( rbList, 5 );

  rbGet = new QRadioButton( i18n("Get"), opButtons );
  opButtons->insert( rbGet, Get );
  hbLayout->addWidget( rbGet, 5 );

  rbCopy = new QRadioButton( i18n("Copy"), opButtons );
  opButtons->insert( rbCopy, Copy );
  hbLayout->addWidget( rbCopy, 5 );

  rbMove = new QRadioButton( i18n("Move"), opButtons );
  opButtons->insert( rbMove, Move );
  hbLayout->addWidget( rbMove, 5 );

  rbDelete = new QRadioButton( i18n("Delete"), opButtons );
  opButtons->insert( rbDelete, Delete );
  hbLayout->addWidget( rbDelete, 5 );

  opButtons->setButton( op );
  changeOperation( op );

  // Progress groupbox & buttons
  progressButtons = new QButtonGroup( i18n("Progress dialog mode"), main_widget );
  topLayout->addWidget( progressButtons, 10 );
  connect( progressButtons, SIGNAL(clicked(int)), SLOT(changeProgressMode(int)) );

  hbLayout = new QHBoxLayout( progressButtons, 15 );

  rbProgressNone = new QRadioButton( i18n("None"), progressButtons );
  progressButtons->insert( rbProgressNone, ProgressNone );
  hbLayout->addWidget( rbProgressNone, 5 );

  rbProgressSimple = new QRadioButton( i18n("Simple"), progressButtons );
  progressButtons->insert( rbProgressSimple, ProgressSimple );
  hbLayout->addWidget( rbProgressSimple, 5 );

  rbProgressList = new QRadioButton( i18n("List"), progressButtons );
  progressButtons->insert( rbProgressList, ProgressList );
  hbLayout->addWidget( rbProgressList, 5 );

  rbProgressLittle = new QRadioButton( i18n("Little"), progressButtons );
  progressButtons->insert( rbProgressLittle, ProgressLittle );
  hbLayout->addWidget( rbProgressLittle, 5 );

  progressButtons->setButton( pr );
  changeProgressMode( pr );

  // run & stop butons
  hbLayout = new QHBoxLayout( topLayout, 15 );

  pbStart = new QPushButton( i18n("Start"), main_widget );
  pbStart->setFixedSize( pbStart->sizeHint() );
  connect( pbStart, SIGNAL(clicked()), SLOT(startJob()) );
  hbLayout->addWidget( pbStart, 5 );

  pbStop = new QPushButton( i18n("Stop"), main_widget );
  pbStop->setFixedSize( pbStop->sizeHint() );
  pbStop->setEnabled( false );
  connect( pbStop, SIGNAL(clicked()), SLOT(stopJob()) );
  hbLayout->addWidget( pbStop, 5 );

  // close button
  close = new QPushButton( i18n("Close"), main_widget );
  close->setFixedSize( close->sizeHint() );
  connect(close, SIGNAL(clicked()), kapp, SLOT(quit()));

  topLayout->addWidget( close, 5 );

  main_widget->setMinimumSize( main_widget->sizeHint() );
  setView( main_widget );

  littleProgress = new KIOLittleProgressDlg( statusBar() );
  statusBar()->insertWidget( littleProgress, littleProgress->width() , 0 );

  kmain = this;

  show();
}


void KioslaveTest::closeEvent( QCloseEvent * ){
  kapp->quit();
}


void KioslaveTest::changeOperation( int id ) {
  bool enab;

  enab = rbDelete->isChecked() ||
    rbList->isChecked() ||
    rbGet->isChecked();

  le_dest->setEnabled( ! enab );

  selectedOperation = id;
}


void KioslaveTest::changeProgressMode( int id ) {
  progressMode = id;

  if ( progressMode == ProgressLittle ) {
    enableStatusBar( KStatusBar::Show );
  } else {
    enableStatusBar( KStatusBar::Hide );
  }
}


void KioslaveTest::startJob() {
  KURL url = le_source->text();

  if ( url.isMalformed() ) {
    QMessageBox::critical(this, i18n("Kioslave Error Message"), i18n("Source URL is malformed") );
    return;
  }

  url = le_dest->text();
  if ( url.isMalformed() &&
       ( selectedOperation == Copy || selectedOperation == Move ) ) {
    QMessageBox::critical(this, i18n("Kioslave Error Message"),
		       i18n("Destination URL is malformed") );
    return;
  }

  pbStart->setEnabled( false );
  job = new KIOJob;

  switch ( progressMode ) {
  case ProgressSimple:
    job->setGUImode( KIOJob::SIMPLE );
    break;

  case ProgressList:
    job->setGUImode( KIOJob::LIST );
    break;

  case ProgressLittle:
    job->setGUImode( KIOJob::LITTLE );
    job->connectProgress( littleProgress );
    break;

  default:
    job->setGUImode( KIOJob::NONE );
    break;
  }

  switch ( selectedOperation ) {
  case List:
    job->listDir( le_source->text() );
    break;

  case Get:
    job->get( le_source->text() );
    break;

  case Copy:
    job->copy( le_source->text(), le_dest->text() );
    break;

  case Move:
    job->move( le_source->text(), le_dest->text() );
    break;

  case Delete:
    job->del( le_source->text() );
    break;

  }

  connect( job, SIGNAL( sigFinished( int ) ),
	   SLOT( slotFinished() ) );
  connect( job, SIGNAL( sigCanceled( int ) ),
	   SLOT( slotFinished() ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ),
	   SLOT( slotError( int, int, const char* ) ) );

  pbStop->setEnabled( true );
}


void KioslaveTest::slotError( int, int errid, const char* errortext ) {
  QString msg = KIO::kioErrorString( errid, errortext );
  QMessageBox::critical(this, i18n("Kioslave Error Message"), msg );

  slotFinished();
}


void KioslaveTest::slotFinished() {
  pbStart->setEnabled( true );
  pbStop->setEnabled( false );
  littleProgress->clean();
}


void KioslaveTest::stopJob() {
  pbStop->setEnabled( false );

  job->kill();
  job = 0L;

  pbStart->setEnabled( true );
}


int main(int argc, char **argv) {
  KApplication *app = new KApplication( argc, argv, "kioslavetest" );

  argc--;
  argv++;

  QString src;
  QString dest;
  uint op = KioslaveTest::Copy;
  uint pr = KioslaveTest::ProgressSimple;

  QString tmps;
  
  for ( int i = 0; i < argc; i++ ) {
    if (strcmp(argv[i],"--help")==0) {
      usage();
    } else if (strcmp(argv[i],"-s")==0){
      if ( i < argc-1 )
	src = argv[++i];
    } else if (strcmp(argv[i],"-d")==0){
      if ( i < argc-1 )
	dest = argv[++i];
    } else if (strcmp(argv[i],"-o")==0){
      if ( i < argc-1 ) {
	tmps = argv[++i];
	if ( tmps == "list") {
	  op = KioslaveTest::List;
	} else if ( tmps == "get") {
	  op = KioslaveTest::Get;
	} else if ( tmps == "copy") {
	  op = KioslaveTest::Copy;
	} else if ( tmps == "move") {
	  op = KioslaveTest::Move;
	} else if ( tmps == "del") {
	  op = KioslaveTest::Delete;
	}
      }
    } else if (strcmp(argv[i],"-p")==0) {
      if ( i < argc-1 ) {
	tmps = argv[++i];
	if ( tmps == "none") {
	  op = KioslaveTest::ProgressNone;
	} else if ( tmps == "simple") {
	  op = KioslaveTest::ProgressSimple;
	} else if ( tmps == "list") {
	  op = KioslaveTest::ProgressList;
	} else if ( tmps == "little") {
	  op = KioslaveTest::ProgressLittle;
	}
      }
    } else {
      usage();
    }
  }

  KioslaveTest test( src, dest, op, pr );

  app->setMainWidget(kmain);
  app->exec();
}


#include "kioslavetest.moc"
