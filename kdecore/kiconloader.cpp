/* 
   $Id$
   
   This file is part of the KDE libraries
   Copyright (C) 1997 Christoph Neerfeld (chris@kde.org)
   
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
   
   $Log$
   Revision 1.32  1999/03/10 15:21:34  kulow
   going over sprintfs in kdelibs to find dangerous usings :)
#include <qapplication.h>
   Revision 1.31  1999/03/09 16:39:48  dfaure
   Merging with 1.1 branch : initPath(), and header doc update.
#include <qpainter.h>
   Revision 1.30  1999/03/09 06:47:13  antlarr
   Added the getIconPath function that returns the full path to a icon filename and
   changed loadInternal to use it.

   Revision 1.30  1999/03/09 07:40:26  antlarr
   Added the getIconPath function that returns the full path to a icon filename,
   it can be useful for many applications. Changed loadInternal to use it.
			+ kapp->appName() + "/pics" ); 
   Revision 1.29  1999/03/02 15:56:26  kulow
   CVS_SILENT replacing klocale->translate with i18n
	if ( large )
   Revision 1.28  1999/03/02 00:09:42  dfaure
   Fix for ICON() when icon not found. Now returns a default pixmap, unknown.xpm,
   instead of 0L. Will prevent koffice apps and some others from crashing when
   not finding an icon. Approved by Reggie.
}
   loadIcon not changed, since I tried and it broke kpanel (which uses loadIcon
   even on empty string in configuration item). This means loadIcon and ICON are
   no longer equivalent : loadIcon is for apps that want to do complex things
   with icons, based on whether they're installed or not, ICON() is for apps
   that just want an Icon and don't want to care about it !
{
   Of course, unknown.xpm is WAYS to big for a toolbar - that's the point :
   you easily see that the icon is missing....   :)))
	config = conf;
   Not tested with Qt2.0, of course, but it's time for binary incompat changes...
	QStringList::Iterator it = list.begin();
   Revision 1.27  1999/03/01 23:33:30  kulow
   CVS_SILENT ported to Qt 2.0
	for ( ; it != list.end(); ++it ) {
   Revision 1.26  1999/01/18 10:56:20  kulow
   .moc files are back in kdelibs. Built fine here using automake 1.3
	}
   Revision 1.25  1999/01/15 09:30:37  kulow
   it's official - kdelibs builds with srcdir != builddir. For this I
   automocifized it, the generated rules are easier to maintain than
   selfwritten rules. I have to fight with some bugs of this tool, but
   generally it's better than keeping them updated by hand.

   Revision 1.24  1998/11/02 10:08:34  ettrich
   new reload method for kiconloader (Rene Beutler)
}
   Revision 1.23  1998/10/04 11:05:30  neerfeld
   fixed a bug in loadInternal; fixes the crash of kmenuedit

   Revision 1.22  1998/09/01 20:21:25  kulow
   I renamed all old qt header files to the new versions. I think, this looks
   nicer (and gives the change in configure a sense :)

   Revision 1.21  1998/08/26 18:37:47  neerfeld
   bug fix for loadInternal;  changed email address
	config = KApplication::getKApplication()->getConfig();
   Revision 1.20  1998/08/17 10:34:02  konold
	QStringList::Iterator it = list.begin();
   Martin K.: Fixed a typo
	for ( ; it != list.end(); ++it ) {
   Revision 1.19  1998/06/15 12:49:32  kulow
   applied patch to replace .kde with localkdedir()
		addPath( *it );
   Revision 1.18  1998/04/25 13:15:28  mark
   MD: Added KPixmap and changed KIconLoader to use it.
	/*
   Revision 1.17  1998/03/08 18:49:03  wuebben
   Bernd: fixed up the kiconloader class -- it was completely busted
	   printf("()in path:%s\n",c);
}
*/
	if (result.isNull() && !canReturnNull) {
#include <qapplication.h>
#include <qdir.h>
#include <qpainter.h>
#include <qwmatrix.h>
		result = loadInternal("unknown.xpm", w, h);
#include "kiconloader.h"
	}
#include <kpixmap.h>
#include <klocale.h>
#include <kapp.h>
}
void KIconLoader::initPath()
{
  // DF ---- Large icons --------------
  // set the key depending on the current application
  QString key = "KDE";
  if (strcmp( kapp->name(), "kpanel" ) == 0)
    key = "kpanel";
  if (strcmp( kapp->name(), "kfm" ) == 0)
    key = "kfm";
  KConfig config; // read .kderc
  config.setGroup("KDE");
  QString setting = config.readEntry( key + "IconStyle", "Normal" );
  //debug("App is %s - setting is %s", kapp->name(), setting.data());
  // DF
  
  // order is important! -- Bernd
  // higher priority at the end

  addPath( KApplication::kde_toolbardir() );
  addPath( KApplication::kde_icondir() );
  if (setting == "Large")
    addPath( KApplication::kde_icondir() + "/large" );

  addPath( KApplication::localkdedir() + "/share/toolbar" ); 
  addPath( KApplication::localkdedir() + "/share/icons" ); 
  if (setting == "Large")
    addPath( KApplication::localkdedir() + "/share/icons/large" );

  addPath( KApplication::kde_datadir() + "/" + kapp->appName() + "/toolbar" );
  addPath( KApplication::localkdedir() + "/share/apps/" + kapp->appName() + "/toolbar" ); 
  addPath( KApplication::kde_datadir() + "/" + kapp->appName() + "/pics" );
  if (setting == "Large")
    addPath( KApplication::kde_datadir() + "/" + kapp->appName() + "/pics/large" );
  addPath( KApplication::localkdedir() + "/share/apps/" + kapp->appName() + "/pics" ); 
  if (setting == "Large")
    addPath( KApplication::localkdedir() + "/share/apps/" + kapp->appName() + "/pics/large" ); 

QPixmap KIconLoader::reloadIcon ( const QString& name, int w, int h ){
KIconLoader::KIconLoader( KConfig *conf, 
			  const QString &app_name, const QString &var_name ){
	flush( name );
  QStrList list;
	return loadInternal( name, w, h );
  config = conf;
  config->setGroup(app_name);
  config->readListEntry( var_name, list, ':' );

  for (const char *it=list.first(); it; it = list.next())
    addPath(it);
QPixmap KIconLoader::loadMiniIcon ( const QString& name, int w, int h ){
  initPath();

  name_list.setAutoDelete(TRUE);
  pixmap_dirs.setAutoDelete(TRUE);
  pixmap_list.setAutoDelete(TRUE);
		result = loadInternal( (QString("mini/") + name).ascii(), w, h);
  /*
  for(char* c = pixmap_dirs.first(); c ; c = pixmap_dirs.next()){
    printf("in path:%s\n",pixmap_dirs.current());
  }
  */
	}
}
 
KIconLoader::KIconLoader( ){

  QStrList list;
Stephan: See above
  config = KApplication::getKApplication()->getConfig();
  config->setGroup("KDE Setup");
  config->readListEntry( "IconPath", list, ':' );

  for (const char *it=list.first(); it; it = list.next())
    addPath(it);

  initPath();

  name_list.setAutoDelete(TRUE);
  pixmap_dirs.setAutoDelete(TRUE);
  pixmap_list.setAutoDelete(TRUE);

  /*
  for(char* c = pixmap_dirs.first(); c ; c = pixmap_dirs.next()){
    printf("()in path:%s\n",c);
  }
  */

}
 
KIconLoader::~KIconLoader()

  name_list.clear();
  pixmap_list.clear();
}

QPixmap KIconLoader::loadIcon ( const QString &name, int w, int h, bool canReturnNull ){
  QPixmap result = loadInternal(name, w, h);
	return result;
/* Stephan: It's OK to know, how many icons are still missing, but
       we don't need to tell everybody ;) Perhaps this can be con-
       verted to a KDEBUG solution, that is more silent? Don't know.
   David: Re-enabled the warning. Most applications (esp. koffice) crash
        if the icon doesn't exist, anyway. And base apps should be ok now.
*/
  if (result.isNull() && !canReturnNull) {
    warning("%s : ERROR: couldn't find icon: %s", kapp->appName().data(), name.data());
    result = loadInternal("unknown.xpm", w, h);
  }
QPixmap KIconLoader::loadApplicationIcon ( const QString& name, int w, int h )
  return result;
}

	// this is trouble since you don't know whether the addPath was
QPixmap KIconLoader::reloadIcon ( const QString &name, int w, int h ){
  flush( name );
	// paths. I hope this will not give us too much of a performance
  return loadInternal( name, w, h );
	// hit. Other wise I will have to break binary compatibiliy
	// -- Bernd

QPixmap KIconLoader::loadMiniIcon ( const QString &name, int w, int h ){

  QPixmap result;
	int w, int h )
  if (name.left(1).at(0) != '/'){
    result = loadInternal( "mini/" + name, w, h);
  }

  if (result.isNull())
    result = loadInternal(name, w, h);
	// this is trouble since you don't know whether the addPath was
	// paths. I hope this will not give us too much of a performance
/* 
   Stephan: See above
   if (result.isNull())
    warning(i18n("ERROR: couldn't find mini icon: %1").arg(name));
	// -- Bernd
*/

  return result;
			// Let's be recursive (but just once at most)
			full_path = getIconPath( "unknown.xpm" , false); 
QPixmap KIconLoader::loadApplicationIcon ( const QString &name, int w, int h ){
	return full_path;
  //  addPath(KApplication::kde_icondir());
  //  addPath(KApplication::localkdedir() + "/share/icons" );
{
  QPixmap result = loadIcon(name, w, h);
	KPixmap pix;
  // this is trouble since you don't know whether the addPath was successful
  // I simply added icon dir to the set of standard paths. I hope
  // this will not give us too much of a performance hit. Other wise
  // I will have to break binary compatibiliy -- Bernd
	if ( QPixmapCache::find( cacheKey, pix ) == true ) {
  // pixmap_dirs.remove((unsigned int) 0);
  // pixmap_dirs.remove((unsigned int) 0);
	pix.load( getIconPath(name), 0, KPixmap::LowColor );
  return result;
	if ( pix.isNull() ) {
}

		QWMatrix m;
QPixmap KIconLoader::loadApplicationMiniIcon ( const QString &name, int w, int h ){

  //  addPath(KApplication::kde_icondir());
  //  addPath(KApplication::localkdedir() + "/share/icons" );

  QPixmap result = loadMiniIcon(name, w, h);

  // this is trouble since you don't know whether the addPath was successful
  // I simply added icon dir to the set of standard paths. I hope
  // this will not give us too much of a performance hit. Other wise
  // I will have to break binary compatibiliy -- Bernd
{ 
  //  pixmap_dirs.remove((unsigned int) 0);
  //  pixmap_dirs.remove((unsigned int) 0);
	}
  return result;

}
	pixmap_dirs.insert( pixmap_dirs.at(index), dir_name ); 
QString KIconLoader::getIconPath( const QString &name, bool always_valid)
}
    QString full_path;
    QFileInfo finfo;

    if( name.left(1) == "/" ){
      full_path = name;
    }
    else{
      QStrListIterator it( pixmap_dirs );
      while ( it.current() ){
	
	full_path = it.current();
	full_path += '/';
	full_path += name;
	finfo.setFile( full_path );
	if ( finfo.exists() )
	      break;
	++it;
      }
      if ( (always_valid) && (!it.current()) ){
        // Let's be recursive (but just once at most)
        full_path = getIconPath( "unknown.xpm" , false); 
      }
    }
    return full_path;
}

QPixmap KIconLoader::loadInternal ( const QString &name, int w,  int h ){

  QPixmap *pix;
  KPixmap new_xpm;

  int index;

  if ( (index = name_list.find(name)) < 0){
    
    pix = new QPixmap;
    new_xpm.load( getIconPath(name), QString::null, KPixmap::LowColor );
    *pix = new_xpm;
    
    if( !(pix->isNull()) ){
      
      name_list.append(name);
      pixmap_list.append(pix);

    } 
    else {
      delete pix;
      return new_xpm;
    }
  }
  else{
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


void KIconLoader::addPath(QString path){

  QDir dir(path.data());

  if (dir.exists()){
    pixmap_dirs.insert(0, path);
  }
  else{
    //    fprintf(stderr,"Path %s doesn't exist\n",path.data());
  }
		pixmap_dirs.insert( pixmap_dirs.begin(), path );
	}
	else{
void KIconLoader::flush( const QString& name  )
	}
  int index;

  if( (index = name_list.find(name)) >= 0 ) {
     name_list.remove( index );
     pixmap_list.remove( index );
  }

void KIconLoader::flush( const QString& )
{
	// This is a NO-OP
	warning( "KIconLoader::flush is deprecated." );
}

#include "kiconloader.moc"

