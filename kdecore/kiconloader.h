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

   Multiple loads of the same icon using this class will be cached
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
   using @ref KStandardDirs. Look up the various methods for details how.
   
   All keys used in @ref QPixmapCache by this class have the "$kico_.." prefix.
   
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
     * @li Default - the global setting or Medium if it doesn't exist
     **/
    typedef enum { Small, Medium, Large, Default } Size;

  /**
   * Constructor. Adds some application specific paths to lookup
   * toolbar icons. These are below the application's data dir 
   * (@see KStandardDirs for details) and are namely @p pics/ and
   * @p toolbar/
   *
   * @param app_name specifies the name of the application to add
   * paths of. If the name is null (default) the name from 
   * @ref KGlobal::instance() is used.
   * 
   */
  KIconLoader ( const QString &app_name = QString::null );

  /**
   * Constructs an KIconLoader for a component stored in a shared library.
   * Objects constructed with this constructor access all instance related
   * data (search paths, application name) from the given library instead of
   * @ref KGlobal::instance().
   *
   */
  KIconLoader( const KInstance* library );
    
  /**
  	Load an icon from disk without cache.

	This is useful if the icon has changed on the filesystem and
	you want to be sure that you get the new version, not the old
	one from the cache. The returned pixmap will be inserted in
	the cache so you use loadIcon after that.

	@see  loadIcon()
  */
  QPixmap reloadIcon( const QString& name);

  /**
   * This method should be used for loading most non-toolbar
   * icons (it @bf will load toolbar icons, but the recommended
   * way is to use @ref BarIcon).
   *
   * @param name Icon name without extension. An example is @p "konqueror" or
   *             @p "mimetypes/postscript"
   * @param size The prefered size to load. 
   * @param path_store This output parameter will contain the full path to the icon
   *                if not 0.
   * @param canReturnNull If this is @p false, this function will return
		the "unknown" icon if the requested icon is not found.
		The default is to return @p null.
   */
    QPixmap loadIcon( const QString& name, Size size = Default,
                      QString *path_store = 0, bool can_return_null = true );

  /**
     Get the complete path for a toolbar icon name.
     
     @param name	The name of the icon to search for.
     @param always_valid If true, the function will return the path to
                 "unknown" if the icon is not found. Note that it will
		 return null if "unknown" was also not found.
     
     @return The physical path to the named icon.
  */
  QString iconPath( const QString& name,
		    bool always_valid=false);


  /**
   * Sets the @see KStandardDirs type of icons  loadIcon() will load. 
   * The default is "toolbar".
   **/
  void setIconType(const char *type) { iconType = type; }

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
  QCString iconType;
  
private:
  // adds toolbar paths to the KStandardDirs object of the instance
  void initPath();

  // @internal Disallow assignment and copy-construction
  KIconLoader( const KIconLoader& );
  // @internal Disallow assignment and copy-construction
  KIconLoader& operator= ( const KIconLoader& );

  Size defaultSize;
};

/**
 * Helper function to load toolbar icons with. It simply returns
 * library->iconLoader()->loadIcon("toolbar/" + pixmap)
 * so @see loadIcon
 */
QPixmap BarIcon(const QString& pixmap, 
		const KInstance* library = KGlobal::instance());

/**
 * Helper function to load toolbar icons with a very specific size. It
 * simply returns
 * library->iconLoader()->loadIcon("toolbar/" + pixmap, size)
 * so @see loadIcon
 */
QPixmap BarIcon(const QString& pixmap, KIconLoader::Size size,
		const KInstance* library = KGlobal::instance());

/**
 * Helper function to load application icons with. It simply returns
 * library->iconLoader()->loadIcon("apps/" + pixmap)
 * so @see loadIcon
 */
QPixmap AppIcon(const QString& pixmap,
		const KInstance* library = KGlobal::instance());

/**
 * Helper function to load mimetype icons with. It simply returns
 * library->iconLoader()->loadIcon("mimetypes/" + pixmap)
 * so @see loadIcon
 */
QPixmap MimeIcon(const QString& pixmap,
		 const KInstance* library = KGlobal::instance());

/**
 * Helper function to load device icons with. It simply returns
 * library->iconLoader()->loadIcon("devices/" + pixmap)
 * so @see loadIcon
 */
QPixmap DevIcon(const QString& pixmap,
		const KInstance* library = KGlobal::instance());

/**
 * Helper function to load filesystem icons with. It simply returns
 * library->iconLoader()->loadIcon("filesystems/" + pixmap)
 * so @see loadIcon
 */
QPixmap FileIcon(const QString& pixmap,
		 const KInstance* library = KGlobal::instance());

/**
 * Helper function to load iconlist icons with. It simply returns
 * library->iconLoader()->loadIcon("listitems/" + pixmap)
 * so @see loadIcon
 */
QPixmap ListIcon(const QString& pixmap ,
		 const KInstance* library = KGlobal::instance());

#endif // KICONLOADER_H




