#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <klocale.h>
#include <kapp.h>

#include "klined.h"
#include "klineeditdlg.h"

KLineEditDlg::KLineEditDlg( const QString&_text, const QString& _value, 
			    QWidget *parent, bool _file_mode )
    : QDialog( parent, 0L, true )
{
//   setGeometry( x(), y(), 350, 100 );
  resize( 350, 100 );

  QLabel *label = new QLabel( _text , this );
  label->setGeometry( 10, 10, 330, 15 );

  edit = new KLineEdit( this, 0L );
    
  if ( _file_mode )
  {
    completion = new KURLCompletion();
    connect ( edit, SIGNAL (completion()),
	      completion, SLOT (make_completion()));
    connect ( edit, SIGNAL (rotation()),
	      completion, SLOT (make_rotation()));
    connect ( edit, SIGNAL (textChanged(const QString&)),
	      completion, SLOT (edited(const QString&)));
    connect ( completion, SIGNAL (setText (const QString&)),
	      edit, SLOT (setText (const QString&)));
  }
  else
    completion = 0L;

  edit->setGeometry( 10, 40, 330, 25 );
  connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

  QPushButton *ok;
  QPushButton *clear;
  QPushButton *cancel;
  ok = new QPushButton( i18n("OK"), this );
  ok->setGeometry( 10,70, 80,25 );
  connect( ok, SIGNAL(clicked()), SLOT(accept()) );

  clear = new QPushButton( i18n("Clear"), this );
  clear->setGeometry( 135, 70, 80, 25 );
  connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );
  
  cancel = new QPushButton( i18n("Cancel"), this );
  cancel->setGeometry( 260, 70, 80, 25 );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
  
  edit->setText( _value );
  edit->setFocus();
}

KLineEditDlg::~KLineEditDlg()
{
	delete completion;
}

void KLineEditDlg::slotClear()
{
    edit->setText("");
}

#include "klineeditdlg.moc"
