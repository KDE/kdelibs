#include <qtimer.h>

#include <kapp.h>
#include <kwm.h>

#include "kio_job.h"
#include "kio_listbox.h"
#include "kio_listprogress_dlg.h"


KIOListProgressDlg::KIOListProgressDlg() : QWidget( 0L )
{

//   readSettings();

  listBox = new KIOListBox( this, "kiolist", 8 );

  // Setup animation timer
  updateTimer = new QTimer( this );
  connect( updateTimer, SIGNAL( timeout() ),
	   this, SLOT( slotUpdate() ) );


  resize( sizeHint() );
}


void KIOListProgressDlg::slotUpdate()
{

  

//   // fill the list with kiojobs
//   map<int,KIOJob*>::iterator it = s_mapJobs->begin();
//   for( ; it != s_mapJobs->end(); ++it )
//     if ( it->second.m_iGUImode == LIST )
//       m_pListProgressDlg->addJob( it->second );

}


#include "kio_listprogress_dlg.moc"
