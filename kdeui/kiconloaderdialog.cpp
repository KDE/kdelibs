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
#include "kiconloaderdialog.h"

#include <kpixmap.h>
#include <klocale.h>
#define klocale KApplication::getKApplication()->getLocale()

//----------------------------------------------------------------------
//---------------  KICONLOADERCANVAS   ---------------------------------
//----------------------------------------------------------------------
KIconLoaderCanvas::KIconLoaderCanvas (QWidget *parent, const QString& name )
  :QTableView( parent, name )
{
  max_width = 0;
  max_height = 0;
  setFrameStyle(Panel | Sunken);
  setTableFlags(Tbl_autoScrollBars);
  pixmap_list.setAutoDelete(TRUE);
  name_list.setAutoDelete(TRUE);
  sel_id = 0;
  timer = new QTimer( this );
  connect( timer, SIGNAL( timeout() ), SLOT( process() ) );
}

KIconLoaderCanvas::~KIconLoaderCanvas()
{
  delete timer;
  name_list.clear();
  pixmap_list.clear(); 
}

void KIconLoaderCanvas::loadDir( QString dirname, QString filter )
{
  if ( timer->isActive() )
    {
      timer->stop();
      QApplication::restoreOverrideCursor();
    }
  dir_name = dirname;
  QDir d(dir_name);
  name_list.clear();
  pixmap_list.clear();
  if( !filter.isEmpty() )
    {
      d.setNameFilter(filter);
    }
  if( d.exists() )
    {
      file_list = *d.entryList( QDir::Files | QDir::Readable, QDir::Name );
      QApplication::setOverrideCursor( waitCursor );
      curr_indx = 0;
      sel_id = 0;
      max_width  = 16;
      max_height = 16;
      setTopLeftCell(0,0);
      timer->start( 0, true );
    }
  else
    {
      setNumCols( 0 );
      setNumRows( 0 );
      max_width = 20;
      max_height = 20;
      setCellWidth(max_width+4);
      setCellHeight(max_height+4);
      repaint();
    }
}

void KIconLoaderCanvas::process()
{
  KPixmap *new_xpm = 0;
  const char *current = file_list.at( curr_indx );

  for( int i = 0; i < 10 && current != 0; i++, curr_indx++ )
    {
      new_xpm = new KPixmap;
      new_xpm->load( dir_name + '/' + current, QString::null, KPixmap::LowColor );
      if( new_xpm->isNull() )
        {
          delete new_xpm;
          continue;
        }
      if( new_xpm->width() > 60 || new_xpm->height() > 60 )
        { // scale pixmap to a size of 60*60
          QWMatrix m;
          float scale;
          if( new_xpm->width() > new_xpm->height() )
            scale = 60 / (float) new_xpm->width();
	  else
	    scale = 60 / (float) new_xpm->height();
	  m.scale( scale, scale );
	       QPixmap tmp_xpm = new_xpm->xForm(m);
	       new_xpm->resize( tmp_xpm.width(), tmp_xpm.height() );
	       bitBlt( new_xpm, 0, 0, &tmp_xpm );
	       if ( tmp_xpm.mask() )
		   new_xpm->setMask( *tmp_xpm.mask() );
	 }
       max_width  = ( max_width  > new_xpm->width()  ? max_width  : new_xpm->width() );
       max_height = ( max_height > new_xpm->height() ? max_height : new_xpm->height() );
       name_list.append(current);
       pixmap_list.append(new_xpm);
       current = file_list.next();
    }

  if ( !current )
    {
      QApplication::restoreOverrideCursor();
      file_list.clear();
    }
  else
      timer->start( 0, true );

  // progressive display is nicer if these don't change too often
  max_width  = ( max_width + 7 ) / 8 * 8;
  max_height = ( max_height + 7 ) / 8 * 8;

  if( name_list.count() == 0 )
    {
      setNumCols( 0 );
      setNumRows( 0 );
      max_width = 20;
      max_height = 20;
      setCellWidth(max_width+4);
      setCellHeight(max_height+4);
      repaint();
    }
  else
    {
      setNumCols( width() / (max_width+4) );
      setNumRows( name_list.count() / numCols() + 1 );
      setCellWidth(max_width+4);
      setCellHeight(max_height+4);
      if ( rowIsVisible( curr_indx / numCols() ) )
	  repaint();
    }

}

void KIconLoaderCanvas::paintCell( QPainter *p, int r, int c )
{
  int item_nr = r * numCols() + c;
  if( item_nr >= (int) pixmap_list.count() || item_nr < 0 )
    {
      return;
    }
  QPixmap *pixmap = pixmap_list.at(item_nr);
  if( !pixmap )
    return;
  int w = cellWidth();
  int h = cellHeight();
  int pw = pixmap->width();
  int ph = pixmap->height();
  int x = (w-pw)/2;
  int y = (h-ph)/2;
  p->drawPixmap( x, y, *pixmap );
  if( item_nr == sel_id )
    p->drawRect( 0, 0, cellWidth(), cellHeight() );
}

void KIconLoaderCanvas::mouseMoveEvent( QMouseEvent *e)
{
  if( (e->pos().x() > numCols() * cellWidth()) || (e->pos().y() > numRows() * cellHeight()) )
    {
      emit nameChanged("");
      return;
    }
  int item_nr = findRow(e->pos().y()) * numCols() + findCol(e->pos().x());
  if( item_nr >= (int) name_list.count() || item_nr < 0 )
    {
      emit nameChanged("");
      return;
    }
  QString name = name_list.at(item_nr);
  emit nameChanged( name );
}

void KIconLoaderCanvas::mousePressEvent( QMouseEvent *e)
{
  if( name_list.count () == 0 )
    return;
  int i = sel_id;
  sel_id = findRow(e->pos().y()) * numCols() + findCol(e->pos().x());
  updateCell( i / numCols(), i % numCols() );
  updateCell( findRow(e->pos().y()), findCol(e->pos().x()) );
}

void KIconLoaderCanvas::mouseDoubleClickEvent( QMouseEvent * )
{
  emit doubleClicked();
}

void KIconLoaderCanvas::resizeEvent( QResizeEvent * e)
{
  if( !isVisible() )
    return;
  setNumCols( width() / (max_width+4) );
  setNumRows( name_list.count() / numCols() + 1 );
  QTableView::resizeEvent(e);
  repaint(TRUE);
}

void KIconLoaderCanvas::cancelLoad()
{
  if ( timer->isActive() )
    {
      timer->stop();
      QApplication::restoreOverrideCursor();
      emit interrupted();
    }
}

//----------------------------------------------------------------------
//---------------  KICONLOADERDIALOG   ---------------------------------
//----------------------------------------------------------------------

void KIconLoaderDialog::init()
{
  setCaption(i18n("Select Icon"));
  //---
  i_filter = new QLineEdit(this);
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
  ok->setGeometry(65, 200, 80, 30);
  cancel->setGeometry(325, 200, 80, 30);
  connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
  connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
  connect( canvas, SIGNAL(nameChanged(const QString&)), l_name, SLOT(setText(const QString&)) );
  connect( canvas, SIGNAL(doubleClicked()), this, SLOT(accept()) );
  connect( canvas, SIGNAL(interrupted()), this, SLOT(needReload()) );
  connect( i_filter, SIGNAL(returnPressed()), this, SLOT(filterChanged()) );
  connect( cb_dirs, SIGNAL(activated(const QString&)), this, SLOT(dirChanged(const QString&)) );
  setDir(icon_loader->getDirList());
  resize( 470, 350 );
  setMinimumSize( 470, 250 );
}

KIconLoaderDialog::KIconLoaderDialog ( QWidget *parent, const QString& name )
  : QDialog( parent, name, TRUE )
{
  icon_loader = KApplication::getKApplication()->getIconLoader();
  init();
}

KIconLoaderDialog::KIconLoaderDialog ( KIconLoader *loader, QWidget *parent, const QString& name )
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
  canvas->cancelLoad(); 
  QDialog::reject();
}

void KIconLoaderDialog::needReload()
{
  i_filter->setText("");
}

int KIconLoaderDialog::exec(QString filter)
{
  setResult( 0 );
  if( filter != i_filter->text() )
    {
      canvas->loadDir( cb_dirs->currentText(), filter );
      i_filter->setText( filter );
    }
  show();
  return result();
}

void KIconLoaderDialog::resizeEvent( QResizeEvent * )
{
  int w = width();
  int h = height();
  canvas->resize( w - 20, h - 106 );
  l_name->resize( canvas->width(), 24 );
  l_name->move( 10, 38 + canvas->height() );
  i_filter->move( w - 160, 8 );
  l_filter->move( w - 200, 8 );
  cb_dirs->resize( canvas->width() - i_filter->width() - l_filter->width() - 18, 24 );
  ok->move( 65, h - 40  );
  cancel->move( w - 145, h - 40 );
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
      if( (pix_name = canvas->getCurrent()) )
	  pixmap = icon_loader->loadIcon( pix_name );
  }
  name = pix_name;
  return pixmap;
}

KIconLoaderButton::KIconLoaderButton( QWidget *_parent ) : QPushButton( _parent )
{
    iconStr = "";
    connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
    iconLoader = kapp->getIconLoader();
    loaderDialog = new KIconLoaderDialog();
}

KIconLoaderButton::KIconLoaderButton( KIconLoader *_icon_loader, QWidget *_parent ) : QPushButton( _parent )
{
    iconStr = "";
    connect( this, SIGNAL( clicked() ), this, SLOT( slotChangeIcon() ) );
    loaderDialog = new KIconLoaderDialog( _icon_loader );
    iconLoader = _icon_loader;
}

void KIconLoaderButton::slotChangeIcon()
{
    QString name;
    QPixmap pix = loaderDialog->selectIcon( name, "*" );
    if( !pix.isNull() )
    {
	setPixmap(pix);
	iconStr = name.data();
	emit iconChanged( iconStr );
    }    
}

void KIconLoaderButton::setIcon(const QString& _icon)
{
    iconStr = _icon;

    // A Hack, since it uses loadApplicationIcon!!!
    setPixmap( KApplication::getKApplication()->getIconLoader()->loadApplicationIcon( iconStr ) );
}

KIconLoaderButton::~KIconLoaderButton() 
{
    if ( loaderDialog )
	delete loaderDialog;
}
#include "kiconloaderdialog.moc"

