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
#include <kcombobox.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcompletion.h>

#include "kurlrequester.h"


class KURLRequester::KURLRequesterPrivate
{
public:
    KURLRequesterPrivate() {
	edit = 0L;
	combo = 0L;
    }
    
    void setText( const QString& text ) {
	if ( combo )
	    combo->setEditText( text );
	else
	    edit->setText( text );
    }
    
    void connectSignals( QObject *receiver ) {
	QObject *sender;
	if ( combo )
	    sender = combo;
	else
	    sender = edit;

	connect( sender, SIGNAL( textChanged( const QString& )),
		 receiver, SIGNAL( textChanged( const QString& )));
	connect( sender, SIGNAL( returnPressed() ),
		 receiver, SIGNAL( returnPressed() ));
	connect( sender, SIGNAL( returnPressed( const QString& ) ),
		 receiver, SIGNAL( returnPressed( const QString& ) ));
    }

    void setCompletionObject( KCompletion *comp ) {
	if ( combo )
	    combo->setCompletionObject( comp );
	else
	    edit->setCompletionObject( comp );
    }
	
    QString text() {
	return combo ? combo->currentText() : edit->text();
    }
    
    
    KLineEdit *edit;
    KComboBox *combo;
};



KURLRequester::KURLRequester( QWidget *editWidget, QWidget *parent, 
			      const char *name, bool modal )
  : QHBox( parent, name )
{
    myModal = modal;
    d = new KURLRequesterPrivate;

    // must have this as parent
    editWidget->reparent( this, 0, QPoint(0,0) );
    d->edit = dynamic_cast<KLineEdit*>( editWidget );
    d->combo = dynamic_cast<KComboBox*>( editWidget );

    init();
}


KURLRequester::KURLRequester( QWidget *parent, const char *name, bool modal )
  : QHBox( parent, name )
{
    myModal = modal;
    d = new KURLRequesterPrivate;
    init();
}


KURLRequester::KURLRequester( const QString& url, QWidget *parent,
			      const char *name, bool modal )
  : QHBox( parent, name )
{
    myModal = modal;
    d = new KURLRequesterPrivate;
    init();
    setURL( url );
}


KURLRequester::~KURLRequester()
{
    delete myCompletion;
    delete myFileDialog;
    delete d;
}


void KURLRequester::init()
{
    myFileDialog    = 0L;
    myShowLocalProt = false;

    if ( !d->combo && !d->edit )
	d->edit = new KLineEdit( this, "line edit" );
    
    myButton = new QPushButton( this, "kfile button" );
    myButton->setPixmap(SmallIcon(QString::fromLatin1("folder")));
    QToolTip::add(myButton, i18n("Open filedialog"));

    setSpacing( KDialog::spacingHint() );
    myButton->setFixedSize( myButton->sizeHint().width(),
    			    myButton->sizeHint().width() );

    if ( d->combo )
	setFocusProxy( d->combo );
    else
	setFocusProxy( d->edit );

    d->connectSignals( this );
    connect( myButton, SIGNAL( clicked() ), this, SLOT( slotOpenDialog() ));

    myCompletion = new KURLCompletion();
    d->setCompletionObject( myCompletion );

    KAccel *accel = new KAccel( this );
    accel->connectItem( KStdAccel::Open, this, SLOT( slotOpenDialog() ));
    accel->readSettings();
}


void KURLRequester::setURL( const QString& url )
{
    bool hasLocalPrefix = (url.left(5) == QString::fromLatin1("file:"));

    if ( !myShowLocalProt && hasLocalPrefix )
	d->setText( url.mid( 5, url.length()-5 ));
    else
	d->setText( url );
}


QString KURLRequester::url() const
{
    return d->text();
};


void KURLRequester::slotOpenDialog()
{
    KFileDialog *dlg = fileDialog();
    if ( !d->text().isEmpty() )
    {
	dlg->setSelection( url() );
    }

    if ( dlg->exec() == QDialog::Accepted )
	setURL( dlg->selectedURL().url() );
}


KFileDialog * KURLRequester::fileDialog() const
{
    if ( !myFileDialog ) {
	QWidget *p = parentWidget();
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

KLineEdit * KURLRequester::lineEdit() const
{
    return d->edit;
}

KComboBox * KURLRequester::comboBox() const
{
    return d->combo;
}

#include "kurlrequester.moc"
