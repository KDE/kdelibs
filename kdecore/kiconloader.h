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
   Revision 1.19  1999/03/29 04:14:37  ssk
   Rewrote some docs.

   Revision 1.18  1999/03/09 16:39:50  dfaure
   Merging with 1.1 branch : initPath(), and header doc update.

   Revision 1.17  1999/03/09 06:47:14  antlarr
   Added the getIconPath function that returns the full path to a icon filename and
   changed loadInternal to use it.

   Revision 1.17  1999/03/09 07:40:26  antlarr
   Added the getIconPath function that returns the full path to a icon filename,
   it can be useful for many applications. Changed loadInternal to use it.

   Revision 1.16  1999/03/02 00:09:43  dfaure
   Fix for ICON() when icon not found. Now returns a default pixmap, unknown.xpm,
   instead of 0L. Will prevent koffice apps and some others from crashing when
   not finding an icon. Approved by Reggie.

   loadIcon not changed, since I tried and it broke kpanel (which uses loadIcon
   even on empty string in configuration item). This means loadIcon and ICON are
   no longer equivalent : loadIcon is for apps that want to do complex things
   with icons, based on whether they're installed or not, ICON() is for apps
   that just want an Icon and don't want to care about it !

   Of course, unknown.xpm is WAYS to big for a toolbar - that's the point :
   you easily see that the icon is missing....   :)))

   Not tested with Qt2.0, of course, but it's time for binary incompat changes...

   Revision 1.15  1998/11/02 10:08:35  ettrich
   new reload method for kiconloader (Rene Beutler)

   Revision 1.14  1998/09/01 20:21:27  kulow
   I renamed all old qt header files to the new versions. I think, this looks
   nicer (and gives the change in configure a sense :)

   Revision 1.13  1998/08/26 18:37:48  neerfeld
   bug fix for loadInternal;  changed email address

   Revision 1.12  1998/08/22 20:02:41  kulow
   make kdecore have nicer output, when compiled with -Weffc++ :)

   Revision 1.11  1998/08/17 10:34:03  konold

   Martin K.: Fixed a typo

   Revision 1.10  1998/06/16 06:03:22  kalle
   Implemented copy constructors and assignment operators or disabled them

   Revision 1.9  1998/03/08 18:50:23  wuebben
   Bernd: fixed up the kiconloader class -- it was completely busted


*/


#ifndef KICONLOADER_H
#define KICONLOADER_H

#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

#include <qapplication.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qstrlist.h>
#include <qstring.h>

/**
	Icon loader with caching.

	Multiples loads of the same icons using this class will be cached,
	saving memory and loading time. 
	
	Icons are searched for according to the KDE file system standard.
	Extra directories can be added, see @ref insertDirectory.

	@author Christoph Neerfeld (chris@kde.org)
	@version $Id$
*/
class KIconLoader : public QObject
{
  Q_OBJECT
public:
  /**
  	Constructor.

	If you want to use another path in your application then write into
	your .my_application_rc:

<pre>
	 [MyApplication]
	 PixmapPath=/..../my_pixmap_path
</pre>

	and call KIconLoader( config, "MyApplication", "PixmapPath" ).

	@param config	Pointer to a KConfig object which will be searched
	 		for additional paths.
	@param group	Group to search for paths. Normally "KDE Setup" is used.
	@param key	Key to search for paths. Normally "IconPath" is used.
	
  */
  KIconLoader ( KConfig *conf, const QString &app_name, const QString &var_name );

  /** Constructor. Searches for path in [KDE Setup]/IconPath.  */
  KIconLoader();

  /** Destructor. */
  ~KIconLoader ();

  /** 
  	Load an icon from disk or cache.

	@param name	The name of the icon to load. Absolute pathnames are
	 		allowed.

  	@param w	The max width of the resulting pixmap. Larger icons
			are scaled down. The default is no maximum.
  	@param h	The max height of the resulting pixmap. Larger icons
			are scaled down. The default is no maximum.
	@param canReturnNull	If this is false, this function will return
		the "unknown.xpm" icon if the requested icon is not found.
		The default is to return null.

	@return	The loaded icon.
  */
  QPixmap loadIcon( const QString &name, int w = 0, int h = 0, 
  		bool canReturnNull = true );


  /** 
  	Load an icon from disk without cache.

	This is useful if the icon has changed on the filesystem and
	you want to be sure that you get the new version, not the old
	one from the cache.

	@see loadIcon
  */
  QPixmap reloadIcon( const QString &name, int w = 0, int h = 0);
  
  /** 
  	Load a mini icon from disk or cache.
	Like loadIcon, but looks for "mini/name" first.

	@see loadIcon
  */
  QPixmap loadMiniIcon( const QString &name , int w = 0, int h = 0 );

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
   * @see loadIcon
   */
  QPixmap loadApplicationIcon( const QString &name, int w = 0, int h = 0 );

  /**
  	Similar to loadMiniIcon, but searches for a mini icon.

	@see loadMiniIcon, loadApplicationIcon
  */
  QPixmap loadApplicationMiniIcon( const QString &name, int w = 0, int h = 0 );


  /** 
  	Insert a directory into icon search path.
	Note that the default searchpath looks like this:

	@li $HOME/.kde/share/apps/<appName>/pics
	@li $KDEDIR/share/apps/<appName>/pics
	@li $HOME/.kde/share/apps/<appName>/toolbar
	@li $KDEDIR/share/apps/<appName>/toolbar

	@li $HOME/.kde/share/icons
	@li $HOME/.kde/share/toolbar

	@li $KDEDIR/share/icons
	@li $KDEDIR/share/toolbar

	@li list of directories in [KDE Setup]:IconPath=...

	 @param index	The index in the search path at which to insert
	 		the new directory.
	@param dir_name	The directory to insert into the search path.
	@return true on success, false on index out of range.

  */

  bool insertDirectory( int index, const QString &dir_name ) {
    return pixmap_dirs.insert( index, dir_name ); }
  QStrList* getDirList() { return &pixmap_dirs; }

  /** 
	Get the complete path for an icon name.

	@param name	The name of the icon to search for.
	@param always_valid If true, the function will return the path to
		unknown.xpm if the icon is not found. Note that it will
		return null if unknown.xpm was also not found.

	@return the physical path to the named icon.
  */
  QString getIconPath( const QString &name, 
  		bool always_valid=false);


  /** Remove an icon from the cache. */
  void flush( const QString &name ); 

protected:
  KConfig           *config;
  QStrList           name_list;
  QStrList           pixmap_dirs;
  QList<QPixmap>     pixmap_list;
  QPixmap loadInternal(const QString &name, int w = 0, int h = 0 );

private:
  void initPath();
  void addPath(QString path);

  // Disallow assignment and copy-construction
  KIconLoader( const KIconLoader& );
  KIconLoader& operator= ( const KIconLoader& );

};

#endif // KICONLOADER_H




