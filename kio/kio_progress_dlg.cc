#include "kio_progress_dlg.h"
#include "kio_job.h"

#include <qpushbt.h>

#include <kapp.h>
#include <kwm.h>

KIOCopyProgressDlg::KIOCopyProgressDlg( KIOJob* _job, bool m_bStartIconified ) : QDialog( 0L )
{
  m_iTotalSize = 0;
  m_iTotalFiles = 0;
  m_iTotalDirs = 0;
  m_iProcessedSize = 0;

  m_pJob = _job;
  
  m_pProgressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, this );
  m_pLine1 = new QLabel( this );
  m_pLine2 = new QLabel( this );
  m_pLine3 = new QLabel( this );
  m_pLine4 = new QLabel( this );
  m_pLine5 = new QLabel( this );

  m_pLayout = new QVBoxLayout( this, 10, 0 );
  m_pLayout->addStrut( 360 );	// makes dlg at least that wide
  if ( m_pLine1 != 0L )
  {
    m_pLine1->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine1 );
  }

  if ( m_pLine2 != 0L )
  {
    m_pLine2->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine2 );
  }

  if ( m_pLine3 != 0L )
  {
    m_pLine3->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine3 );
  }

  if ( m_pProgressBar != 0L )
  {
    m_pProgressBar->setFixedHeight( 20 );
    m_pLayout->addSpacing( 10 );
    m_pLayout->addWidget( m_pProgressBar );
  }

  if ( m_pLine4 != 0L )
  {
    m_pLine4->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine4 );
  }

  if ( m_pLine5 != 0L )
  {
    m_pLine5->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine5 );
  }

  QPushButton *pb = new QPushButton( i18n("Cancel"), this );
  pb->setFixedSize( pb->sizeHint() );
  connect( pb, SIGNAL( clicked() ), m_pJob, SLOT( slotCancel() ) );
  m_pLayout->addSpacing( 10 );
  m_pLayout->addWidget( pb );

  m_pLayout->addStretch( 10 );
  m_pLayout->activate();
  resize( sizeHint() );

  this->show();

  if ( m_bStartIconified )
    KWM::setIconify( this->winId(), true );
}

void KIOCopyProgressDlg::totalSize( unsigned long _bytes )
{
  m_iTotalSize = _bytes;
}

void KIOCopyProgressDlg::totalDirs( unsigned long _dirs )
{
  m_iTotalDirs = _dirs;
}

void KIOCopyProgressDlg::totalFiles( unsigned long _files )
{
  m_iTotalFiles = _files;
  processedFiles( 0 );
}

void KIOCopyProgressDlg::processedSize( unsigned long _bytes )
{
  if ( _bytes == 0 )
    return;
  
  m_iProcessedSize = _bytes;
  
  char ext1[ 100 ];
  char ext2[ 100 ];
  unsigned long div1, div2;
  if ( _bytes <= 2000 )
  {
    strcpy( ext1, i18n( "Bytes" ) );
    div1 = 1;
  }
  else if ( _bytes <= 1000000 )
  {
    strcpy( ext1, i18n( "Kb" ) );
    div1 = 1000;
  }
  else
  {
    strcpy( ext1, i18n( "Mb" ) );
    div1 = 1000000;
  }

  if ( m_iTotalSize <= 2000 )
  {
    strcpy( ext2, i18n( "Bytes" ) );
    div2 = 1;
  }
  else if ( m_iTotalSize <= 1000000 )
  {
    strcpy( ext2, i18n( "Kb" ) );
    div2 = 1000;
  }
  else
  {
    strcpy( ext2, i18n( "Mb" ) );
    div2 = 1000000;
  }

  char buffer[ 100 ];
  sprintf( buffer, "%i %s %s %i %s", (int)_bytes / (int)div1, ext1, i18n("of"), (int)m_iTotalSize / (int)div2, ext2 );
  m_pLine4->setText( buffer );

  int progress = (int)( (float)_bytes / (float)m_iTotalSize * 100.0 );
  if ( m_iTotalSize )
    m_pProgressBar->setValue( progress );

  sprintf( buffer, "%i %% %s %i %s", progress, i18n("of"), (int)m_iTotalSize / (int)div2, ext2 );
  setCaption( buffer );
}

void KIOCopyProgressDlg::processedDirs( unsigned long _dirs )
{
  char buffer[ 200 ];
  sprintf( buffer, "%i/%i directories created", (int)_dirs, (int)m_iTotalDirs );
  m_pLine1->setText( buffer );
}

void KIOCopyProgressDlg::processedFiles( unsigned long _files )
{
  char buffer[ 200 ];
  sprintf( buffer, "%i/%i files", (int)_files, (int)m_iTotalFiles );
  m_pLine1->setText( buffer );
}

void KIOCopyProgressDlg::speed( unsigned long _bytes_per_second )
{
  if ( m_iProcessedSize == 0 )
    return;
  
  if ( _bytes_per_second == 0 )
  {
    m_pLine5->setText( i18n( "Stalled" ) );
    return;
  }
  
  unsigned long div1;
  char ext1[ 100 ];
  if ( _bytes_per_second <= 2000 )
  {
    strcpy( ext1, i18n( " Bytes/s" ) );
    div1 = 1;
  }
  else if ( _bytes_per_second <= 1000000 )
  {
    strcpy( ext1, i18n( " Kb/s" ) );
    div1 = 1000;
  }
  else
  {
    strcpy( ext1, i18n( " Mb/s" ) );
    div1 = 1000000;
  }

  char t[ 100 ];
  
  if ( _bytes_per_second == 0 )
  {
    m_pLine5->setText( i18n( "Stalled" ) );
    return;
  }
  
  unsigned long secs = ( m_iTotalSize - m_iProcessedSize ) / _bytes_per_second;
  if ( secs < 60 )
  {
    sprintf( t, i18n( "%i seconds" ), secs );
  }
  else if ( secs < 60 * 60 )
  {
    int m = secs / 60;
    int s = secs - m * 60;
    sprintf( t, i18n( "%i:%i" ), m, s );
  }
  else
  {
    int h = secs / ( 60 * 60 );
    int m = ( secs - h * 60 * 60 ) / 60;
    int s = ( secs - h * 60 * 60 - m * 60 );
    sprintf( t, i18n( "%i:%i:%i" ), h, m, s );
  }
  
  char buffer[ 200 ];
  sprintf( buffer, i18n( "%i%s  Remaining time: %s" ), _bytes_per_second / div1, ext1, t );
  m_pLine5->setText( buffer );
}

void KIOCopyProgressDlg::scanningDir( const char *_dir )
{
  string tmp = i18n( "Scanning " );
  tmp += _dir;
  m_pLine2->setText( tmp.c_str() );
}

void KIOCopyProgressDlg::copyingFile( const char *_from, const char *_to )
{
  string tmp = i18n("From : ");
  tmp += _from;
  m_pLine2->setText( tmp.c_str() );

  tmp = i18n("To: ");
  tmp += _to;
  m_pLine3->setText( tmp.c_str() );
}

void KIOCopyProgressDlg::makingDir( const char *_dir )
{
  string tmp = i18n( "Creating dir " );
  tmp += _dir;
  m_pLine2->setText( tmp.c_str() );
}

void KIOCopyProgressDlg::gettingFile( const char *_url )
{
  m_pLine1->setText( i18n("Fetching file") );
  m_pLine2->setText( _url );
}

void KIOCopyProgressDlg::deletingFile( const char *_url )
{
  m_pLine1->setText( i18n("Deleting file") );
  m_pLine2->setText( _url );
}

#include "kio_progress_dlg.moc"
