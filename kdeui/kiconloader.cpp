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
#include <qwmatrix.h>

#include "kiconloader.h"
#include "kiconloader.moc"

#include <klocale.h>
#define klocale KLocale::klocale()

//----------------------------------------------------------------------
//---------------  KICONLOADERCANVAS   ---------------------------------
//----------------------------------------------------------------------
KIconLoaderCanvas::KIconLoaderCanvas (QWidget *parent, const char *name )
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
  QApplication::setOverrideCursor( waitCursor );
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
      if( new_xpm->width() > 60 || new_xpm->height() > 60 )
		{ // scale pixmap to a size of 60*60
		  QWMatrix m;
		  float scale;
		  if( new_xpm->width() > new_xpm->height() )
			scale = 60 / (float) new_xpm->width();
		  else
			scale = 60 / (float) new_xpm->height();
		  m.scale( scale, scale );
		  *new_xpm = new_xpm->xForm(m);
		}
      max_width  = ( max_width  > new_xpm->width()  ? max_width  : new_xpm->width() );
      max_height = ( max_height > new_xpm->height() ? max_height : new_xpm->height() );
      name_list.append(current);
      pixmap_list.append(new_xpm);
    }
  QApplication::restoreOverrideCursor();
  setNumCols( width() / (max_width+4) );
  setNumRows( name_list.count() / numCols() + 1 );
  setCellWidth(max_width+4);
  setCellHeight(max_height+4);
  setTopLeftCell(0,0);
  sel_id = 0;
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

void KIconLoaderCanvas::resizeEvent( QResizeEvent * e)
{
  if( !isVisible() )
    return;
  setNumCols( width() / (max_width+4) );
  setNumRows( name_list.count() / numCols() + 1 );
  QTableView::resizeEvent(e);
  repaint(TRUE);
}

//----------------------------------------------------------------------
//---------------  KICONLOADERDIALOG   ---------------------------------
//----------------------------------------------------------------------

KIconLoaderDialog::KIconLoaderDialog ( QWidget *parent, const char *name )
  : QDialog( parent, name, TRUE )
{
  setCaption(klocale->translate("Select Icon"));
  QFont font("Helvetica", 12, QFont::Bold);
  //---
  i_filter = new QLineEdit(this);
  i_filter->setGeometry(310, 8, 150, 24);
  //---
  l_filter = new QLabel( klocale->translate("Filter:"), this );
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
  ok = new QPushButton( klocale->translate("Ok"), this );
  cancel = new QPushButton( klocale->translate("Cancel"), this );
  ok->setFont(font);
  cancel->setFont(font);
  ok->setGeometry(65, 200, 80, 30);
  cancel->setGeometry(325, 200, 80, 30);
  connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
  connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
  connect( canvas, SIGNAL(nameChanged(const char *)), l_name, SLOT(setText(const char *)) );
  connect( canvas, SIGNAL(doubleClicked()), this, SLOT(accept()) );
  connect( i_filter, SIGNAL(returnPressed()), this, SLOT(filterChanged()) );
  connect( cb_dirs, SIGNAL(activated(const char *)), this, SLOT(dirChanged(const char*)) );
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
  ok->move( 65, h - 40  );
  cancel->move( w - 145, h - 40 );
}

void KIconLoaderDialog::filterChanged()
{
  canvas->loadDir( cb_dirs->currentText(), i_filter->text() );
}

void KIconLoaderDialog::dirChanged(const char * dir)
{
  canvas->loadDir( dir, i_filter->text() );
}

//----------------------------------------------------------------------
//---------------  KICONLOADER   ---------------------------------------
//----------------------------------------------------------------------

KIconLoader::KIconLoader( KConfig *conf, const QString &app_name, const QString &var_name )
{
  //debug( "KIconLoader( a, b, c ) is obsolete. Please use KIconloader()" );
  pix_dialog = NULL;
  caching = FALSE;
  config = conf;
  config->setGroup(app_name);
  if( !readListConf( var_name, pixmap_dirs, ':' ) )
    {
      QString temp = KApplication::kdedir();
      if( temp.isNull() )
		{
		  debug("KDE: KDEDIR not set. Using '/usr/local/lib/kde' as default.");
		  temp = "/usr/local/lib/kde";
		}
      temp += "/lib/pics";
      pixmap_dirs.append(temp);                          // using KDEDIR/lib/pics as default
    }
  name_list.setAutoDelete(TRUE);
  pixmap_dirs.setAutoDelete(TRUE);
  pixmap_list.setAutoDelete(TRUE);
}
 
KIconLoader::KIconLoader( )
{
  pix_dialog = NULL;
  caching = FALSE;
  config = KApplication::getKApplication()->getConfig();
  config->setGroup("KDE Setup");
  if( !readListConf( "IconPath", pixmap_dirs, ':' ) )
    {
      QString temp = KApplication::kdedir();
      if( temp.isNull() )
		{
		  debug("KDE: KDEDIR not set. Using '/usr/local/lib/kde' as default.");
		  temp = "/usr/local/lib/kde";
		}
      temp += "/lib/pics";
      pixmap_dirs.append(temp);                          // using KDEDIR/lib/pics as default
    }
  name_list.setAutoDelete(TRUE);
  pixmap_dirs.setAutoDelete(TRUE);
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
      QString full_path;
      QFileInfo finfo;
      pix = new QPixmap;
      if( name.left(1) == '/' )
		full_path = name;
      else
		{
		  QStrListIterator it( pixmap_dirs );
		  while ( it.current() )
			{
			  full_path = it.current();
			  full_path += '/';
			  full_path += name;
			  finfo.setFile( full_path );
			  if ( finfo.exists() )
				break;
			  ++it;
			}
		}
      new_xpm.load( full_path );
      *pix = new_xpm;
      if( pix->isNull() )
		{
		  warning(klocale->translate("ERROR: couldn't find icon: %s"), (const char *) name);
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
    {
      pix_dialog = new KIconLoaderDialog;
      pix_dialog->setDir( &pixmap_dirs );
    }
  QPixmap pixmap;
  QString pix_name, old_filter;
  old_filter = pix_dialog->getFilter();
  if( !caching || old_filter.isEmpty() )
    old_filter = filter;
  if( pix_dialog->exec(old_filter) )
    {
      if( (pix_name = pix_dialog->getCurrent()) )
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

int KIconLoader::readListConf( QString key, QStrList &list, char sep )
{
  if( !config->hasKey( key ) )
    return 0;
  QString str_list, value;
  str_list = config->readEntry(key);
  if(str_list.isEmpty())
    return 0;
  list.clear();
  int i;
  int len = str_list.length();
  for( i = 0; i < len; i++ )
    {
      if( str_list[i] != sep && str_list[i] != '\\' )
        {
          value += str_list[i];
          continue;
        }
      if( str_list[i] == '\\' )
        {
          i++;
          value += str_list[i];
          continue;
        }
      list.append(value);
      value.truncate(0);
    }
  list.append(value);
  return list.count();
}



