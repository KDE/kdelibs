 /*
  This file is or will be part of KDE desktop environment

  Copyright 1999 Matt Koss <koss@miesto.sk>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <qlayout.h>
#include <qmessagebox.h>
#include <qdir.h>

#include <kapp.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio_job.h>

#include "kioslavetest.h"

using namespace KIO;

KioslaveTest *kmain;

KioslaveTest::KioslaveTest( QString src, QString dest, uint op, uint pr )
  : KTMainWindow("")
{

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
  opButtons = new QButtonGroup( "Operation", main_widget );
  topLayout->addWidget( opButtons, 10 );
  connect( opButtons, SIGNAL(clicked(int)), SLOT(changeOperation(int)) );

  QBoxLayout *hbLayout = new QHBoxLayout( opButtons, 15 );

  rbList = new QRadioButton( "List", opButtons );
  opButtons->insert( rbList, List );
  hbLayout->addWidget( rbList, 5 );

  rbListRecursive = new QRadioButton( "ListRecursive", opButtons );
  opButtons->insert( rbListRecursive, ListRecursive );
  hbLayout->addWidget( rbListRecursive, 5 );

  rbStat = new QRadioButton( "Stat", opButtons );
  opButtons->insert( rbStat, Stat );
  hbLayout->addWidget( rbStat, 5 );

  rbGet = new QRadioButton( "Get", opButtons );
  opButtons->insert( rbGet, Get );
  hbLayout->addWidget( rbGet, 5 );

  rbPut = new QRadioButton( "Put", opButtons );
  opButtons->insert( rbPut, Put );
  hbLayout->addWidget( rbPut, 5 );

  rbCopy = new QRadioButton( "Copy", opButtons );
  opButtons->insert( rbCopy, Copy );
  hbLayout->addWidget( rbCopy, 5 );

  rbMove = new QRadioButton( "Move", opButtons );
  opButtons->insert( rbMove, Move );
  hbLayout->addWidget( rbMove, 5 );

  rbDelete = new QRadioButton( "Delete", opButtons );
  opButtons->insert( rbDelete, Delete );
  hbLayout->addWidget( rbDelete, 5 );

  rbMkdir = new QRadioButton( "Mkdir", opButtons );
  opButtons->insert( rbMkdir, Mkdir );
  hbLayout->addWidget( rbMkdir, 5 );

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

  //littleProgress = new KIOLittleProgressDlg( statusBar() );
  //statusBar()->insertWidget( littleProgress, littleProgress->width() , 0 );

  kmain = this;

  show();
}


void KioslaveTest::closeEvent( QCloseEvent * ){
  kapp->quit();
}


void KioslaveTest::changeOperation( int id ) {
  // only two urls for copy and move
  bool enab = rbCopy->isChecked() ||
    rbMove->isChecked();

  le_dest->setEnabled( enab );

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
  KURL current;
  current.setPath(QDir::currentDirPath());
  QString sSrc( le_source->text() );
  KURL src( current, sSrc );

  if ( src.isMalformed() ) {
    QMessageBox::critical(this, i18n("Kioslave Error Message"), i18n("Source URL is malformed") );
    return;
  }

  QString sDest( le_dest->text() );
  KURL dest( current, sDest );

  if ( dest.isMalformed() &&
       ( selectedOperation == Copy || selectedOperation == Move ) ) {
    QMessageBox::critical(this, i18n("Kioslave Error Message"),
		       i18n("Destination URL is malformed") );
    return;
  }

  pbStart->setEnabled( false );

  /*
  job = new KIOJob;
  job->cacheToPool(true);
  */

  /*
  switch ( progressMode ) {
  case ProgressSimple:
    job->setGUImode( KIOJob::SIMPLE );
    break;

  case ProgressList:
    job->setGUImode( KIOJob::LIST );
    break;

  case ProgressLittle:
    job->setGUImode( KIOJob::CUSTOM );
    job->setProgressDlg( littleProgress );
    break;

  default:
    job->setGUImode( KIOJob::NONE );
    break;
  }
  */

  switch ( selectedOperation ) {
  case List:
    job = KIO::listDir( src );
    connect(job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
            SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)));
    break;

  case ListRecursive:
    job = KIO::listRecursive( src );
    connect(job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
            SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)));
    break;

  case Stat:
    job = KIO::stat( src );
    break;

  case Get:
    job = KIO::get( src );
    connect(job, SIGNAL( data( KIO::Job*, const QByteArray &)),
            SLOT( slotData( KIO::Job*, const QByteArray &)));
    break;

  case Put:
    putBuffer = 0;
    job = KIO::put( src, -1, true, false);
    connect(job, SIGNAL( dataReq( KIO::Job*, QByteArray &)),
            SLOT( slotDataReq( KIO::Job*, QByteArray &)));
    break;

  case Copy:
    job = KIO::copy( src, dest );
    break;

  case Move:
    job = KIO::move( src, dest );
    break;

  case Delete:
    job = KIO::del( src );
    break;

  case Mkdir:
    job = KIO::mkdir( src );
    break;
  }

  connect( job, SIGNAL( result( KIO::Job * ) ),
	   SLOT( slotResult( KIO::Job * ) ) );

  pbStop->setEnabled( true );
}


void KioslaveTest::slotResult( KIO::Job * job )
{
  if ( job->error() )
  {
    job->showErrorDialog();
  }
  else if ( selectedOperation == Stat )
  {
      UDSEntry entry = ((KIO::StatJob*)job)->statResult();
      printUDSEntry( entry );
  }
  pbStart->setEnabled( true );
  pbStop->setEnabled( false );
}

void KioslaveTest::printUDSEntry( const KIO::UDSEntry & entry )
{
    KIO::UDSEntry::ConstIterator it = entry.begin();
    for( ; it != entry.end(); it++ ) {
        switch ((*it).m_uds) {
            case KIO::UDS_FILE_TYPE:
                kDebugInfo("File Type : %d", (mode_t)((*it).m_long) );
                if ( S_ISDIR( (mode_t)((*it).m_long) ) )
                {
                    kDebugInfo("is a dir");
                }
                break;
            case KIO::UDS_ACCESS:
                kDebugInfo("Access permissions : %d", (mode_t)((*it).m_long) );
                break;
            case KIO::UDS_USER:
                kDebugInfo("User : %s", ((*it).m_str.ascii() ) );
                break;
            case KIO::UDS_GROUP:
                kDebugInfo("Group : %s", ((*it).m_str.ascii() ) );
                break;
            case KIO::UDS_NAME:
                kDebugInfo("Name : %s", ((*it).m_str.ascii() ) );
                //m_strText = decodeFileName( (*it).m_str );
                break;
            case KIO::UDS_URL:
                kDebugInfo("URL : %s", ((*it).m_str.ascii() ) );
                break;
            case KIO::UDS_MIME_TYPE:
                kDebugInfo("MimeType : %s", ((*it).m_str.ascii() ) );
                break;
            case KIO::UDS_LINK_DEST:
                kDebugInfo("LinkDest : %s", ((*it).m_str.ascii() ) );
                break;
        }
    }
}

void KioslaveTest::slotEntries(KIO::Job*, const KIO::UDSEntryList& list) {

    UDSEntryListIterator it(list);
    for (; it.current(); ++it) {
        UDSEntry::ConstIterator it2 = it.current()->begin();
        for( ; it2 != it.current()->end(); it2++ ) {
            if ((*it2).m_uds == UDS_NAME)
                kDebugInfo( "%s", ( *it2 ).m_str.latin1() );
        }
    }
}

void KioslaveTest::slotData(KIO::Job*, const QByteArray &data)
{
    if (data.size() == 0)
    {
       kDebugInfo( 0, "Data: <End>");
    }
    else
    {
       QCString c_string(data.data(), data.size()); // Make it 0-terminated!
       kDebugInfo( 0, "Data: \"%s\"", c_string.data() );
    }
}

void KioslaveTest::slotDataReq(KIO::Job*, QByteArray &data)
{
    const char *fileDataArray[] =
       { 
         "Hello world\n", 
         "This is a test file\n", 
         "You can safely delete it.\n",
         0
       };
    const char *fileData = fileDataArray[putBuffer++];
 
    if (!fileData) 
    {
       kDebugInfo( 0, "DataReq: <End>");
       return;
    }
    data.duplicate(fileData, strlen(fileData));
    kDebugInfo( 0, "DataReq: \"%s\"", fileData );
}

void KioslaveTest::stopJob() {
  pbStop->setEnabled( false );

  // TODO  job->kill();
  job = 0L;

  pbStart->setEnabled( true );
}

static const char *version = "v0.0.0 0000";   // :-)
static const char *description = "Test for kioslaves";
static KCmdLineOptions options[] =
{
 { "s", 0, 0 },
 { "src <src>", "Source URL", "" },
 { "d", 0, 0 },
 { "dest <dest>", "Destination URL", "" },
 { "o", 0, 0 },
 { "operation <operation>", "Operation (list,listrecursive,stat,get,copy,move,del,mkdir)", "copy" },
 { "p", 0, 0 },
 { "progress <progress>", "Progress Type (none,simple,list,little)", "simple" }
};

int main(int argc, char **argv) {
  KCmdLineArgs::init( argc, argv, "kioslavetest", description, version );
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString src = args->getOption("src");
  QString dest = args->getOption("dest");

  uint op = 0;
  uint pr = 0;

  QString tmps;

  tmps = args->getOption("operation");
  if ( tmps == "list") {
    op = KioslaveTest::List;
  } else if ( tmps == "listrecursive") {
    op = KioslaveTest::ListRecursive;
  } else if ( tmps == "stat") {
    op = KioslaveTest::Stat;
  } else if ( tmps == "get") {
    op = KioslaveTest::Get;
  } else if ( tmps == "copy") {
    op = KioslaveTest::Copy;
  } else if ( tmps == "move") {
    op = KioslaveTest::Move;
  } else if ( tmps == "del") {
    op = KioslaveTest::Delete;
  } else if ( tmps == "mkdir") {
    op = KioslaveTest::Mkdir;
  } else KCmdLineArgs::usage("unknown operation");

  tmps = args->getOption("progress");
  if ( tmps == "none") {
    pr = KioslaveTest::ProgressNone;
  } else if ( tmps == "simple") {
    pr = KioslaveTest::ProgressSimple;
  } else if ( tmps == "list") {
    pr = KioslaveTest::ProgressList;
  } else if ( tmps == "little") {
    pr = KioslaveTest::ProgressLittle;
  } else KCmdLineArgs::usage("unknown progress mode");

  args->clear(); // Free up memory

  KioslaveTest test( src, dest, op, pr );

  app.setMainWidget(kmain);
  app.exec();
}


#include "kioslavetest.moc"
