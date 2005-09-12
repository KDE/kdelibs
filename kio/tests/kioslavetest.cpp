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
#include <qgroupbox.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kstatusbar.h>
#include <kio/job.h>
#include <kio/scheduler.h>
#include <kprotocolinfo.h>
#include <qtimer.h>

#include "kioslavetest.h"

using namespace KIO;

KioslaveTest::KioslaveTest( QString src, QString dest, uint op, uint pr )
  : KMainWindow(0, "")
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

  lb_from = new QLabel( "From:", main_widget );
  grid->addWidget( lb_from, 0, 0 );

  le_source = new QLineEdit( main_widget );
  grid->addWidget( le_source, 0, 1 );
  le_source->setText( src );

  lb_to = new QLabel( "To:", main_widget );
  grid->addWidget( lb_to, 1, 0 );

  le_dest = new QLineEdit( main_widget );
  grid->addWidget( le_dest, 1, 1 );
  le_dest->setText( dest );

  // Operation groupbox & buttons
  opButtons = new QButtonGroup( main_widget );
  QGroupBox *box = new QGroupBox( "Operation", main_widget );
  topLayout->addWidget( box, 10 );
  connect( opButtons, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(changeOperation(QAbstractButton*)) );

  QBoxLayout *hbLayout = new QHBoxLayout( box, 15 );

  rbList = new QRadioButton( "List", box );
  opButtons->addButton( rbList );
  hbLayout->addWidget( rbList, 5 );

  rbListRecursive = new QRadioButton( "ListRecursive", box );
  opButtons->addButton( rbListRecursive );
  hbLayout->addWidget( rbListRecursive, 5 );

  rbStat = new QRadioButton( "Stat", box );
  opButtons->addButton( rbStat );
  hbLayout->addWidget( rbStat, 5 );

  rbGet = new QRadioButton( "Get", box );
  opButtons->addButton( rbGet );
  hbLayout->addWidget( rbGet, 5 );

  rbPut = new QRadioButton( "Put", box );
  opButtons->addButton( rbPut );
  hbLayout->addWidget( rbPut, 5 );

  rbCopy = new QRadioButton( "Copy", box );
  opButtons->addButton( rbCopy );
  hbLayout->addWidget( rbCopy, 5 );

  rbMove = new QRadioButton( "Move", box );
  opButtons->addButton( rbMove );
  hbLayout->addWidget( rbMove, 5 );

  rbDelete = new QRadioButton( "Delete", box );
  opButtons->addButton( rbDelete );
  hbLayout->addWidget( rbDelete, 5 );

  rbShred = new QRadioButton( "Shred", box );
  opButtons->addButton( rbShred );
  hbLayout->addWidget( rbShred, 5 );

  rbMkdir = new QRadioButton( "Mkdir", box );
  opButtons->addButton( rbMkdir );
  hbLayout->addWidget( rbMkdir, 5 );

  rbMimetype = new QRadioButton( "Mimetype", box );
  opButtons->addButton( rbMimetype );
  hbLayout->addWidget( rbMimetype, 5 );

  QAbstractButton *b = opButtons->buttons()[op];
  b->setChecked( true );
  changeOperation( b );

  // Progress groupbox & buttons
  progressButtons = new QButtonGroup( main_widget );
  box = new QGroupBox( "Progress dialog mode", main_widget );
  topLayout->addWidget( box, 10 );
  connect( progressButtons, SIGNAL(clicked(int)), SLOT(changeProgressMode(int)) );

  hbLayout = new QHBoxLayout( box, 15 );

  rbProgressNone = new QRadioButton( "None", box );
  progressButtons->insert( rbProgressNone );
  hbLayout->addWidget( rbProgressNone, 5 );

  rbProgressDefault = new QRadioButton( "Default", box );
  progressButtons->insert( rbProgressDefault );
  hbLayout->addWidget( rbProgressDefault, 5 );

  rbProgressStatus = new QRadioButton( "Status", box );
  progressButtons->insert( rbProgressStatus );
  hbLayout->addWidget( rbProgressStatus, 5 );

  b = progressButtons->buttons()[pr];
  b->setChecked( true );
  changeProgressMode( b );

  // statusbar progress widget
  statusProgress = new StatusbarProgress( statusBar() );
  statusBar()->addWidget( statusProgress, 0, true );

  // run & stop butons
  hbLayout = new QHBoxLayout( topLayout, 15 );

  pbStart = new QPushButton( "&Start", main_widget );
  pbStart->setFixedSize( pbStart->sizeHint() );
  connect( pbStart, SIGNAL(clicked()), SLOT(startJob()) );
  hbLayout->addWidget( pbStart, 5 );

  pbStop = new QPushButton( "Sto&p", main_widget );
  pbStop->setFixedSize( pbStop->sizeHint() );
  pbStop->setEnabled( false );
  connect( pbStop, SIGNAL(clicked()), SLOT(stopJob()) );
  hbLayout->addWidget( pbStop, 5 );

  // close button
  close = new QPushButton( "&Close", main_widget );
  close->setFixedSize( close->sizeHint() );
  connect(close, SIGNAL(clicked()), this, SLOT(slotQuit()));

  topLayout->addWidget( close, 5 );

  main_widget->setMinimumSize( main_widget->sizeHint() );
  setCentralWidget( main_widget );

  slave = 0;
//  slave = KIO::Scheduler::getConnectedSlave(KURL("ftp://ftp.kde.org"));
  KIO::Scheduler::connect(SIGNAL(slaveConnected(KIO::Slave*)),
	this, SLOT(slotSlaveConnected()));
  KIO::Scheduler::connect(SIGNAL(slaveError(KIO::Slave*,int,const QString&)),
	this, SLOT(slotSlaveError()));
}


void KioslaveTest::closeEvent( QCloseEvent * ){
  slotQuit();
}


void KioslaveTest::slotQuit(){
  if ( job ) {
    job->kill( true );  // kill the job quietly
  }
  if (slave)
    KIO::Scheduler::disconnectSlave(slave);
  qApp->quit();
}


void KioslaveTest::changeOperation( QAbstractButton *b ) {
  // only two urls for copy and move
  bool enab = rbCopy->isChecked() || rbMove->isChecked();

  le_dest->setEnabled( enab );

  selectedOperation = opButtons->buttons().indexOf( b );
}


void KioslaveTest::changeProgressMode( QAbstractButton *b ) {
  progressMode = progressButtons->buttons().indexOf( b ); 

  if ( progressMode == ProgressStatus ) {
    statusBar()->show();
  } else {
    statusBar()->hide();
  }
}


void KioslaveTest::startJob() {
  KURL sCurrent = KURL::fromPathOrURL (QDir::currentDirPath() );
  QString sSrc( le_source->text() );
  KURL src = KURL( sCurrent, sSrc );

  if ( !src.isValid() ) {
    QMessageBox::critical(this, "Kioslave Error Message", "Source URL is malformed" );
    return;
  }

  QString sDest( le_dest->text() );
  KURL dest( sCurrent, sDest );

  if ( !dest.isValid() &&
       ( selectedOperation == Copy || selectedOperation == Move ) ) {
    QMessageBox::critical(this, "Kioslave Error Message",
                       "Destination URL is malformed" );
    return;
  }

  pbStart->setEnabled( false );

  bool observe = true;
  if (progressMode != ProgressDefault) {
    observe = false;
  }

  SimpleJob *myJob = 0;

  switch ( selectedOperation ) {
  case List:
    myJob = KIO::listDir( src );
    connect(myJob, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
            SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)));
    break;

  case ListRecursive:
    myJob = KIO::listRecursive( src );
    connect(myJob, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
            SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)));
    break;

  case Stat:
    myJob = KIO::stat( src, false, 2 );
    break;

  case Get:
    myJob = KIO::get( src, true );
    connect(myJob, SIGNAL( data( KIO::Job*, const QByteArray &)),
            SLOT( slotData( KIO::Job*, const QByteArray &)));
    break;

  case Put:
    putBuffer = 0;
    myJob = KIO::put( src, -1, true, false);
    connect(myJob, SIGNAL( dataReq( KIO::Job*, QByteArray &)),
            SLOT( slotDataReq( KIO::Job*, QByteArray &)));
    break;

  case Copy:
    job = KIO::copy( src, dest, observe );
    break;

  case Move:
    job = KIO::move( src, dest, observe );
    break;

  case Delete:
    job = KIO::del( src, false, observe );
    break;

  case Shred:
    job = KIO::del(src, true, observe);
    break;

  case Mkdir:
    myJob = KIO::mkdir( src );
    break;

  case Mimetype:
    myJob = KIO::mimetype( src );
    break;
  }
  if (myJob)
  {
    if (slave)
      KIO::Scheduler::assignJobToSlave(slave, myJob);
    job = myJob;
  }

  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );

  connect( job, SIGNAL( canceled( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );

  if (progressMode == ProgressStatus) {
    statusProgress->setJob( job );
  }

  pbStop->setEnabled( true );
}


void KioslaveTest::slotResult( KIO::Job * _job )
{
  if ( _job->error() )
  {
    _job->showErrorDialog();
  }
  else if ( selectedOperation == Stat )
  {
      UDSEntry entry = ((KIO::StatJob*)_job)->statResult();
      printUDSEntry( entry );
  }
  else if ( selectedOperation == Mimetype )
  {
      kdDebug() << "mimetype is " << ((KIO::MimetypeJob*)_job)->mimetype() << endl;
  }

  if (job == _job)
     job = 0L;
  pbStart->setEnabled( true );
  pbStop->setEnabled( false );
}

void KioslaveTest::slotSlaveConnected()
{
   kdDebug() << "Slave connected." << endl;
}

void KioslaveTest::slotSlaveError()
{
   kdDebug() << "Error connected." << endl;
   slave = 0;
}

void KioslaveTest::printUDSEntry( const KIO::UDSEntry & entry )
{
    KIO::UDSEntry::ConstIterator it = entry.begin();
    for( ; it != entry.end(); it++ ) {
        switch ((*it).m_uds) {
            case KIO::UDS_FILE_TYPE:
                kdDebug() << "File Type : " << (mode_t)((*it).m_long) << endl;
                if ( S_ISDIR( (mode_t)((*it).m_long) ) )
                {
                    kdDebug() << "is a dir" << endl;
                }
                break;
            case KIO::UDS_ACCESS:
                kdDebug() << "Access permissions : " << (mode_t)((*it).m_long) << endl;
                break;
            case KIO::UDS_USER:
                kdDebug() << "User : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_GROUP:
                kdDebug() << "Group : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_NAME:
                kdDebug() << "Name : " << ((*it).m_str.ascii() ) << endl;
                //m_strText = decodeFileName( (*it).m_str );
                break;
            case KIO::UDS_URL:
                kdDebug() << "URL : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_MIME_TYPE:
                kdDebug() << "MimeType : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_LINK_DEST:
                kdDebug() << "LinkDest : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_SIZE:
                kdDebug() << "Size: " << KIO::convertSize((*it).m_long) << endl;
                break;
        }
    }
}

void KioslaveTest::slotEntries(KIO::Job* job, const KIO::UDSEntryList& list) {

    KURL url = static_cast<KIO::ListJob*>( job )->url();
    KProtocolInfo::ExtraFieldList extraFields = KProtocolInfo::extraFields(url);
    UDSEntryListConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
        // For each file...
        KProtocolInfo::ExtraFieldList::Iterator extraFieldsIt = extraFields.begin();
        UDSEntry::ConstIterator it2 = (*it).begin();
        for( ; it2 != (*it).end(); it2++ ) {
            if ((*it2).m_uds == UDS_NAME)
                kdDebug() << "" << ( *it2 ).m_str << endl;
            else if ( (*it2).m_uds == UDS_EXTRA) {
                if ( extraFieldsIt != extraFields.end() ) {
                    QString column = (*extraFieldsIt).name;
                    //QString type = (*extraFieldsIt).type;
                    kdDebug() << "  Extra data (" << column << ") :" << ( *it2 ).m_str << endl;
                    ++extraFieldsIt;
                } else {
                    kdDebug() << "  Extra data (UNDEFINED) :" << ( *it2 ).m_str << endl;
                }
            }
        }
    }
}

void KioslaveTest::slotData(KIO::Job*, const QByteArray &data)
{
    if (data.size() == 0)
    {
       kdDebug(0) << "Data: <End>" << endl;
    }
    else
    {
       kdDebug(0) << "Data: \"" << QString( data ) << "\"" << endl;
    }
}

void KioslaveTest::slotDataReq(KIO::Job*, QByteArray &data)
{
    const char *fileDataArray[] =
       {
         "Hello world\n",
         "This is a test file\n",
         "You can safely delete it.\n",
	 "BIG\n",
         0
       };
    const char *fileData = fileDataArray[putBuffer++];

    if (!fileData)
    {
       kdDebug(0) << "DataReq: <End>" << endl;
       return;
    }
    if (!strcmp(fileData, "BIG\n"))
	data.fill(0, 29*1024*1024);
    else
	data.duplicate(fileData, strlen(fileData));
    kdDebug(0) << "DataReq: \"" << fileData << "\"" << endl;
}

void KioslaveTest::stopJob() {
  kdDebug() << "KioslaveTest::stopJob()" << endl;
  job->kill();
  job = 0L;

  pbStop->setEnabled( false );
  pbStart->setEnabled( true );
}

static const char version[] = "v0.0.0 0000";   // :-)
static const char description[] = "Test for kioslaves";
static KCmdLineOptions options[] =
{
 { "s", 0, 0 },
 { "src <src>", "Source URL", "" },
 { "d", 0, 0 },
 { "dest <dest>", "Destination URL", "" },
 { "o", 0, 0 },
 { "operation <operation>", "Operation (list,listrecursive,stat,get,put,copy,move,del,shred,mkdir)", "copy" },
 { "p", 0, 0 },
 { "progress <progress>", "Progress Type (none,default,status)", "default" },
 KCmdLineLastOption
};

int main(int argc, char **argv) {
  KCmdLineArgs::init( argc, argv, "kioslavetest", "KIOSlave test", description, version );
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
  } else if ( tmps == "put") {
    op = KioslaveTest::Put;
  } else if ( tmps == "copy") {
    op = KioslaveTest::Copy;
  } else if ( tmps == "move") {
    op = KioslaveTest::Move;
  } else if ( tmps == "del") {
    op = KioslaveTest::Delete;
  } else if ( tmps == "shred") {
    op = KioslaveTest::Shred;
  } else if ( tmps == "mkdir") {
    op = KioslaveTest::Mkdir;
  } else KCmdLineArgs::usage("unknown operation");

  tmps = args->getOption("progress");
  if ( tmps == "none") {
    pr = KioslaveTest::ProgressNone;
  } else if ( tmps == "default") {
    pr = KioslaveTest::ProgressDefault;
  } else if ( tmps == "status") {
    pr = KioslaveTest::ProgressStatus;
  } else KCmdLineArgs::usage("unknown progress mode");

  args->clear(); // Free up memory

  KioslaveTest test( src, dest, op, pr );
  QTimer::singleShot(100, &test, SLOT(startJob()));
  test.show();
  // Bug in KTMW / Qt / layouts ?
  test.resize( test.sizeHint() );

  app.setMainWidget(&test);
  app.exec();
}


#include "kioslavetest.moc"
