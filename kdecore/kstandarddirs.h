/*
  This file is part of the KDE libraries
  Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
                     Stephan Kulow <coolo@kde.org>
		     Waldo Bastian <bastian@kde.org>

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

#ifndef SSK_KSTDDIRS_H
#define SSK_KSTDDIRS_H

#include <qstring.h>
#include <qdict.h>
#include <qstringlist.h>
#include <kglobal.h>

class KConfig;
class KStandardDirsPrivate;

/**
 * * @short Site-independent access to standard KDE directories.
* @author Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
* @version $Id$
*
* This is one of the most central classes in kdelibs as
* it provides a basic service: It knows where the files
* reside on the user's hard disk. And it's meant to be the
* only one that knows -- to make the real location as
* transparent as possible to both the user and the applications.
*
* To this end it insulates the application from all information
* and applications always refer to a file with a resource type
* (e.g. icon) and a filename (e.g. khexdit.xpm). In an ideal world
* the application would make no assumption where this file is and
* leave it up to @ref KStandardDirs::findResource("apps", "Home.desktop")
* to apply this knowledge to return /opt/kde/share/applnk/Home.desktop
* or ::locate("data", "kgame/background.jpg") to return
* /opt/kde/share/apps/kgame/background.jpg
*
* The main idea behind KStandardDirs is that there are several
* toplevel prefixes below which the files lie. One of these prefixes is
* the one where the user installed kdelibs, one is where the
* application was installed, and one is $HOME/.kde, but there
* may be even more. Under these prefixes there are several well
* defined suffixes where specific resource types are to be found.
* For example, for the resource type "html" the suffixes could be
* share/doc/HTML and share/doc/kde/HTML.
* So the search algorithm basicly appends to each prefix each registered
* suffix and tries to locate the file there.
* To make the thing even more complex, it's also possible to register
* absolute paths that KStandardDirs looks up after not finding anything
* in the former steps. They can be useful if the user wants to provide
* specific directories that aren't in his $HOME/.kde directory for,
* for example, icons.
*
* @sect Standard resources that kdelibs allocates are:
*
* @li apps - Applications menu (.desktop files).
* @li cache - Cached information (e.g. favicons, web-pages)
* @li cgi - CGIs to run from kdehelp.
* @li config - Configuration files.
* @li data - Where applications store data.
* @li exe - Executables in $prefix/bin. @ref #findExe for a function that takes $PATH into account.
* @li html - HTML documentation.
* @li icon - Icons, see @ref KIconLoader.
* @li lib - Libraries.
* @li locale - Translation files for @ref KLocale.
* @li mime - Mime types.
* @li module - Module (dynamically loaded library).
* @li qtplugins - Qt plugins (dynamically loaded objects for Qt)
* @li services - Services.
* @li servicetypes - Service types.
* @li scripts - Application scripting additions.
* @li sound - Application sounds.
* @li templates - Templates
* @li wallpaper - Wallpapers.
* @li tmp - Temporary files (specfic for both current host and current user)
* @li socket - UNIX Sockets (specific for both current host and current user)
*
* A type that is added by the class @ref KApplication if you use it, is
* appdata. This one makes the use of the type data a bit easier as it
* appends the name of the application.
* So while you had to ::locate("data", "appname/filename") so you can
* also write ::locate("appdata", "filename") if your KApplication instance
* is called "appname" (as set via KApplication's constructor or KAboutData, if
* you use the global KStandardDirs object @ref KGlobal::dirs()).
* Please note though that you cannot use the "appdata"
* type if you intend to use it in an applet for Kicker because 'appname' would
* be "Kicker" instead of the applet's name. Therefore, for applets, you've got
* to work around this by using ::locate("data", "appletname/filename").
*
* @sect KStandardDirs supports the following environment variables:
*
* @li KDEDIRS: This may set an additional number of directory prefixes to
*          search for resources. The directories should be seperated
*          by ':'. The directories are searched in the order they are
*          specified.
* @li KDEDIR:  Used for backwards compatibility. As KDEDIRS but only a single
*          directory may be specified. If KDEDIRS is set KDEDIR is
*          ignored.
* @li KDEHOME: The directory where changes are saved to. This directory is
*          used to search for resources first. If KDEHOME is not
*          specified it defaults to "$HOME/.kde"
* @li KDEROOTHOME: Like KDEHOME, but used for the root user.
*          If KDEROOTHOME is not set it defaults to the .kde directory in the
*          home directory of root, usually "/root/.kde".
*          Note that the setting of $HOME is ignored in this case.
*
**/
class KStandardDirs
{
public:
        /**
	 * KStandardDirs constructor. It just initializes the caches.
	 **/
	KStandardDirs( );

	/**
	 * KStandardDirs destructor.
	 */
	virtual ~KStandardDirs();

	/**
	 * Adds another search dir to front of the @p fsstnd list.
	 *
	 * @li When compiling kdelibs, the prefix is added to this.
	 * @li KDEDIRS or KDEDIR is taking into account
	 * @li Additional dirs may be loaded from kdeglobals.
	 *
	 * @param dir The directory to append relative paths to.
	 */
	void addPrefix( const QString& dir );

	/**
	 * Adds suffixes for types.
	 *
	 * You may add as many as you need, but it is advised that there
	 * is exactly one to make writing definite.
	 * All basic types (@ref kde_default) are added by @ref addKDEDefaults(),
	 * but for those you can add more relative paths as well.
	 *
	 * The later a suffix is added, the higher its priority. Note, that the
	 * suffix should end with / but doesn't have to start with one (as prefixes
	 * should end with one). So adding a suffix for app_pics would look
	 * like KGlobal::dirs()->addResourceType("app_pics", "share/app/pics");
	 *
	 * @param type Specifies a short descriptive string to access
	 * files of this type.
	 * @param relativename Specifies a directory relative to the root
	 * of the KFSSTND.
	 */
	bool addResourceType( const char *type,
			      const QString& relativename );

	/**
	 * Adds absolute path at the end of the search path for
	 * particular types (for example in case of icons where
	 * the user specifies extra paths).
	 *
	 * You shouldn't need this
	 * function in 99% of all cases besides adding user-given
	 * paths.
	 *
	 * @param type Specifies a short descriptive string to access files
	 * of this type.
	 * @param absdir Points to directory where to look for this specific
	 * type. Non-existant directories may be saved but pruned.
	 *
	 */
	bool addResourceDir( const char *type,
			     const QString& absdir);

	/**
	 * Tries to find a resource in the following order:
	 * @li All PREFIX/<relativename> paths (most recent first).
	 * @li All absolute paths (most recent first).
	 *
	 * The filename should be a filename relative to the base dir
	 * for resources. So is a way to get the path to libkdecore.la
	 * to findResource("lib", "libkdecore.la"). KStandardDirs will
	 * then look into the subdir lib of all elements of all prefixes
	 * ($KDEDIRS) for a file libkdecore.la and return the path to
	 * the first one it finds (e.g. /opt/kde/lib/libkdecore.la)
	 *
	 * @param type The type of the wanted resource
	 * @param filename A relative filename of the resource.
	 *
	 * @return A full path to the filename specified in the second
	 *         argument, or QString::null if not found.
	 */
	QString findResource( const char *type,
			      const QString& filename ) const;

        /**
         * Returns a number that identifies this version of the resource.
         * When a change is made to the resource this number will change.
         *
	 * @param type The type of the wanted resource
	 * @param filename A relative filename of the resource.
	 * @param deep If true, all resources are taken into account
	 *        otherwise only the one returned by findResource().
	 *
	 * @return A number identifying the current version of the
	 *          resource.
	 */
	Q_UINT32 calcResourceHash( const char *type,
			      const QString& filename, bool deep) const;

	/**
	 * Tries to find all directories whose names consist of the
	 * specified type and a relative path. So would
	 * findDirs("apps", "Settings") return
	 * @li /opt/kde/share/applnk/Settings/
	 * @li /home/joe/.kde/share/applnk/Settings/
	 *
	 * Note that it appends / to the end of the directories,
	 * so you can use this right away as directory names.
	 *
	 * @param type The type of the base directory.
	 * @param reldir Relative directory.
	 *
	 * @return A list of matching directories, or an empty
	 *         list if the resource specified is not found.
	 */
	QStringList findDirs( const char *type,
                              const QString& reldir ) const;

	/**
	 * Tries to find the directory the file is in.
	 * It works the same as @ref findResource(), but it doesn't
	 * return the filename but the name of the directory.
	 *
	 * This way the application can access a couple of files
	 * that have been installed into the same directory without
	 * having to look for each file.
	 *
	 * findResourceDir("lib", "libkdecore.la") would return the
	 * path of the subdir libkdecore.la is found first in
	 * (e.g. /opt/kde/lib/)
	 *
	 * @return The directory where the file specified in the second
	 *         argument is located, or QString::null if the type
	 *         of resource specified is unknown or the resource
	 *         cannot be found.
	 */
	QString findResourceDir( const char *type,
				 const QString& filename) const;


	/**
	 * Tries to find all resources with the specified type.
	 *
	 * The function will look into all specified directories
	 * and return all filenames in these directories.
	 *
	 * @param type The type of resource to locate directories for.
	 * @param filter Only accept filenames that fit to filter. The filter
	 *        may consist of an optional directory and a @ref QRegExp
	 *        wildcard expression. E.g. "images\*.jpg"
	 * @param recursive Specifies if the function should decend
	 *        into subdirectories.
	 * @param uniq If specified,  only return items which have
	 *        unique suffixes - suppressing duplicated filenames.
	 *
	 * @return A list of directories matching the resource specified,
	 *         or an empty list if the resource type is unknown.
	 */
	QStringList findAllResources( const char *type,
				       const QString& filter = QString::null,
				       bool recursive = false,
				       bool uniq = false) const;

	/**
	 * Tries to find all resources with the specified type.
	 *
	 * The function will look into all specified directories
	 * and return all filenames (full and relative paths) in
	 * these directories.
	 *
	 * @param type The type of resource to locate directories for.
	 * @param filter Only accept filenames that fit to filter. The filter
	 *        may consist of an optional directory and a @ref QRegExp
	 *        wildcard expression. E.g. "images\*.jpg"
	 * @param recursive Specifies if the function should decend
	 *        into subdirectories.
	 * @param uniq If specified,  only return items which have
	 *        unique suffixes.
	 * @param list Of relative paths for the given type.
	 * @param relPaths The list to store the relative paths into
	 *        These can be used later to ::locate() the file
	 *
	 * @return A list of directories matching the resource specified,
	 *         or an empty list if the resource type is unknown.
	 */
	QStringList findAllResources( const char *type,
				       const QString& filter,
				       bool recursive,
				       bool uniq,
				       QStringList &relPaths) const;

	/**
	 * Finds the executable in the system path.
	 *
	 * A valid executable must
	 * be a file and have its executable bit set.
	 *
	 * @see findAllExe()
	 * @param appname The name of the executable file for which to search.
	 * @param pathstr The path which will be searched. If this is
	 * 		null (default), the $PATH environment variable will
	 *		be searched.
	 * @param ignoreExecBit	If true, an existing file will be returned
	 *			even if its executable bit is not set.
	 *
	 * @return The path of the executable. If it was not found,
	 *         it will return QString::null.
	 */
	static QString findExe( const QString& appname,
				const QString& pathstr=QString::null,
				bool ignoreExecBit=false );

	/**
	 * Finds all occurences of an executable in the system path.
	 *
	 * @see	findExe()
	 *
	 * @param list	Will be filled with the pathnames of all the
	 *		executables found. Will be empty if the executable
	 *		was not found.
	 * @param appname	The name of the executable for which to
	 *	 		search.
	 * @param pathstr	The path list which will be searched. If this
	 *		is 0 (default), the $PATH environment variable will
	 *		be searched.
	 * @param ignoreExecBit If true, an existing file will be returned
	 *			even if its executable bit is not set.
	 *
	 * @return The number of executables found, 0 if none were found.
	 */
	static int findAllExe( QStringList& list, const QString& appname,
			       const QString& pathstr=QString::null,
			       bool ignoreExecBit=false );

	/**
	 * This function adds the defaults that are used by the current
	 * KDE version.
	 *
	 * It's a series of @ref addResourceTypes()
	 * and @ref addPrefix() calls.
	 * You normally wouldn't call this function because it's called
	 * for you from @ref KGlobal.
	 */
	void addKDEDefaults();

	/**
	 * Reads customized entries out of the given config object and add
	 * them via @ref addResourceDirs().
	 *
	 * @param config The object the entries are read from. This should
	 *        contain global config files
	 * @return true if new config paths have been added
	 * from @p config.
	 **/
	bool addCustomized(KConfig *config);

	/**
	 * This function is used internally by almost all other function as
	 * it serves and fills the directories cache.
         *
	 * @return The list of possible directories for the specified @p type.
	 * The function updates the cache if possible.  If the resource
	 * type specified is unknown, it will return an empty list.
         * Note, that the directories are assured to exist beside the save
         * location, which may not exist, but is returned anyway.
	 */
	QStringList resourceDirs(const char *type) const;

	/**
	 * This function will return a list of all the types that KStandardDirs
	 * supports.
	 *
	 * @return All types that KDE supports
	 */
	QStringList allTypes() const;

	/**
	 * Finds a location to save files into for the given type
	 * in the user's home directory.
	 *
	 * @param type The type of location to return.
	 * @param suffix A subdirectory name.
	 *             Makes it easier for you to create subdirectories.
	 *   You can't pass filenames here, you _have_ to pass
	 *       directory names only and add possible filename in
	 *       that directory yourself. A directory name always has a
 	 *       trailing slash ('/').
	 * @param create If set, saveLocation() will create the directories
	 *        needed (including those given by @p suffix).
	 *
	 * @return A path where resources of the specified type should be
	 *         saved, or QString::null if the resource type is unknown.
	 */
	 QString saveLocation(const char *type,
			      const QString& suffix = QString::null,
			      bool create = true) const;

        /**
         * Converts an absolute path to a path relative to a certain
         * resource.
         *
         * If "abs = ::locate(resource, rel)"
         * then "rel = relativeLocation(resource, abs)" and vice versa.
         *
         * @param type The type of resource.
         *
         * @param absPath An absolute path to make relative.
         *
         * @return A relative path relative to resource @p type that
         * will find @p absPath. If no such relative path exists, absPath
         * will be returned unchanged.
         */
         QString relativeLocation(const char *type, const QString &absPath);

	/**
	 * Recursively creates still-missing directories in the given path.
	 *
	 * The resulting permissions will depend on the current umask setting.
	 * permission = mode & ~umask.
	 *
	 * @param dir Absolute path of the directory to be made.
	 * @param mode Directory permissions.
	 */
	static bool makeDir(const QString& dir, int mode = 0755);

	/**
	 * This returns a default relative path for the standard KDE
	 * resource types. Below is a list of them so you get an idea
	 * of what this is all about.
	 *
	 * @li data - share/apps
	 * @li html - share/doc/HTML
	 * @li icon - share/icon
	 * @li config - share/config
	 * @li pixmap - share/pixmaps
	 * @li apps - share/applnk
	 * @li sound - share/sounds
	 * @li locale - share/locale
	 * @li services - share/services
	 * @li servicetypes - share/servicetypes
	 * @li mime - share/mimelnk
	 * @li wallpaper - share/wallpapers
	 * @li templates - share/templates
	 * @li exe - bin
	 * @li lib - lib
	 *
	 * @returns Static default for the specified resource.  You
	 *          should probably be using locate() or locateLocal()
	 *          instead.
	 * @see locate()
	 * @see locateLocal()
	 */
	static QString kde_default(const char *type);

	/**
	 * @internal (for use by sycoca only)
	 */
	QString kfsstnd_prefixes();

	/**
	 * @returns the toplevel directory in which KStandardDirs
	 * will store things. Most likely $HOME/.kde
	 *
	 * Don't use this function if you can use locateLocal
	 */
	QString localkdedir() const;

	/**
	 * checks for existence and accessability
	 * faster than creating a QFileInfo first
	 */
	static bool exists(const QString &fullPath);

 private:

	QStringList prefixes;

	// Directory dictionaries
	QDict<QStringList> absolutes;
	QDict<QStringList> relatives;

	mutable QDict<QStringList> dircache;
	mutable QDict<QString> savelocations;

	// Disallow assignment and copy-construction
	KStandardDirs( const KStandardDirs& );
	KStandardDirs& operator= ( const KStandardDirs& );

	bool addedCustoms;

	KStandardDirsPrivate *d;

	void checkConfig() const;
};

/**
 * \addtogroup locates Locate Functions
 *  @{
 * On The Usage Of 'locate' and 'locateLocal'
 *
 * Typical KDE applications use resource files in one out of
 * three ways:
 *
 * 1) A resource file is read but is never written. A system
 *    default is supplied but the user can override this
 *    default in his local .kde directory:
 *
 *    \code
 *    // Code example
 *    myFile = locate("appdata", "groups.lst");
 *    myData =  myReadGroups(myFile); // myFile may be null
 *    \endcode
 *
 * 2) A resource file is read and written. If the user has no
 *    local version of the file the system default is used.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    \code
 *    // Code example
 *    myFile = locate("appdata", "groups.lst")
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = locateLocal("appdata", "groups.lst");
 *    myWriteGroups(myFile, myData);
 *    \endcode
 *
 * 3) A resource file is read and written. No system default
 *    is used if the user has no local version of the file.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    \code
 *    // Code example
 *    myFile = locateLocal("appdata", "groups.lst");
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = locateLocal("appdata", "groups.lst");
 *    myWriteGroups(myFile, myData);
 *    \endcode
 **/

/*!
 * \relates KStandardDirs
 * This function is just for convenience. It simply calls
 *instance->dirs()->\link KStandardDirs::findResource() findResource\endlink(type, filename).
 **/
QString locate( const char *type, const QString& filename, const KInstance* instance = KGlobal::instance() );

/*!
 * \relates KStandardDirs
 * This function is much like locate. However it returns a
 * filename suitable for writing to. No check is made if the
 * specified filename actually exists. Missing directories
 * are created. If filename is only a directory, without a
 * specific file, filename must have a trailing slash.
 *
 **/
QString locateLocal( const char *type, const QString& filename, const KInstance* instance = KGlobal::instance() );

/*! @} */

#endif // SSK_KSTDDIRS_H
