/* -*- c++ -*-
   $Id$

   This file is part of the KDE libraries
   Copyright (C) 1997 Christoph Neerfeld (chris@kde.org)
             (C) 1999 Stephan Kulow (coolo@kde.org)

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
   
   Within KDE there are two distinct groups of Icons:
   
   @li toolbar - Toolbar icons are small icons used on pushbuttons.
   The size is 22x22 pixels.
   
   @li icon - These are icons used to identify an application,
   a file type or a directory. They are typically shown
   on the desktop and in directory listings. Their
   size varies between 16x16 (Small), 32x32 (Medium)
   and 64x64 (Large). 
   
   Icons are searched for according to the KDE file system standard
   using KStandardDirs. Look up the various methods for details how.
   
   All keys used in QPixmapCache by this class have the "$kico_.." prefix.
   
   @author Christoph Neerfeld (chris@kde.org) and Stephan Kulow (coolo@kde.org)
   @version $Id$
*/
class KIconLoader 
{

public:
    
    /**
     * Specifies the size of the requested icon:
     * @li Small  - 16x16 pixels
     * @li Medium - 32x32 pixels
     * @li Large  - 64x64 pixels
     **/
    typedef enum { Small, Medium, Large } Size;

  /**
   * Constructor. Adds some application specific paths to lookup
   * toolbar icons. These are below the application's data dir 
   * (@see KStandardDirs for details) and are namely pics/ and
   * toolbar/
   *
   * @param app_name specifies the name of the application to add
   * paths of. If the name is null (default) the name from 
   * KGlobal::instance() is used.
   * 
   */
  KIconLoader ( const QString &app_name = QString::null );

  /**
   * Constructs an KIconLoader for a component stored in a shared library.
   * Objects constructed with this constructor access all instance related
   * data (search paths, application name) from the given library instead of
   * KGlobal::instance()
   *
   */
  KIconLoader( const KInstance* library );
    
  /**
     Load a toolbar icon from disk or cache. Use this function to load
     toolbar icons specific to your application. Don't use it to load
     application icons. 
     You shouldn't have to call this function on your own. Rather use
     @see BarIcon
     
     @param name	The name of the icon to load. Absolute pathnames are
                        allowed, but not recommended. An example of a name is
			"filenew"
     @param canReturnNull	If this is false, this function will return
		the "unknown" icon if the requested icon is not found.
		The default is to return null.

     @return	The loaded icon.
  */
  QPixmap loadIcon( const QString& name, bool canReturnNull = true );

  /**
  	Load an icon from disk without cache.

	This is useful if the icon has changed on the filesystem and
	you want to be sure that you get the new version, not the old
	one from the cache. The returned pixmap will be inserted in
	the cache so you use loadIcon after that.

	@see loadIcon
  */
  QPixmap reloadIcon( const QString& name);

  /**
   * The loadApplication-Icon is for loading applications icons (and more) 
   *
   * @param name icon name without extension. An example is "konqueror" or
   *             "mimetypes/postscript"
   * @param the prefered size to load. 
   * @param path_store output parameter, will contain the full path to the icon
   *                if not 0
   * @param canReturnNull If this is false, this function will return
		the "unknown" icon if the requested icon is not found.
		The default is to return null.
   * @see loadIcon
   */
  QPixmap loadApplicationIcon( const QString& name, Size size = Medium,
                               QString *path_store = 0, 
			       bool canReturnNull = true);

  /**
     Get the complete path for a toolbar icon name.
     
     @param name	The name of the icon to search for.
     @param always_valid If true, the function will return the path to
                 "unknown" if the icon is not found. Note that it will
		 return null if "unknown" was also not found.
     
     @return the physical path to the named icon.
  */
  QString iconPath( const QString& name,
		    bool always_valid=false);


  /**
   * sets the @see KStandardDirs type of icons loadIcon will load. 
   * Default is "toolbar"
   **/
  void setIconType(const QString &type) { iconType = type; }

protected:

  // the instance to get the KStandardDirs object from to load the icons
  const KInstance  * library;
    
  /**
     honourcache will check if the icon is contained in the cache before
     trying to load it. Used by loadIcon and reloadIcon (with different
     honourcache parameter)
  */
  QPixmap loadInternal( const QString& name,
			bool honourcache = true );

  // the application name - by default the one of KGlobal::instance()
  QString appname;

  // the icon type to load in loadIcon - by default "toolbar"
  QString iconType;
  
private:
  // adds toolbar paths to the KStandardDirs object of the instance
  void initPath();

  // @internal Disallow assignment and copy-construction
  KIconLoader( const KIconLoader& );
  // @internal Disallow assignment and copy-construction
  KIconLoader& operator= ( const KIconLoader& );

};

/**
 * Helper function to load toolbar icons with. It simply returns
 * library->iconLoader()->loadIcon(pixmap)
 * so @see loadIcon
 */
QPixmap BarIcon(const QString& pixmap, 
		const KInstance* library = KGlobal::instance());

#endif // KICONLOADER_H
