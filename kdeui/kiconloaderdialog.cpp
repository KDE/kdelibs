// -*- C++ -*-

//
//  kiconloaderdialog
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <qapplication.h>
#include <qdir.h>
#include <qpainter.h>
#include <qwmatrix.h>

#include "kiconloaderdialog.h"

#include <kpixmap.h>
#include <klined.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>

//----------------------------------------------------------------------
//---------------  KICONLOADERCANVAS   ---------------------------------
//----------------------------------------------------------------------
KIconLoaderCanvas::KIconLoaderCanvas (QWidget *parent, const char *name )
    : QIconView( parent, name )
{
    setGridX( 65 );
    setResizeMode( Adjust );
    connect( this, SIGNAL( doubleClicked( QIconViewItem * ) ),
	     this, SIGNAL( doubleClicked() ) );
    loadTimer = new QTimer( this );
    connect( loadTimer, SIGNAL( timeout() ),
	     this, SLOT( slotLoadDir() ) );
    setViewMode( QIconSet::Large );
    connect( this, SIGNAL( currentChanged( QIconViewItem * ) ),
	     this, SLOT( slotCurrentChanged( QIconViewItem * ) ) );
}

KIconLoaderCanvas::~KIconLoaderCanvas()
{
    delete loadTimer;
}

void KIconLoaderCanvas::loadDir( QString dirname, QString filter_ )
{
    dir_name = dirname;
    filter = filter_;
    loadTimer->start( 0, TRUE );
}

void KIconLoaderCanvas::slotLoadDir()
{
    QDir d( dir_name );
    if( !filter.isEmpty() )
   	d.setNameFilter(filter);

    if( d.exists() ) {
	int i = 0;
	QApplication::setOverrideCursor( waitCursor );
	QStringList file_list = d.entryList( QDir::Files | QDir::Readable, QDir::Name );
	QStringList::Iterator it = file_list.begin();
	emit startLoading( file_list.count() );
	for ( ; it != file_list.end(); ++it, ++i ) {
	    emit progress( i );
	    kapp->processEvents();
	    KPixmap new_xpm;
	    new_xpm.load( dir_name + '/' + *it, 0, KPixmap::LowColor );
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
	    QIconViewItem *item = new QIconViewItem( this, fi.baseName(), QIconSet( new_xpm, QIconSet::Large ) );
	    item->setKey( *it );
	    item->setRenameEnabled( FALSE );
	    item->setDragEnabled( FALSE );
	    item->setDropEnabled( FALSE );
	}
	QApplication::restoreOverrideCursor();
	emit finished();
    }
}

//----------------------------------------------------------------------
//---------------  KICONLOADERDIALOG   ---------------------------------
//----------------------------------------------------------------------

void KIconLoaderDialog::init()
{
    setCaption(i18n("Select Icon"));
    //---
    i_filter = new KLineEdit(this);
    i_filter->setGeometry(310, 8, 150, 24);
    //---
    l_filter = new QLabel( i18n("Filter:"), this );
    l_filter->setGeometry( 310 - 50, 8, 40, 24 );
    //---
    canvas = new KIconLoaderCanvas(this);
    canvas->setGeometry(10, 38, 450, 124);
    //---
    l_name = new QLabel("", this);
    l_name->setGeometry(10, 165, 110, 24);
    //---
    cb_dirs = new QComboBox(FALSE, this);
    cb_dirs->setGeometry(10, 8, 230, 24);
    //---
    ok = new QPushButton( i18n("OK"), this );
    cancel = new QPushButton( i18n("Cancel"), this );
    int w = QMAX( ok->sizeHint().width(), cancel->sizeHint().width() );
    ok->resize( w, ok->sizeHint().height() );
    cancel->resize( w, cancel->sizeHint().height() );

    cancel->move( width() - cancel->width() - 10, canvas->y() + canvas->height() + 10 );
    ok->move( width() - cancel->width() -  ok->width() - 15, cancel->y() );

    progressBar = new QProgressBar( this );
    progressBar->move( 10, cancel->y() );
    progressBar->resize( width() - 20 - ok->x(), progressBar->sizeHint().height() );

    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    connect( canvas, SIGNAL(nameChanged(const QString&)), l_name, SLOT(setText(const QString&)) );
    connect( canvas, SIGNAL(doubleClicked()), this, SLOT(accept()) );
    connect( canvas, SIGNAL(interrupted()), this, SLOT(needReload()) );
    connect( i_filter, SIGNAL(returnPressed()), this, SLOT(filterChanged()) );
    connect( cb_dirs, SIGNAL(activated(const QString&)), this, SLOT(dirChanged(const QString&)) );
    changeDirs(KGlobal::dirs()->resourceDirs("toolbar"));
    connect( canvas, SIGNAL( startLoading( int ) ),
	     this, SLOT( initProgressBar( int ) ) );
    connect( canvas, SIGNAL( progress( int ) ),
	     this, SLOT( progress( int ) ) );
    connect( canvas, SIGNAL( finished () ),
	     this, SLOT( hideProgressBar() ) );

    resize( 470, 350 );
    setMinimumSize( 470, 250 );
}

KIconLoaderDialog::KIconLoaderDialog ( QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    icon_loader = KGlobal::iconLoader();
    init();
}

KIconLoaderDialog::KIconLoaderDialog ( KIconLoader *loader, QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    icon_loader = loader;
    init();
}

KIconLoaderDialog::~KIconLoaderDialog()
{
    disconnect( ok );
    disconnect( cancel );
    disconnect( canvas );
}

void KIconLoaderDialog::reject()
{
    QDialog::reject();
}

void KIconLoaderDialog::needReload()
{
    i_filter->setText("");
}

int KIconLoaderDialog::exec(QString filter)
{
    setResult( 0 );
//     if( filter != i_filter->text() )
//     {
	canvas->loadDir( cb_dirs->currentText(), filter );
	i_filter->setText( filter );
//     } else
// 	canvas->orderItemsInGrid();
    show();
    return result();
}

void KIconLoaderDialog::resizeEvent( QResizeEvent * )
{
    int w = width();
    int h = height();
    canvas->resize( w - 20, h - i_filter->height() - ok->height() - 30 );
    l_name->resize( canvas->width(), 24 );
    l_name->move( 10, 38 + canvas->height() );
    i_filter->move( w - 160, 8 );
    l_filter->move( w - 200, 8 );
    cb_dirs->resize( canvas->width() - i_filter->width() - l_filter->width() - 18, 24 );
    cancel->move( width() - cancel->width() - 10, canvas->y() + canvas->height() + 10 );
    ok->move( width() - cancel->width() -  ok->width() - 15, cancel->y() );
    progressBar->move( 10, cancel->y() );
    progressBar->resize( width() - 20 - ( width() - ok->x() ), progressBar->sizeHint().height() );
}

void KIconLoaderDialog::filterChanged()
{
    canvas->loadDir( cb_dirs->currentText(), i_filter->text() );
}

void KIconLoaderDialog::dirChanged(const QString& dir)
{
    canvas->loadDir( dir, i_filter->text() );
}

QPixmap KIconLoaderDialog::selectIcon( QString &name, const QString &filter)
{
    QPixmap pixmap;
    QString pix_name, old_filter;
    old_filter = i_filter->text();
    if( old_filter.isEmpty() )
	old_filter = filter;
    if( exec(old_filter) )
    {
	if( !(pix_name = canvas->getCurrent()).isNull() )
	    // David : give a full path to loadIcon
	    pixmap = icon_loader->loadIcon( canvas->currentDir() + "/" + pix_name );
    }
    name = pix_name;
    return pixmap;
}

KIconLoaderButton::KIconLoaderButton( QWidget *_parent ) : QPushButton( _parent )
{
    iconStr = "";
    resType = "toolbar";
    connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
    iconLoader = KGlobal::iconLoader();
    loaderDialog = new KIconLoaderDialog();
}

KIconLoaderButton::KIconLoaderButton( KIconLoader *_icon_loader, QWidget *_parent ) : QPushButton( _parent )
{
    iconStr = "";
    resType = "toolbar";
    connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
    loaderDialog = new KIconLoaderDialog( _icon_loader );
    iconLoader = _icon_loader;
}

void KIconLoaderButton::setIconType(const QString& _resType)
{
    resType = _resType;
    loaderDialog->changeDirs(KGlobal::dirs()->resourceDirs(resType));

    // Reload icon (might differ in new resource type)
    if(!iconStr.isEmpty())
	setIcon(iconStr);
}

void KIconLoaderButton::slotChangeIcon()
{
    QString name;
    QPixmap pix = loaderDialog->selectIcon( name, "*" );
    if( !pix.isNull() )
    {
	setPixmap(pix);
	iconStr = name;
	emit iconChanged( iconStr );
    }
}

void KIconLoaderButton::setIcon(const QString& _icon)
{
    iconStr = _icon;
    setPixmap( locate(resType, iconStr) );
}

KIconLoaderButton::~KIconLoaderButton()
{
    delete loaderDialog;
}
#include "kiconloaderdialog.moc"

