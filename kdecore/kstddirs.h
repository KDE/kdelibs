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
#include <qlist.h>
#include <qstringlist.h>

class KConfig;

/**
* @short Site-independent access to standard KDE directories.
* @author Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
* @version $Id$
*
* This is one of the most central classes in kdelibs as
* it provides a basic service: it knows where the files
* reside on the user's harddisc. And it's meant to be the
* only one that knows - to make the real location as 
* transparent as possible to both the user and the applications.
* 
* For this it encapsulates all informations from the application
* and applications always refer to a file with a resource type
* (e.g. icon) and a filename (e.g. khexdit.xpm). In an ideal world
* the application would make no assumption where this file is and
* leaves it up to KStandardDirs::Resource("icon", "khexdit.xpm")
* to apply this knowledge.
*
* The main idea behind KStandardDirs is that there are several
* toplevel prefixes where files are below. One of this prefixes is
* the one where the user installed kdelibs into, one where the 
* application has installed to and one is $HOME/.kde, but there
* may be even more. Under these prefixes there are several well
* defined suffixes where specific resource types are to be found.
* For example for icon that is share/icons and share/apps/<appname>/icons,
* but also share/icons/large if the user prefers large icons.
* So the search algorithm basicly appends to each prefix each registered
* suffix and tries to locate the file there.
* To make the thing even more complex, it's also possible to register
* absolute paths that KStandardDirs looks up after not finding anything
* in the former steps. They can be useful if the user wants to provide
* specific directories that aren't in his $HOME/.kde directory for 
* example for icons.
*
* standard resources that kdelibs allocates are:
*
* @li appdata - application specific data dir (if created after KApplication)
* @li apps - applications menu (.desktop files)
* @li cgi - CGIs to run from kdehelp
* @li config - configuration files
* @li data - where applications store data
* @li exe - executables in $prefix/bin. @ref #findExe for a function that takes $PATH into account 
* @li html - HTML documentation 
* @li icon - icons
* @li lib - libraries
* @li locale - translation files for KLocale
* @li mime - mime types
* @li mini - miniature icons
* @li services - services
* @li servicetypes - service types
* @li sound - application sounds
* @li toolbar - toolbar pictures
* @li wallpaper - wallpapers
* 
*/
class KStandardDirs
{
public:
        /**
	 * KStandardDirs Constructor. It just initializes the cache
	 */
	KStandardDirs( );

	/**
	 * KStandardDirs Destructor.
	 */
	virtual ~KStandardDirs();

	/*
	 * This adds another search dir to front of the fsstnd list.
	 * @li when compiling kdelibs, the prefix is added to this.
	 * @li when compiling a separate app, kapp adds the new prefix
	 * @li additional dirs may be loaded from kdeglobals
	 *
	 * @param dir the directory to append relative paths to
	 */
	void addPrefix( QString dir );


	/*
	 * with this we add suffixes for types. You may add as many as
	 * you need, but it is advised that there is exactly one to make
	 * writing definite.
	 * all basic types (@see kde_default) are added by addKDEDefaults,
	 * but for those you can add more relative paths as well.
	 *
	 * The later a suffix is added, the higher its priority.
	 *
	 * @param type specifies a short descriptive string to access
	 * files of this type.
	 * @param relativename specifies a directory relative to the root
	 * of the KFSSTND
	 */
	bool addResourceType( const QString& type,
			      const QString& relativename );


	/**
	 * Adds absolute path at the end of the search path for
	 * particular types for example in case of icons where
	 * the user specifies extra paths. You shouldn't need this
	 * function in 99% of all cases besides adding user given
	 * paths.
	 * 
	 * @param type specifies a short descriptive string to access files 
	 * of this type.
	 * @param absdir points to directory where to look for this specific
	 * type. Non-existant directories may be saved but pruned.
	 * 
	 */
	bool addResourceDir( const QString& type, 
			     const QString& absdir);

	/**
	 * Tries to find resource in the following order:
	 * @li all PREFIX/<relativename> paths (most recent first)
	 * @li all hard paths (most recent first)
	 *
	 * @return a full path to the filename specified in the second
	 *         argument, or QString::null if not found.
	 */
	QString findResource( const QString& type, 
			      const QString& filename ) const;

	/**
	 * Tries to find all directories whose name consists of the
	 * specified type and a relative path.
	 *
	 * @param type the type of the base directory.
	 * @param reldir relative directory.
	 *
	 * @return a list of matching directories, or an empty
	 *         list if the resource specified is not found.
	 */
	QStringList findDirs( const QString& type, 
                              const QString& reldir ) const;

	/**
	 * Tries to find the directory the file is in.
	 * It works the same as findResource, but it doesn't
	 * return the filename but the name of the directory.
	 * This way the application can access a couple of files
	 * that have been installed into the same directory without
	 * having to look for each file.
	 *
	 * @return the directory where the file specified in the second
	 *         argument is located, or QString::null if the type
	 *         of resource specified is unknown or the resource
	 *         cannot be found.
	 */
	QString findResourceDir( const QString& type,
				 const QString& filename) const;


	/**
	 * Tries to find all resources with the specified type.
	 * The function will look into all specified directories
	 * and returns all filenames in these directories.
	 *
	 * @param type the type of resource to locate directories for.
	 * @param filter only accept filenames that fit to filter. The filter
	 *        may consist of an optional directory and a QRexExp 
	 *        wildcard expression. E.g. "images\*.jpg" 
	 * @param recursive specifies if the function should decend
	 *        into subdirectories.
	 * @param uniq if specified,  only return items which have 
	 *        unique suffixes.
	 *        
	 * @return a list of directories matching the resource specified,
	 *         or an empty list if the resource type is unknown.
	 */
	QStringList findAllResources( const QString& type, 
				       const QString& filter = QString::null,
				       bool recursive = false,
				       bool uniq = false) const;

	QStringList findAllResources( const QString& type, 
				       const QString& filter,
				       bool recursive,
				       bool uniq,
				       QStringList &relPaths) const;

	/** 
	 * Finds the executable in the system path. A valid executable must
	 * be a file and have its executable bit set.
	 *
	 * @see #findAllExe
	 * @param appname the name of the executable file for which to search.
	 * @param pathstr The path which will be searched. If this is 
	 * 		0 (default), the $PATH environment variable will 
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
	 * @see	#findExe
	 *
	 * @param list	will be filled with the pathnames of all the
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
	 * KDE version. It's a serie of addResourceTypes and addPrefix calls.
	 * You normally wouldn't call this function, it's called for you from
	 * KGlobal.
	 */
	void addKDEDefaults();

	/**
	 * Reads customized entries out of the given config object and adds
	 * them via addResourceDirs
	 * 
	 * @param config the object the entries are read from. This should
	 *        contain global config files
	 * @return if new config paths have been added that would reflect
	 * to config
	 **/
	bool addCustomized(KConfig *config);

	/**
	 * @return the list of possible directories for the type 
	 * The functions updates the cache if possible.  If the resource
	 * type specified is unknown, it will return an empty list.
	 */
	QStringList getResourceDirs(const QString& type) const;

	/**
	 * Finds a location to save files into for the given type
	 * in the user's home directory.
	 * 
	 * @param suffix makes it easier for you to create subdirectories.
		       You can't pass filenames here, you _have_ to pass
		       directory names only and add possible filename in
		       that directory yourself
	 * @param create if set, getSaveLocation will create the directories
	 *        needed (including those given by suffix)
	 *
	 * @return a path where resources of the specified type should be
	 *         saved, or QString::null if the resource type is unknown.
	 */
	 QString getSaveLocation(const QString& type,
				 const QString& suffix = QString::null, 
				 bool create = true) const;

	/**
	 * Recursively creates still missing directories in the given path.
	 * The resulting permissions will depend on the current umask setting.
	 * permission = mode & ~umask.
	 *
	 * @param dir absolute path of the directory to be made
	 * @param mode directory permissions
	 */
	static bool makeDir(const QString& dir, int mode = 0755);

	/**
	 * @returns static default for the specified resource.  You
	 *          should probably be using locate or locateLocal
	 *          instead.
	 * @ see #locate, #locateLocal
	 */
	static QString kde_default(const QString& type);

 private:

	QString localkdedir() const;

	QStringList prefixes;

	// Directory dictionaries
	QDict<QStringList> absolutes;
	QDict<QStringList> relatives;
	
	mutable QDict<QStringList> dircache;

	// Disallow assignment and copy-construction
	KStandardDirs( const KStandardDirs& );
	KStandardDirs& operator= ( const KStandardDirs& );

	bool addedCustoms;
};

/**
 * On The Usage Of 'locate' and 'locateLocal'
 *
 * Typical KDE applications use resource files in one out of
 * three ways:
 *
 * 1) A resource file is read but is never written. A system
 *    default is supplied but the user can override this
 *    default in his local .kde directory:
 *   
 *    // Code example
 *    myFile = locate("appdata", "groups.lst")
 *    myData =  myReadGroups(myFile);
 *
 * 2) A resource file is read and written. If the user has no
 *    local version of the file the system default is used.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    // Code example
 *    myFile = locate("appdata", "groups.lst")
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = locateLocal("appdata", "groups.lst")
 *    myWriteGroups(myFile, myData);
 *
 * 3) A resource file is read and written. No system default
 *    is used if the user has no local version of the file.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    // Code example
 *    myFile = locateLocal("appdata", "groups.lst")
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = locateLocal("appdata", "groups.lst")
 *    myWriteGroups(myFile, myData);
 **/
 
/**
 * This function is just for convience. It simply calls 
 * KGlobal::dirs()->findResource(type, filename)
 **/
QString locate( const QString& type, const QString& filename );

/**
 * This function is much like locate. However it returns a
 * filename suitable for writing to. No check is made if the
 * specified filename actually exists. Missing directories
 * are created. If filename is only a directory, without a
 * specific file, filename must have a trailing slash. 
 *
 **/
QString locateLocal( const QString& type, const QString& filename );

#endif // SSK_KSTDDIRS_H
