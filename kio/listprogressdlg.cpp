// $Id$

#include <qtimer.h>
#include <qmessagebox.h>

#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kwm.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <ktoolbarbutton.h>

#include "kio_listprogress_dlg.h"

static int defaultColumnWidth[] = { 70,  // SIZE_OPERATION
				    160, // LOCAL_FILENAME
				    40,  // RESUME
				    60,  // COUNT
				    30,  // PROGRESS
				    65,  // TOTAL
				    70,  // SPEED
				    70,  // REMAINING_TIME
				    450  // URL
};

#define INIT_MAX_ITEMS 16
#define ARROW_SPACE 15
#define BUTTON_SPACE 4
#define MINIMUM_SPACE 9

#define NUM_COLS  9

KIOListViewItem::KIOListViewItem( KIOListView* view, KIOJob *job )
  : QListViewItem( view ) {

  listView = view;
  m_pJob = job;
  connect( m_pJob, SIGNAL( sigSpeed( int, unsigned long ) ),
	   SLOT( slotSpeed( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalSize( int, unsigned long ) ),
	   SLOT( slotTotalSize( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalFiles( int, unsigned long ) ),
	   SLOT( slotTotalFiles( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalDirs( int, unsigned long ) ),
	   SLOT( slotTotalDirs( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigPercent( int, unsigned long ) ),
	   SLOT( slotPercent( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigProcessedFiles( int, unsigned long ) ),
	   SLOT( slotProcessedFiles( int, unsigned long ) ) );
//   connect( m_pJob, SIGNAL( sigProcessedDirs( int, unsigned long ) ),
// 	   SLOT( slotProcessedDirs( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigCopying( int, const char*, const char* ) ),
	   SLOT( slotCopyingFile( int, const char*, const char* ) ) );
  connect( m_pJob, SIGNAL( sigCanResume( int, bool ) ),
 	   SLOT( slotCanResume( int, bool ) ) );
  connect( m_pJob, SIGNAL( sigScanningDir( int, const char* ) ),
	   SLOT( slotScanningDir( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigMakingDir( int, const char* ) ),
	   SLOT( slotMakingDir( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigGettingFile( int, const char* ) ),
	   SLOT( slotGettingFile( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigDeletingFile( int, const char* ) ),
	   SLOT( slotDeletingFile( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigRenamed( int, const char* ) ),
	   SLOT( slotRenamed( int, const char* ) ) );

  connect( job, SIGNAL( sigFinished( int ) ),
	   SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigCanceled( int ) ),
	   SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ),
	   SLOT( slotFinished( int ) ) );
}


void KIOListViewItem::update() {
  QString tmps;

  // set operation
  switch ( m_pJob->m_cmd ) {
  case CMD_COPY:
  case CMD_MCOPY:
    tmps = i18n("Copying");
    break;
  case CMD_MOVE:
  case CMD_MMOVE:
    tmps = i18n("Moving");
    break;
  case CMD_DEL:
  case CMD_MDEL:
    tmps = i18n("Deleting");
    break;
  case CMD_GET:
    tmps = i18n("Getting");
    break;
  case CMD_MOUNT:
    tmps = i18n("Mounting");
    break;
  case CMD_UNMOUNT:
    tmps = i18n("Umnounting");
    break;
  case CMD_MKDIR:
    tmps = i18n("New dir");
    break;
  }
  setText( listView->lv_operation, tmps );
}


void KIOListViewItem::slotTotalSize( int, unsigned long _bytes ) {
  m_iTotalSize = _bytes;
}


void KIOListViewItem::slotTotalFiles( int, unsigned long _files ) {
  m_iTotalFiles = _files;
}


void KIOListViewItem::slotTotalDirs( int, unsigned long _dirs ) {
  m_iTotalDirs = _dirs;
}


void KIOListViewItem::slotPercent( int, unsigned long _percent ) {
  QString tmps = i18n( "%1 % of %2 ").arg( _percent ).arg( KIOJob::convertSize(m_iTotalSize));

  setText( listView->lv_progress, tmps );
}


void KIOListViewItem::slotProcessedFiles( int, unsigned long _files ) {
  QString tmps;
  tmps.sprintf( "%lu / %lu\n", _files, m_iTotalFiles );
  setText( listView->lv_count, tmps );
}


void KIOListViewItem::slotSpeed( int, unsigned long _bytes_per_second ) {
  QString tmps, tmps2;
  if ( _bytes_per_second == 0 ) {
    tmps = i18n( "Stalled");
    tmps2 = tmps;
  } else {
    tmps = i18n( "%1/s %2").arg( KIOJob::convertSize( _bytes_per_second ));
    tmps2 = m_pJob->getRemainingTime().toString();
  }

  setText( listView->lv_speed, tmps );
  setText( listView->lv_remaining, tmps2 );
}


void KIOListViewItem::slotScanningDir( int , const char *_dir) {
  KURL url( _dir );
  setText( listView->lv_operation, i18n("Scanning") );
  setText( listView->lv_url, _dir );
  setText( listView->lv_filename, url.filename() );
}


void KIOListViewItem::slotCopyingFile( int, const char *_from, const char *_to ) {
  KURL url( _to );
  setText( listView->lv_operation, i18n("Copying") );
  setText( listView->lv_url, _from );
  setText( listView->lv_filename, url.filename() );
}


void KIOListViewItem::slotRenamed( int, const char *_new_url ) {
  KURL url( _new_url );
  setText( listView->lv_filename, url.filename() );
}


void KIOListViewItem::slotMakingDir( int, const char *_dir ) {
  KURL url( _dir );
  setText( listView->lv_operation, i18n("Creating") );
  setText( listView->lv_url, _dir );
  setText( listView->lv_filename, url.filename() );
}


void KIOListViewItem::slotGettingFile( int, const char *_url ) {
  KURL url( _url );
  setText( listView->lv_operation, i18n("Fetching") );
  setText( listView->lv_url, _url );
  setText( listView->lv_filename, url.filename() );
}


void KIOListViewItem::slotDeletingFile( int, const char *_url ) {
  KURL url( _url );
  setText( listView->lv_operation, i18n("Deleting") );
  setText( listView->lv_url, _url );
  setText( listView->lv_filename, url.filename() );
}


void KIOListViewItem::slotCanResume( int, bool _resume ) {
  QString tmps;
  // set canResume
  if ( _resume ) {
    tmps = i18n("Yes");
  } else {
    tmps = i18n("No");
  }
  setText( listView->lv_resume, tmps );
}


void KIOListViewItem::slotFinished( int ) {
  emit statusChanged( this );
}


//-----------------------------------------------------------------------------

KIOListView::KIOListView (QWidget *parent, const char *name)
  : QListView (parent, name) {

  // enable selection of more than one item
  setMultiSelection( true );

  setAllColumnsShowFocus( true );

  lv_operation = addColumn( i18n("Operation") );
  lv_filename = addColumn( i18n("Local Filename") );
  lv_resume = addColumn( i18n("Res.") );
  lv_count = addColumn( i18n("Count") );
  lv_progress = addColumn( i18n("%") );
  lv_total = addColumn( i18n("Total") );
  lv_speed = addColumn( i18n("Speed") );
  lv_remaining = addColumn( i18n("Rem. Time") );
  lv_url = addColumn( i18n("Address( URL )") );

  readConfig();
}



KIOListView::~KIOListView() {
  writeConfig();
}


void KIOListView::readConfig() {
  KConfig* config = new KConfig("kioslaverc");

  // read listview geometry properties
  config->setGroup( "ListProgress" );
  for ( int i = 0; i < NUM_COLS; i++ ) {
    QString tmps;
    tmps.sprintf( "Col%d", i );
    setColumnWidth( i, config->readNumEntry( tmps, defaultColumnWidth[i] ) );
  }
}


void KIOListView::writeConfig() {
  KConfig* config = new KConfig("kioslaverc");

  // write listview geometry properties
  config->setGroup( "ListProgress" );
  for ( int i = 0; i < NUM_COLS; i++ ) {
    QString tmps;
    tmps.sprintf( "Col%d", i );
    config->writeEntry( tmps, columnWidth( i ) );
  }

  config->sync();
}



//-----------------------------------------------------------------------------

KIOListProgressDlg::KIOListProgressDlg() : KTMainWindow( "" ) {
  readSettings();

  // setup toolbar
  toolBar()->insertButton(BarIcon("delete"), TOOL_CANCEL,
			  SIGNAL(clicked()), this,
			  SLOT(cancelCurrent()), FALSE, i18n("Cancel"));

  toolBar()->insertButton(BarIcon("bottom"), TOOL_DOCK,
			  SIGNAL(clicked()), this,
			  SLOT(toggleDocking()), TRUE, i18n("Dock"));
  toolBar()->setToggle(TOOL_DOCK, true);
  ((KToolBarButton*)toolBar()->getButton(TOOL_DOCK))->on( false ); // default is undocked

  // setup statusbar
  statusBar()->setInsertOrder(KStatusBar::LeftToRight);

  statusBar()->insertItem( i18n(" Files : %1 ").arg( 555 ), ID_TOTAL_FILES);
  statusBar()->insertItem( i18n(" Size : %1 kB ").arg( "134.56" ), ID_TOTAL_SIZE);
  statusBar()->insertItem( i18n(" Time : 00:00:00 "), ID_TOTAL_TIME);
  statusBar()->insertItem( i18n(" %1 kB/s ").arg("123.34"), ID_TOTAL_SPEED);

  // setup listview
  myListView = new KIOListView( this, "kiolist" );

  setView( myListView, true);

  connect( myListView, SIGNAL( selectionChanged() ), 
	   SLOT( slotSelection() ) );
  connect( myListView, SIGNAL( doubleClicked( QListViewItem* ) ), 
	   SLOT( slotOpenSimple( QListViewItem* ) ) );

  // setup animation timer
  updateTimer = new QTimer( this );
  connect( updateTimer, SIGNAL( timeout() ),
	   SLOT( slotUpdate() ) );

  updateTimer->start( 1000 );

  setCaption("Progress Dialog");
  setMinimumSize( 350, 150 );
  resize( 460, 150 );

  hide();
}


KIOListProgressDlg::~KIOListProgressDlg() {
  updateTimer->stop();
  writeSettings();
}


void KIOListProgressDlg::closeEvent( QCloseEvent * ){
  updateTimer->stop();
  writeSettings();
  hide();
}


void KIOListProgressDlg::addJob( KIOJob *job ) {
  KIOListViewItem *item;

  QListViewItemIterator it( myListView );
  for ( ; it.current(); ++it ) {
    item = (KIOListViewItem*) it.current();
    if ( item->id() == job->id() ) {
      item->update();
      return;
    }
  }

  item = new KIOListViewItem( myListView, job );
  connect( item, SIGNAL( statusChanged( QListViewItem* ) ),
	   SLOT( slotStatusChanged( QListViewItem* ) ) );
}


void KIOListProgressDlg::slotStatusChanged( QListViewItem *item ) {
  delete item;
}


void KIOListProgressDlg::slotUpdate() {
  if ( myListView->childCount() == 0 ) {
    hide();
    return;
  }

  int totalFiles = 0;
  int totalSize = 0;
  int totalSpeed = 0;
  QTime totalRemTime;

  // count totals !!! how ?

  // update statusbar
  statusBar()->changeItem( i18n( " Files : %1 ").arg( totalFiles ), ID_TOTAL_FILES);
  statusBar()->changeItem( i18n( " Size : %1 ").arg( KIOJob::convertSize( totalSize ) ),
			   ID_TOTAL_SIZE);
  statusBar()->changeItem( i18n( " Time : %1 ").arg( totalRemTime.toString() ), ID_TOTAL_TIME);
  statusBar()->changeItem( i18n( " %1/s ").arg( KIOJob::convertSize( totalSpeed ) ),
			   ID_TOTAL_SPEED);

  show();
}


void KIOListProgressDlg::slotOpenSimple( QListViewItem *item ) {
  ((KIOListViewItem*) item )->showSimpleGUI( true );
}


void KIOListProgressDlg::slotSelection() {
  QListViewItemIterator it( myListView );

  for ( ; it.current(); ++it ) {
    if ( it.current()->isSelected() ) {
      toolBar()->setItemEnabled( TOOL_CANCEL, TRUE);
      return;
    }
  }
  toolBar()->setItemEnabled( TOOL_CANCEL, FALSE); 
}


void KIOListProgressDlg::readSettings() { // finish !!!
  KConfig config("kioslaverc");
  config.setGroup( "ListProgress" );
}


void KIOListProgressDlg::writeSettings() { // finish !!!
  KConfig config("kioslaverc");
  config.setGroup( "ListProgress" );
}


void KIOListProgressDlg::cancelCurrent() {
  QListViewItemIterator it( myListView );

  while ( it.current() ) {
    if ( it.current()->isSelected() ) {
      KIOListViewItem *item = (KIOListViewItem*) it.current();
      item->remove();
    } else {
      it++; // update counts
    }
  }
}


void KIOListProgressDlg::toggleDocking() {
}


void KIOListProgressDlg::showGUI( bool _mode ) {
  if ( _mode ) {
    show();
  } else {
    hide();
  }
}

#include "kio_listprogress_dlg.moc"
