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
   Revision 1.72  1999/10/09 00:08:27  kalle
   The dreaded library cleanup: getConfig() -> config() and friends (see separate mail)

   Revision 1.71  1999/10/08 23:04:36  torben
   For components in libraries one has to use KLibGlobal instead
   of KGlobal

   Revision 1.70  1999/08/20 08:27:37  kulow
   the endless story about unknown.png continues :)
   time to go with the #include "unknown.xpm"

   Revision 1.69  1999/08/15 22:02:40  porten
   unknown.png

   Revision 1.68  1999/08/15 13:14:36  kulow
   get rid of ICON and Icon. These macros names are very misleading as it doesn't
   load anything than toolbar pictures. I added the function BarIcon as replacement
   for these macros which doesn't require klobal.h to be included

   Revision 1.67  1999/08/10 16:50:12  kulow
   what would you do with absolute paths in "getIconPath"? Returning them
   seems to be the only good thing - instead of this warning

   Revision 1.66  1999/08/09 02:51:14  tibirna
   CT: needed ugly special treatment for icons given as fully qualified path filenames. Perhaps temporary, but I finally tested it and works :-(

   Revision 1.65  1999/08/08 22:46:29  tibirna
   CT: Sorry, Coolo. In the light of the last explanation (using "name" instead of "name.png" or "name.xpm" in the ICON macro) i reverted the code to your version and put an additional (fallback) check for cases of erroneous (or extraneous) calls, where developers will still include ".png" extensions or use any other (are ".bmp", ".xbm", ".pbm" ever used or supposed to be used?)

   I apologize for this mess.

   Revision 1.64  1999/08/08 22:07:08  mosfet
   CT: Uh, you may want to wait to switch to unknown.png from unknown.xpm until
   unknown.png exists... Just because you are old doesn't mean you are right :P

   Revision 1.63  1999/08/08 19:58:12  tibirna
   CT: Mosfet, you should listen to elder people from time to time.
   	when I said put the .png recompose inside the "if", I also
   	said add the else (line 332). Oh, and "unknown.xpm" should
   	be "unknown.png", Coolo. (That's the reason, Mos)

   Revision 1.62  1999/08/08 17:55:19  mosfet
   Moved the appending of .png into the check for .xpm, or else you get
   .png.png ;-)

   Revision 1.61  1999/08/07 18:56:59  kulow
   strip away the extension if it's .xpm and look for .png first and then .xpm

   Revision 1.60  1999/06/27 17:33:28  waba
   WABA: Updated docu, fixed directory scan order

   Revision 1.59  1999/06/24 21:03:15  kulow
   register appname + "/icons/mini" to "mini" too

   Revision 1.58  1999/06/23 11:14:08  kulow
   my first approach with addCustomized was not general enough as it relied
   on config() and dirs(). Now I abuse the KConfig constructor to add himself
   to the dirs() object (which will refuse to do that twice).
   application using just one KSimpleConfig without a KConfig object (as I'm
   afraid kioslaves could be) would still not find anything besides compiled in
   defaults. Hmm, I leave this to the application - if they want to use full
   dirs() power, they need to create a KConfig object somewhen or try
   KGlobal::config() != 0 :)

   Revision 1.57  1999/06/21 06:22:48  tibirna
   CT: let's see some icons.

   (Yepppeee!! Finaly doing real coding and testing. This feels waaaay cool :-)

   And, BTW, I officially switched to KDE-2.0 this night :-)

   Revision 1.56  1999/06/20 12:34:55  kulow
   some fix for loadApplicationIcon.

   Revision 1.55  1999/06/19 22:42:56  kulow
   making Robert's change right

   Revision 1.54  1999/06/18 20:23:08  kulow
   some more cleanups of KApplication. the KConfig instance is controlled
   by KGlobal to make it independent from KApplication.
   KApplication::getConfig() just calls KGlobal::config()

   Revision 1.53  1999/06/18 18:44:56  dmuell
   CVS_SILENT coolo is to fast for me ;)
   Indentation fix.

   Revision 1.52  1999/06/18 18:14:59  kulow
   CVS_SILENT small fix

   Revision 1.51  1999/06/18 18:06:19  kulow
   - KStandardDirs::kde_data_relative() + "/pics/");
   + KStandardDirs::kde_data_relative() + appName + "/pics/");

   Revision 1.50  1999/06/17 17:28:01  kulow
   taking out assert in kglobal and make kiconloader a bit more defensive

   Revision 1.49  1999/06/17 11:50:37  kulow
   yet another addition to make KIconLoader flexible (some kcontrol apps tried
   really hard to ruin all internals of KIconLoader ;(
   You can now kiconLoader->setIconType("kpanel_pics") and add a standard resource
   type into KGlobal::dirs() with this type name and then ICON will load from this
   directories. Way more flexible than the old hacks

   Revision 1.48  1999/06/17 10:20:24  kulow
   some changes to make KIconloader flexible enough to let konqueror load
   kwm icons :)

   Revision 1.47  1999/06/16 21:01:23  kulow
   kiconloader doesn't use it's own directories, but use KStandardDirs from now
   on - for this I removed insertDirectory and appendDirectory from it's API.
   Afaik only koffice used it.
   And I also added KStandardDirs::kde_data_relative() which just returns
   /share/apps to add a central place to define such paths. I think about
   adding more of these static functions to make the whole thing as configurable
   as it used to be.

   Revision 1.46  1999/06/14 10:42:43  kulow
   some more correct const char*ness

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

#include "kpixmap.h"
#include "klocale.h"
#include "kapp.h"
#include "kconfig.h"
#include "kglobal.h"
#include "klibglobal.h"
#include "kstddirs.h"

void KIconLoader::initPath()
{
    // DF ---- Large icons --------------
    // set the key depending on the current application
    // FIXME: This is not a very nice hack at all. The app should be
    // able to specify its own key. (Taj)

    KConfig* init_config = 0;

    if (config) {
	config->setGroup(appname);
	QStringList list = config->readListEntry( varname, ':' );
	
	QStringList::Iterator it = list.begin();
	
	for ( ; it != list.end(); ++it ) {
	    addPath( *it );
	}

	init_config = config;
    } else
	init_config = new KConfig();

  QString key = "KDE";
  if (appname == "kpanel")
    key = "kpanel";
  if (appname == "kfm")
    key = "kfm";

  KConfigGroupSaver(init_config, "KDE");
  QString setting = init_config->readEntry( key + "IconStyle", "Normal" );

  // order is important! -- Bernd
  // higher priority at the end

  bool large = (setting == "Large" );

  KStandardDirs* dirs;
  if ( library )
      dirs = library->dirs();
  else
      dirs = KGlobal::dirs();
  
  if ( large )
      dirs->addResourceType("icon",
			    KStandardDirs::kde_default("data") +
			    "icons/large");

  dirs->addResourceType("toolbar",
			KStandardDirs::kde_default("data") +
			appname + "/pics/");

  if ( large )
      dirs->addResourceType("toolbar",
			    KStandardDirs::kde_default("data") +
			    appname + "/pics/large");

  dirs->addResourceType("toolbar",
			KStandardDirs::kde_default("data") +
			appname + "/toolbar/");

  dirs->addResourceType("mini",
			KStandardDirs::kde_default("data") +
			appname + "/icons/mini/");
}

KIconLoader::KIconLoader( KConfig *conf, const QString &app_name, const QString &var_name ) :
  config(conf), library( 0 ), appname(app_name), varname(var_name)
{
    iconType = "toolbar";
    initPath();
}

KIconLoader::KIconLoader( KLibGlobal* _library, const QString& var_name )
    : config( _library->config() ), appname( _library->name() ), varname(var_name)
{
    library = _library;
    iconType = "toolbar";
    initPath();
}

KIconLoader::KIconLoader() : config(0), library( 0 ), varname("IconPath")
{
  KApplication *app = KApplication::kApplication();
  if (app) {
    config = KGlobal::config();
    config->setGroup("KDE Setup");
    appname = KApplication::kApplication()->name();
  }
  iconType = "toolbar";

  initPath();
}

QPixmap KIconLoader::loadIcon ( const QString& name, int w,
		int h, bool canReturnNull )
{
	QPixmap result = loadInternal(name, w, h);

	if (result.isNull() && !canReturnNull) {
	    warning("%s : ERROR: couldn't find icon: %s",
		    appname.ascii(), name.ascii() );
	    result = loadInternal("unknown", w, h);
	}

	return result;
}


QPixmap KIconLoader::reloadIcon ( const QString& name, int w, int h )
{
	return loadInternal( name, w, h, false );
}

QPixmap KIconLoader::loadApplicationIcon ( const QString& name, int w, int h )
{
	QString path = locate("icon", name);
	if (!path.isEmpty()) //CT 21Jun1999 if is *not* empty!!!
          return loadInternal(path, w, h);
	else
	  return QPixmap();
}


QPixmap KIconLoader::loadApplicationMiniIcon ( const QString& name,
	int w, int h )
{
        QPixmap result = loadInternal(locate("mini", name), w, h);
		
	if (result.isNull())
	  result = loadInternal(locate("icon", name), w, h);

	return result;
}

QString KIconLoader::iconPath( const QString& name, bool always_valid)
{
  if (name.at(0) == '/') // we can't do anything with an absolute path than returning
    return name;

  QString full_path;
  if (!name.isEmpty()) {
    QString path = name;

    if (path.right(4) == ".xpm") {
      path.truncate(path.length() - 4);
      warning("stripping .xpm from icon %s", name.ascii());
    }
    full_path = locate(iconType, path + ".png");
    if (full_path.isNull())
      full_path = locate(iconType, path + ".xpm" );

    if (full_path.isNull())
      full_path = locate(iconType, path);
  }
  if (full_path.isNull() && always_valid)
    full_path = locate(iconType, "unknown.png");

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

	pix.load( iconPath(name), 0, KPixmap::LowColor );

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

void KIconLoader::addPath( QString path )
{
    if ( library )
	library->dirs()->addResourceDir("toolbar", path);
    else
	KGlobal::dirs()->addResourceDir("toolbar", path);
}

void KIconLoader::flush( const QString& )
{
	// This is a NO-OP
	warning( "KIconLoader::flush is deprecated." );
}

QPixmap BarIcon(const QString& pixmap , KLibGlobal* library )
{
    if ( library )
	return library->iconLoader()->loadIcon(pixmap, 0, 0, false);
    else
	return KGlobal::iconLoader()->loadIcon(pixmap, 0, 0, false);
}

#include "kiconloader.moc"

