/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>
                 2001 George Staikos <staikos@kde.org>

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

#include <iostream.h>

#include <qtimer.h>

#include <qregexp.h>
#include <qheader.h>
#include <qevent.h>

#include <ksqueezedtextlabel.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kdesu/client.h>
#include <kwin.h>

#include "observer_stub.h"
#include "observer.h" // for static methods only
#include "kio/defaultprogress.h"
#include "kio/jobclasses.h"
#include "uiserver.h"
#include "passdlg.h"
#include "kio/renamedlg.h"
#include "kio/skipdlg.h"
#include "slavebase.h" // for QuestionYesNo etc.
#include <ksslinfodlg.h>
#include <ksslcertdlg.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>


// pointer for main instance of UIServer
UIServer* uiserver;

// ToolBar field IDs
enum { TOOL_CANCEL };

// StatusBar field IDs
enum { ID_TOTAL_FILES = 1, ID_TOTAL_SIZE, ID_TOTAL_TIME, ID_TOTAL_SPEED };

//static
int UIServer::s_jobId = 0;

static const int defaultColumnWidth[] = { 70,  // SIZE_OPERATION
                                    160, // LOCAL_FILENAME
                                    40,  // RESUME
                                    60,  // COUNT
                                    30,  // PROGRESS
                                    65,  // TOTAL
                                    70,  // SPEED
                                    70,  // REMAINING_TIME
                                    450  // URL
};

ProgressItem::ProgressItem( ListProgress* view, QListViewItem *after, QCString app_id, int job_id,
                            bool showDefault )
  : QListViewItem( view, after ) {

  listProgress = view;

  m_iTotalSize = 0;
  m_iTotalFiles = 0;
  m_iProcessedSize = 0;
  m_iProcessedFiles = 0;
  m_iSpeed = 0;

  m_sAppId = app_id;
  m_iJobId = job_id;
  m_visible = true;
  m_defaultProgressVisible = true;

  // create dialog, but don't show it
  defaultProgress = new KIO::DefaultProgress( false );
  defaultProgress->setOnlyClean( true );
  connect ( defaultProgress, SIGNAL( stopped() ), this, SLOT( slotCanceled() ) );
  connect ( &m_showTimer, SIGNAL( timeout() ), this, SLOT(slotShowDefaultProgress()) );
  
  if ( showDefault ) {
    m_showTimer.start( 500, true );
  }
}


ProgressItem::~ProgressItem() {
  if (defaultProgress->onlyClean())
    defaultProgress->finished();
  else
    delete defaultProgress;
}


void ProgressItem::setTotalSize( KIO::filesize_t size ) {
  m_iTotalSize = size;

  // It's already in the % column...
  //setText( listProgress->lv_total, KIO::convertSize( m_iTotalSize ) );

  defaultProgress->slotTotalSize( 0, m_iTotalSize );
}


void ProgressItem::setTotalFiles( unsigned long files ) {
  m_iTotalFiles = files;

  defaultProgress->slotTotalFiles( 0, m_iTotalFiles );
}


void ProgressItem::setTotalDirs( unsigned long dirs ) {
  defaultProgress->slotTotalDirs( 0, dirs );
}


void ProgressItem::setProcessedSize( KIO::filesize_t size ) {
  m_iProcessedSize = size;

  setText( ListProgress::TB_TOTAL, KIO::convertSize( size ) );

  defaultProgress->slotProcessedSize( 0, size );
}


void ProgressItem::setProcessedFiles( unsigned long files ) {
  m_iProcessedFiles = files;

  QString tmps = i18n("%1 / %2").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  setText( ListProgress::TB_COUNT, tmps );

  defaultProgress->slotProcessedFiles( 0, m_iProcessedFiles );
}


void ProgressItem::setProcessedDirs( unsigned long dirs ) {
  defaultProgress->slotProcessedDirs( 0, dirs );
}


void ProgressItem::setPercent( unsigned long percent ) {
  QString tmps = i18n( "%1 % of %2 ").arg( percent ).arg( KIO::convertSize(m_iTotalSize));
  setText( ListProgress::TB_PROGRESS, tmps );

  defaultProgress->slotPercent( 0, percent );
}

void ProgressItem::setInfoMessage( const QString & msg ) {
  QString plainTextMsg(msg);
  plainTextMsg.replace( QRegExp( "</?b>" ), QString::null );
  plainTextMsg.replace( QRegExp( "<img.*>" ), QString::null );
  setText( ListProgress::TB_PROGRESS, plainTextMsg );

  defaultProgress->slotInfoMessage( 0, msg );
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
  setText( ListProgress::TB_SPEED, tmps );
  setText( ListProgress::TB_REMAINING_TIME, tmps2 );

  defaultProgress->slotSpeed( 0, m_iSpeed );
}


void ProgressItem::setCopying( const KURL& from, const KURL& to ) {
   setText( ListProgress::TB_OPERATION, i18n("Copying") );
   setText( ListProgress::TB_ADDRESS, from.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, to.fileName() );

  defaultProgress->slotCopying( 0, from, to );
}


void ProgressItem::setMoving( const KURL& from, const KURL& to ) {
   setText( ListProgress::TB_OPERATION, i18n("Moving") );
   setText( ListProgress::TB_ADDRESS, from.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, to.fileName() );

  defaultProgress->slotMoving( 0, from, to );
}


void ProgressItem::setCreatingDir( const KURL& dir ) {
   setText( ListProgress::TB_OPERATION, i18n("Creating") );
   setText( ListProgress::TB_ADDRESS, dir.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, dir.fileName() );

  defaultProgress->slotCreatingDir( 0, dir );
}


void ProgressItem::setDeleting( const KURL& url ) {
   setText( ListProgress::TB_OPERATION, i18n("Deleting") );
   setText( ListProgress::TB_ADDRESS, url.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, url.fileName() );

  defaultProgress->slotDeleting( 0, url );
}

void ProgressItem::setTransferring( const KURL& url ) {
   setText( ListProgress::TB_OPERATION, i18n("Loading") );
   setText( ListProgress::TB_ADDRESS, url.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, url.fileName() );

  defaultProgress->slotTransferring( 0, url );
}

void ProgressItem::setText(ListProgress::ListProgressFields field, const QString& text)
{
  if (listProgress->m_lpcc[field].enabled)
  {
     QString t=text;
     if ((field==ListProgress::TB_ADDRESS) && (listProgress->m_fixedColumnWidths))
//     if (((field==ListProgress::TB_LOCAL_FILENAME) || (field==ListProgress::TB_ADDRESS)) && (listProgress->m_fixedColumnWidths))
     {
        m_fullLengthAddress=text;
        listProgress->m_squeezer->resize(listProgress->columnWidth(listProgress->m_lpcc[field].index),50);
        listProgress->m_squeezer->setText(t);
        t=listProgress->m_squeezer->text();
     }
     QListViewItem::setText(listProgress->m_lpcc[field].index,t);
  }
}

void ProgressItem::setStating( const KURL& url ) {
   setText( ListProgress::TB_OPERATION, i18n("Examining") );
   setText( ListProgress::TB_ADDRESS, url.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, url.fileName() );

  defaultProgress->slotStating( 0, url );
}

void ProgressItem::setMounting( const QString& dev, const QString & point ) {
   setText( ListProgress::TB_OPERATION, i18n("Mounting") );
   setText( ListProgress::TB_ADDRESS, point ); // ?
   setText( ListProgress::TB_LOCAL_FILENAME, dev ); // ?

  defaultProgress->slotMounting( 0, dev, point );
}

void ProgressItem::setUnmounting( const QString & point ) {
   setText( ListProgress::TB_OPERATION, i18n("Unmounting") );
   setText( ListProgress::TB_ADDRESS, point ); // ?
   setText( ListProgress::TB_LOCAL_FILENAME, "" ); // ?

  defaultProgress->slotUnmounting( 0, point );
}

void ProgressItem::setCanResume( KIO::filesize_t offset ) {
  /*
  QString tmps;
  // set canResume
  if ( _resume ) {
    tmps = i18n("Yes");
  } else {
    tmps = i18n("No");
  }
  setText( listProgress->lv_resume, tmps );
  */
  defaultProgress->slotCanResume( 0, offset );
}


void ProgressItem::slotCanceled() {
  emit jobCanceled( this );
}

// Called 0.5s after the job has been started
void ProgressItem::slotShowDefaultProgress() {
  if (defaultProgress)
  {
    if ( m_visible && m_defaultProgressVisible )
      defaultProgress->show();
    else
      defaultProgress->hide();      
  }
}

void ProgressItem::slotToggleDefaultProgress() {
  setDefaultProgressVisible( !m_defaultProgressVisible );
}

// Called when a rename or skip dialog pops up
// We want to prevent someone from killing the job in the uiserver then
void ProgressItem::setVisible( bool visible ) {
  if ( m_visible != visible )
  {
    m_visible = visible;
    updateVisibility();
  }
}

// Can be toggled by the user
void ProgressItem::setDefaultProgressVisible( bool visible ) {
  if ( m_defaultProgressVisible != visible )
  {
    m_defaultProgressVisible = visible;
    updateVisibility();
  }
}

// Update according to state
void ProgressItem::updateVisibility()
{
  if (defaultProgress)
  {
    if ( m_visible && m_defaultProgressVisible )
    {
      m_showTimer.start(250, true); // Show delayed
    }      
    else
    {
      m_showTimer.stop();
      defaultProgress->hide();
    }
  }
}


//-----------------------------------------------------------------------------
ListProgress::ListProgress (QWidget *parent, const char *name)
: KListView (parent, name)
{

  // enable selection of more than one item
  setMultiSelection( true );

  setAllColumnsShowFocus( true );

  m_lpcc[TB_OPERATION].title=i18n("Operation");
  m_lpcc[TB_LOCAL_FILENAME].title=i18n("Local Filename");
  m_lpcc[TB_RESUME].title=i18n("Res.");
  m_lpcc[TB_COUNT].title=i18n("Count");
  m_lpcc[TB_PROGRESS].title=i18n("%");
  m_lpcc[TB_TOTAL].title=i18n("Size");
  m_lpcc[TB_SPEED].title=i18n("Speed");
  m_lpcc[TB_REMAINING_TIME].title=i18n("Rem. Time");
  m_lpcc[TB_ADDRESS].title=i18n("URL");
  readConfig();

  createColumns();

  if (!m_showHeader)
     header()->hide();

  //used for squeezing the text in local file name and url
  m_squeezer=new KSqueezedTextLabel(this);
  m_squeezer->hide();
  connect(header(),SIGNAL(sizeChange(int,int,int)),this,SLOT(columnWidthChanged(int)));
}


ListProgress::~ListProgress() {
  writeConfig();
}

void ListProgress::createColumns()
{
   //remove all but the first column
   for (int i=columns()-1; i>=0; i--)
      removeColumn(i);

   for (int i=0; i<TB_MAX; i++)
   {
      if (m_lpcc[i].enabled)
      {
         m_lpcc[i].index=addColumn(m_lpcc[i].title, m_fixedColumnWidths?m_lpcc[i].width:-1);
         setColumnWidth(i, m_lpcc[i].width); //yes, this is required here, alexxx
      }
   }
}

void ListProgress::readConfig() {
  KConfig config("uiserverrc");

  // read listview geometry properties
  config.setGroup( "ProgressList" );
  for ( int i = 0; i < TB_MAX; i++ ) {
     QString tmps="Col"+QString::number(i);
     m_lpcc[i].width=config.readNumEntry( tmps, 0);
     if (m_lpcc[i].width==0) m_lpcc[i].width=defaultColumnWidth[i];

     tmps="Enabled"+QString::number(i);
     m_lpcc[i].enabled=config.readBoolEntry(tmps,true);
  }
  m_showHeader=config.readBoolEntry("ShowListHeader",true);
  m_fixedColumnWidths=config.readBoolEntry("FixedColumnWidths",false);

  m_lpcc[TB_RESUME].enabled=false;
}

void ListProgress::columnWidthChanged(int column)
{
   //resqueeze if necessary
   if ((m_lpcc[TB_ADDRESS].enabled) && (column==m_lpcc[TB_ADDRESS].index))
   {
      for (QListViewItem* lvi=firstChild(); lvi!=0; lvi=lvi->nextSibling())
      {
         ProgressItem *pi=(ProgressItem*)lvi;
         pi->setText(TB_ADDRESS,pi->fullLengthAddress());
      }
   }
   writeConfig();
}

void ListProgress::writeConfig() {
   KConfig config("uiserverrc");

   // write listview geometry properties
   config.setGroup( "ProgressList" );
   for ( int i = 0; i < TB_MAX; i++ ) {
      m_lpcc[i].width=columnWidth(i);
      QString tmps;
      tmps.sprintf( "Col%d", i );
      config.writeEntry( tmps, m_lpcc[i].width);
   }
   config.sync();
}


//------------------------------------------------------------


UIServer::UIServer() : KMainWindow(0, ""), DCOPObject("UIServer")
{

  readSettings();

  // setup toolbar
  toolBar()->insertButton("editdelete", TOOL_CANCEL,
                          SIGNAL(clicked()), this,
                          SLOT(cancelCurrent()), FALSE, i18n("Cancel"));

  toolBar()->setBarPos( KToolBar::Left );

  // setup statusbar
  statusBar()->insertItem( i18n(" Files: %1 ").arg( 555 ), ID_TOTAL_FILES);
  statusBar()->insertItem( i18n(" Size: %1 kB ").arg( "134.56" ), ID_TOTAL_SIZE);
  statusBar()->insertItem( i18n(" Time: 00:00:00 "), ID_TOTAL_TIME);
  statusBar()->insertItem( i18n(" %1 kB/s ").arg("123.34"), ID_TOTAL_SPEED);

  // setup listview
  listProgress = new ListProgress( this, "progresslist" );

  setCentralWidget( listProgress );

  connect( listProgress, SIGNAL( selectionChanged() ),
           SLOT( slotSelection() ) );
  connect( listProgress, SIGNAL( executed( QListViewItem* ) ),
           SLOT( slotToggleDefaultProgress( QListViewItem* ) ) );

  // setup animation timer
  updateTimer = new QTimer( this );
  connect( updateTimer, SIGNAL( timeout() ),
           SLOT( slotUpdate() ) );
  m_bUpdateNewJob=false;

  setCaption(i18n("Progress Dialog"));
  setMinimumSize( 150, 50 );
  resize( m_initWidth, m_initHeight);

  if (m_showStatusBar==false)
     statusBar()->hide();
  if (m_showToolBar==false)
     toolBar()->hide();

/*  if ((m_bShowList) && (m_keepListOpen))
  {
     cerr<<"show() !"<<endl;
     show();
  }
  else*/
     hide();
}


UIServer::~UIServer() {
  updateTimer->stop();
}


int UIServer::newJob( QCString observerAppId, bool showProgress )
{
  kdDebug(7024) << "UIServer::newJob observerAppId=" << observerAppId << ". "
            << "Giving id=" << s_jobId+1 << endl;

  QListViewItemIterator it( listProgress );
  for ( ; it.current(); ++it ) {
    if ( it.current()->itemBelow() == 0L ) { // this will find the end of list
      break;
    }
  }

  // increment counter
  s_jobId++;

  bool show = !m_bShowList && showProgress;

  ProgressItem *item = new ProgressItem( listProgress, it.current(), observerAppId, s_jobId, show );
  connect( item, SIGNAL( jobCanceled( ProgressItem* ) ),
           SLOT( slotJobCanceled( ProgressItem* ) ) );

  if ( m_bShowList && !updateTimer->isActive() )
    updateTimer->start( 1000 );

  m_bUpdateNewJob=true;

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


void UIServer::setItemVisible( ProgressItem * item, bool visible )
{
  item->setVisible( visible );
  // Check if we were the last one to be visible
  // or the first one -> hide/show the list in that case
  // (Note that the user could have hidden the listview by hand yet, no time)
  if ( m_bShowList ) {
      m_bUpdateNewJob = true;
      slotUpdate();
  }
}

// Called by Observer when opening a skip or rename dialog
void UIServer::setJobVisible( int id, bool visible )
{
  kdDebug(7024) << "UIServer::setJobVisible id=" << id << " visible=" << visible << endl;
  ProgressItem *item = findItem( id );
  Q_ASSERT( item );
  if ( item )
      setItemVisible( item, visible );
}

void UIServer::jobFinished( int id )
{
  kdDebug(7024) << "UIServer::jobFinished id=" << id << endl;
  ProgressItem *item = findItem( id );

  // remove item from the list and delete the corresponding defaultprogress
  if ( item ) {
    delete item;
  }
}


void UIServer::totalSize( int id, unsigned long size )
{ totalSize64(id, size); }

void UIServer::totalSize64( int id, KIO::filesize_t size )
{
//  kdDebug(7024) << "UIServer::totalSize " << id << " " << KIO::number(size) << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalSize( size );
  }
}

void UIServer::totalFiles( int id, unsigned long files )
{
  kdDebug(7024) << "UIServer::totalFiles " << id << " " << (unsigned int) files << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalFiles( files );
  }
}

void UIServer::totalDirs( int id, unsigned long dirs )
{
  kdDebug(7024) << "UIServer::totalDirs " << id << " " << (unsigned int) dirs << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalDirs( dirs );
  }
}

void UIServer::processedSize( int id, unsigned long size )
{ processedSize64(id, size); }

void UIServer::processedSize64( int id, KIO::filesize_t size )
{
  //kdDebug(7024) << "UIServer::processedSize " << id << " " << KIO::number(size) << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedSize( size );
  }
}

void UIServer::processedFiles( int id, unsigned long files )
{
  //kdDebug(7024) << "UIServer::processedFiles " << id << " " << (unsigned int) files << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedFiles( files );
  }
}

void UIServer::processedDirs( int id, unsigned long dirs )
{
  kdDebug(7024) << "UIServer::processedDirs " << id << " " << (unsigned int) dirs << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedDirs( dirs );
  }
}

void UIServer::percent( int id, unsigned long ipercent )
{
  //kdDebug(7024) << "UIServer::percent " << id << " " << (unsigned int) ipercent << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setPercent( ipercent );
  }
}

void UIServer::speed( int id, unsigned long bytes_per_second )
{
  //kdDebug(7024) << "UIServer::speed " << id << " " << (unsigned int) bytes_per_second << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setSpeed( bytes_per_second );
  }
}

void UIServer::infoMessage( int id, const QString & msg )
{
  //kdDebug(7024) << "UIServer::infoMessage " << id << " " << msg << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setInfoMessage( msg );
  }
}

void UIServer::canResume( int id, unsigned long offset )
{ canResume64(id, offset); }

void UIServer::canResume64( int id, KIO::filesize_t offset )
{
  //kdDebug(7024) << "UIServer::canResume " << id << " " << offset << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCanResume( offset );
  }
}

void UIServer::copying( int id, KURL from, KURL to )
{
  //kdDebug(7024) << "UIServer::copying " << id << " " << from.url() << "  " << to.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCopying( from, to );
  }
}

void UIServer::moving( int id, KURL from, KURL to )
{
  //kdDebug(7024) << "UIServer::moving " << id << " " << from.url() << "  " << to.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setMoving( from, to );
  }
}

void UIServer::deleting( int id, KURL url )
{
  //kdDebug(7024) << "UIServer::deleting " << id << " " << url.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setDeleting( url );
  }
}

void UIServer::transferring( int id, KURL url )
{
  //kdDebug(7024) << "UIServer::transferring " << id << " " << url.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTransferring( url );
  }
}

void UIServer::creatingDir( int id, KURL dir )
{
  kdDebug(7024) << "UIServer::creatingDir " << id << " " << dir.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCreatingDir( dir );
  }
}

void UIServer::stating( int id, KURL url )
{
  kdDebug(7024) << "UIServer::stating " << id << " " << url.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setStating( url );
  }
}

void UIServer::mounting( int id, QString dev, QString point )
{
  kdDebug(7024) << "UIServer::mounting " << id << " " << dev << " " << point << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setMounting( dev, point );
  }
}

void UIServer::unmounting( int id, QString point )
{
  kdDebug(7024) << "UIServer::unmounting " << id << " " << point << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setUnmounting( point );
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
#ifndef Q_WS_QWS //FIXME(E): Implement for QT Embedded
   KWin::iconifyWindow(winId());
#endif
}


void UIServer::slotJobCanceled( ProgressItem *item ) {
  kdDebug(7024) << "UIServer::slotJobCanceled appid=" << item->appId() << " jobid=" << item->jobId() << endl;
  // kill the corresponding job
  killJob( item->appId(), item->jobId() );

  // KIO::Job, when killed, should call back jobFinished(), but we can't
  // really rely on that - the app may have crashed
  delete item;
}


void UIServer::slotUpdate() {
  // don't do anything if we don't have any inserted progress item
  // or if they're all hidden
   QListViewItemIterator lvit( listProgress );
   bool visible = false;
   for ( ; lvit.current(); ++lvit )
      if ( ((ProgressItem*)lvit.current())->isVisible() ) {
         visible = true;
         break;
      }

   if ( !visible || !m_bShowList ) {
      if (!m_keepListOpen) hide();
      updateTimer->stop();
      return;
   }

  // Calling show() is conditional, so that users can close the window
  // and it only pops up back when a new job is started
  if (m_bUpdateNewJob)
  {
    m_bUpdateNewJob=false;
    show();

    // Make sure we'll be called back
    if ( m_bShowList && !updateTimer->isActive() )
      updateTimer->start( 1000 );
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
  statusBar()->changeItem( i18n( " Files: %1 ").arg( iTotalFiles ), ID_TOTAL_FILES);
  statusBar()->changeItem( i18n( " Size: %1 ").arg( KIO::convertSize( iTotalSize ) ),
                           ID_TOTAL_SIZE);
  statusBar()->changeItem( i18n( " Time: %1 ").arg( totalRemTime.toString() ), ID_TOTAL_TIME);
  statusBar()->changeItem( i18n( " %1/s ").arg( KIO::convertSize( iTotalSpeed ) ),
                           ID_TOTAL_SPEED);

}

void UIServer::setListMode( bool list )
{
  m_bShowList = list;
  QListViewItemIterator it( listProgress );
  for ( ; it.current(); ++it ) {
    // When going to list mode -> hide all progress dialogs
    // When going back to separate dialogs -> show them all
    ((ProgressItem*) it.current())->setDefaultProgressVisible( !list );
  }

  if (m_bShowList)
  {
    show();
    updateTimer->start( 1000 );
  }
  else
  {
    hide();
    updateTimer->stop();
  }
}

void UIServer::slotToggleDefaultProgress( QListViewItem *item ) {
  ((ProgressItem*) item )->slotToggleDefaultProgress();
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

// This code is deprecated, slaves go to Observer::openPassDlg now,
// but this is kept for compat (DCOP calls to kio_uiserver).
QByteArray UIServer::openPassDlg( const KIO::AuthInfo &info )
{
    kdDebug(7024) << "UIServer::openPassDlg: User= " << info.username
                  << ", Msg= " << info.prompt << endl;
    KIO::AuthInfo inf(info);
    int result = KIO::PasswordDialog::getNameAndPassword( inf.username, inf.password,
                                                          &inf.keepPassword, inf.prompt,
                                                          inf.readOnly, inf.caption,
                                                          inf.comment, inf.commentLabel );
    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    if ( result == QDialog::Accepted )
        inf.setModified( true );
    else
        inf.setModified( false );
    stream << inf;
    return data;
}

int UIServer::messageBox( int progressId, int type, const QString &text, const QString &caption, const QString &buttonYes, const QString &buttonNo )
{
    return Observer::messageBox( progressId, type, text, caption, buttonYes, buttonNo );
}

void UIServer::showSSLInfoDialog(const QString &url, const KIO::MetaData &meta)
{
   KSSLInfoDlg *kid = new KSSLInfoDlg(meta["ssl_in_use"].upper()=="TRUE", 0L /*parent?*/, 0L, true);
   KSSLCertificate *x = KSSLCertificate::fromString(meta["ssl_peer_certificate"].local8Bit());
   if (x) {
      // Set the chain back onto the certificate
      QStringList cl =
                      QStringList::split(QString("\n"), meta["ssl_peer_chain"]);
      QPtrList<KSSLCertificate> ncl;

      ncl.setAutoDelete(true);
      for (QStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
         KSSLCertificate *y = KSSLCertificate::fromString((*it).local8Bit());
         if (y) ncl.append(y);
      }

      if (ncl.count() > 0)
         x->chain().setChain(ncl);

      kid->setup( x,
                  meta["ssl_peer_ip"],
                  url, // the URL
                  meta["ssl_cipher"],
                  meta["ssl_cipher_desc"],
                  meta["ssl_cipher_version"],
                  meta["ssl_cipher_used_bits"].toInt(),
                  meta["ssl_cipher_bits"].toInt(),
                  KSSLCertificate::KSSLValidation(meta["ssl_cert_state"].toInt()));
      kdDebug(7024) << "Showing SSL Info dialog" << endl;
      kid->exec();
      delete x;
      kdDebug(7024) << "SSL Info dialog closed" << endl;
   } else {
      KMessageBox::information( 0L, // parent ?
                              i18n("The peer SSL certificate appears to be corrupt."), i18n("SSL") );
   }
   // Don't delete kid!!
}

KSSLCertDlgRet UIServer::showSSLCertDialog(const QString& host, const QStringList& certList)
{
   KSSLCertDlgRet rc;
   rc.ok = false;
   if (!certList.isEmpty()) {
      KSSLCertDlg *kcd = new KSSLCertDlg(0L, 0L, true);
      kcd->setup(certList);
      kcd->setHost(host);
      kdDebug(7024) << "Showing SSL certificate dialog" << endl;
      kcd->exec();
      rc.ok = true;
      rc.choice = kcd->getChoice();
      rc.save = kcd->saveChoice();
      rc.send = kcd->wantsToSend();
      kdDebug(7024) << "SSL certificate dialog closed" << endl;
      delete kcd;
   }
   return rc;
}


QByteArray UIServer::open_RenameDlg( int id,
                                     const QString & caption,
                                     const QString& src, const QString & dest,
                                     int mode,
                                     unsigned long sizeSrc,
                                     unsigned long sizeDest,
                                     unsigned long ctimeSrc,
                                     unsigned long ctimeDest,
                                     unsigned long mtimeSrc,
                                     unsigned long mtimeDest
                                     )
{ return open_RenameDlg64(id, caption, src, dest, mode, sizeSrc, sizeDest,
                          ctimeSrc, ctimeDest, mtimeSrc, mtimeDest); }


QByteArray UIServer::open_RenameDlg64( int id,
                                     const QString & caption,
                                     const QString& src, const QString & dest,
                                     int mode,
                                     KIO::filesize_t sizeSrc,
                                     KIO::filesize_t sizeDest,
                                     unsigned long ctimeSrc,
                                     unsigned long ctimeDest,
                                     unsigned long mtimeSrc,
                                     unsigned long mtimeDest
                                     )
{
  // Hide existing dialog box if any
  ProgressItem *item = findItem( id );
  if ( item )
    setItemVisible( item, false );
  QString newDest;
  kdDebug(7024) << "Calling KIO::open_RenameDlg" << endl;
  KIO::RenameDlg_Result result = KIO::open_RenameDlg( caption, src, dest,
                                                      (KIO::RenameDlg_Mode) mode, newDest,
                                                      sizeSrc, sizeDest,
                                                      (time_t)ctimeSrc, (time_t)ctimeDest,
                                                      (time_t)mtimeSrc, (time_t)mtimeDest );
  kdDebug(7024) << "KIO::open_RenameDlg done" << endl;
  QByteArray data;
  QDataStream stream( data, IO_WriteOnly );
  stream << Q_UINT8(result) << newDest;
  if ( item && result != KIO::R_CANCEL )
    setItemVisible( item, true );
  return data;
}

int UIServer::open_SkipDlg( int id,
                            int /*bool*/ multi,
                            const QString & error_text )
{
  // Hide existing dialog box if any
  ProgressItem *item = findItem( id );
  if ( item )
    setItemVisible( item, false );
  kdDebug(7024) << "Calling KIO::open_SkipDlg" << endl;
  KIO::SkipDlg_Result result = KIO::open_SkipDlg( (bool)multi, error_text );
  if ( item && result != KIO::S_CANCEL )
    setItemVisible( item, true );
  return (KIO::SkipDlg_Result) result;
}


void UIServer::readSettings() {
  KConfig config("uiserverrc");
  config.setGroup( "UIServer" );
  m_showStatusBar=config.readBoolEntry("ShowStatusBar",true);
  m_showToolBar=config.readBoolEntry("ShowToolBar",true);
  m_keepListOpen=config.readBoolEntry("KeepListOpen",false);
  m_initWidth=config.readNumEntry("InitialWidth",460);
  m_initHeight=config.readNumEntry("InitialHeight",150);
  m_bShowList = config.readBoolEntry( "ShowList", false );
}

void UIServer::writeSettings() {
  KConfig config("uiserverrc");
  config.setGroup( "UIServer" );
  config.writeEntry("InitialWidth",width());
  config.writeEntry("InitialHeight",height());
}


void UIServer::cancelCurrent() {
  QListViewItemIterator it( listProgress );
  ProgressItem *item;

  // kill selected jobs
  for ( ; it.current() ; ++it )
  {
    if ( it.current()->isSelected() ) {
      item = (ProgressItem*) it.current();
      killJob( item->appId(), item->jobId() );
      return;
    }
  }
}

void UIServer::resizeEvent(QResizeEvent* e)
{
   KMainWindow::resizeEvent(e);
   writeSettings();
}

//------------------------------------------------------------

int main(int argc, char **argv)
{
    KLocale::setMainCatalogue("kdelibs");
    //  GS 5/2001 - I changed the name to "KDE" to make it look better
    //              in the titles of dialogs which are displayed.
    KAboutData aboutdata("kio_uiserver", I18N_NOOP("KDE"),
                         "0.8", I18N_NOOP("KDE Progress Information UI Server"),
                         KAboutData::License_GPL, "(C) 2000, David Faure & Matt Koss");
    // Who's the maintainer ? :)
    aboutdata.addAuthor("David Faure",I18N_NOOP("Developer"),"faure@kde.org");
    aboutdata.addAuthor("Matej Koss",I18N_NOOP("Developer"),"koss@miesto.sk");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    // KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start())
    {
      kdDebug(7024) << "kio_uiserver is already running!" << endl;
      return (0);
    }

    KUniqueApplication app;

    // This app is started automatically, no need for session management
    app.disableSessionManagement();
    app.dcopClient()->setDaemonMode( true );

    uiserver = new UIServer();
    app.setMainWidget( uiserver );

    return app.exec();
}

#include "uiserver.moc"
