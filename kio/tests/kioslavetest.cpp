#include <qlayout.h>
#include <qmessagebox.h>

#include <kapp.h>
#include <klocale.h>
#include <kurl.h>

#include "kioslavetest.h"

KioslaveTest::KioslaveTest() : QWidget(0, "") {

  job = 0L;

  QBoxLayout *topLayout = new QVBoxLayout( this, 10, 5 );
	
  QGridLayout *grid = new QGridLayout( 2, 2, 10 );
  topLayout->addLayout( grid );

  grid->setRowStretch(0,1);
  grid->setRowStretch(1,1);
    
  grid->setColStretch(0,1);
  grid->setColStretch(1,100);

  lb_from = new QLabel( i18n("From :"), this );
  grid->addWidget( lb_from, 0, 0 );

  le_source = new QLineEdit( this );
  grid->addWidget( le_source, 0, 1 );

  lb_to = new QLabel( i18n("To :"), this );
  grid->addWidget( lb_to, 1, 0 );

  le_dest = new QLineEdit( this );
  grid->addWidget( le_dest, 1, 1 );

  // Operation groupbox & buttons
  opButtons = new QButtonGroup( i18n("Operation"), this );
  topLayout->addWidget( opButtons, 10 );
  connect( opButtons, SIGNAL(clicked(int)), SLOT(changeOperation(int)) );

  QBoxLayout *hbLayout = new QHBoxLayout(opButtons, 15 );

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

  opButtons->setButton( Copy );
  selectedOperation = Copy;

  // Progress groupbox & buttons
  progressButtons = new QButtonGroup( i18n("Progress dialog mode"), this );
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

  progressButtons->setButton( ProgressNone );
  progressMode = ProgressNone;

  // run & stop butons
  hbLayout = new QHBoxLayout( topLayout, 15 );

  pbStart = new QPushButton( i18n("Start"), this );
  pbStart->setFixedSize( pbStart->sizeHint() );
  connect( pbStart, SIGNAL(clicked()), SLOT(startJob()) );
  hbLayout->addWidget( pbStart, 5 );

  pbStop = new QPushButton( i18n("Stop"), this );
  pbStop->setFixedSize( pbStop->sizeHint() );
  pbStop->setEnabled( false );
  connect( pbStop, SIGNAL(clicked()), SLOT(stopJob()) );
  hbLayout->addWidget( pbStop, 5 );

  // close button
  close = new QPushButton( i18n("Close"), this );
  close->setFixedSize( close->sizeHint() );
  connect(close, SIGNAL(clicked()), kapp, SLOT(quit()));

  topLayout->addWidget( close, 5 );

  // my name is
  setCaption(i18n("Kioslave test"));

  show();
}


KioslaveTest::~KioslaveTest() {
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
	   SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigCanceled( int ) ),
	   SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ),
	   SLOT( slotError( int, int, const char* ) ) );

  pbStop->setEnabled( true );
}


void KioslaveTest::slotError( int, int errid, const char* errortext ) {
  QString msg = KIO::kioErrorString( errid, errortext );
  QMessageBox::critical(this, i18n("Kioslave Error Message"), msg );

  pbStart->setEnabled( true );
  pbStop->setEnabled( false );
}


void KioslaveTest::slotFinished( int ) {
  pbStart->setEnabled( true );
  pbStop->setEnabled( false );
}


void KioslaveTest::stopJob() {
  pbStop->setEnabled( false );

  job->kill();
  job = 0L;

  pbStart->setEnabled( true );
}


int main(int argc, char **argv) {
  KApplication app( argc, argv, "kioslavetest" );
  KioslaveTest test;

  app.setMainWidget(&test);

  return app.exec();
}

#include "kioslavetest.moc"
