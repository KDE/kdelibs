#include "kio_pass_dlg.h"

#include <string.h>

#include <qpushbt.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qaccel.h>

#include <klocale.h>
#include <kapp.h>

KIOPassDlg::KIOPassDlg( QWidget* parent, const char* name, bool modal, WFlags wflags,
				const char *_head, const char *_user, const char *_pass )
   : QDialog(parent, name, modal, wflags)
{
   QVBoxLayout *layout = new QVBoxLayout( this );

   //
   // Bei Bedarf einen kleinen Kommentar als Label einfuegen
   //
   if ( _head )
   {
      QLabel *l = new QLabel(_head, this);
      l->setFixedSize( l->sizeHint() );
      layout->addWidget( l );
      layout->addSpacing( 5 );
   }
   
   QGridLayout *grid = new QGridLayout( 3, 5 );
   layout->addLayout( grid );

   QLabel *l = new QLabel( i18n( "User" ), this );
   l->setFixedSize( l->sizeHint() );
   grid->addWidget( l, 0, 1 );
   grid->addRowSpacing( 1, 5 );
   grid->addColSpacing( 2, 5 );
   grid->addColSpacing( 0, 5 );
   grid->addColSpacing( 4, 5 );
   m_pUser = new QLineEdit( this );
   m_pUser->setFixedHeight( m_pUser->sizeHint().height() );
   m_pUser->setMinimumWidth( 200 );
   grid->addWidget( m_pUser, 0, 3 );
   l = new QLabel( i18n( "Password" ), this );
   l->setFixedSize( l->sizeHint() );
   grid->addWidget( l, 2, 1 );
   m_pPass = new QLineEdit( this );
   m_pPass->setFixedHeight( m_pPass->sizeHint().height() );
   m_pPass->setEchoMode( QLineEdit::Password );
   m_pPass->setMinimumWidth( 200 );
   grid->addWidget( m_pPass, 2, 3 );

   if ( _user )
     m_pUser->setText( _user );
   if ( _pass )
     m_pPass->setText( _pass );
      
   layout->addSpacing( 10 );

   //
   // Connect vom LineEdit herstellen und Accelerator
   //
   QAccel *ac = new QAccel(this);
   ac->connectItem( ac->insertItem(Key_Escape), this, SLOT(reject()) );
   
   connect( m_pPass, SIGNAL(returnPressed()), SLOT(accept()) );
   
   //
   // Die Buttons "Ok" & "Cancel" erzeugen
   //
   QHBoxLayout *hlayout = new QHBoxLayout;
   layout->addLayout( hlayout );

   QPushButton *b1, *b2;
   b1 = new QPushButton( i18n("Ok"), this);
   b1->setFixedSize( b1->sizeHint() );
   hlayout->addWidget( b1 );
   b2 = new QPushButton( i18n("Cancel"), this);
   b2->setFixedSize( b2->sizeHint() );
   hlayout->addWidget( b2 );
   
   
   // Buttons mit Funktionaliataet belegen
   connect( b1, SIGNAL(clicked()), SLOT(accept()) );
   connect( b2, SIGNAL(clicked()), SLOT(reject()) );
   
   // Fenstertitel
   setCaption( i18n("Password") );
   
   // Focus
   if ( _user == 0 || _user[0] == 0 )
     m_pUser->setFocus();
   else
     m_pPass->setFocus();
   
   layout->addStretch( 10 );
   layout->activate();
   resize( sizeHint() );
}

bool open_PassDlg( const char *_head, string& _user, string& _pass )
{
  if ( kapp == 0L )
  {
    char* a[2] = { "dummy", 0L };
    int b = 1;
    (void)new KApplication( b, a );
  }
  
  KIOPassDlg dlg( 0L, 0L, true, 0, _head, _user.c_str(), _pass.c_str() );
  if ( dlg.exec() )
  {
    _user = dlg.user();
    _pass = dlg.password();
    return true;
  }
  else
    return false;
}

#include "kio_pass_dlg.moc"
