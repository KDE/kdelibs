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

  typedef enum { Small, Medium, Large } Size;

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

  */
  KIconLoader ( KConfig *conf, const QString &app_name);

  /**
   * Constructs an KIconLoader for a component stored in a shared library.
   * In this case KInstance has to be used instead of the @ref KGlobal.
   */
  KIconLoader( const KInstance* library );
    
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
  QPixmap loadIcon( const QString& name, bool canReturnNull = true );


  /**
  	Load an icon from disk without cache.

	This is useful if the icon has changed on the filesystem and
	you want to be sure that you get the new version, not the old
	one from the cache.

	@see loadIcon
  */
  QPixmap reloadIcon( const QString& name);

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
  QPixmap loadApplicationIcon( const QString& name, Size size = Medium);

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

protected:

  KConfig		*config;
  const KInstance             * library;
    
  /**
	honourcache will check if the icon is contained in the cache before
	trying to load it.
  */
  QPixmap loadInternal( const QString& name,
			bool honourcache = true );

  QString appname;
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




