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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "kurlrequester.h"

#include <kaccel.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdirselectdialog.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcompletion.h>
#include <kprotocolinfo.h>

#include <qevent.h>
#include <qstring.h>
#include <qdrag.h>
#include <qmimedata.h>

#include <sys/stat.h>
#include <unistd.h>
#include <kvbox.h>


class KUrlDragPushButton : public KPushButton
{
public:
    KUrlDragPushButton( QWidget *parent)
	: KPushButton( parent) {
    	setDragEnabled( true );
    }
    ~KUrlDragPushButton() {}

    void setURL( const KUrl& url ) {
	m_urls.clear();
	m_urls.append( url );
    }

    /* not needed so far
    void setURLs( const KUrl::List& urls ) {
	m_urls = urls;
    }
    const KUrl::List& urls() const { return m_urls; }
    */

protected:
    virtual QDrag *dragObject() {
	if ( m_urls.isEmpty() )
	    return 0;

	QDrag *drag = new QDrag( this );
        m_urls.populateMimeData( drag->mimeData() );
	return drag;
    }

private:
    KUrl::List m_urls;

};


/*
*************************************************************************
*/

class KUrlRequester::KUrlRequesterPrivate
{
public:
    KUrlRequesterPrivate() {
	edit = 0L;
	combo = 0L;
        fileDialogMode = KFile::File | KFile::ExistingOnly | KFile::LocalOnly;
    }

    void setText( const QString& text ) {
	if ( combo )
	{
	    if (combo->editable())
	    {
               combo->setEditText( text );
            }
            else
            {
               combo->insertItem( text );
               combo->setCurrentItem( combo->count()-1 );
            }
        }
	else
	{
	    edit->setText( text );
	}
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
        KUrlCompletion *comp;
        if ( combo )
            comp = dynamic_cast<KUrlCompletion*>(combo->completionObject());
        else
            comp = dynamic_cast<KUrlCompletion*>(edit->completionObject());

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



KUrlRequester::KUrlRequester( QWidget *editWidget, QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate)
{

    // must have this as parent
    editWidget->reparent( this, 0, QPoint(0,0) );
    d->edit = dynamic_cast<KLineEdit*>( editWidget );
    d->combo = dynamic_cast<KComboBox*>( editWidget );

    init();
}


KUrlRequester::KUrlRequester( QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate)
{
    init();
}


KUrlRequester::KUrlRequester( const QString& url, QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate)
{
    init();
    setKUrl( KUrl::fromPathOrURL( url ) );
}


KUrlRequester::~KUrlRequester()
{
    delete myCompletion;
    delete myFileDialog;
    delete d;
}


void KUrlRequester::init()
{
    setMargin(0);

    myFileDialog    = 0L;
    myShowLocalProt = false;

    if ( !d->combo && !d->edit )
	d->edit = new KLineEdit( this);

    myButton = new KUrlDragPushButton( this);
    QIcon iconSet = SmallIconSet(QLatin1String("fileopen"));
    QPixmap pixMap = iconSet.pixmap( QIcon::Small, QIcon::Normal );
    myButton->setIcon( iconSet );
    myButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
    myButton->setToolTip(i18n("Open file dialog"));

    connect( myButton, SIGNAL( pressed() ), SLOT( slotUpdateURL() ));

    setSpacing( KDialog::spacingHint() );

    QWidget *widget = d->combo ? (QWidget*) d->combo : (QWidget*) d->edit;
    setFocusProxy( widget );

    d->connectSignals( this );
    connect( myButton, SIGNAL( clicked() ), this, SLOT( slotOpenDialog() ));

    myCompletion = new KUrlCompletion();
    d->setCompletionObject( myCompletion );

    KAccel *accel = new KAccel( this );
    accel->insert( KStdAccel::Open, this, SLOT( slotOpenDialog() ));
    accel->readSettings();
}


void KUrlRequester::setURL( const QString& url )
{
    if ( myShowLocalProt )
    {
        d->setText( url );
    }
    else
    {
        // ### This code is broken (e.g. for paths with '#')
        if ( url.startsWith("file://") )
            d->setText( url.mid( 7 ) );
        else if ( url.startsWith("file:") )
            d->setText( url.mid( 5 ) );
        else
            d->setText( url );
    }
}

void KUrlRequester::setKUrl( const KUrl& url )
{
    if ( myShowLocalProt )
        d->setText( url.url() );
    else
        d->setText( url.pathOrURL() );
}

void KUrlRequester::changeEvent(QEvent *e)
{
   if (e->type()==QEvent::WindowTitleChange) {
     if (myFileDialog) {
	//kdDebug()<<"CHANGEEVENT"<<endl;
        myFileDialog->setWindowTitle( windowTitle() );
     }
   }
   KHBox::changeEvent(e);
}

QString KUrlRequester::url() const
{
    return d->url();
}

void KUrlRequester::slotOpenDialog()
{
    KUrl newurl;
    if ( (d->fileDialogMode & KFile::Directory) && !(d->fileDialogMode & KFile::File) ||
         /* catch possible fileDialog()->setMode( KFile::Directory ) changes */
         (myFileDialog && ( (myFileDialog->mode() & KFile::Directory) &&
         (myFileDialog->mode() & (KFile::File | KFile::Files)) == 0 ) ) )
    {
        newurl = KDirSelectDialog::selectDirectory(url(), d->fileDialogMode & KFile::LocalOnly);
        if ( !newurl.isValid() )
        {
            return;
        }
    }
    else
    {
      emit openFileDialog( this );

      KFileDialog *dlg = fileDialog();
      if ( !d->url().isEmpty() ) {
          KUrl u( url() );
          // If we won't be able to list it (e.g. http), then don't try :)
          if ( KProtocolInfo::supportsListing( u ) )
              dlg->setSelection( u.url() );
      }

      if ( dlg->exec() != QDialog::Accepted )
      {
          return;
      }

      newurl = dlg->selectedURL();
    }

    setKUrl( newurl );
    emit urlSelected( d->url() );
}

void KUrlRequester::setMode(unsigned int mode)
{
    Q_ASSERT( (mode & KFile::Files) == 0 );
    d->fileDialogMode = mode;
    if ( (mode & KFile::Directory) && !(mode & KFile::File) )
        myCompletion->setMode( KUrlCompletion::DirCompletion );

    if (myFileDialog)
       myFileDialog->setMode( d->fileDialogMode );
}

unsigned int KUrlRequester::mode() const
{
    return d->fileDialogMode;
}

void KUrlRequester::setFilter(const QString &filter)
{
    d->fileDialogFilter = filter;
    if (myFileDialog)
       myFileDialog->setFilter( d->fileDialogFilter );
}

QString KUrlRequester::filter( ) const
{
    return d->fileDialogFilter;
}


KFileDialog * KUrlRequester::fileDialog() const
{
    if ( !myFileDialog ) {
        QWidget *p = parentWidget();
        myFileDialog = new KFileDialog( QString(), d->fileDialogFilter, p,
                                        "file dialog", true );

        myFileDialog->setMode( d->fileDialogMode );
        myFileDialog->setWindowTitle( windowTitle() );
    }

    return myFileDialog;
}


void KUrlRequester::setShowLocalProtocol( bool b )
{
    if ( myShowLocalProt == b )
	return;

    myShowLocalProt = b;
    setKUrl( url() );
}

void KUrlRequester::clear()
{
    d->setText( QString() );
}

KLineEdit * KUrlRequester::lineEdit() const
{
    return d->edit;
}

KComboBox * KUrlRequester::comboBox() const
{
    return d->combo;
}

void KUrlRequester::slotUpdateURL()
{
    // bin compat, myButton is declared as QPushButton
    KUrl u;
    u = KUrl( KUrl( QDir::currentPath() + '/' ), url() );
    (static_cast<KUrlDragPushButton *>( myButton ))->setURL( u );
}

KPushButton * KUrlRequester::button() const
{
    return myButton;
}

KEditListBox::CustomEditor KUrlRequester::customEditor()
{
    setSizePolicy(QSizePolicy( QSizePolicy::Preferred,
                               QSizePolicy::Fixed));

    KLineEdit *edit = d->edit;
    if ( !edit && d->combo )
        edit = dynamic_cast<KLineEdit*>( d->combo->lineEdit() );

#ifndef NDEBUG
    if ( !edit )
        kdWarning() << "KUrlRequester's lineedit is not a KLineEdit!??\n";
#endif

    KEditListBox::CustomEditor editor( this, edit );
    return editor;
}

void KUrlRequester::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

KUrlComboRequester::KUrlComboRequester( QWidget *parent)
  : KUrlRequester( new KComboBox(false), parent)
{
}

#include "kurlrequester.moc"
