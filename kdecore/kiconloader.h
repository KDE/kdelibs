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
   Revision 1.41  1999/10/23 16:16:24  kulow
   here comes KInstance - "KApplication light"
   It's a new KLibGlobal and KGlobal only keeps a pointer to a global
   instance. KApplication is derived from KInstance - making it possible
   to move things out of KApplication into KInstance to make Components
   easier.
   Needs some testings and I bet some tweaks here and there :)

   Revision 1.40  1999/10/09 09:48:42  kalle
   more get killing
   You need to cvs update your libc (joke!)

   Revision 1.39  1999/10/08 23:04:37  torben
   For components in libraries one has to use KLibGlobal instead
   of KGlobal

   Revision 1.38  1999/09/12 14:56:09  dfaure
   docu fixes (loadMiniIcon doesn't exist anymore)

   Revision 1.37  1999/08/15 13:14:36  kulow
   get rid of ICON and Icon. These macros names are very misleading as it doesn't
   load anything than toolbar pictures. I added the function BarIcon as replacement
   for these macros which doesn't require klobal.h to be included

   Revision 1.36  1999/06/27 17:33:28  waba
   WABA: Updated docu, fixed directory scan order

   Revision 1.35  1999/06/17 11:50:38  kulow
   yet another addition to make KIconLoader flexible (some kcontrol apps tried
   really hard to ruin all internals of KIconLoader ;(
   You can now kiconLoader->setIconType("kpanel_pics") and add a standard resource
   type into KGlobal::dirs() with this type name and then ICON will load from this
   directories. Way more flexible than the old hacks

   Revision 1.34  1999/06/17 10:20:24  kulow
   some changes to make KIconloader flexible enough to let konqueror load
   kwm icons :)

   Revision 1.33  1999/06/16 21:01:24  kulow
   kiconloader doesn't use it's own directories, but use KStandardDirs from now
   on - for this I removed insertDirectory and appendDirectory from it's API.
   Afaik only koffice used it.
   And I also added KStandardDirs::kde_data_relative() which just returns
   /share/apps to add a central place to define such paths. I think about
   adding more of these static functions to make the whole thing as configurable
   as it used to be.

   Revision 1.32  1999/06/05 01:15:11  dmuell
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

   Revision 1.31  1999/06/03 09:21:32  ssk
   Added arg to loadInternal to ignore cache if requested.
   Updated reloadIcon to use it.

   Revision 1.30  1999/05/26 18:02:14  kulow
   David is right when he says addIcons is overkill. Removed the function
   again and renamed the old function setDir to changeDirs as this is what
   it does. For this I added getResourceDirs(type) to kstddirs. Don't use
   it if you can avoid it ;)

   Revision 1.29  1999/05/26 12:29:32  kulow
   adding getIconList() - the iconloader will find all icons itself. Currently
   it's just a call to findAllResources("toolbar"), but in this case we urgently
   need an abstraction layer as KIconLoader may be extended to look for large
   icons too

   Revision 1.28  1999/05/26 08:14:05  dfaure
   Docu improvements. Moved more to the top comment (like the search path) ;
   Added a line about "large/", explain that both constructors keep the default
   search path,  and some more fixes.
   BTW : reloadIcon calls flush which is deprecated...

   Revision 1.27  1999/05/25 16:17:24  kulow
   two changes:
     kde_icondir() has been removed. Use locate("icon", pixmap) instead
     KIconLoader::loadMiniIcon has been removed. It does the same as
     loadApplicationMiniIcon and the later is less confusing in what it
     does (loading icons out of share/icons/mini)

   Revision 1.26  1999/05/23 17:04:52  kulow
   let KGlobal create the IconLoader instance. Removed KApplication::getIconLoader.
   Steffen's idea - just more consequent ;)

   removing some little used kde_*dir functions. May break kdebase parts,
   I'm compiling right now

   Revision 1.25  1999/05/09 23:58:34  pbrown
   was broken with QStringList.  fixed.

   Revision 1.24  1999/05/09 17:22:54  ssk
   API_CHANGE: KIconLoader::getDirList now returns QStringList.
   KIconLoader now uses QPixmapCache.
   Added a new appendDirectory method.

   Revision 1.23  1999/05/07 16:45:13  kulow
   adding more explicit calls to ascii()

   Revision 1.22  1999/05/07 15:42:36  kulow
   making some changes to the code and partly to the API to make it
   -DQT_NO_ASCII_CAST compatible.
   The job is quite boring, but triggers some abuses of QString. BTW:
   I added some TODOs to the code where I was too lazy to continue.
   Someone should start a grep for TODO in the code on a regular base ;)

   Revision 1.21  1999/04/18 19:55:42  kulow
   CVS_SILENT some more fixes

   Revision 1.20  1999/04/06 12:54:11  dfaure
   warning: extra qualification `KIconLoader::' on member `getIconPath' ignored

   Revision 1.19  1999/03/29 04:14:37  ssk
   Rewrote some docs.
*/


#ifndef KICONLOADER_H
#define KICONLOADER_H

class KConfig;
class KInstance;

#include <qobject.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qstring.h>
#include <kglobal.h>

/**
	Icon loader with caching.

	Multiples loads of the same icons using this class will be cached
	using @ref QPixmapCache, saving memory and loading time.

        Within KDE there are three distinct groups of Icons:

        @li toolbar - Toolbar icons are small icons used on pushbuttons.
                      The size is 22x22 pixels.

        @li icon - These are icons used to identify an application,
                   a file type or a directory. They are typically shown
                   on the desktop and in directory listings. Their
                   size is 32x32 pixels or, if KDE has been configured
                   to use large icons, 64x64 pixels

        @li mini - Like 'icon' but with a size of 16x16 pixels.
	
	Icons are searched for according to the KDE file system standard.


	The default search path for 'toolbar' icons is:

	@li $HOME/.kde/share/apps/<appName>/toolbar
	@li $HOME/.kde/share/apps/<appName>/pics
	@li $HOME/.kde/share/toolbar

	@li $KDEDIR/share/apps/<appName>/toolbar
	@li $KDEDIR/share/apps/<appName>/pics
	@li $KDEDIR/share/toolbar


        The default search path for 'icon' icons is:

	@li $HOME/.kde/share/apps/<appName>/icons/large (*)
	@li $HOME/.kde/share/apps/<appName>/icons
	@li $HOME/.kde/share/icons/large (*)
	@li $HOME/.kde/share/icons

	@li $KDEDIR/share/apps/<appName>/icons/large (*)
	@li $KDEDIR/share/apps/<appName>/icons
	@li $KDEDIR/share/icons/large (*)
	@li $KDEDIR/share/icons/

        The paths marked with (*) are only searched if KDE
        has been configured to use 'large' icons.


        The default search path for 'mini' icons is:

	@li $HOME/.kde/share/apps/<appName>/icons/mini
	@li $HOME/.kde/share/icons/mini

	@li $KDEDIR/share/apps/<appName>/icons/mini
	@li $KDEDIR/share/icons/mini

	Extra directories can be added, see @ref KStandardDirs.

	All keys used in QPixmapCache by this class have the "$kico_.." prefix.

	@author Christoph Neerfeld (chris@kde.org)
	@version $Id$
*/
class KIconLoader : public QObject
{
  Q_OBJECT

public:
  /** Default constructor.
   * Adds to the search path the ones listed in [KDE Setup]/IconPath.
   * (in .kderc or the application config file)
   */
  KIconLoader();

  /**
    Constructor.
    If you want to use another path in your application then write into
    a config file :

    <pre>
    [MyApplication]
    PixmapPath=/..../my_pixmap_path
    </pre>

    and call KIconLoader( config, "MyApplication", "PixmapPath" ).

    @param conf		Pointer to a KConfig object which will be searched
    for additional paths.
    @param app_name	Group to search for paths.
    @param var_name	Key to search for paths.

  */
  KIconLoader ( KConfig *conf, const QString &app_name,
		const QString &var_name = "PixmapPath");

  /**
   * Constructs an KIconLoader for a component stored in a shared library.
   * In this case KInstance has to be used instead of the @ref KGlobal.
   */
  KIconLoader( const KInstance* library, const QString &var_name = "PixmapPath" );
    
  /** Destructor. */
  ~KIconLoader () {}

  /**
     Load an icon from disk or cache.
     
	@param name	The name of the icon to load. Absolute pathnames are
	 		allowed.
  	@param w	The max width of the resulting pixmap. Larger icons
			are scaled down. The default is no maximum.
  	@param h	The max height of the resulting pixmap. Larger icons
			are scaled down. The default is no maximum.
	@param canReturnNull	If this is false, this function will return
		the "unknown.png" icon if the requested icon is not found.
		The default is to return null.

	@return	The loaded icon.
  */
  QPixmap loadIcon( const QString& name, int w = 0, int h = 0,
  		bool canReturnNull = true );


  /**
  	Load an icon from disk without cache.

	This is useful if the icon has changed on the filesystem and
	you want to be sure that you get the new version, not the old
	one from the cache.

	@see loadIcon
  */
  QPixmap reloadIcon( const QString& name, int w = 0, int h = 0);

  /**
   * The loadApplication-Icon functions are similar to the
   * usual loadIcon functions except that they look in
   * kdedir()/share/icon first.
   *
   * These function should be used if you are loading the
   * application icons. Normally KApplication does this for
   * you, but special programs like kpanel or kmenuedit
   * need to load the application icons of foreign applications.
   *
   * @see loadIcon, loadApplicationMiniIcon
   */
  QPixmap loadApplicationIcon( const QString& name, int w = 0, int h = 0 );

  /**
  	Similar to loadApplicationIcon, but searches for a mini icon.

	@see loadApplicationIcon
  */
  QPixmap loadApplicationMiniIcon( const QString& name, int w = 0, int h = 0 );

  /**
	Get the complete path for an icon name.

	@param name	The name of the icon to search for.
	@param always_valid If true, the function will return the path to
		unknown.xpm if the icon is not found. Note that it will
		return null if unknown.xpm was also not found.

	@return the physical path to the named icon.
  */
  QString iconPath( const QString& name,
		       bool always_valid=false);


  /**
   * sets the type of icons loadIcon will load. Default is "toolbar"
   **/
  void setIconType(const QString &type) { iconType = type; }

  /**
   * Remove an icon from the cache. This is no longer required since @ref
   * QPixmapCache does this for us.
   * @deprecated
   */
  void flush( const QString &name );

protected:

  KConfig		*config;
  const KInstance             * library;
    
  /**
	honourcache will check if the icon is contained in the cache before
	trying to load it.
  */
  QPixmap loadInternal( const QString& name, int w = 0, int h = 0,
			bool honourcache = true );

  QString appname;
  QString varname;
  QString iconType;
    
private:
  void initPath();
  void addPath(QString path);

  // Disallow assignment and copy-construction
  KIconLoader( const KIconLoader& );
  KIconLoader& operator= ( const KIconLoader& );

};

QPixmap BarIcon(const QString& pixmap, const KInstance* library = KGlobal::instance());

#endif // KICONLOADER_H




