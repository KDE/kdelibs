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
#include <kprotocolmanager.h>
#include <kstdaccel.h>

#include <qevent.h>
#include <qstring.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <QAction>

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
	    if (combo->isEditable())
	    {
               combo->setEditText( text );
            }
            else
            {
               combo->addItem( text );
               combo->setCurrentIndex( combo->count()-1 );
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
    KUrl url() {
        QString txt = combo ? combo->currentText() : edit->text();
        KUrlCompletion *comp;
        if ( combo )
            comp = qobject_cast<KUrlCompletion*>(combo->completionObject());
        else
            comp = qobject_cast<KUrlCompletion*>(edit->completionObject());

        if ( comp )
            return KUrl( comp->replacedPath( txt ) );
        else
            return KUrl( txt );
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
    editWidget->setParent( this );
    d->edit = qobject_cast<KLineEdit*>( editWidget );
    d->combo = qobject_cast<KComboBox*>( editWidget );

    init();
}


KUrlRequester::KUrlRequester( QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate)
{
    init();
}


KUrlRequester::KUrlRequester( const KUrl& url, QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate)
{
    init();
    setUrl( url );
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

    if ( !d->combo && !d->edit )
	d->edit = new KLineEdit( this);

    myButton = new KUrlDragPushButton( this);
    QIcon iconSet = SmallIconSet(QLatin1String("fileopen"));
    QPixmap pixMap = iconSet.pixmap( style()->pixelMetric(QStyle::PM_SmallIconSize) );
    myButton->setIcon( iconSet );
    myButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
    myButton->setToolTip(i18n("Open file dialog"));

    connect( myButton, SIGNAL( pressed() ), SLOT( slotUpdateUrl() ));

    setSpacing( KDialog::spacingHint() );

    QWidget *widget = d->combo ? (QWidget*) d->combo : (QWidget*) d->edit;
    setFocusProxy( widget );
    setFocusPolicy(Qt::StrongFocus);

    d->connectSignals( this );
    connect( myButton, SIGNAL( clicked() ), this, SLOT( slotOpenDialog() ));

    myCompletion = new KUrlCompletion();
    d->setCompletionObject( myCompletion );

    QAction* openAction = new QAction(this);
    openAction->setShortcut(KStdAccel::Open);
    connect(openAction, SIGNAL(triggered(bool)), SLOT( slotOpenDialog() ));
}


void KUrlRequester::setUrl( const KUrl& url )
{
    d->setText( url.pathOrUrl() );
}

void KUrlRequester::setPath( const QString& path )
{
    d->setText( path );
}

void KUrlRequester::changeEvent(QEvent *e)
{
   if (e->type()==QEvent::WindowTitleChange) {
     if (myFileDialog) {
	//kDebug()<<"CHANGEEVENT"<<endl;
        myFileDialog->setWindowTitle( windowTitle() );
     }
   }
   KHBox::changeEvent(e);
}

KUrl KUrlRequester::url() const
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
          if ( KProtocolManager::supportsListing( u ) )
              dlg->setSelection( u.url() );
      }

      if ( dlg->exec() != QDialog::Accepted )
      {
          return;
      }

      newurl = dlg->selectedUrl();
    }

    setUrl( newurl );
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
        myFileDialog = new KFileDialog( QString(), d->fileDialogFilter, p);
	myFileDialog->setModal(true);
        myFileDialog->setMode( d->fileDialogMode );
        myFileDialog->setWindowTitle( windowTitle() );
    }

    return myFileDialog;
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

void KUrlRequester::slotUpdateUrl()
{
    KUrl u( KUrl::fromPath( QDir::currentPath() + '/' ), url().url() );
    myButton->setURL( u );
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
        edit = qobject_cast<KLineEdit*>( d->combo->lineEdit() );

#ifndef NDEBUG
    if ( !edit )
        kWarning() << "KUrlRequester's lineedit is not a KLineEdit!??\n";
#endif

    KEditListBox::CustomEditor editor( this, edit );
    return editor;
}


KUrlComboRequester::KUrlComboRequester( QWidget *parent)
  : KUrlRequester( new KComboBox(false), parent)
{
}

#include "kurlrequester.moc"
