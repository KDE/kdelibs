/* This file is part of the KDE libraries
    Copyright (C) 1999 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include <sys/stat.h>
#include <unistd.h>

#include <klineedit.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtoolbutton.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "kurledit.h"


KURLEdit::KURLEdit( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  init();
}


KURLEdit::KURLEdit( const QString& url, QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  init();
  setURL( url );
}


KURLEdit::~KURLEdit()
{
}


void KURLEdit::init()
{
  myVerifyTimer = 0L;
  myIsDirty     = true;

  myEdit = new KLineEdit( this, "line edit" );
  myButton = new QToolButton( this, "kfile button" );
  myButton->setPixmap(
	 KGlobal::iconLoader()->loadIcon(QString::fromLatin1("folder"),
						    KIconLoader::Small,
						    0L, false ) );

  myEdit->move( 0, 0 );
  myButton->setFixedSize( myButton->sizeHint().width(),
			  myEdit->sizeHint().height() );

  connect( myEdit, SIGNAL( textChanged( const QString& )),
	   this, SLOT( slotTextChanged( const QString& )));
  connect( myEdit, SIGNAL( returnPressed() ),
	   this, SIGNAL( returnPressed() ));
  connect( myButton, SIGNAL( clicked() ),
	   this, SLOT( slotOpenDialog() ));

  setEnableVerify( true ); // default: do verify
}


void KURLEdit::setURL( const QString& url )
{
  myURL = url;
  myIsDirty = true;
  myEdit->setText( url );

  if ( myVerifyTimer )
    slotDoVerify();
}

QString KURLEdit::url( bool omitProtocol ) const
{
    if ( omitProtocol )
	return myURL.path();
    else
	return myURL.url();
}


void KURLEdit::setEnableVerify( bool enable )
{
  if ( enable ) {
    if ( myVerifyTimer )
      return;

    myVerifyTimer = new QTimer( this, "verify timer" );
    connect( myVerifyTimer, SIGNAL( timeout() ), this, SLOT( slotDoVerify() ));
  }
  else {
    delete myVerifyTimer;
    myVerifyTimer = 0L;
  }
}


bool KURLEdit::exists()
{
  if ( myIsDirty ) {
    slotDoVerify();
  }

  return myExists;
}


// slots:

// FIXME: follow symlinks or not??
void KURLEdit::slotDoVerify()
{
  if ( !myIsDirty )
    return;

  if ( myURL.isLocalFile() ) {
    struct stat buf;
    myExists = (stat( myURL.path(-1).local8Bit(), &buf ) == 0);
  }

  else { // TODO: remote files

  }

  // italic font when not existant?
  //  QFont font = myEdit->font();
  //  font.setItalic( !myExists );
  //  myEdit->setFont( font );

  myIsDirty = false;
}


// TODO: completion (use KEdit instead of KLineEdit for the signal?)
void KURLEdit::slotTextChanged( const QString& text )
{
  myURL = text;
  myIsDirty = true;
  myVerifyTimer->start( 200, true );

  emit urlChanged( text );
}


void KURLEdit::slotOpenDialog()
{

}


QSize KURLEdit::sizeHint() const
{
    QSize s1 = myButton->sizeHint();
    QSize s2 = myEdit->sizeHint();
    int w = s1.width() + KDialog::spacingHint() + s2.width();
    int h = QMAX( s1.height(), s2.height() );
    return QSize( w, h );
}


void KURLEdit::resizeEvent( QResizeEvent * )
{
  int s = KDialog::spacingHint();

  myEdit->resize( (width() - myButton->width() - s), myEdit->height() );
  myButton->move( myEdit->x() + myEdit->width() + s, myButton->y() );
}


#include "kurledit.moc"
