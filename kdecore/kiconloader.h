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
#include <qobject.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qstring.h>

#define Icon(x) KGlobal::iconLoader()->loadIcon(x,0,0,false)
#define ICON(x) KGlobal::iconLoader()->loadIcon(x,0,0,false)

/**
	Icon loader with caching.

	Multiples loads of the same icons using this class will be cached
	using @ref QPixmapCache, saving memory and loading time. 
	
	Icons are searched for according to the KDE file system standard.
	Extra directories can be added, see @ref insertDirectory.

	All keys used in QPixmapCache by this class have the "$kico_.." prefix.

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
  KIconLoader ( KConfig *conf, const QString &app_name, 
		  const QString &var_name );

  /** Constructor. Searches for path in [KDE Setup]/IconPath.  */
  KIconLoader();

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
		the "unknown.xpm" icon if the requested icon is not found.
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
   * @see loadIcon
   */
  QPixmap loadApplicationIcon( const QString& name, int w = 0, int h = 0 );

  /**
  	Similar to loadMiniIcon, but searches for a mini icon.

	@see loadMiniIcon, loadApplicationIcon
  */
  QPixmap loadApplicationMiniIcon( const QString& name, int w = 0, int h = 0 );


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

  bool insertDirectory( int index, const QString& dir_name );
  
  /**
   * Append a directory to the end of the search directory list.
   */
  bool appendDirectory( const QString& dir_name )
  		{ pixmap_dirs.append( dir_name ); return true; }


  QStringList getDirList() const { return pixmap_dirs; }

  /** 
	Get the complete path for an icon name.

	@param name	The name of the icon to search for.
	@param always_valid If true, the function will return the path to
		unknown.xpm if the icon is not found. Note that it will
		return null if unknown.xpm was also not found.

	@return the physical path to the named icon.
  */
  QString getIconPath( const QString& name, 
  		bool always_valid=false);


  /** 
   * Remove an icon from the cache. This is no longer required since @ref
   * QPixmapCache does this for us.
   * @deprecated
   */
  void flush( const QString &name ); 

protected:

  KConfig		*config;
  QStringList           pixmap_dirs;

  QPixmap loadInternal( const QString& name, int w = 0, int h = 0 );

private:
  void initPath();
  void addPath(QString path);

  // Disallow assignment and copy-construction
  KIconLoader( const KIconLoader& );
  KIconLoader& operator= ( const KIconLoader& );

};

#endif // KICONLOADER_H




