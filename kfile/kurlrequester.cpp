/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <qstring.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <kaccel.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcompletion.h>

#include "kurlrequester.h"


KURLRequester::KURLRequester( QWidget *parent, const char *name, bool modal )
  : QHBox( parent, name )
{
    myModal = modal;
    init();
}


KURLRequester::KURLRequester( const QString& url, QWidget *parent,
			      const char *name, bool modal )
  : QHBox( parent, name )
{
    myModal = modal;
    init();
    setURL( url );
}


KURLRequester::~KURLRequester()
{
    delete myCompletion;
    delete myFileDialog;
}


void KURLRequester::init()
{
    myFileDialog    = 0L;
    myShowLocalProt = false;

    myEdit = new KLineEdit( this, "line edit" );
    myButton = new QPushButton( this, "kfile button" );
    myButton->setPixmap(SmallIcon(QString::fromLatin1("folder")));
    QToolTip::add(myButton, i18n("Open filedialog"));

    setSpacing( KDialog::spacingHint() );
    myButton->setFixedSize( myButton->sizeHint().width(),
    			    myButton->sizeHint().width() );

    setFocusProxy( myEdit );

    connect( myEdit, SIGNAL( textChanged( const QString& )),
	     this, SIGNAL( textChanged( const QString& )));
    connect( myButton, SIGNAL( clicked() ),
	     this, SLOT( slotOpenDialog() ));
    connect( myEdit, SIGNAL( returnPressed() ),
	     this, SIGNAL( returnPressed() ));
    connect( myEdit, SIGNAL( returnPressed( const QString& ) ),
	     this, SIGNAL( returnPressed( const QString& ) ));

    myCompletion = new KURLCompletion();

    myEdit->setCompletionObject( myCompletion );

    KAccel *accel = new KAccel( this );
    accel->connectItem( KStdAccel::Open, this, SLOT( slotOpenDialog() ));
    accel->readSettings();
}


void KURLRequester::setURL( const QString& url )
{
    bool hasLocalPrefix = (url.left(5) == QString::fromLatin1("file:"));

    if ( !myShowLocalProt && hasLocalPrefix )
	myEdit->setText( url.mid( 5, url.length()-5 ));
    else
	myEdit->setText( url );
}


QString KURLRequester::url() const
{
    return myEdit->text();
};


void KURLRequester::slotOpenDialog()
{
    KFileDialog *dlg = fileDialog();
    if ( !myEdit->text().isEmpty() ) 
    {
	dlg->setSelection( url() );
    }

    if ( dlg->exec() == QDialog::Accepted )
	setURL( dlg->selectedURL().url() );
}


KFileDialog * KURLRequester::fileDialog() const
{
    if ( !myFileDialog ) {
	QWidget *p = static_cast<QWidget *>( parent() );
	myFileDialog = new KFileDialog( QString::null, QString::null, p,
					"file dialog", myModal );
	KFile::Mode mode = static_cast<KFile::Mode>( KFile::File |
						     KFile::ExistingOnly |
						     KFile::LocalOnly );
	myFileDialog->setMode( mode );
    }

    return myFileDialog;
}


void KURLRequester::setShowLocalProtocol( bool b )
{
    if ( myShowLocalProt == b )
	return;

    myShowLocalProt = b;
    setURL( url() );
}

#include "kurlrequester.moc"
