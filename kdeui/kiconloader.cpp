// -*- C++ -*-

//
//  kiconloader
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@mail.bonn.netsurf.de
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <qapp.h>
#include <qdir.h>
#include <qpainter.h>

#include "kiconloader.h"
#include "kiconloader.moc"

//----------------------------------------------------------------------
//---------------  KICONLOADERCANVAS   ---------------------------------
//----------------------------------------------------------------------
KIconLoaderCanvas::KIconLoaderCanvas (QWidget *parent=0, const char *name=0)
  :QTableView( parent, name )
{
  max_width = 0;
  max_height = 0;
  setFrameStyle(Panel | Sunken);
  setTableFlags(Tbl_autoScrollBars);
  pixmap_list.setAutoDelete(TRUE);
  name_list.setAutoDelete(TRUE);
  sel_id = 0;
}

KIconLoaderCanvas::~KIconLoaderCanvas()
{
  name_list.clear();
  pixmap_list.clear(); 
}

void KIconLoaderCanvas::loadDir( QString dir_name, QString filter )
{
  max_width  = 0;
  max_height = 0;
  QDir d(dir_name);
  if( !d.exists() )
    return;
  name_list.clear();
  pixmap_list.clear();
  if( !filter.isEmpty() )
    {
      d.setNameFilter(filter);
    }
  const QStrList *file_list = d.entryList( QDir::Files | QDir::Readable, QDir::Name );
  QStrListIterator it(*file_list);
  QString current;

  QPixmap *new_xpm;
  it.toFirst();
  for( ; it.current(); ++it )
    {
      new_xpm = new QPixmap;
      current = it.current();
      new_xpm->load( dir_name + '/' + current );
      if( new_xpm->isNull() )
	{
	  delete new_xpm;
	  continue;
	}
      if( new_xpm->width() > 70 || new_xpm->height() > 70 )
	{
	  delete new_xpm;
	  continue;
	}
      max_width  = ( max_width  > new_xpm->width()  ? max_width  : new_xpm->width() );
      max_height = ( max_height > new_xpm->height() ? max_height : new_xpm->height() );
      name_list.append(current);
      pixmap_list.append(new_xpm);
    }
  setNumCols( width() / (max_width+4) );
  setNumRows( name_list.count() / numCols() + 1 );
  setCellWidth(max_width+4);
  setCellHeight(max_height+4);
  setTopLeftCell(0,0);
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
  emit nameChanged( (const char *) name );
}

void KIconLoaderCanvas::mousePressEvent( QMouseEvent *e)
{
  int i = sel_id;
  sel_id = findRow(e->pos().y()) * numCols() + findCol(e->pos().x());
  updateCell( i / numCols(), i % numCols() );
  updateCell( findRow(e->pos().y()), findCol(e->pos().x()) );
}

void KIconLoaderCanvas::mouseDoubleClickEvent( QMouseEvent * )
{
  emit doubleClicked();
}

void KIconLoaderCanvas::resizeEvent( QResizeEvent * )
{
  if( !isVisible() )
    return;
  setNumCols( width() / (max_width+4) );
  setNumRows( name_list.count() / numCols() + 1 );
  repaint();
}

//----------------------------------------------------------------------
//---------------  KICONLOADERDIALOG   ---------------------------------
//----------------------------------------------------------------------

KIconLoaderDialog::KIconLoaderDialog ( QWidget *parent=0, const char *name=0 )
  : QDialog( parent, name, TRUE )
{
  QFont font("Helvetica", 12, QFont::Bold);
  QLabel *text = new QLabel(this);
  text->setFont(font);
  text->setText("Select icon:");
  text->setGeometry(10, 10, 100, 30);
  //---
  i_filter = new QLineEdit(this);
  i_filter->setGeometry(310, 13, 150, 24);
  //---
  l_filter = new QLabel( "Filter:", this );
  l_filter->setGeometry( 310 - 50, 13, 40, 24 );
  //---
  canvas = new KIconLoaderCanvas(this);
  canvas->setGeometry(10, 50, 450, 115);
  //---
  l_name = new QLabel("", this);
  l_name->setGeometry(10, 165, canvas->width(), 30);
  //---
  ok = new QPushButton( "Ok", this );
  cancel = new QPushButton( "Cancel", this );
  ok->setFont(font);
  cancel->setFont(font);
  ok->setGeometry(65, 200, 80, 30);
  cancel->setGeometry(325, 200, 80, 30);
  connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
  connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
  connect( canvas, SIGNAL(nameChanged(const char *)), l_name, SLOT(setText(const char *)) );
  connect( canvas, SIGNAL(doubleClicked()), this, SLOT(accept()) );
  connect( i_filter, SIGNAL(returnPressed()), this, SLOT(filterChanged()) );
  resize( 470, 250 );
  setMinimumSize( 470, 250 );
}

KIconLoaderDialog::~KIconLoaderDialog()
{
  disconnect( ok );
  disconnect( cancel );
  disconnect( canvas );
}

int KIconLoaderDialog::exec(QString filter)
{
  setResult( 0 );
  if( filter != i_filter->text() )
    {
      canvas->loadDir( dir_name, filter );
      i_filter->setText( filter );
    }
  show();
  return result();
}

void KIconLoaderDialog::resizeEvent( QResizeEvent * )
{
  int w = width();
  int h = height();
  canvas->resize( w-20, h - 145 );
  l_name->resize( canvas->width(), l_name->height() );
  l_name->move( canvas->x(), canvas->y()+canvas->height()+10 );
  i_filter->move( w - 160, i_filter->y() );
  l_filter->move( w - 160 - 50, l_filter->y() );
  ok->move( 65, h - 50  );
  cancel->move( w - 65 - 80, h - 50 );
}

void KIconLoaderDialog::filterChanged()
{
  canvas->loadDir( dir_name, i_filter->text() );
}

//----------------------------------------------------------------------
//---------------  KICONLOADER   ---------------------------------------
//----------------------------------------------------------------------

KIconLoader::KIconLoader( KConfig *conf, const QString &app_name, const QString &var_name )
{
  pix_dialog = NULL;
  caching = FALSE;
  config = conf;
  config->setGroup(app_name);
  pixmap_path = config->readEntry(var_name);
  name_list.setAutoDelete(TRUE);
  pixmap_list.setAutoDelete(TRUE);
}

KIconLoader::~KIconLoader()
{
  name_list.clear();
  pixmap_list.clear();
}

QPixmap KIconLoader::loadIcon ( const QString &name )
{
  QPixmap *pix;
  QPixmap new_xpm;
  int index;
  if ( (index = name_list.find(name)) < 0)
    {  
      pix = new QPixmap;
      if( name[0] == '/' )
	new_xpm.load(name);
      else
	new_xpm.load( pixmap_path + '/' + name );
      *pix = new_xpm;
      if( pix->isNull() )
	{
	  debug("ERROR: couldn't find icon: %s", (const char *) name);
	}
      else
	{
	  name_list.append(name);
	  pixmap_list.append(pix);
	}
    }
  else
    {
      pix = pixmap_list.at(index);
    }
  return *pix;
}

QPixmap KIconLoader::selectIcon(QString &name, const QString &filter)
{
  if( pix_dialog == NULL )
    pix_dialog = new KIconLoaderDialog;
  pix_dialog->setDir( pixmap_path );
  QPixmap pixmap;
  QString pix_name;
  if( pix_dialog->exec(filter) )
    {
      pix_name = pix_dialog->getCurrent();
      pixmap = loadIcon( pix_name );
    }
  name = pix_name;
  if( !caching )
    {
      delete pix_dialog;
      pix_dialog = NULL;
    }
  return pixmap;
}

void KIconLoader::setCaching( bool b )
{
  if( b == TRUE )
    {
      caching = TRUE;
      return;
    }
  caching = FALSE;
  if( pix_dialog == NULL )
    return;
  if( pix_dialog->isVisible() )
    return;
  delete pix_dialog;
  pix_dialog = NULL;
}




