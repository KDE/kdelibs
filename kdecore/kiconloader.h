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
   Revision 1.15.4.2  1999/03/09 15:46:29  dfaure
   Doc update (the path order was changed)

   Revision 1.15.4.1  1999/03/09 15:22:52  dfaure
   Moved path-list initialisation to a private initPath().
   Merged with Antonio's getIconPath(...) new method. (fixed missing .detach())
    loadInternal() uses it.
   Binary compatible. kfm will use it in a second :)

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

/// Load icons from disk
/**
   KIconLoader is a derived class from QObject.
   It supports loading of icons from disk. It puts the icon and its name
   into a QList and if you call loadIcon() for a second time, the icon is taken
   out of the list and not reread from disk.
   So you can call loadIcon() as many times as you wish and you don't have
   to take care about multiple copies of the icon in memory.
*/
class KIconLoader : public QObject
{
  Q_OBJECT
public:
  /// Constructor
  /**
	 config is the pointer to a KConfig object; 
	 normally the global KConfig object.
	 group is the name of a group in a config file.
	 key is the name of an entry within this group.
	 
	 Normaly group == "KDE Setup" and key == "IconPath"
	 Example for an entry in .kderc:
	 [KDE Setup]
	 IconPath=/usr/local/lib/kde/lib/pics:/usr/local/lib/kde/lib/pics/toolbar
	 
	 This gives KIconLoader the path to search the icons in.
	 
	 If you want to use another path in your application then write into
	 your .my_application_rc:
	 [MyApplication]
	 PixmapPath=/..../my_pixmap_path
	 and call KIconLoader( config, "MyApplication", "PixmapPath" ).
  */
  KIconLoader ( KConfig *conf, const QString &app_name, const QString &var_name );

  /**
	 There now exists a simple-to-use version of KIconLoader. If you
 	 create a KIconLoader without giving arguments, KIconLoader searches for 
	 the path in [KDE Setup]:IconPath=... as a default.
  */
  KIconLoader();

  /// Destructor
  ~KIconLoader ();

  /// Load an icon from disk
  /**
	 This function searches for the icon called name 
	 and returns a QPixmap object
	 of this icon if it was found and 0 otherwise.
	 If name starts with "/..." loadIcon treats it as an absolut pathname.
	 LoadIcon() creates a list of all loaded icons, 
	 so calling loadIcon() a second time
	 with the same name argument won't load the icon again, but gets it out of
	 its cache. By this you don't have to worry about multiple copies
	 of one and the same icon in memory, and you can call loadIcon() 
	 as often as you like.

         If the icon is larger then the specified size, it is 
         scaled down automatically. If the specified size is 
         0, the icon is not scaled at all.

  */
  QPixmap loadIcon( const QString &name, int w = 0, int h = 0 );


  /// Load an icon from disk without cache
  /**
      Same like loadIcon, except that cached icons will be reloaded.
      This is useful if the icon has changed on the filesystem and you want to be
      sure that you get the new version, not the old one from the cache.
  */
  QPixmap reloadIcon( const QString &name, int w = 0, int h = 0);
  
  /// Load an mini icon from disk
  /**
	 Same like loadIcon, but looks for "mini/name" first.
  */
  QPixmap loadMiniIcon( const QString &name , int w = 0, int h = 0 );

  /* 
   * The loadApplication-Icon functions are similar to the 
   * usual loadIcon functions except that they look in
   * kdedir()/share/icon first.
   *
   * These function should be used if you are loading the
   * application icons. Normally KApplication does this for
   * you, but special programs like kpanel or kmenuedit
   * need to load the application icons of foreign applications.
   */
  QPixmap loadApplicationIcon( const QString &name, int w = 0, int h = 0 );
  QPixmap loadApplicationMiniIcon( const QString &name, int w = 0, int h = 0 );


  /// Insert directory into searchpath
  /**
         This functions inserts a new directory into the searchpath at 
	 position index.
	 It returns TRUE if successful, or FALSE if index is out of range.
	 Note that the default searchpath looks like this:

	       1: $HOME/.kde/share/apps/<appName>/pics
	       2: $KDEDIR/share/apps/<appName>/pics
	       3: $HOME/.kde/share/apps/<appName>/toolbar
	       4: $KDEDIR/share/apps/<appName>/toolbar

	       5: $HOME/.kde/share/icons
	       6: $HOME/.kde/share/toolbar

	       7: $KDEDIR/share/icons
	       8: $KDEDIR/share/toolbar

	     9-x: list of directories in [KDE Setup]:IconPath=...

  */

  bool insertDirectory( int index, const QString &dir_name ) {
    return pixmap_dirs.insert( index, dir_name ); }
  QStrList* getDirList() { return &pixmap_dirs; }

  /// Get the complete path for an icon filename
  /**
      Set always_valid to true if you want this function to return a valid
      pixmap is your wishes cannot be satisfied (Be aware, that if unknown.xpm
      is not found you will receive a null string)
  */
  QString getIconPath( const QString &name, bool always_valid=false);


  /// Flush cache
  /**
      Remove an icon from the cache given it's name
  */
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




