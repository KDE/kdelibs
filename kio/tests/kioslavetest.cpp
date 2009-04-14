 /*
  This file is or will be part of KDE desktop environment

  Copyright 1999 Matt Koss <koss@miesto.sk>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtCore/QDir>
#include <QtGui/QGroupBox>

#include <unistd.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kstatusbar.h>
#include <kjobuidelegate.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/scheduler.h>
#include <kprotocolinfo.h>
#include <QtCore/QTimer>

#include "kioslavetest.h"

using namespace KIO;

KioslaveTest::KioslaveTest( QString src, QString dest, uint op, uint pr )
  : KMainWindow(0)
{

  job = 0L;

  main_widget = new QWidget( this );
  QBoxLayout *topLayout = new QVBoxLayout( main_widget );

  QGridLayout *grid = new QGridLayout();
  topLayout->addLayout( grid );

  grid->setRowStretch(0,1);
  grid->setRowStretch(1,1);

  grid->setColumnStretch(0,1);
  grid->setColumnStretch(1,100);

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

  QBoxLayout *hbLayout = new QHBoxLayout( box );

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
  connect( progressButtons, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(changeProgressMode(QAbstractButton*)) );

  hbLayout = new QHBoxLayout( box );

  rbProgressNone = new QRadioButton( "None", box );
  progressButtons->addButton( rbProgressNone );
  hbLayout->addWidget( rbProgressNone, 5 );

  rbProgressDefault = new QRadioButton( "Default", box );
  progressButtons->addButton( rbProgressDefault );
  hbLayout->addWidget( rbProgressDefault, 5 );

  rbProgressStatus = new QRadioButton( "Status", box );
  progressButtons->addButton( rbProgressStatus );
  hbLayout->addWidget( rbProgressStatus, 5 );

  b = progressButtons->buttons()[pr];
  b->setChecked( true );
  changeProgressMode( b );

  // statusbar progress widget
  statusTracker = new KStatusBarJobTracker( statusBar() );

  // run & stop butons
  hbLayout = new QHBoxLayout();
  topLayout->addLayout( hbLayout );
  hbLayout->setParent( topLayout );

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
//  slave = KIO::Scheduler::getConnectedSlave(KUrl("ftp://ftp.kde.org"));
  KIO::Scheduler::connect(SIGNAL(slaveConnected(KIO::Slave*)),
	this, SLOT(slotSlaveConnected()));
  KIO::Scheduler::connect(SIGNAL(slaveError(KIO::Slave*,int,const QString&)),
	this, SLOT(slotSlaveError()));
}

void KioslaveTest::slotQuit(){
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
  KUrl sCurrent( QUrl::fromLocalFile( QDir::currentPath() ) );
  QString sSrc( le_source->text() );
  KUrl src = KUrl( sCurrent, sSrc );

  if ( !src.isValid() ) {
    QMessageBox::critical(this, "Kioslave Error Message", "Source URL is malformed" );
    return;
  }

  QString sDest( le_dest->text() );
  KUrl dest( sCurrent, sDest );

  if ( !dest.isValid() &&
       ( selectedOperation == Copy || selectedOperation == Move ) ) {
    QMessageBox::critical(this, "Kioslave Error Message",
                       "Destination URL is malformed" );
    return;
  }

  pbStart->setEnabled( false );

  KIO::JobFlags observe = DefaultFlags;
  if (progressMode != ProgressDefault) {
    observe = HideProgressInfo;
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
    myJob = KIO::stat( src, KIO::StatJob::SourceSide, 2 );
    break;

  case Get:
    myJob = KIO::get( src, KIO::Reload );
    connect(myJob, SIGNAL( data( KIO::Job*, const QByteArray &)),
            SLOT( slotData( KIO::Job*, const QByteArray &)));
    break;

  case Put:
  {
    putBuffer = 0;
    KIO::TransferJob* tjob = KIO::put( src, -1, KIO::Overwrite );
    tjob->setTotalSize(48*1024*1024);
    myJob = tjob;
    connect(tjob, SIGNAL( dataReq( KIO::Job*, QByteArray &)),
            SLOT( slotDataReq( KIO::Job*, QByteArray &)));
    break;
  }

  case Copy:
    job = KIO::copy( src, dest, observe );
    break;

  case Move:
    job = KIO::move( src, dest, observe );
    break;

  case Delete:
    job = KIO::del( src, observe );
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

  statusBar()->addWidget( statusTracker->widget(job), 0 );

  connect( job, SIGNAL( result( KJob * ) ),
           SLOT( slotResult( KJob * ) ) );

  connect( job, SIGNAL( canceled( KJob * ) ),
           SLOT( slotResult( KJob * ) ) );

  if (progressMode == ProgressStatus) {
    statusTracker->registerJob( job );
  }

  pbStop->setEnabled( true );
}


void KioslaveTest::slotResult( KJob * _job )
{
  if ( _job->error() )
  {
      job->uiDelegate()->showErrorMessage();
  }
  else if ( selectedOperation == Stat )
  {
      UDSEntry entry = ((KIO::StatJob*)_job)->statResult();
      printUDSEntry( entry );
  }
  else if ( selectedOperation == Mimetype )
  {
      kDebug() << "mimetype is " << ((KIO::MimetypeJob*)_job)->mimetype();
  }

  if (job == _job)
     job = 0L;

  pbStart->setEnabled( true );
  pbStop->setEnabled( false );

  //statusBar()->removeWidget( statusTracker->widget(job) );
}

void KioslaveTest::slotSlaveConnected()
{
   kDebug() << "Slave connected.";
}

void KioslaveTest::slotSlaveError()
{
   kDebug() << "Error connected.";
   slave = 0;
}

void KioslaveTest::printUDSEntry( const KIO::UDSEntry & entry )
{
    // It's rather rare to iterate that way, usually you'd use numberValue/stringValue directly.
    // This is just to print out all that we got

    const QList<uint> keys = entry.listFields();
    QList<uint>::const_iterator it = keys.begin();
    for( ; it != keys.end(); it++ ) {
        switch ( *it ) {
            case KIO::UDSEntry::UDS_FILE_TYPE:
                {
                    mode_t mode = (mode_t)entry.numberValue(*it);
                    kDebug() << "File Type : " << mode;
                    if ( S_ISDIR( mode ) )
                    {
                        kDebug() << "is a dir";
                    }
                }
                break;
            case KIO::UDSEntry::UDS_ACCESS:
                kDebug() << "Access permissions : " << (mode_t)( entry.numberValue(*it) ) ;
                break;
            case KIO::UDSEntry::UDS_USER:
                kDebug() << "User : " << ( entry.stringValue(*it) );
                break;
            case KIO::UDSEntry::UDS_GROUP:
                kDebug() << "Group : " << ( entry.stringValue(*it) );
                break;
            case KIO::UDSEntry::UDS_NAME:
                kDebug() << "Name : " << ( entry.stringValue(*it) );
                //m_strText = decodeFileName( it.value().toString() );
                break;
            case KIO::UDSEntry::UDS_URL:
                kDebug() << "URL : " << ( entry.stringValue(*it) );
                break;
            case KIO::UDSEntry::UDS_MIME_TYPE:
                kDebug() << "MimeType : " << ( entry.stringValue(*it) );
                break;
            case KIO::UDSEntry::UDS_LINK_DEST:
                kDebug() << "LinkDest : " << ( entry.stringValue(*it) );
                break;
            case KIO::UDSEntry::UDS_SIZE:
                kDebug() << "Size: " << KIO::convertSize(entry.numberValue(*it));
                break;
        }
    }
}

void KioslaveTest::slotEntries(KIO::Job* job, const KIO::UDSEntryList& list) {

    KUrl url = static_cast<KIO::ListJob*>( job )->url();
    KProtocolInfo::ExtraFieldList extraFields = KProtocolInfo::extraFields(url);
    UDSEntryList::ConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
        // For each file...
        QString name = (*it).stringValue( KIO::UDSEntry::UDS_NAME );
        kDebug() << name;

        KProtocolInfo::ExtraFieldList::Iterator extraFieldsIt = extraFields.begin();
        const QList<uint> fields = it->listFields();
        QList<uint>::ConstIterator it2 = fields.begin();
        for( ; it2 != fields.end(); it2++ ) {
            if ( *it2 >= UDSEntry::UDS_EXTRA && *it2 <= UDSEntry::UDS_EXTRA_END) {
                if ( extraFieldsIt != extraFields.end() ) {
                    QString column = (*extraFieldsIt).name;
                    //QString type = (*extraFieldsIt).type;
                    kDebug() << "  Extra data (" << column << ") :" << it->stringValue(*it2);
                    ++extraFieldsIt;
                } else {
                    kDebug() << "  Extra data (UNDEFINED) :" << it->stringValue(*it2);
                }
            }
        }
    }
}

void KioslaveTest::slotData(KIO::Job*, const QByteArray &data)
{
    if (data.size() == 0)
    {
       kDebug(0) << "Data: <End>";
    }
    else
    {
       kDebug(0) << "Data: \"" << QString( data ) << "\"";
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
	 "BIG1\n",
	 "BIG2\n",
	 "BIG3\n",
	 "BIG4\n",
	 "BIG5\n",
         0
       };
    const char *fileData = fileDataArray[putBuffer++];

    if (!fileData)
    {
       kDebug(0) << "DataReq: <End>";
       return;
    }
    if (!strncmp(fileData, "BIG", 3))
	data.fill(0, 8*1024*1024);
    else
	data = QByteArray(fileData, strlen(fileData));
    kDebug(0) << "DataReq: \"" << fileData << "\"";
    sleep(1); // want to see progress info...
}

void KioslaveTest::stopJob() {
  kDebug() << "KioslaveTest::stopJob()";
  job->kill();
  job = 0L;

  pbStop->setEnabled( false );
  pbStart->setEnabled( true );
}

int main(int argc, char **argv) {

  KCmdLineOptions options;
  options.add("s");
  options.add("src <src>", ki18n("Source URL"), QByteArray());
  options.add("d");
  options.add("dest <dest>", ki18n("Destination URL"), QByteArray());
  options.add("o");
  options.add("operation <operation>", ki18n("Operation (list,listrecursive,stat,get,put,copy,move,del,mkdir)"));
  options.add("p");
  options.add("progress <progress>", ki18n("Progress Type (none,default,status)"), QByteArray("default"));

  const char version[] = "v0.0.0 0000";   // :-)
  KLocalizedString description = ki18n("Test for kioslaves");

  KCmdLineArgs::init( argc, argv, "kioslavetest", 0, ki18n("KIOSlave test"), version, description );
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString src = args->getOption("src");
  QString dest = args->getOption("dest");

  uint op = KioslaveTest::Copy;
  uint pr = 0;

  QString operation = args->getOption("operation");
  if ( operation == "list") {
    op = KioslaveTest::List;
  } else if ( operation == "listrecursive") {
    op = KioslaveTest::ListRecursive;
  } else if ( operation == "stat") {
    op = KioslaveTest::Stat;
  } else if ( operation == "get") {
    op = KioslaveTest::Get;
  } else if ( operation == "put") {
    op = KioslaveTest::Put;
  } else if ( operation == "copy") {
    op = KioslaveTest::Copy;
  } else if ( operation == "move") {
    op = KioslaveTest::Move;
  } else if ( operation == "del") {
    op = KioslaveTest::Delete;
  } else if ( operation == "mkdir") {
    op = KioslaveTest::Mkdir;
  } else if (!operation.isEmpty()) {
    KCmdLineArgs::usage(QByteArray("unknown operation"));
  }

  QString progress = args->getOption("progress");
  if ( progress == "none") {
    pr = KioslaveTest::ProgressNone;
  } else if ( progress == "default") {
    pr = KioslaveTest::ProgressDefault;
  } else if ( progress == "status") {
    pr = KioslaveTest::ProgressStatus;
  } else KCmdLineArgs::usage(QByteArray("unknown progress mode"));

  args->clear(); // Free up memory

  KioslaveTest* test = new KioslaveTest( src, dest, op, pr );
  if (!operation.isEmpty())
      QTimer::singleShot(100, test, SLOT(startJob()));
  test->show();
  test->resize( test->sizeHint() );

  app.exec();
}


#include "kioslavetest.moc"
