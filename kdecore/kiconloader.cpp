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

// CHANGES
// Torben: Priority for local icons, added full KDEFSSTD to ~/.kde
//         /share/icons is exclusively searched in if you want to
//        load an applications icon.
	if (result.isNull() && !canReturnNull) {
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
  /*  pixmap_dirs.insert( 0, temp + "/share/icons" );
  pixmap_dirs.insert( 1, temp + "/share/toolbar" );
  pixmap_dirs.insert( 2, temp + "/share/apps/" + kapp->appName() + "/toolbar" );
  pixmap_dirs.insert( 3, temp + "/share/apps/" + kapp->appName() + "/pics" );
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/icons" ); */
  // Torben
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/share/toolbar" ); 
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/toolbar" ); 
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/pics" ); 
  pixmap_dirs.append( temp + "/share/toolbar" );
  pixmap_dirs.append( temp + "/share/apps/" + kapp->appName() + "/toolbar" );
  pixmap_dirs.append( temp + "/share/apps/" + kapp->appName() + "/pics" );
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
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/share/toolbar" ); 
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/toolbar" ); 
  pixmap_dirs.append( QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/pics" ); 
  pixmap_dirs.append( temp + "/share/toolbar" );
  pixmap_dirs.append( temp + "/share/apps/" + kapp->appName() + "/toolbar" );
  pixmap_dirs.append( temp + "/share/apps/" + kapp->appName() + "/pics" );
  // pixmap_dirs.append( QDir::homeDirPath() + "/.kde/icons" );
  // Torben
  name_list.setAutoDelete(TRUE);
  pixmap_dirs.setAutoDelete(TRUE);
  pixmap_list.setAutoDelete(TRUE);
}
 
KIconLoader::~KIconLoader()

  name_list.clear();
  pixmap_list.clear();
}

QPixmap KIconLoader::loadIcon ( const QString &name, int w, int h ){
  QPixmap result = loadInternal(name, w, h);
  if (result.isNull())
    warning(klocale->translate("ERROR: couldn't find icon: %s"), (const char *) name);
  return result;
}

QPixmap KIconLoader::loadMiniIcon ( const QString &name, int w, int h ){
  QPixmap result;
  if (name.left(1)!='/'){
    result = loadInternal(QString("mini/")+name, w, h);
  }
  if (result.isNull())
    result = loadInternal(name, w, h);
  if (result.isNull())
    warning(klocale->translate("ERROR: couldn't find mini icon: %s"), (const char *) name);
  return result;
			// Let's be recursive (but just once at most)
			full_path = getIconPath( "unknown.xpm" , false); 
QPixmap KIconLoader::loadApplicationIcon ( const QString &name, int w, int h ){
  pixmap_dirs.insert( 0, KApplication::kdedir() + "/share/icons" );
  pixmap_dirs.insert( 0, QDir::homeDirPath() + "/.kde/share/icons" );
  QPixmap result = loadIcon(name, w, h);
  pixmap_dirs.remove((unsigned int) 0);
  return result;
}

QPixmap KIconLoader::loadApplicationMiniIcon ( const QString &name, int w, int h ){
  pixmap_dirs.insert( 0, KApplication::kdedir() + "/share/icons" );
  pixmap_dirs.insert( 0, QDir::homeDirPath() + "/.kde/share/icons" );
  QPixmap result = loadMiniIcon(name, w, h);
  pixmap_dirs.remove((unsigned int) 0);
  return result;
}
	pixmap_dirs.insert( pixmap_dirs.at(index), dir_name ); 
QPixmap KIconLoader::loadInternal ( const QString &name, int w,  int h )
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
      if( !pix->isNull() )
	{
	  name_list.append(name);
	  pixmap_list.append(pix);
	}
    }
  else
    {
      pix = pixmap_list.at(index);
    }


  if (!pix->isNull() && 
      w > 0 && h > 0 && 
      (pix->width() > w || pix->height() > h)){
    QWMatrix m;
    m.scale(w/(float)pix->width(), h/(float)pix->height());
    return pix->xForm(m);
  }
  return *pix;
}

	warning( "KIconLoader::flush is deprecated." );
}

#include "kiconloader.moc"

