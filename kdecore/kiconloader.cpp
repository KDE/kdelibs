// -*- C++ -*-

//
//  kiconloader
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@bonn.netsurf.de
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
#include <klocale.h>
#include <kapp.h>
#define klocale KApplication::getKApplication()->getLocale()
}
//----------------------------------------------------------------------
//---------------  KICONLOADER   ---------------------------------------
//----------------------------------------------------------------------
QPixmap KIconLoader::reloadIcon ( const QString& name, int w, int h ){
KIconLoader::KIconLoader( KConfig *conf, const QString &app_name, const QString &var_name )
{
  config = conf;
  config->setGroup(app_name);
QPixmap KIconLoader::loadMiniIcon ( const QString& name, int w, int h ){
  config->readListEntry( var_name, pixmap_dirs, ':' );
  QString temp = KApplication::kdedir();
  pixmap_dirs.insert( 0, temp + "/share/toolbar" );
  pixmap_dirs.insert( 1, temp + "/share/apps/" + kapp->appName() + "/toolbar" );
  pixmap_dirs.insert( 2, temp + "/share/apps/" + kapp->appName() + "/pics" );
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/icons" );
  name_list.setAutoDelete(TRUE);
  pixmap_dirs.setAutoDelete(TRUE);
  pixmap_list.setAutoDelete(TRUE);
}
 
KIconLoader::KIconLoader( )
{
  config = KApplication::getKApplication()->getConfig();
  config->setGroup("KDE Setup");
  config->readListEntry( "IconPath", pixmap_dirs, ':' );
  QString temp = KApplication::kdedir();
  pixmap_dirs.insert( 0, temp + "/share/toolbar" );
  pixmap_dirs.insert( 1, temp + "/share/apps/" + kapp->appName() + "/toolbar" );
  pixmap_dirs.insert( 2, temp + "/share/apps/" + kapp->appName() + "/pics" );
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/icons" );
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


	warning( "KIconLoader::flush is deprecated." );
}

#include "kiconloader.moc"

