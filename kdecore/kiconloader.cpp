// -*- C++ -*-
// -*- C++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Christoph Neerfeld (Christoph.Neerfeld@boon.netsurf.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
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
}
//----------------------------------------------------------------------
//---------------  KICONLOADER   ---------------------------------------
//----------------------------------------------------------------------
QPixmap KIconLoader::reloadIcon ( const QString& name, int w, int h ){
KIconLoader::KIconLoader( KConfig *conf, const QString &app_name, const QString &var_name )
{
  QStrList list;
	return loadInternal( name, w, h );
  config = conf;
  config->setGroup(app_name);
  config->readListEntry( var_name, list, ':' );

  for (const char *it=list.first(); it; it = list.next())
    addPath(it);
QPixmap KIconLoader::loadMiniIcon ( const QString& name, int w, int h ){
  addPath(KApplication::kde_datadir() + "/" + kapp->appName() + "/pics" );
  addPath(KApplication::kde_toolbardir());
  addPath(KApplication::kde_datadir() + "/" + kapp->appName() + "/toolbar" );

  addPath(QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/pics" ); 
  addPath(QDir::homeDirPath() + "/.kde/share/toolbar" ); 
  addPath(QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/toolbar" ); 

  name_list.setAutoDelete(TRUE);
  pixmap_dirs.setAutoDelete(TRUE);
  pixmap_list.setAutoDelete(TRUE);
}
 
KIconLoader::KIconLoader( )
{
  QStrList list;
Stephan: See above
  config = KApplication::getKApplication()->getConfig();
  config->setGroup("KDE Setup");
  config->readListEntry( "IconPath", list, ':' );

  for (const char *it=list.first(); it; it = list.next())
    addPath(it);

  addPath( KApplication::kde_datadir() + "/" + kapp->appName() + "/pics" );
  addPath( KApplication::kde_toolbardir() );
  addPath( KApplication::kde_datadir() + "/" + kapp->appName() + "/toolbar" );

  addPath(QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/pics" ); 
  addPath(QDir::homeDirPath() + "/.kde/share/toolbar" ); 
  addPath(QDir::homeDirPath() + "/.kde/share/apps/" + kapp->appName() + "/toolbar" ); 

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
/* Stephan: It's OK to know, how many icons are still missing, but
       we don't need to tell everybody ;) Perhaps this can be con-
       verted to a KDEBUG solution, that is more silent? Don't know.
  if (result.isNull())
    warning(klocale->translate("ERROR: couldn't find icon: %s"), (const char *) name);
*/
  return result;
}

QPixmap KIconLoader::loadMiniIcon ( const QString &name, int w, int h ){
  QPixmap result;
  if (name.left(1)!='/'){
    result = loadInternal( "mini/" + name, w, h);
  }
  if (result.isNull())
    result = loadInternal(name, w, h);
/* Stephan: See above
  if (result.isNull())
    warning(klocale->translate("ERROR: couldn't find mini icon: %s"), (const char *) name);
*/
  return result;
			// Let's be recursive (but just once at most)
			full_path = getIconPath( "unknown.xpm" , false); 
QPixmap KIconLoader::loadApplicationIcon ( const QString &name, int w, int h ){
  addPath(KApplication::kde_icondir());
  addPath(QDir::homeDirPath() + "/.kde/share/icons" );
  QPixmap result = loadIcon(name, w, h);
  pixmap_dirs.remove((unsigned int) 0);
  pixmap_dirs.remove((unsigned int) 0);
  return result;
}

QPixmap KIconLoader::loadApplicationMiniIcon ( const QString &name, int w, int h ){
  addPath(KApplication::kde_icondir());
  addPath(QDir::homeDirPath() + "/.kde/share/icons" );
  QPixmap result = loadMiniIcon(name, w, h);
  pixmap_dirs.remove((unsigned int) 0);
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
      if( !(pix->isNull()) )
	{ 
	  name_list.append(name);
	  pixmap_list.append(pix);
	} else {
	  // free it, when it's not used
          delete pix;
	  return new_xpm;
	 }
    }
  else
    {
	pix = pixmap_list.at(index);
    }


  if (pix && !pix->isNull() && w > 0 && h > 0 && 
      (pix->width() > w || pix->height() > h)){
    QWMatrix m;
    m.scale(w/(float)pix->width(), h/(float)pix->height());
    return pix->xForm(m);
  }
  return *pix;
}


void KIconLoader::addPath(QString path)
{
  QDir dir(path.data());

  if (dir.exists())
    pixmap_dirs.insert(0, path);

	warning( "KIconLoader::flush is deprecated." );
}

#include "kiconloader.moc"

