/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

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
// -*- mode: c++; c-basic-offset: 4 -*-

#include <qtimer.h>

#include <kconfig.h>
#include <kstddirs.h>
#include <kuniqueapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "observer_stub.h"
#include "kio/defaultprogress.h"
#include "kio/jobclasses.h"
#include "kio/uiserver.h"

// pointer for main instance of UIServer
UIServer* uiserver;

// ToolBar field IDs
enum { TOOL_CANCEL };

// StatusBar field IDs
enum { ID_TOTAL_FILES = 1, ID_TOTAL_SIZE, ID_TOTAL_TIME, ID_TOTAL_SPEED };

//static
int UIServer::s_jobId = 0;

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

// number of listview columns
#define NUM_COLS  9

ProgressItem::ProgressItem( ListProgress* view, QListViewItem *after, QCString app_id, int job_id,
			    bool showDefault )
  : QListViewItem( view, after ) {

  listProgress = view;
  
  m_sAppId = app_id;
  m_iJobId = job_id;

  // create dialog, but don't show it
  defaultProgress = new DefaultProgress( false );
  defaultProgress->setOnlyClean( true );
  connect ( defaultProgress, SIGNAL( stopped() ), this, SLOT( slotCanceled() ) );

  if ( showDefault ) {
    QTimer::singleShot( 500, this, SLOT(slotShowDefaultProgress()) ); // start a 1/2 second timer
  }
}


ProgressItem::~ProgressItem() {
  delete defaultProgress;
}


void ProgressItem::setTotalSize( unsigned long size ) {
  m_iTotalSize = size;

  setText( listProgress->lv_total, KIO::convertSize( m_iTotalSize ) );

  defaultProgress->slotTotalSize( 0, m_iTotalSize );
}


void ProgressItem::setTotalFiles( unsigned long files ) {
  m_iTotalFiles = files;

  defaultProgress->slotTotalFiles( 0, m_iTotalFiles );
}


void ProgressItem::setTotalDirs( unsigned long dirs ) {
  defaultProgress->slotTotalDirs( 0, dirs );
}


void ProgressItem::setProcessedSize( unsigned long size ) {
  m_iProcessedSize = size;

  defaultProgress->slotProcessedSize( 0, size );
}


void ProgressItem::setProcessedFiles( unsigned long files ) {
  m_iProcessedFiles = files;

  QString tmps;
  tmps.sprintf( "%u / %u", m_iProcessedFiles, m_iTotalFiles );
  setText( listProgress->lv_count, tmps );

  defaultProgress->slotProcessedFiles( 0, m_iProcessedFiles );
}


void ProgressItem::setProcessedDirs( unsigned long dirs ) {
  defaultProgress->slotProcessedDirs( 0, dirs );
}


void ProgressItem::setPercent( unsigned long percent ) {
  QString tmps = i18n( "%1 % of %2 ").arg( percent ).arg( KIO::convertSize(m_iTotalSize));
  setText( listProgress->lv_progress, tmps );

  defaultProgress->slotPercent( 0, percent );
}


void ProgressItem::setSpeed( unsigned long bytes_per_second ) {
  m_iSpeed = bytes_per_second;
  m_remainingTime = KIO::calculateRemaining( m_iTotalSize, m_iProcessedSize, m_iSpeed );

  QString tmps, tmps2;
  if ( m_iSpeed == 0 ) {
    tmps = i18n( "Stalled");
    tmps2 = tmps;
  } else {
    tmps = i18n( "%1/s").arg( KIO::convertSize( m_iSpeed ));
    tmps2 = m_remainingTime.toString();
  }
  setText( listProgress->lv_speed, tmps );
  setText( listProgress->lv_remaining, tmps2 );

  defaultProgress->slotSpeed( 0, m_iSpeed );
}


void ProgressItem::setCopying( const KURL& from, const KURL& to ) {
  setText( listProgress->lv_operation, i18n("Copying") );
  setText( listProgress->lv_url, from.url() );
  setText( listProgress->lv_filename, to.filename() );

  defaultProgress->slotCopying( 0, from, to );
}


void ProgressItem::setMoving( const KURL& from, const KURL& to ) {
  setText( listProgress->lv_operation, i18n("Moving") );
  setText( listProgress->lv_url, from.url() );
  setText( listProgress->lv_filename, to.filename() );

  defaultProgress->slotMoving( 0, from, to );
}


void ProgressItem::setRenaming( const KURL& old_name, const KURL& new_name ) {
  setText( listProgress->lv_filename, new_name.filename() );

  defaultProgress->slotRenaming( 0, old_name, new_name );
}


void ProgressItem::setCreatingDir( const KURL& dir ) {
  setText( listProgress->lv_operation, i18n("Creating") );
  setText( listProgress->lv_url, dir.url() );
  setText( listProgress->lv_filename, dir.filename() );

  defaultProgress->slotCreatingDir( 0, dir );
}


void ProgressItem::setDeleting( const KURL& url ) {
  setText( listProgress->lv_operation, i18n("Deleting") );
  setText( listProgress->lv_url, url.url() );
  setText( listProgress->lv_filename, url.filename() );

  defaultProgress->slotDeleting( 0, url );
}


void ProgressItem::setCanResume( bool _resume ) {
  QString tmps;
  // set canResume
  if ( _resume ) {
    tmps = i18n("Yes");
  } else {
    tmps = i18n("No");
  }
  setText( listProgress->lv_resume, tmps );
}


void ProgressItem::slotCanceled() {
  emit jobCanceled( this );
}


void ProgressItem::slotShowDefaultProgress() {
  defaultProgress->show();
}


//-----------------------------------------------------------------------------

ListProgress::ListProgress (QWidget *parent, const char *name)
  : KListView (parent, name) {

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
  lv_url = addColumn( i18n("URL") );

  readConfig();
}


ListProgress::~ListProgress() {
  writeConfig();
}


void ListProgress::readConfig() {
  KConfig config("uiserverrc");

  // read listview geometry properties
  config.setGroup( "ProgressList" );
  for ( int i = 0; i < NUM_COLS; i++ ) {
    QString tmps;
    tmps.sprintf( "Col%d", i );
    setColumnWidth( i, config.readNumEntry( tmps, defaultColumnWidth[i] ) );
  }
}


void ListProgress::writeConfig() {
  KConfig config("uiserverrc");

  // write listview geometry properties
  config.setGroup( "ProgressList" );
  for ( int i = 0; i < NUM_COLS; i++ ) {
    QString tmps;
    tmps.sprintf( "Col%d", i );
    config.writeEntry( tmps, columnWidth( i ) );
  }

  config.sync();
}


//------------------------------------------------------------


UIServer::UIServer() : KTMainWindow( "" ), DCOPObject("UIServer")
{
  readSettings();

  // setup toolbar
  toolBar()->insertButton(BarIcon("delete"), TOOL_CANCEL,
			  SIGNAL(clicked()), this,
			  SLOT(cancelCurrent()), FALSE, i18n("Cancel"));

  toolBar()->setBarPos( KToolBar::Left );

  // setup statusbar
  statusBar()->insertItem( i18n(" Files : %1 ").arg( 555 ), ID_TOTAL_FILES);
  statusBar()->insertItem( i18n(" Size : %1 kB ").arg( "134.56" ), ID_TOTAL_SIZE);
  statusBar()->insertItem( i18n(" Time : 00:00:00 "), ID_TOTAL_TIME);
  statusBar()->insertItem( i18n(" %1 kB/s ").arg("123.34"), ID_TOTAL_SPEED);

  // setup listview
  listProgress = new ListProgress( this, "progresslist" );

  setView( listProgress, true);

  connect( listProgress, SIGNAL( selectionChanged() ),
	   SLOT( slotSelection() ) );
  connect( listProgress, SIGNAL( executed( QListViewItem* ) ),
	   SLOT( slotDefaultProgress( QListViewItem* ) ) );

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


UIServer::~UIServer() {
  updateTimer->stop();
  writeSettings();
}


int UIServer::newJob( QCString observerAppId )
{
  kdDebug() << "UIServer::newJob observerAppId=" << observerAppId << ". "
	    << "Giving id=" << s_jobId+1 << endl;
  
  QListViewItemIterator it( listProgress );
  for ( ; it.current(); ++it ) {
    if ( it.current()->itemBelow() == 0L ) { // this will find the end of list
      break;
    }
  }
  
  // increment counter
  s_jobId++;

  ProgressItem *item = new ProgressItem( listProgress, it.current(), observerAppId, s_jobId, !m_bShowList );
  connect( item, SIGNAL( jobCanceled( ProgressItem* ) ),
 	   SLOT( slotJobCanceled( ProgressItem* ) ) );

  return s_jobId;
}


ProgressItem* UIServer::findItem( int id )
{
  QListViewItemIterator it( listProgress );

  ProgressItem *item;

  for ( ; it.current(); ++it ) {
    item = (ProgressItem*) it.current();
    if ( item->jobId() == id ) {
      return item;
    }
  }

  return 0L;
}


void UIServer::jobFinished( int id )
{
  ProgressItem *item = findItem( id );

  // remove item from the list and delete the corresponding defaultprogress
  if ( item ) {
    delete item;
  }
}


void UIServer::totalSize( int id, unsigned long size )
{
  kdDebug() << "UIServer::totalSize " << id << " " << size << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalSize( size );
  }
}

void UIServer::totalFiles( int id, unsigned long files )
{
  kdDebug() << "UIServer::totalFiles " << id << " " << files << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalFiles( files );
  }
}

void UIServer::totalDirs( int id, unsigned long dirs )
{
  kdDebug() << "UIServer::totalDirs " << id << " " << dirs << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalDirs( dirs );
  }
}

void UIServer::processedSize( int id, unsigned long size )
{
  kdDebug() << "UIServer::processedSize " << id << " " << size << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedSize( size );
  }
}

void UIServer::processedFiles( int id, unsigned long files )
{
  kdDebug() << "UIServer::processedFiles " << id << " " << files << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedFiles( files );
  }
}

void UIServer::processedDirs( int id, unsigned long dirs )
{
  kdDebug() << "UIServer::processedDirs " << id << " " << dirs << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedDirs( dirs );
  }
}

void UIServer::percent( int id, unsigned long ipercent )
{
  kdDebug() << "UIServer::percent " << id << " " << ipercent << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setPercent( ipercent );
  }
}

void UIServer::speed( int id, unsigned long bytes_per_second )
{
  kdDebug() << "UIServer::speed " << id << " " << bytes_per_second << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setSpeed( bytes_per_second );
  }
}

void UIServer::canResume( int id, unsigned int can_resume )
{
  kdDebug() << "UIServer::canResume " << id << " " << can_resume << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCanResume( can_resume );
  }
}

void UIServer::copying( int id, KURL from, KURL to )
{
  kdDebug() << "UIServer::copying " << id << " " << from.url() << "  " << to.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCopying( from, to );
  }
}

void UIServer::moving( int id, KURL from, KURL to )
{
  kdDebug() << "UIServer::moving " << id << " " << from.url() << "  " << to.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setMoving( from, to );
  }
}

void UIServer::deleting( int id, KURL url )
{
  kdDebug() << "UIServer::deleting " << id << " " << url.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setDeleting( url );
  }
}

void UIServer::renaming( int id, KURL old_name, KURL new_name )
{
  kdDebug() << "UIServer::renaming " << id << " " << old_name.url() << "  " << new_name.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setRenaming( old_name, new_name );
  }
}

void UIServer::creatingDir( int id, KURL dir )
{
  kdDebug() << "UIServer::creatingDir " << id << " " << dir.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCreatingDir( dir );
  }
}

void UIServer::killJob( QCString observerAppId, int progressId )
{
    // Contact the object "KIO::Observer" in the application <appId>
    Observer_stub observer( observerAppId, "KIO::Observer" );
    // Tell it to kill the job
    observer.killJob( progressId );
}


void UIServer::closeEvent( QCloseEvent * ){
  hide();
}


void UIServer::slotJobCanceled( ProgressItem *item ) {
  // kill the corresponding job
  killJob( item->appId(), item->jobId() );

  // don't delete item, because KIO::Job, when killed, should call back jobFinished()
}


void UIServer::slotUpdate() {
  // don't do anything if we don't have any inserted progress item
  if ( listProgress->childCount() == 0 ) {
    hide();
    return;
  }

  int iTotalFiles = 0;
  int iTotalSize = 0;
  int iTotalSpeed = 0;
  QTime totalRemTime;

  ProgressItem *item;

  // count totals for statusbar
  QListViewItemIterator it( listProgress );

  for ( ; it.current(); ++it ) {
    item = (ProgressItem*) it.current();
    if ( item->totalSize() != 0 ) {
      iTotalSize += ( item->totalSize() - item->processedSize() );
    }
    iTotalFiles += ( item->totalFiles() - item->processedFiles() );
    iTotalSpeed += item->speed();
    
    if ( item->remainingTime() > totalRemTime ) {
      totalRemTime = item->remainingTime();
    }
  }

  // update statusbar
  statusBar()->changeItem( i18n( " Files : %1 ").arg( iTotalFiles ), ID_TOTAL_FILES);
  statusBar()->changeItem( i18n( " Size : %1 ").arg( KIO::convertSize( iTotalSize ) ),
			   ID_TOTAL_SIZE);
  statusBar()->changeItem( i18n( " Time : %1 ").arg( totalRemTime.toString() ), ID_TOTAL_TIME);
  statusBar()->changeItem( i18n( " %1/s ").arg( KIO::convertSize( iTotalSpeed ) ),
			   ID_TOTAL_SPEED);

  // show only when desired
  if ( m_bShowList ) {
    show();
  }
}


void UIServer::slotDefaultProgress( QListViewItem *item ) {
  ((ProgressItem*) item )->slotShowDefaultProgress();
}


void UIServer::slotSelection() {
  QListViewItemIterator it( listProgress );

  for ( ; it.current(); ++it ) {
    if ( it.current()->isSelected() ) {
      toolBar()->setItemEnabled( TOOL_CANCEL, TRUE);
      return;
    }
  }
  toolBar()->setItemEnabled( TOOL_CANCEL, FALSE);
}


void UIServer::readSettings() {
  KConfig config("uiserverrc");
  config.setGroup( "UIServer" );

  m_bShowList = config.readBoolEntry( "ShowList", false );
}


void UIServer::writeSettings() {
  KConfig config("uiserverrc");
  config.setGroup( "UIServer" );

  config.writeEntry( "ShowList", m_bShowList );
}


void UIServer::cancelCurrent() {
  QListViewItemIterator it( listProgress );
  ProgressItem *item;
  
  // kill all selected jobs
  while ( it.current() ) {
    if ( it.current()->isSelected() ) {
      item = (ProgressItem*) it.current();
      killJob( item->appId(), item->jobId() );
    } else {
      it++; // update counts
    }
  }
}

//------------------------------------------------------------

int main(int argc, char **argv)
{
    KLocale::setMainCatalogue("kdelibs");
    KAboutData aboutdata("kio_uiserver", I18N_NOOP("UIServer"),
                         "0.8", I18N_NOOP("KDE Progress Information UI Server"),
                         KAboutData::License_GPL, "(C) 2000, David Faure & Matt Koss");
    // Who's the maintainer ? :)
    aboutdata.addAuthor("David Faure",I18N_NOOP("Developer"),"faure@kde.org");
    aboutdata.addAuthor("Matej Koss",I18N_NOOP("Developer"),"koss@miesto.sk");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    // KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();
    KUniqueApplication app;

    // This app is started automatically, no need for session management
    app.disableSessionManagement();

    uiserver =  new UIServer;

    app.setMainWidget( uiserver );

    return app.exec();
}


#include "uiserver.moc"
