/*
 *  -*- C++ -*-
 *
 *  kiconloaderdialog.
 *
 *  Copyright (C) 1997 Christoph Neerfeld
 *  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.	If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#include <qapplication.h>
#include <qdir.h>
#include <qpainter.h>
#include <qwmatrix.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>

#include <qlabel.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <kprogress.h>

#include "kiconloaderdialog.h"

#include <kpixmap.h>
#include <klineedit.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kiconloader.h>
#include <kglobal.h>

//
//----------------------------------------------------------------------
//---------------  KICONLOADERCANVAS   ---------------------------------
//----------------------------------------------------------------------
//
KIconLoaderCanvas::KIconLoaderCanvas (QWidget *parent, const char *name )
    : KIconView( parent, name )
{
    mpLoader = KGlobal::iconLoader();
    setGridX( 65 );
    loadTimer = new QTimer( this );
    connect( loadTimer, SIGNAL( timeout() ), this, SLOT( slotLoadFiles() ) );
    connect( this, SIGNAL( currentChanged( QIconViewItem * ) ),
	     this, SLOT( slotCurrentChanged( QIconViewItem * ) ) );
    setWordWrapIconText( FALSE );
    setShowToolTips( TRUE );
}

KIconLoaderCanvas::~KIconLoaderCanvas()
{
    delete loadTimer;
}

void KIconLoaderCanvas::loadFiles( QStringList files )
{
    clear();
    mFiles = files;
    loadTimer->start( 0, TRUE );
}

void KIconLoaderCanvas::slotLoadFiles()
{
    setResizeMode( Fixed );

    int i = 0;
    QApplication::setOverrideCursor( waitCursor );
    emit startLoading( mFiles.count() );
    QStringList::ConstIterator it;
    for (it=mFiles.begin(); it != mFiles.end(); ++it, ++i ) {
	emit progress( i );
	kapp->processEvents();
	KPixmap new_xpm;
	new_xpm.load( *it, 0, KPixmap::LowColor );
	if( new_xpm.isNull() )
	    continue;
    
	if( new_xpm.width() > 60 || new_xpm.height() > 60 ) {
	    QWMatrix m;
	    float scale;
	    if( new_xpm.width() > new_xpm.height() )
		scale = 60 / (float) new_xpm.width();
	    else
		scale = 60 / (float) new_xpm.height();
	    m.scale( scale, scale );
	    new_xpm = new_xpm.xForm(m);
	}
    
	QFileInfo fi( *it );
	QIconViewItem *item = new QIconViewItem( this, fi.baseName(), new_xpm );
	item->setKey( *it );
	item->setRenameEnabled( FALSE );
	item->setDragEnabled( FALSE );
	item->setDropEnabled( FALSE );
    }
    QApplication::restoreOverrideCursor();
    emit finished();

    setResizeMode( Adjust );
}


void KIconLoaderCanvas::slotCurrentChanged( QIconViewItem *item )
{
    emit nameChanged( (item != 0 ? item->text() : QString::null) );
}

QString KIconLoaderCanvas::getCurrent( void )
{
    if (!currentItem())
	return QString::null;
    QString current = currentItem()->key();

    // Strip path and extension
    int n = current.findRev('/');
    current = current.mid(n+1);
    n = current.findRev('.');
    current = current.left(n);
    return current;
}

//
// 1999-10-17 Espen Sand
// Added this one so that the KIconLoaderDialog can be closed by pressing
// Key_Escape when KIconLoaderCanvas has focus.
//
void KIconLoaderCanvas::keyPressEvent(QKeyEvent *e)
{
    if( e->key() == Key_Escape )
    {
        e->ignore();
    }
    else
    {
        KIconView::keyPressEvent(e);
    }
}


//
//----------------------------------------------------------------------
//---------------  KICONLOADERDIALOG   ---------------------------------
//----------------------------------------------------------------------
//
KIconLoaderDialog::KIconLoaderDialog ( QWidget *parent, const char *name )
    : KDialogBase( parent, name, TRUE, i18n("Select Icon"), Help|Ok|Cancel, Ok )
{
    icon_loader = KGlobal::iconLoader();
    init();
}

KIconLoaderDialog::KIconLoaderDialog ( KIconLoader *loader, QWidget *parent,
				       const char *name )
    : KDialogBase( parent, name, TRUE, i18n("Select Icon"), Help|Ok|Cancel, Ok )
{
    icon_loader = loader;
    init();
}

void KIconLoaderDialog::init( void )
{
    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
    QHBoxLayout *hbox = new QHBoxLayout();
    topLayout->addLayout(hbox);

    cb_types = new QComboBox(FALSE, page);
    cb_types->insertItem("Action");
    cb_types->insertItem("Application");
    cb_types->insertItem("Device");
    cb_types->insertItem("Filesystem");
    cb_types->insertItem("MimeType");
    hbox->addWidget( cb_types, 10 );
    hbox->addStretch(10);

    canvas = new KIconLoaderCanvas(page);
    canvas->setMinimumSize( 500, 125 );
    topLayout->addWidget( canvas );

    progressBar = new KProgress( page );
    topLayout->addWidget( progressBar );

    connect( canvas, SIGNAL(executed( QIconViewItem * )), this, SLOT(accept()) );
    //connect( canvas, SIGNAL(interrupted()), this, SLOT(needReload()) );
    connect( cb_types, SIGNAL(activated(int)), this, SLOT(typeChanged(int)) );
    connect( canvas, SIGNAL( startLoading( int ) ),
	     this, SLOT( initProgressBar( int ) ) );
    connect( canvas, SIGNAL( progress( int ) ),
	     this, SLOT( progress( int ) ) );
    connect( canvas, SIGNAL( finished () ),
	     this, SLOT( hideProgressBar() ) );

    incInitialSize( QSize(0,100) );
}


KIconLoaderDialog::~KIconLoaderDialog()
{
}

void KIconLoaderDialog::initProgressBar( int steps )
{
    //
    // 1999-10-17 Espen Sand
    // If we are going to display a small number of new icons then it
    // looks quite stupid to display a progressbar that is only going
    // to be visible in a fraction of a second. The number below used as
    // a threshold is of course a compromise.
    //
    if( steps < 10 )
    {
        hideProgressBar();
    }
    else
    {
        progressBar->setRange( 0, steps );
        progressBar->setValue( 0 );
        progressBar->show();
    }
}

void KIconLoaderDialog::progress( int p )
{
    progressBar->setValue( p );
}

void KIconLoaderDialog::hideProgressBar( void )
{
    //
    // 1999-10-17 Espen Sand
    // Sometimes the progressbar is not hidden even if I tell it to
    // do so. This happens both with the old version and with this one
    // (which relies on Qt layouts). The 'processOneEvent()' seems to kill
    // this "feature".
    //
    for( int i=0; i<100 && progressBar->isVisible() == true; i++ )
    {
        progressBar->hide();
        kapp->processOneEvent();
    }
}

int KIconLoaderDialog::exec(int group, int context)
{
    mGroup = group;
    mContext = context;
    setResult( 0 );
    QStringList icons = icon_loader->queryIcons( group, context );
    cb_types->setCurrentItem(context-1);
    canvas->loadFiles( icons );
    show();
    return result();
}

void KIconLoaderDialog::typeChanged(int index)
{
    mContext = index+1;
    QStringList icons = icon_loader->queryIcons(mGroup, mContext);
    canvas->loadFiles(icons);
}

QPixmap KIconLoaderDialog::selectIcon( QString &name, int group, 
	int context )
{
    QPixmap pixmap;

    QString pix_name;
    if (exec( group, context ))
    {
	pix_name = canvas->getCurrent();
	if (!pix_name.isEmpty())
	    pixmap = icon_loader->loadIcon( pix_name, group );
    }
    name = pix_name;
    return pixmap;
}


//
//----------------------------------------------------------------------
//---------------  KICONLOADERBUTTON   ---------------------------------
//----------------------------------------------------------------------
//
KIconLoaderButton::KIconLoaderButton( QWidget *_parent )
    : QPushButton( _parent )
{
    iconStr = "";
    mGroup = KIcon::Desktop; mContext = KIcon::Action; // arbitrary
    connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
    iconLoader = KGlobal::iconLoader();
    loaderDialog = new KIconLoaderDialog( this );
}

KIconLoaderButton::KIconLoaderButton( KIconLoader *_icon_loader,
				      QWidget *_parent )
    : QPushButton( _parent )
{
    iconStr = "";
    mGroup = KIcon::Desktop; mContext = KIcon::Action; // arbitrary
    connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
    loaderDialog = new KIconLoaderDialog( _icon_loader, this );
    iconLoader = _icon_loader;
}

void KIconLoaderButton::setIconType(int group, int context)
{
    mGroup = group;
    mContext = context;
}

void KIconLoaderButton::slotChangeIcon()
{
    QString name;
    QPixmap pix = loaderDialog->selectIcon( name, mGroup, mContext );
    if (!pix.isNull())
    {
        setPixmap(pix);
        iconStr = name;
        emit iconChanged( iconStr );
    }
}

void KIconLoaderButton::setIcon(const QString& _icon)
{
    iconStr = _icon;
    setPixmap( iconLoader->loadIcon(iconStr, mGroup) );
}

KIconLoaderButton::~KIconLoaderButton()
{
    delete loaderDialog;
}

#include "kiconloaderdialog.moc"
