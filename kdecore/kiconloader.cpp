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
		result = loadInternal("unknown.xpm", w, h);
#include "kiconloader.h"
#include "kiconloader.moc"
	}
KIconLoaderDialog* createKIconLoaderDialog();

#include <klocale.h>
}
//----------------------------------------------------------------------
//---------------  KICONLOADER   ---------------------------------------
//----------------------------------------------------------------------
QPixmap KIconLoader::reloadIcon ( const QString& name, int w, int h ){
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


// KIconLoader::selectIcon temporarily moved to kiconloaderui.cpp

// KIconLoader::setCaching temporarily moved to kiconloaderui.cpp

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



	warning( "KIconLoader::flush is deprecated." );
}

#include "kiconloader.moc"

