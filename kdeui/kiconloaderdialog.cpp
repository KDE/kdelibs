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
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include <qapplication.h>
#include <qdir.h>
#include <qpainter.h>
#include <qwmatrix.h>

#include <qlabel.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qprogressbar.h>

#include "kiconloaderdialog.h"

#include <kpixmap.h>
#include <klined.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>

//
//----------------------------------------------------------------------
//---------------  KICONLOADERCANVAS   ---------------------------------
//----------------------------------------------------------------------
//
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


void KIconLoaderCanvas::slotCurrentChanged( QIconViewItem *item ) 
{
  emit nameChanged( (item != 0 ? item->text() : QString::null) );
}

QString KIconLoaderCanvas::getCurrent( void )
{
  return( !currentItem() ? QString::null : currentItem()->key() );
}

QString KIconLoaderCanvas::currentDir( void )
{
  return dir_name;
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
    QIconView::keyPressEvent(e);
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
  setInitialSizeStep( QSize(0,100) );
  
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
  QHBoxLayout *hbox = new QHBoxLayout();
  topLayout->addLayout(hbox);

  cb_dirs = new QComboBox(FALSE, page);
  hbox->addWidget( cb_dirs, 10 );
  
  i_filter = new KLineEdit(page);
  l_filter = new QLabel( i_filter, i18n("&Filter:"), page );
  hbox->addWidget( l_filter );
  hbox->addWidget( i_filter );

  canvas = new KIconLoaderCanvas(page);
  canvas->setMinimumSize( 450, 125 );
  topLayout->addWidget( canvas );

  progressBar = new QProgressBar( page );
  topLayout->addWidget( progressBar );

  connect( canvas, SIGNAL(doubleClicked()), this, SLOT(accept()) );
  connect( canvas, SIGNAL(interrupted()), this, SLOT(needReload()) );
  connect( i_filter, SIGNAL(returnPressed()), this, SLOT(filterChanged()) );
  connect( cb_dirs, SIGNAL(activated(const QString&)), this, 
	   SLOT(dirChanged(const QString&)) );
  changeDirs(KGlobal::dirs()->resourceDirs("toolbar"));
  connect( canvas, SIGNAL( startLoading( int ) ),
	   this, SLOT( initProgressBar( int ) ) );
  connect( canvas, SIGNAL( progress( int ) ),
	   this, SLOT( progress( int ) ) );
  connect( canvas, SIGNAL( finished () ),
	   this, SLOT( hideProgressBar() ) );

  topLayout->activate();
}


KIconLoaderDialog::~KIconLoaderDialog()
{
}

void KIconLoaderDialog::needReload()
{
  i_filter->setText("");
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
    progressBar->setTotalSteps( steps );
    progressBar->reset();
    progressBar->show();
  }
}

void KIconLoaderDialog::progress( int p ) 
{
  progressBar->setProgress( p );
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

void KIconLoaderDialog::changeDirs( const QStringList &l ) 
{ 
  cb_dirs->clear(); 
  cb_dirs->insertStringList(l); 
}

int KIconLoaderDialog::exec(QString filter)
{
  setResult( 0 );
  //if( filter != i_filter->text() )
  //{
  canvas->loadDir( cb_dirs->currentText(), filter );
  i_filter->setText( filter );
  //} else
  //canvas->orderItemsInGrid();
  show();
  return result();
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


//
//----------------------------------------------------------------------
//---------------  KICONLOADERBUTTON   ---------------------------------
//----------------------------------------------------------------------
//
KIconLoaderButton::KIconLoaderButton( QWidget *_parent ) 
  : QPushButton( _parent )
{
  iconStr = "";
  resType = "toolbar";
  connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
  iconLoader = KGlobal::iconLoader();
  loaderDialog = new KIconLoaderDialog( this );
}

KIconLoaderButton::KIconLoaderButton( KIconLoader *_icon_loader, 
				      QWidget *_parent ) 
  : QPushButton( _parent )
{
  iconStr = "";
  resType = "toolbar";
  connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
  loaderDialog = new KIconLoaderDialog( _icon_loader, this );
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









