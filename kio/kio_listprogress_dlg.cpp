#include "kio_job.h" 
#include <qtimer.h>

#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kwm.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <ktoolbarbutton.h>

#include "kio_listprogress_dlg.h"

#define SIZE_OPERATION        70
#define SIZE_LOCAL_FILENAME   160
#define SIZE_RESUME           40
#define SIZE_COUNT            60
#define SIZE_PROGRESS         30
#define SIZE_TOTAL            65
#define SIZE_SPEED            70
#define SIZE_REMAINING_TIME   70
#define SIZE_ADDRESS          450

#define INIT_MAX_ITEMS 16
#define ARROW_SPACE 15
#define BUTTON_SPACE 4
#define MINIMUM_SPACE 9

KIOListBox::KIOListBox (QWidget *parent, const char *name, 
			      int columns, WFlags f)
  : KTabListBox (parent, name, columns, f)
{
  setColumn(TB_OPERATION, i18n("Operation"), SIZE_OPERATION);
  setColumn(TB_LOCAL_FILENAME, i18n("Local Filename"), SIZE_LOCAL_FILENAME);
  setColumn(TB_RESUME, i18n("Res."), SIZE_RESUME);
  setColumn(TB_COUNT, i18n("Count"), SIZE_COUNT);
  setColumn(TB_PROGRESS, i18n("%"), SIZE_PROGRESS);
  setColumn(TB_TOTAL, i18n("Total"), SIZE_TOTAL);
  setColumn(TB_SPEED, i18n("Speed"), SIZE_SPEED);
  setColumn(TB_REMAINING_TIME, i18n("Rem. Time"), SIZE_REMAINING_TIME);
  setColumn(TB_ADDRESS, i18n("Address ( URL )"), SIZE_ADDRESS);

  readConfig();

   if ( columnWidth( TB_OPERATION ) == 100 ) {
    setColumnWidth( TB_OPERATION, SIZE_OPERATION );
    setColumnWidth( TB_LOCAL_FILENAME, SIZE_LOCAL_FILENAME );
    setColumnWidth( TB_RESUME, SIZE_RESUME );
    setColumnWidth( TB_COUNT, SIZE_COUNT );
    setColumnWidth( TB_PROGRESS, SIZE_PROGRESS );
    setColumnWidth( TB_TOTAL, SIZE_TOTAL );
    setColumnWidth( TB_SPEED, SIZE_SPEED );
    setColumnWidth( TB_REMAINING_TIME, SIZE_REMAINING_TIME );
    setColumnWidth( TB_ADDRESS, SIZE_ADDRESS );
  }
}



KIOListBox::~KIOListBox()
{
  writeConfig();
}


void KIOListBox::readConfig(void)
{
  KConfig* conf = new KConfig("kioslaverc");

  int beg, end, i, w;
  int cols = numCols(),fixedmin=MINIMUM_SPACE;
  QString str, substr;

  conf->setGroup(name());
  str = conf->readEntry("colwidth");

  if (!str.isEmpty())
    for (i=0, beg=0, end=0; i<cols;)
  {
    end = str.find(',', beg);
    if (end < 0) break;
    w = str.mid(beg,end-beg).toInt();
    if (colList[i]->orderType()==ComplexOrder)
       fixedmin+=ARROW_SPACE+labelHeight-BUTTON_SPACE;
    else
       if (colList[i]->orderType()==SimpleOrder)
          fixedmin+=ARROW_SPACE;
    if (w < fixedmin) w = fixedmin;
    colList[i]->setWidth(w);
    colList[i]->setDefaultWidth(w);
    i++;
    beg = end+1;
  }
  else
   for(i=0;i<cols;i++)
   {
     colList[i]->setWidth(60);
     colList[i]->setDefaultWidth(60);
   }
}


void KIOListBox::writeConfig(void)
{
  KConfig* conf = new KConfig("kioslaverc");

  int t;
  QString str;

  conf->setGroup(name());
  QString tmp;
  for(t=0;t<numCols();t++) {
    tmp.setNum(colList[t]->defaultWidth());
    str += tmp;
  }
  conf->writeEntry("colwidth",str);
  conf->sync();
}



//-----------------------------------------------------------------------------

KIOListProgressDlg::KIOListProgressDlg() : KTMainWindow( "" )
{

  readSettings();

  if ( properties != "" ) // !!!
    setGeometry(KWM::setProperties(winId(), properties));
  else
    resize( 460, 150 );

  setCaption("Progress Dialog");
//   setMinimumSize( 350, 150 ); // !!!

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

//   updateStatusBar();

  // setup listbox
  myTabListBox = new KIOListBox( this, "kiolist", 9 );

  setView( myTabListBox, true);

  connect( myTabListBox, SIGNAL( highlighted( int, int )), 
	   this, SLOT( slotSelected( int )));
  connect( myTabListBox, SIGNAL( midClick( int, int )), 
	   this, SLOT( slotUnselected( int )));

  //
  jobmap = KIOJob::s_mapJobs;

  // setup animation timer
  updateTimer = new QTimer( this );
  connect( updateTimer, SIGNAL( timeout() ),
	   this, SLOT( slotUpdate() ) );

  updateTimer->start( 1000 );

  this->hide();
}


KIOListProgressDlg::~KIOListProgressDlg() {
  updateTimer->stop();
  writeSettings();
  KIOJob::m_pListProgressDlg = 0L;
}


// void KIOListProgressDlg::closeEvent( QCloseEvent * ){
//   updateTimer->stop();
//   writeSettings();
// }


void KIOListProgressDlg::slotUpdate()
{

  KIOJob* job;

  int totalFiles = 0;
  int totalSize = 0;
  int totalSpeed = 0;
  QTime totalRemTime;

  int items = 0;

  myTabListBox->setAutoUpdate( false );

  // fill the list with kiojobs
  QMap<int,KIOJob*>::Iterator it = jobmap->begin();
  for( ; it != jobmap->end(); ++it ) {
    job = *it;

    if ( job->m_iGUImode == KIOJob::LIST && job->m_cmd != CMD_NONE ) {
      QString statusString, tmp;

      items++;

      // operation
      switch ( job->m_cmd ) {
      case CMD_COPY:
      case CMD_MCOPY:
	statusString += i18n("Copying");
	break;
      case CMD_MOVE:
      case CMD_MMOVE:
	statusString += i18n("Moving");
	break;
      case CMD_DEL:
      case CMD_MDEL:
	statusString += i18n("Deleting");
	break;
      case CMD_GET:
	statusString += i18n("Getting");
	break;
      case CMD_MOUNT:
	statusString += i18n("Mounting");
	break;
      case CMD_UNMOUNT:
	statusString += i18n("Umnounting");
	break;
      case CMD_MKDIR:
	statusString += i18n("New dir");
	break;
      }

      statusString += "\n";

      // filename
      if ( ! job->m_strTo.isEmpty() ) {
	KURL ur( job->m_strTo );
	statusString += ur.filename();
	statusString += "\n";
      }
      else
	statusString += " \n";

      // can resume
      if ( job->m_bCanResume )
	statusString += i18n("Yes\n");
      else
	statusString += i18n("No\n");
      
      // files
      tmp.sprintf( "%lu / %lu\n", job->m_iProcessedFiles, job->m_iTotalFiles );
      statusString += tmp;
      totalFiles += ( job->m_iTotalFiles - job->m_iProcessedFiles );

      // progress
      int percent = 0;
      if ( job->m_iTotalSize != 0 )
	percent = (int)(( (float)job->m_iProcessedSize / (float)job->m_iTotalSize ) * 100.0);
      statusString += i18n( "%1\n").arg( percent );

      // total size
      statusString += i18n("%1\n").arg( KIOJob::convertSize( job->m_iTotalSize ));
      totalSize += ( job->m_iTotalSize - job->m_iProcessedSize );

      // speed
      if ( job->m_bStalled )
	statusString += i18n("Stalled\n");
      else {
	statusString += KIOJob::convertSize( job->m_iSpeed );
	statusString += "\n";
      }
      totalSpeed += job->m_iSpeed;
      
      // time
      statusString += job->m_RemainingTime.toString();
      statusString += "\n";
      if ( job->m_RemainingTime > totalRemTime )
	totalRemTime = job->m_RemainingTime;

      // insert item into the listbox
      if ( items < myTabListBox->numRows() )
	myTabListBox->changeItem( statusString, items );
      else
	myTabListBox->insertItem( statusString );

    }

    myTabListBox->setNumRows( items );
  }

  myTabListBox->setAutoUpdate( true );

  if ( this->isVisible() )
    myTabListBox->repaint();

  // update statusbar
  statusBar()->changeItem( i18n( " Files : %1 ").arg( totalFiles ), ID_TOTAL_FILES);
  statusBar()->changeItem( i18n( " Size : %1 ").arg( KIOJob::convertSize( totalSize ) ),
			   ID_TOTAL_SIZE);
  statusBar()->changeItem( i18n( " Time : %1 ").arg( totalRemTime.toString() ), ID_TOTAL_TIME);
  statusBar()->changeItem( i18n( " %1/s ").arg( KIOJob::convertSize( totalSpeed ) ),
			   ID_TOTAL_SPEED);

  if ( items > 0 )
    this->show();
  else
    this->hide();
}


void KIOListProgressDlg::slotSelected( int )
{
  updateToolBar();
}


void KIOListProgressDlg::slotUnselected( int id )
{
  myTabListBox->unmarkItem( id );
  updateToolBar();
}


void KIOListProgressDlg::updateToolBar()
{
  int id;

  int count = myTabListBox->count();
  if ( count == 0)
    goto updcln;

  id = myTabListBox->currentItem();
  if ( id >= count || id < 0)
    goto updcln;

  if ( myTabListBox->isMarked(id) ){
    toolBar()->setItemEnabled( TOOL_CANCEL, TRUE);
    return;
  }

 updcln:
  toolBar()->setItemEnabled( TOOL_CANCEL, FALSE); 

}


void KIOListProgressDlg::readSettings() {

  KConfig config("kioslaverc");
  config.setGroup( "ListProgress" );
}


void KIOListProgressDlg::writeSettings() {

  KConfig config("kioslaverc");

  config.setGroup( "ListProgress" );

  config.sync();

  myTabListBox->writeConfig(); // !!!
}


void KIOListProgressDlg::cancelCurrent() {
}


void KIOListProgressDlg::toggleDocking() {
}


#include "kio_listprogress_dlg.moc"
