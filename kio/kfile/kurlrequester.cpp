/* This file is part of the KDE libraries
    Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

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
#include <qtooltip.h>

#include <kaccel.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcompletion.h>
#include <kurldrag.h>
#include <kprotocolinfo.h>

#include "kurlrequester.h"


class KURLDragPushButton : public KPushButton
{
public:
    KURLDragPushButton( QWidget *parent, const char *name=0 )
	: KPushButton( parent, name ) {
    	setDragEnabled( true );
    }
    ~KURLDragPushButton() {}

    void setURL( const KURL& url ) {
	m_urls.clear();
	m_urls.append( url );
    }

    /* not needed so far
    void setURLs( const KURL::List& urls ) {
	m_urls = urls;	
    }
    const KURL::List& urls() const { return m_urls; }
    */

protected:
    virtual QDragObject *dragObject() {
	if ( m_urls.isEmpty() )
	    return 0L;

	QDragObject *drag = KURLDrag::newDrag( m_urls, this, "url drag" );
	return drag;
    }

private:
    KURL::List m_urls;

};


/*
*************************************************************************
*/

class KURLRequester::KURLRequesterPrivate
{
public:
    KURLRequesterPrivate() {
	edit = 0L;
	combo = 0L;
        fileDialogMode = KFile::File | KFile::ExistingOnly | KFile::LocalOnly;
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
	
    /**
     * replaces ~user or $FOO, if necessary
     */
    QString url() {
        QString txt = combo ? combo->currentText() : edit->text();
        KURLCompletion *comp;
        if ( combo )
            comp = dynamic_cast<KURLCompletion*>(combo->completionObject());
        else
            comp = dynamic_cast<KURLCompletion*>(edit->completionObject());

        if ( comp )
            return comp->replacedPath( txt );
        else
            return txt;
    }

    KLineEdit *edit;
    KComboBox *combo;
    int fileDialogMode;
    QString fileDialogFilter;
};



KURLRequester::KURLRequester( QWidget *editWidget, QWidget *parent,
			      const char *name )
  : QHBox( parent, name )
{
    d = new KURLRequesterPrivate;

    // must have this as parent
    editWidget->reparent( this, 0, QPoint(0,0) );
    d->edit = dynamic_cast<KLineEdit*>( editWidget );
    d->combo = dynamic_cast<KComboBox*>( editWidget );

    init();
}


KURLRequester::KURLRequester( QWidget *parent, const char *name )
  : QHBox( parent, name )
{
    d = new KURLRequesterPrivate;
    init();
}


KURLRequester::KURLRequester( const QString& url, QWidget *parent,
			      const char *name )
  : QHBox( parent, name )
{
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

    myButton = new KURLDragPushButton( this, "kfile button");
    myButton->setPixmap(SmallIcon(QString::fromLatin1("fileopen")));
    QToolTip::add(myButton, i18n("Open File Dialog"));

    connect( myButton, SIGNAL( pressed() ), SLOT( slotUpdateURL() ));

    setSpacing( KDialog::spacingHint() );

    QWidget *widget = d->combo ? (QWidget*) d->combo : (QWidget*) d->edit;
    setFocusProxy( widget );

    // This is really weird, but seems to be the only way to get the button
    // to not grow to some weird big size and the lineedit take the same
    // height. Note: order is important here. As I said, weird.
    myButton->setFixedHeight( myButton->sizeHint().height() );
    myButton->setFixedWidth( myButton->sizeHint().width() );
    widget->setMinimumHeight( QMAX( widget->minimumHeight(),
                                    myButton->height() ));


    d->connectSignals( this );
    connect( myButton, SIGNAL( clicked() ), this, SLOT( slotOpenDialog() ));

    myCompletion = new KURLCompletion();
    d->setCompletionObject( myCompletion );

    KAccel *accel = new KAccel( this );
    accel->insert( KStdAccel::Open, this, SLOT( slotOpenDialog() ));
    accel->readSettings();
}


void KURLRequester::setURL( const QString& url )
{
    bool hasLocalPrefix = (url.startsWith("file:"));

    if ( !myShowLocalProt && hasLocalPrefix )
	d->setText( url.mid( 5, url.length()-5 ));
    else
	d->setText( url );
}

void KURLRequester::setCaption( const QString& caption )
{
	fileDialog()->setCaption( caption );
	QWidget::setCaption( caption );
}

QString KURLRequester::url() const
{
    return d->url();
};


void KURLRequester::slotOpenDialog()
{
    emit openFileDialog( this );

    KFileDialog *dlg = fileDialog();
    if ( !d->url().isEmpty() ) {
        KURL u( url() );
        // If we won't be able to list it (e.g. http), then don't try :)
        if ( KProtocolInfo::supportsListing( u.protocol() ) )
	    dlg->setSelection( u.url() );
    }

    if ( dlg->exec() == QDialog::Accepted )
    {
	setURL( dlg->selectedURL().prettyURL() );
        emit urlSelected( d->url() );
    }
}

void KURLRequester::setMode(unsigned int mode)
{
    Q_ASSERT( (mode & KFile::Files) == 0 );
    d->fileDialogMode = mode;
    if (myFileDialog)
       myFileDialog->setMode( d->fileDialogMode );
}

void KURLRequester::setFilter(const QString &filter)
{
    d->fileDialogFilter = filter;
    if (myFileDialog)
       myFileDialog->setFilter( d->fileDialogFilter );
}

KFileDialog * KURLRequester::fileDialog() const
{
    if ( !myFileDialog ) {
	QWidget *p = parentWidget();
	myFileDialog = new KFileDialog( QString::null, QString::null, p,
					"file dialog", true );
	
	myFileDialog->setMode( d->fileDialogMode );
        if (!d->fileDialogFilter.isEmpty())
           myFileDialog->setFilter( d->fileDialogFilter );
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

void KURLRequester::clear()
{
    d->setText( QString::null );
}

KLineEdit * KURLRequester::lineEdit() const
{
    return d->edit;
}

KComboBox * KURLRequester::comboBox() const
{
    return d->combo;
}

void KURLRequester::slotUpdateURL()
{
    // bin compat, myButton is declared as QPushButton
    KURL u( QDir::currentDirPath() + '/', url() );
    (static_cast<KURLDragPushButton *>( myButton))->setURL( u );
}

KPushButton * KURLRequester::button() const
{
    return myButton;
}

KEditListBox::CustomEditor KURLRequester::customEditor()
{
    setSizePolicy(QSizePolicy( QSizePolicy::Preferred, 
                               QSizePolicy::Fixed));
    
    KLineEdit *edit = d->edit;
    if ( !edit && d->combo )
        edit = dynamic_cast<KLineEdit*>( d->combo->lineEdit() );

#ifndef NDEBUG
    if ( !edit )
        kdWarning() << "KURLRequester's lineedit is not a KLineEdit!??\n";
#endif

    KEditListBox::CustomEditor editor( this, edit );
    return editor;
}

void KURLRequester::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kurlrequester.moc"
