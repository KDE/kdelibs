#include "kio_rename_dlg.h"

#include <stdio.h>
#include <assert.h>

#include <kmessagebox.h>
#include <qfileinfo.h>

#include <kapp.h>
#include <klocale.h>
#include <kurl.h>
#include <kprotocolmanager.h>

KIORenameDlg::KIORenameDlg(QWidget *parent, const char *_src, const char *_dest,
			   RenameDlg_Mode _mode, bool _modal ) :
  QDialog ( parent, "" , _modal )
{
  modal = _modal;
    
  src = _src;
  dest = _dest;

  b0 = b1 = b2 = b3 = b4 = b5 = b6 = b7 = 0L;
    
  setCaption( i18n( "Information" ) );

  b0 = new QPushButton( i18n( "Cancel" ), this );
  connect(b0, SIGNAL(clicked()), this, SLOT(b0Pressed()));
  
  b1 = new QPushButton( i18n( "Rename" ), this );
  connect(b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));
  
  if ( ( _mode & M_MULTI ) && ( _mode & M_SKIP ) )
  {    
    b2 = new QPushButton( i18n( "Skip" ), this );
    connect(b2, SIGNAL(clicked()), this, SLOT(b2Pressed()));

    b3 = new QPushButton( i18n( "Auto Skip" ), this );
    connect(b3, SIGNAL(clicked()), this, SLOT(b3Pressed()));
  }

  if ( _mode & M_OVERWRITE )
  {    
    b4 = new QPushButton( i18n( "Overwrite" ), this );
    connect(b4, SIGNAL(clicked()), this, SLOT(b4Pressed()));
    
    if ( _mode & M_MULTI )
    {
      b5 = new QPushButton( i18n( "Overwrite All" ), this );
      connect(b5, SIGNAL(clicked()), this, SLOT(b5Pressed()));
    }
  }
  
  if ( _mode & M_RESUME )
  {    
    b6 = new QPushButton( i18n( "Resume" ), this );
    connect(b6, SIGNAL(clicked()), this, SLOT(b6Pressed()));
    
    if ( _mode & M_MULTI )
    {
      b7 = new QPushButton( i18n( "Resume All" ), this );
      connect(b7, SIGNAL(clicked()), this, SLOT(b7Pressed()));
    }
  }

  m_pLayout = new QVBoxLayout( this, 10, 0 );
  m_pLayout->addStrut( 360 );	// makes dlg at least that wide
 
  // User tries to overwrite a file with itself ?
  if ( _mode & M_OVERWRITE_ITSELF )
  {
    QLabel *lb = new QLabel( i18n("You try to overwrite"), this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
    m_pLayout->addSpacing( 5 );

    lb = new QLabel( src, this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
    m_pLayout->addSpacing( 5 );

    lb = new QLabel( "with itself. Do you want to rename it to", this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
  }
  else if ( _mode & M_OVERWRITE )
  {	    
    QLabel *lb = new QLabel( dest, this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
    m_pLayout->addSpacing( 5 );

    lb = new QLabel( i18n("already exists. Overwrite with"), this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
    m_pLayout->addSpacing( 5 );

    QString str = src;
    str += " ?";
    lb = new QLabel( str, this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
    m_pLayout->addSpacing( 5 );

    lb = new QLabel( i18n("Or rename to"), this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
  }
  else if ( !(_mode & M_OVERWRITE ) )
  {
    QLabel *lb = new QLabel( src, this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
    m_pLayout->addSpacing( 5 );

    lb = new QLabel( i18n("already exists."), this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
    m_pLayout->addSpacing( 5 );

    lb = new QLabel( i18n("Do you want to rename it ?"), this );
    lb->setFixedHeight( lb->sizeHint().height() );
    m_pLayout->addWidget( lb );
  }
  else
    assert( 0 );
  
  m_pLineEdit = new QLineEdit( this );
  m_pLineEdit->setText( dest );
  m_pLineEdit->setFixedHeight( m_pLineEdit->sizeHint().height() );
  m_pLayout->addSpacing( 10 );
  m_pLayout->addWidget( m_pLineEdit );
  m_pLayout->addSpacing( 10 );
    
  QHBoxLayout* layout = new QHBoxLayout();
  m_pLayout->addLayout( layout );
  if ( b0 )
  {    
    b0->setDefault( true );
    b0->setFixedSize( b0->sizeHint() );
    layout->addWidget( b0 );
    layout->addSpacing( 5 );
  }
  if ( b1 )
  {    
    b1->setFixedSize( b1->sizeHint() );
    layout->addWidget( b1 );
    layout->addSpacing( 5 );
  }
  if ( b2 )
  {    
    b2->setFixedSize( b2->sizeHint() );
    layout->addWidget( b2 );
    layout->addSpacing( 5 );
  }
  if ( b3 )
  {    
    b3->setFixedSize( b3->sizeHint() );
    layout->addWidget( b3 );
    layout->addSpacing( 5 );
  }
  if ( b4 )
  {    
    b4->setFixedSize( b4->sizeHint() );
    layout->addWidget( b4 );
    layout->addSpacing( 5 );
  }
  if ( b5 )
  {
    b5->setFixedSize( b5->sizeHint() );
    layout->addWidget( b5 );
    layout->addSpacing( 5 );
  }
  if ( b6 )
  {
    b6->setFixedSize( b6->sizeHint() );
    layout->addWidget( b6 );
    layout->addSpacing( 5 );
  }
  if ( b7 )
  {
    b7->setFixedSize( b7->sizeHint() );
    layout->addWidget( b7 );
  }
  
  m_pLayout->addStretch( 10 );
  m_pLayout->activate();
  resize( sizeHint() );
}

KIORenameDlg::~KIORenameDlg()
{
}

void KIORenameDlg::b0Pressed()
{
  if ( modal )
    done( 0 );
  else
    emit result( this, 0, src, dest );
}

// Rename
void KIORenameDlg::b1Pressed()
{
    if ( m_pLineEdit->text()  == "" )
	return;

  KURL u( m_pLineEdit->text() );
  if ( u.isMalformed() )
  {
    KMessageBox::error( this, i18n( "Malformed URL\n%1" ).arg(m_pLineEdit->text()));
    return;
  }

  KURL d( dest );
  if ( u.protocol() != d.protocol() ) {
    KMessageBox::error( this, i18n( "You must not change the protocol" ));
    return;
  }

  /*
  if ( dest == m_pLineEdit->text() )
  {
    QMessageBox::warning( this, i18n( "KFM Error" ),
			  i18n( "You did not change the name!\n" ) );
    return;
  }
  */
  if ( modal )
    done( 1 );
  else
    emit result( this, 1, src.ascii(), m_pLineEdit->text().ascii() );
}

void KIORenameDlg::b2Pressed()
{
  if ( modal )
    done( 2 );
  else
    emit result( this, 2, src.ascii(), dest.ascii() );
}

void KIORenameDlg::b3Pressed()
{
  if ( modal )
    done( 3 );
  else
    emit result( this, 3, src.ascii(), dest.ascii() );
}

void KIORenameDlg::b4Pressed()
{
  if ( modal )
    done( 4 );
  else
    emit result( this, 4, src.ascii(), dest.ascii() );
}

void KIORenameDlg::b5Pressed()
{
  if ( modal )
    done( 5 );
  else
    emit result( this, 5, src.ascii(), dest.ascii() );
}

void KIORenameDlg::b6Pressed()
{
  if ( modal )
    done( 6 );
  else
    emit result( this, 6, src.ascii(), dest.ascii() );
}

void KIORenameDlg::b7Pressed()
{
  if ( modal )
    done( 7 );
  else
    emit result( this, 7, src.ascii(), dest.ascii() );
}

RenameDlg_Result open_RenameDlg( const char* _src, const char *_dest, RenameDlg_Mode _mode, QString& _new )
{
  if ( kapp == 0L )
  {
    const char* a[2] = { "dummy", 0L };
    int b = 1;
    (void)new KApplication( b, const_cast<char**>(a) );
  }
  
  KIORenameDlg dlg( 0L, _src, _dest, _mode, true );
  int i = dlg.exec();
  _new = dlg.newName();

  return (RenameDlg_Result)i;
}

unsigned long getOffset( QString dest ) {

  if ( KProtocolManager::self().markPartial() )
    dest += ".part";

  KURL d( dest );
  QFileInfo info;
  info.setFile( d.path() );

  return info.size();
}

#include "kio_rename_dlg.moc"
