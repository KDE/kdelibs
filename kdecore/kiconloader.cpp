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
   Revision 1.45  1999/06/05 01:15:11  dmuell
   global configuration will now be searched in the following paths:

   KDEDIR/share/config/kdeglobals
   /usr/lib/KDE/system.kdeglobals
   /usr/local/lib/KDE/system.kdeglobals
   ~/.kde/share/config/kdeglobals

   it did it previously in

   KDEDIR/share/config/kderc
   /usr/lib/KDE/system.kderc
   /usr/local/lib/KDE/system.kderc
   ~/.kderc

   Note: It's better not to use a "rc" suffix - there might be an
   application that has that name.

   Revision 1.44  1999/06/03 09:21:32  ssk
   Added arg to loadInternal to ignore cache if requested.
   Updated reloadIcon to use it.

   Revision 1.43  1999/06/03 09:07:50  reggie
   made it compiling with recent Qt (there is no operator-> for QValueListIterator anymore)

   Revision 1.42  1999/05/26 18:02:13  kulow
   David is right when he says addIcons is overkill. Removed the function
   again and renamed the old function setDir to changeDirs as this is what
   it does. For this I added getResourceDirs(type) to kstddirs. Don't use
   it if you can avoid it ;)

   Revision 1.41  1999/05/26 12:29:32  kulow
   adding getIconList() - the iconloader will find all icons itself. Currently
   it's just a call to findAllResources("toolbar"), but in this case we urgently
   need an abstraction layer as KIconLoader may be extended to look for large
   icons too

   Revision 1.40  1999/05/25 16:17:24  kulow
   two changes:
     kde_icondir() has been removed. Use locate("icon", pixmap) instead
     KIconLoader::loadMiniIcon has been removed. It does the same as
     loadApplicationMiniIcon and the later is less confusing in what it
     does (loading icons out of share/icons/mini)

   Revision 1.39  1999/05/23 21:59:06  pbrown
   new kconfig system is in.  External API remains the same, but the in-memory
   and on-disk formats have been abstracted.  KConfigBase now is an ADT with
   pure virtual functions.  KConfig implements KConfigBase with a QMap-based
   system, and a coarse cache which will kick the whole lot out of memory
   after a scaled amount of inactivity.  The only backend that is implemented
   right now is the INI-style backend we have had forever, but with this new
   system, it will not be difficult to plug in a XML backend, a database
   backend, or whatever we please, in the future.

   I have worked hard to fully document _everything_ in the API. KDoc should
   provide nice documentation if you are interested.

   Revision 1.38  1999/05/13 19:03:24  bieker
   More QStrings.

   Revision 1.37  1999/05/11 20:43:32  bieker
   * loadInternal wants a QString
   * don't use strcmp to compare QString.

   Revision 1.36  1999/05/09 17:22:53  ssk
   API_CHANGE: KIconLoader::getDirList now returns QStringList.
   KIconLoader now uses QPixmapCache.
   Added a new appendDirectory method.

   Revision 1.35  1999/05/07 16:45:11  kulow
   adding more explicit calls to ascii()

   Revision 1.34  1999/04/19 15:49:41  kulow
   cleaning up yet some more header files (fixheaders is your friend).
   Adding copy constructor to KPixmap to avoid casting while assingment.

   The rest of the fixes in kdelibs and kdebase I will commit silently

   Revision 1.33  1999/03/14 23:50:06  dfaure
Load large icons in icons/large or pics/large if setting is 'large'.
*/

#include <qapplication.h>
#include <qdir.h>
#include <qpainter.h>
#include <qwmatrix.h>
#include <qpixmapcache.h>

#include "kiconloader.h"

#include <kpixmap.h>
#include <klocale.h>
#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>

void KIconLoader::initPath()
{
  // DF ---- Large icons --------------
  // set the key depending on the current application
  // FIXME: This is not a very nice hack at all. The app should be
  // able to specify its own key. (Taj)

  QString key = "KDE";
  if (kapp->name() == "kpanel")
    key = "kpanel";
  if (kapp->name() == "kfm")
    key = "kfm";
  KConfig config; // with no filenames given, it will read only global config 
  config.setGroup("KDE");
  QString setting = config.readEntry( key + "IconStyle", "Normal" );
  //debug("App is %s - setting is %s", kapp->name(), setting.data());
  // DF

  // order is important! -- Bernd
  // higher priority at the end

  bool large = (setting == "Large" );

  addPath( KApplication::kde_toolbardir() );

  addPath( KApplication::localkdedir() + "/share/toolbar" );

  if ( large )
    KGlobal::dirs()->addResourceType("icon", "/share/icons/large");

  addPath( KApplication::kde_datadir() + "/"
	   + kapp->name() + "/toolbar" );
  addPath( KApplication::localkdedir() + "/share/apps/"
	   + kapp->name() + "/toolbar" );
  addPath( KApplication::kde_datadir() + "/"
	   + kapp->name() + "/pics" );

  if ( large )
    addPath( KApplication::kde_datadir() + "/"
	     + kapp->name() + "/pics/large" );

  addPath( KApplication::localkdedir() + "/share/apps/"
	   + kapp->name() + "/pics" );

  if ( large )
    addPath( KApplication::localkdedir() + "/share/apps/"
	     + kapp->name() + "/pics/large" );

}

KIconLoader::KIconLoader( KConfig *conf,
		const QString &app_name, const QString &var_name )
{

	config = conf;
	config->setGroup(app_name);
	QStringList list = config->readListEntry( var_name, ':' );

	QStringList::Iterator it = list.begin();

	for ( ; it != list.end(); ++it ) {
		addPath( *it );
	}

	initPath();

}

KIconLoader::KIconLoader()
{
	config = KApplication::getKApplication()->getConfig();
	config->setGroup("KDE Setup");
	QStringList list = config->readListEntry( "IconPath", ':' );

	QStringList::Iterator it = list.begin();

	for ( ; it != list.end(); ++it ) {
		addPath( *it );
	}

	initPath();


	/*
	   for(char* c = pixmap_dirs.first(); c ; c = pixmap_dirs.next()){
	   printf("()in path:%s\n",c);
	   }
	 */

}

QPixmap KIconLoader::loadIcon ( const QString& name, int w,
		int h, bool canReturnNull ) {
	QPixmap result = loadInternal(name, w, h);

	/* Stephan: It's OK to know, how many icons are still missing, but
	   we don't need to tell everybody ;) Perhaps this can be con-
	   verted to a KDEBUG solution, that is more silent? Don't know.
	David: Re-enabled the warning. Most applications (esp. koffice) crash
	if the icon doesn't exist, anyway. And base apps should be ok now.
	 */
	if (result.isNull() && !canReturnNull) {
		warning("%s : ERROR: couldn't find icon: %s",
				kapp->name(), name.ascii() );
		result = loadInternal("unknown.xpm", w, h);
	}

	return result;
}


QPixmap KIconLoader::reloadIcon ( const QString& name, int w, int h )
{
	return loadInternal( name, w, h, false );
}

QPixmap KIconLoader::loadApplicationIcon ( const QString& name, int w, int h )
{
        QPixmap result = loadInternal(locate("icon", name), w, h);
	return result;
}


QPixmap KIconLoader::loadApplicationMiniIcon ( const QString& name,
	int w, int h )
{
        QPixmap result = loadInternal(locate("mini", name), w, h);
		
	if (result.isNull())
	  result = loadInternal(locate("icon", name), w, h);

	return result;
}

QString KIconLoader::getIconPath( const QString& name, bool always_valid)
{
	QString full_path;
	QFileInfo finfo;

	if( name[0] == '/' ){
		full_path = name;
	}
	else{
		QStringList::Iterator it = pixmap_dirs.begin();

		while ( it != pixmap_dirs.end() ){

			full_path = *it;
			full_path += '/';
			full_path += name;
			finfo.setFile( full_path );
			if ( finfo.exists() )
				break;
			++it;
		}
		if ( (always_valid) && ( (*it).isNull()) ){
			// Let's be recursive (but just once at most)
			full_path = getIconPath( "unknown.xpm" , false);
		}
	}
	return full_path;
}

QPixmap KIconLoader::loadInternal ( const QString& name, int w,  int h,
	bool hcache )
{

	QString cacheKey = "$kico_";
	cacheKey += name;
	KPixmap pix;

	if ( hcache && QPixmapCache::find( cacheKey, pix ) == true ) {
		return pix;
	}

	pix.load( getIconPath(name), 0, KPixmap::LowColor );

	if ( pix.isNull() ) {
		return pix;
	}

	if ( (w > 0 && h > 0 ) && ( pix.width() > w || pix.height() > h) ) {

		QWMatrix m;
		m.scale(w/(float)pix.width(), h/(float)pix.height());

		pix =  pix.xForm(m);
	}

	QPixmapCache::insert( cacheKey, pix );

	return pix;
}

bool KIconLoader::insertDirectory( int index, const QString& dir_name )
{
	if( index < 0 || (unsigned)index > pixmap_dirs.count() ) {
		return false;

	}

	pixmap_dirs.insert( pixmap_dirs.at(index), dir_name );
	return true;
}



void KIconLoader::addPath( QString path )
{

	QDir dir(path);

	if (dir.exists()){
		pixmap_dirs.insert( pixmap_dirs.begin(), path );
	}
	else{
		//    fprintf(stderr,"Path %s doesn't exist\n",path.data());
	}

}

void KIconLoader::flush( const QString& )
{
	// This is a NO-OP
	warning( "KIconLoader::flush is deprecated." );
}

#include "kiconloader.moc"

