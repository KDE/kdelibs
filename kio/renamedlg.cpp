#include "kio/renamedlg.h"

#include <stdio.h>
#include <assert.h>

#include <kmessagebox.h>
#include <qfileinfo.h>

#include <kapp.h>
#include <kdialog.h>
#include <klocale.h>
#include <kurl.h>
#include <kprotocolmanager.h>

using namespace KIO;

RenameDlg::RenameDlg(QWidget *parent, const QString &_src, const QString &_dest,
                     RenameDlg_Mode _mode, bool _srcNewer, bool _modal)
  : QDialog ( parent, "" , _modal )
{
  modal = _modal;
  srcNewer = _srcNewer;

  src = _src;
  dest = _dest;

  b0 = b1 = b2 = b3 = b4 = b5 = b6 = b7 = 0L;

  setCaption( i18n( "Rename File" ) );

  b0 = new QPushButton( i18n( "Cancel" ), this );
  connect(b0, SIGNAL(clicked()), this, SLOT(b0Pressed()));

  b1 = new QPushButton( i18n( "Rename" ), this );
  b1->setEnabled(false);
  connect(b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));

  if ( ( _mode & M_MULTI ) && ( _mode & M_SKIP ) ) {
    b2 = new QPushButton( i18n( "Skip" ), this );
    connect(b2, SIGNAL(clicked()), this, SLOT(b2Pressed()));

    b3 = new QPushButton( i18n( "Auto Skip" ), this );
    connect(b3, SIGNAL(clicked()), this, SLOT(b3Pressed()));
  }

  if ( _mode & M_OVERWRITE ) {
    b4 = new QPushButton( i18n( "Overwrite" ), this );
    connect(b4, SIGNAL(clicked()), this, SLOT(b4Pressed()));

    if ( _mode & M_MULTI ) {
      b5 = new QPushButton( i18n( "Overwrite All" ), this );
      connect(b5, SIGNAL(clicked()), this, SLOT(b5Pressed()));
    }
  }

  if ( _mode & M_RESUME ) {
    b6 = new QPushButton( i18n( "Resume" ), this );
    connect(b6, SIGNAL(clicked()), this, SLOT(b6Pressed()));

    if ( _mode & M_MULTI )
      {
	b7 = new QPushButton( i18n( "Resume All" ), this );
	connect(b7, SIGNAL(clicked()), this, SLOT(b7Pressed()));
      }
  }

  m_pLayout = new QVBoxLayout( this, KDialog::marginHint(),
			       KDialog::spacingHint() );
  m_pLayout->addStrut( 360 );	// makes dlg at least that wide

  // User tries to overwrite a file with itself ?
  QLabel *lb;

  if ( _mode & M_OVERWRITE_ITSELF ) {
    lb = new QLabel( i18n("This action would overwrite %1 with itself.\n"
			  "Do you want to rename it instead?").arg(src), this );
  }  else if ( _mode & M_OVERWRITE ) {
      QString sentense1;
      if (srcNewer)
	  sentense1 = i18n("An older item named %1 already exists.\n").arg(dest);
      else
	  sentense1 = i18n("A newer item named %1 already exists.\n").arg(dest);

      lb = new QLabel( sentense1 +
		       i18n("Do you want to replace it with %1,\n"
			    "or rename it?").arg(src), this );
  }  else if ( !(_mode & M_OVERWRITE ) ) {
      QString sentense1;
      if (srcNewer)
	  sentense1 = i18n("An older item than %1 already exists.\n").arg(src);
      else
	  sentense1 = i18n("A newer item than %1 already exists.\n").arg(src);

      lb = new QLabel( sentense1 + i18n("Do you want to rename the existing item?"), this );
  } else
      assert( 0 );

  m_pLayout->addWidget(lb);
  m_pLineEdit = new QLineEdit( this );
  m_pLayout->addWidget( m_pLineEdit );
  m_pLineEdit->setText( dest );
  connect(m_pLineEdit, SIGNAL(textChanged(const QString &)),
	  SLOT(enableRenameButton(const QString &)));

  m_pLayout->addSpacing( 10 );

  QHBoxLayout* layout = new QHBoxLayout();
  m_pLayout->addLayout( layout );

  layout->addStretch(1);

  if ( b1 )
    layout->addWidget( b1 );
  if ( b2 )
    layout->addWidget( b2 );
  if ( b3 )
    layout->addWidget( b3 );
  if ( b4 )
    layout->addWidget( b4 );
  if ( b5 )
    layout->addWidget( b5 );
  if ( b6 )
    layout->addWidget( b6 );
  if ( b7 )
    layout->addWidget( b7 );

  b0->setDefault( true );
  layout->addWidget( b0 );

  resize( sizeHint() );
}

RenameDlg::~RenameDlg()
{
}

void RenameDlg::enableRenameButton(const QString &newDest)
{
  if (newDest != dest)
    b1->setEnabled(true);
  else
    b1->setEnabled(false);
}

void RenameDlg::b0Pressed()
{
  if ( modal )
    done( 0 );
  else
    emit result( this, 0, src, dest );
}

// Rename
void RenameDlg::b1Pressed()
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
    emit result( this, 1, src, m_pLineEdit->text() );
}

void RenameDlg::b2Pressed()
{
  if ( modal )
    done( 2 );
  else
    emit result( this, 2, src, dest );
}

void RenameDlg::b3Pressed()
{
  if ( modal )
    done( 3 );
  else
    emit result( this, 3, src, dest );
}

void RenameDlg::b4Pressed()
{
  if ( modal )
    done( 4 );
  else
    emit result( this, 4, src, dest );
}

void RenameDlg::b5Pressed()
{
  if ( modal )
    done( 5 );
  else
    emit result( this, 5, src, dest );
}

void RenameDlg::b6Pressed()
{
  if ( modal )
    done( 6 );
  else
    emit result( this, 6, src, dest );
}

void RenameDlg::b7Pressed()
{
  if ( modal )
    done( 7 );
  else
    emit result( this, 7, src, dest );
}

RenameDlg_Result KIO::open_RenameDlg( const QString & _src, const QString & _dest,
				 RenameDlg_Mode _mode, bool _srcNewer,
				 QString& _new )
{
  ASSERT(kapp);

  RenameDlg dlg( 0L, _src, _dest, _mode, _srcNewer, true );
  int i = dlg.exec();
  _new = dlg.newName();

  return (RenameDlg_Result)i;
}

unsigned long KIO::getOffset( QString dest ) {

  if ( KProtocolManager::self().markPartial() )
    dest += ".part";

  KURL d( dest );
  QFileInfo info;
  info.setFile( d.path() );

  return info.size();
}

#include "renamedlg.moc"
