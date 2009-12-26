/*
  This file is part of the KDE libraries
  Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
  Copyright (C) 1999 Stephan Kulow <coolo@kde.org>
  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KSTANDARDDIRS_H
#define KSTANDARDDIRS_H

#include <QtCore/QStringList>
#include <kglobal.h>
#include <QtCore/QMap>

class KConfig;

/**
 * @short Site-independent access to standard KDE directories.
 * @author Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 *
 * This is one of the most central classes in kdelibs as
 * it provides a basic service: It knows where the files
 * reside on the user's hard disk. And it's meant to be the
 * only one that knows -- to make the real location as
 * transparent as possible to both the user and the applications.
 *
 * To this end it insulates the application from all information
 * and applications always refer to a file with a resource type
 * (e.g. icon) and a filename (e.g. <tt>khexdit.xpm</tt>). In an ideal world
 * the application would make no assumption where this file is and
 * leave it up to KStandardDirs::findResource("apps", "Home.desktop")
 * to apply this knowledge to return <tt>/opt/kde/share/applnk/Home.desktop</tt>
 * or ::locate("data", "kgame/background.jpg") to return
 * <tt>/opt/kde/share/apps/kgame/background.jpg</tt>
 *
 * The main idea behind KStandardDirs is that there are several
 * toplevel prefixes below which the files lie. One of these prefixes is
 * the one where the user installed kdelibs, one is where the
 * application was installed, and one is <tt>$HOME/.kde</tt>, but there
 * may be even more. Under these prefixes there are several well
 * defined suffixes where specific resource types are to be found.
 * For example, for the resource type @c "html" the suffixes could be
 * @c share/doc/HTML and @c share/doc/kde/HTML.
 * So the search algorithm basically appends to each prefix each registered
 * suffix and tries to locate the file there.
 * To make the thing even more complex, it's also possible to register
 * absolute paths that KStandardDirs looks up after not finding anything
 * in the former steps. They can be useful if the user wants to provide
 * specific directories that aren't in his <tt>$HOME/.kde</tt> directory for,
 * for example, icons.
 *
 * <b>Standard resources that kdelibs allocates are:</b>\n
 *
 * @li @c apps - Applications menu (.desktop files).
 * @li @c autostart - Autostart directories (both XDG and kde-specific)
 * @li @c cache - Cached information (e.g. favicons, web-pages)
 * @li @c cgi - CGIs to run from kdehelp.
 * @li @c config - Configuration files.
 * @li @c data - Where applications store data.
 * @li @c emoticons - Emoticons themes
 * @li @c exe - Executables in $prefix/bin. findExe() for a function that takes $PATH into account.
 * @li @c html - HTML documentation.
 * @li @c icon - Icons, see KIconLoader.
 * @li @c kcfg - KConfigXT config files.
 * @li @c lib - Libraries.
 * @li @c locale - Translation files for KLocale.
 * @li @c mime - Mime types defined by KDE-specific .desktop files.
 * @li @c module - Module (dynamically loaded library).
 * @li @c qtplugins - Qt plugins (dynamically loaded objects for Qt)
 * @li @c services - Services.
 * @li @c servicetypes - Service types.
 * @li @c sound - Application sounds.
 * @li @c templates - Templates for the "Create new file" functionality.
 * @li @c wallpaper - Wallpapers.
 * @li @c tmp - Temporary files (specific for both current host and current user)
 * @li @c socket - UNIX Sockets (specific for both current host and current user)
 * @li @c xdgconf-menu - Freedesktop.org standard location for menu layout (.menu) files.
 * @li @c xdgdata-apps - Freedesktop.org standard location for application desktop files.
 * @li @c xdgdata-dirs - Freedesktop.org standard location for menu descriptions (.directory files).
 * @li @c xdgdata-mime - Freedesktop.org standard location for MIME type definitions.
 * @li @c xdgdata-icon - Freedesktop.org standard location for icons.
 * @li @c xdgdata-pixmap - Gnome-compatibility location for pixmaps.
 *
 * A type that is added by the class KApplication if you use it, is
 * @c appdata. This one makes the use of the type data a bit easier as it
 * appends the name of the application.
 * So while you had to ::locate("data", "appname/filename") so you can
 * also write ::locate("appdata", "filename") if your KApplication instance
 * is called @c "appname" (as set via KApplication's constructor or KAboutData, if
 * you use the global KStandardDirs object KGlobal::dirs()).
 * Please note though that you cannot use the @c "appdata"
 * type if you intend to use it in an applet for Kicker because 'appname' would
 * be @c "Kicker" instead of the applet's name. Therefore, for applets, you've got
 * to work around this by using ::locate("data", "appletname/filename").
 *
 * <b>KStandardDirs supports the following environment variables:</b>
 *
 * @li @c KDEDIRS - This may set an additional number of directory prefixes to
 *          search for resources. The directories should be separated
 *          by <tt>':'</tt>. The directories are searched in the order they are
 *          specified.
 * @li @c KDEHOME - The directory where changes are saved to. This directory is
 *          used to search for resources first. If @c KDEHOME is not
 *          specified it defaults to @c "$HOME/.kde"
 * @li @c KDEROOTHOME - Like KDEHOME, but used for the root user.
 *          If @c KDEROOTHOME is not set it defaults to the <tt>.kde</tt> directory in the
 *          home directory of root, usually @c "/root/.kde".
 *          Note that the setting of @c $HOME is ignored in this case.
 *
 * @see KGlobalSettings
 *
 *
 * On The Usage Of 'locate' and 'locateLocal'
 *
 * Typical KDE applications use resource files in one out of
 * three ways:
 *
 * 1) A resource file is read but is never written. A system
 *    default is supplied but the user can override this
 *    default in his local .kde directory:
 *
 *    @code
 *    // Code example
 *    myFile = KStandardDirs::locate("appdata", "groups.lst");
 *    myData =  myReadGroups(myFile); // myFile may be null
 *    @endcode
 *
 * 2) A resource file is read and written. If the user has no
 *    local version of the file the system default is used.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    @code
 *    // Code example
 *    myFile = KStandardDirs::locate("appdata", "groups.lst")
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = KStandardDirs::locateLocal("appdata", "groups.lst");
 *    myWriteGroups(myFile, myData);
 *    @endcode
 *
 * 3) A resource file is read and written. No system default
 *    is used if the user has no local version of the file.
 *    The resource file is always written to the users local
 *    .kde directory.
 *
 *    @code
 *    // Code example
 *    myFile = KStandardDirs::locateLocal("appdata", "groups.lst");
 *    myData =  myReadGroups(myFile);
 *    ...
 *    doSomething(myData);
 *    ...
 *    myFile = KStandardDirs::locateLocal("appdata", "groups.lst");
 *    myWriteGroups(myFile, myData);
 *    @endcode
 **/
class KDECORE_EXPORT KStandardDirs
{
public:
    /**
     * KStandardDirs' constructor. It just initializes the caches.
     * Note that you should normally not call this, but use KGlobal::dirs()
     * instead, in order to reuse the same KStandardDirs object as much as possible.
     *
     * Creating other KStandardDirs instances can be useful in other threads.
     *
     * Thread safety note: using a shared KStandardDirs instance (such as KGlobal::dirs())
     * in multiple threads is thread-safe if you only call the readonly "lookup" methods
     * (findExe, resourceDirs, findDirs, findResourceDir, findAllResources, saveLocation,
     * relativeLocation). The methods that modify the object (all those starting with "add",
     * basically all non-const methods) are obviously not thread-safe; set things up
     * before creating threads.
     */
    KStandardDirs();

    enum SearchOption { NoSearchOptions = 0,
                        Recursive = 1,
                        NoDuplicates = 2,
                        IgnoreExecBit = 4 };
    Q_DECLARE_FLAGS( SearchOptions, SearchOption )

    /**
     * KStandardDirs' destructor.
     */
    virtual ~KStandardDirs();

    /**
     * Adds another search dir to front of the @p fsstnd list.
     *
     * @li When compiling kdelibs, the prefix is added to this.
     * @li @c KDEDIRS is taken into account
     * @li Additional dirs may be loaded from kdeglobals.
     *
     * @param dir The directory to append relative paths to.
     */
    void addPrefix( const QString& dir );

    /**
     * Adds another search dir to front of the @c XDG_CONFIG_XXX list
     * of prefixes.
     * This prefix is only used for resources that start with @c "xdgconf-"
     *
     * @param dir The directory to append relative paths to.
     */
    void addXdgConfigPrefix( const QString& dir );

    /**
     * Adds another search dir to front of the @c XDG_DATA_XXX list
     * of prefixes.
     * This prefix is only used for resources that start with @c "xdgdata-"
     *
     * @param dir The directory to append relative paths to.
     */
    void addXdgDataPrefix( const QString& dir );

    /**
     * Adds suffixes for types.
     *
     * You may add as many as you need, but it is advised that there
     * is exactly one to make writing definite.
     * All basic types are added by addKDEDefaults(),
     * but for those you can add more relative paths as well.
     *
     * The later a suffix is added, the higher its priority. Note, that the
     * suffix should end with / but doesn't have to start with one (as prefixes
     * should end with one). So adding a suffix for app_pics would look
     * like KGlobal::dirs()->addResourceType("app_pics", "data" ,"app/pics");
     *
     * @param type Specifies a short descriptive string to access
     * files of this type.
     * @param relativename Specifies a directory relative to the root
     * of the KFSSTND.
     * @param priority if true, the directory is added before any other,
     * otherwise after
     * @return true if successful, false otherwise.
     *
     * @deprecated
     */
    KDE_DEPRECATED bool addResourceType( const char *type,
                                         const QString& relativename, bool priority = true );

    /**
     * Adds suffixes for types.
     *
     * You may add as many as you need, but it is advised that there
     * is exactly one to make writing definite.
     * All basic types are added by addKDEDefaults(),
     * but for those you can add more relative paths as well.
     *
     * The later a suffix is added, the higher its priority. Note, that the
     * suffix should end with / but doesn't have to start with one (as prefixes
     * should end with one). So adding a suffix for app_pics would look
     * like KGlobal::dirs()->addResourceType("app_pics", "data", "app/pics");
     *
     * @param type Specifies a short descriptive string to access
     * files of this type.
     * @param basetype Specifies an already known type, or 0 if none
     * @param relativename Specifies a directory relative to the basetype
     * @param priority if true, the directory is added before any other,
     * otherwise after
     * @return true if successful, false otherwise.
     */
    bool addResourceType( const char *type, const char *basetype,
                          const QString& relativename, bool priority = true );

    /// @internal - just to avoid unwanted overload
    bool addResourceType( const char *type, const char *basetype,
                          const char* relativename, bool priority = true )
    {
        return addResourceType(type, basetype, QLatin1String(relativename), priority);
    }

    /**
     * Adds absolute path at the beginning of the search path for
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
     * type. Non-existent directories may be saved but pruned.
     * @param priority if true, the directory is added before any other,
     * otherwise after
     * @return true if successful, false otherwise.
     */
    bool addResourceDir( const char *type,
                         const QString& absdir, bool priority = true );

    /**
     * Tries to find a resource in the following order:
     * @li All PREFIX/\<relativename> paths (most recent first).
     * @li All absolute paths (most recent first).
     *
     * The filename should be a filename relative to the base dir
     * for resources. So is a way to get the path to libkdecore.la
     * to findResource("lib", "libkdecore.la"). KStandardDirs will
     * then look into the subdir lib of all elements of all prefixes
     * ($KDEDIRS) for a file libkdecore.la and return the path to
     * the first one it finds (e.g. /opt/kde/lib/libkdecore.la).
     * You can use the program kde4-config to list all resource types:
     * @code
     * $ kde4-config --types
     * @endcode
     *
     * Example:
     * @code
     * QString iconfilename=KGlobal::dirs()->findResource("icon",QString("oxygen/22x22/apps/ktip.png"));
     * @endcode
     * 
     * @param type The type of the wanted resource
     * @param filename A relative filename of the resource.
     *
     * @return A full path to the filename specified in the second
     *         argument, or QString() if not found.
     */
    QString findResource( const char *type,
                          const QString& filename ) const;

    /**
     * Checks whether a resource is restricted as part of the KIOSK
     * framework. When a resource is restricted it means that user-
     * specific files in the resource are ignored.
     *
     * E.g. by restricting the @c "wallpaper" resource, only system-wide
     * installed wallpapers will be found by this class. Wallpapers
     * installed under the @c $KDEHOME directory will be ignored.
     *
     * @param type The type of the resource to check
     * @param relPath A relative path in the resource.
     *
     * @return True if the resource is restricted.
     */
    bool isRestrictedResource( const char *type,
                               const QString& relPath=QString() ) const;

    /**
     * Returns a number that identifies this version of the resource.
     * When a change is made to the resource this number will change.
     *
     * @param type The type of the wanted resource
     * @param filename A relative filename of the resource.
     * @param options If the flags includes Recursive,
     *                all resources are taken into account
     *                otherwise only the one returned by findResource().
     *
     * @return A number identifying the current version of the
     *          resource.
     */
    quint32 calcResourceHash( const char *type,
                              const QString& filename,
                              SearchOptions options = NoSearchOptions) const;

    /**
     * Tries to find all directories whose names consist of the
     * specified type and a relative path. So
     * findDirs("apps", "Settings") would return
     * @li /home/joe/.kde/share/applnk/Settings/
     * @li /opt/kde/share/applnk/Settings/
     *
     * (from the most local to the most global)
     *
     * Note that it appends @c / to the end of the directories,
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
     * It works the same as findResource(), but it doesn't
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
     * @param type The type of the wanted resource
     * @param filename A relative filename of the resource.
     * @return The directory where the file specified in the second
     *         argument is located, or QString() if the type
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
     * The "most local" files are returned before the "more global" files.
     *
     * @param type The type of resource to locate directories for.
     * @param filter Only accept filenames that fit to filter. The filter
     *        may consist of an optional directory and a QRegExp
     *        wildcard expression. E.g. <tt>"images\*.jpg"</tt>.
     *        Use QString() if you do not want a filter.
     * @param options if the flags passed include Recursive, subdirectories
     *        will also be search; if NoDuplicates is passed then only entries with
     *        unique filenames will be returned eliminating duplicates.
     *
     * @return List of all the files whose filename matches the
     *         specified filter.
     */
    QStringList findAllResources( const char *type,
                                  const QString& filter = QString(),
                                  SearchOptions options = NoSearchOptions ) const;

    /**
     * Tries to find all resources with the specified type.
     *
     * The function will look into all specified directories
     * and return all filenames (full and relative paths) in
     * these directories.
     *
     * The "most local" files are returned before the "more global" files.
     *
     * @param type The type of resource to locate directories for. Can be icon,
     *        lib, pixmap, .... To get a complete list, call
     *        @code
     *        kde4-config --types
     *        @endcode
     * @param filter Only accept filenames that fit to filter. The filter
     *        may consist of an optional directory and a QRegExp
     *        wildcard expression. E.g. <tt>"images\*.jpg"</tt>.
     *        Use QString() if you do not want a filter.
     * @param options if the flags passed include Recursive, subdirectories
     *        will also be search; if NoDuplicates is passed then only entries with
     *        unique filenames will be returned eliminating duplicates.
     *
     * @param relPaths The list to store the relative paths into
     *        These can be used later to ::locate() the file
     *
     * @return List of all the files whose filename matches the
     *         specified filter.
     */
    QStringList findAllResources( const char *type,
                                  const QString& filter,
                                  SearchOptions options,
                                  QStringList &relPaths) const;

    /**
     * Returns a QStringList list of pathnames in the system path.
     *
     * @param pstr  The path which will be searched. If this is
     * 		null (default), the @c $PATH environment variable will
     *		be searched.
     *
     * @return a QStringList list of pathnames in the system path.
     */
    static QStringList systemPaths( const QString& pstr=QString() );

    /**
     * Finds the executable in the system path.
     *
     * A valid executable must
     * be a file and have its executable bit set.
     *
     * @param appname The name of the executable file for which to search.
     *                if this contains a path separator, it will be resolved
     *                according to the current working directory
     *                (shell-like behaviour).
     * @param pathstr The path which will be searched. If this is
     *                null (default), the @c $PATH environment variable will
     *                be searched.
     * @param options if the flags passed include IgnoreExecBit the path returned
     *                may not have the executable bit set.
     *
     * @return The path of the executable. If it was not found,
     *         it will return QString().
     * @see findAllExe()
     */
    static QString findExe( const QString& appname,
                            const QString& pathstr = QString(),
                            SearchOptions options = NoSearchOptions );

    /**
     * Finds all occurrences of an executable in the system path.
     *
     * @param list will be filled with the pathnames of all the
     *             executables found. Will be empty if the executable
     *             was not found.
     * @param appname the name of the executable for which to
     *                search.
     * @param pathstr the path list which will be searched. If this
     *                is 0 (default), the @c $PATH environment variable will
     *                be searched.
     * @param options if the flags passed include IgnoreExecBit the path returned
     *                may not have the executable bit set.
     *
     * @return The number of executables found, 0 if none were found.
     *
     * @see findExe()
     */
    static int findAllExe( QStringList& list, const QString& appname,
                           const QString& pathstr=QString(),
                           SearchOptions options = NoSearchOptions );

    /**
     * Reads customized entries out of the given config object and add
     * them via addResourceDirs().
     *
     * @param config The object the entries are read from. This should
     *        contain global config files
     * @return @c true if new config paths have been added
     * from @p config.
     **/
    bool addCustomized(KConfig *config);

    /**
     * This function is used internally by almost all other function as
     * it serves and fills the directories cache.
     *
     * @param type The type of resource
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
     *         saved, or QString() if the resource type is unknown.
     */
    QString saveLocation(const char *type,
                         const QString& suffix = QString(),
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
     * will find @p absPath. If no such relative path exists, @p absPath
     * will be returned unchanged.
     */
    QString relativeLocation(const char *type, const QString &absPath);

    /**
     * Recursively creates still-missing directories in the given path.
     *
     * The resulting permissions will depend on the current umask setting.
     * <tt>permission = mode & ~umask</tt>.
     *
     * @param dir Absolute path of the directory to be made.
     * @param mode Directory permissions.
     * @return true if successful, false otherwise
     */
    static bool makeDir(const QString& dir, int mode = 0755);

    /**
     * This returns a default relative path for the standard KDE
     * resource types. Below is a list of them so you get an idea
     * of what this is all about.
     *
     * @li @c data - @c share/apps
     * @li @c html - @c share/doc/HTML
     * @li @c icon - @c share/icon
     * @li @c config - @c share/config
     * @li @c pixmap - @c share/pixmaps
     * @li @c apps - @c share/applnk
     * @li @c sound - @c share/sounds
     * @li @c locale - @c share/locale
     * @li @c services - @c share/kde4/services
     * @li @c servicetypes - @c share/kde4/servicetypes
     * @li @c mime - @c share/mimelnk
     * @li @c cgi - @c cgi-bin
     * @li @c wallpaper - @c share/wallpapers
     * @li @c templates - @c share/templates
     * @li @c exe - @c bin
     * @li @c lib - @c lib[suffix]
     * @li @c module - @c lib[suffix]/kde4
     * @li @c qtplugins - @c lib[suffix]/kde4/plugins
     * @li @c kcfg - @c share/config.kcfg
     * @li @c emoticons - @c share/emoticons
     * @li @c xdgdata-apps - @c applications
     * @li @c xdgdata-icon - @c icons
     * @li @c xdgdata-pixmap - @c pixmaps
     * @li @c xdgdata-dirs - @c desktop-directories
     * @li @c xdgdata-mime - @c mime
     * @li @c xdgconf-menu - @c menus
     *
     * @returns Static default for the specified resource.  You
     *          should probably be using locate() or locateLocal()
     *          instead.
     * @see locate()
     * @see locateLocal()
     */
    static KDE_DEPRECATED QString kde_default(const char *type);

    /**
     * @internal (for use by sycoca only)
     */
    QString kfsstnd_prefixes();

    /**
     * @internal (for use by sycoca only)
     */
    QString kfsstnd_xdg_conf_prefixes();

    /**
     * @internal (for use by sycoca only)
     */
    QString kfsstnd_xdg_data_prefixes();

    /**
     * Returns the toplevel directory in which KStandardDirs
     * will store things. Most likely <tt>$HOME/.kde</tt>.
     * Don't use this function if you can use locateLocal()
     * @return the toplevel directory
     */
    QString localkdedir() const;

    /**
     * @return @c $XDG_DATA_HOME
     * See also http://www.freedesktop.org/standards/basedir/draft/basedir-spec/basedir-spec.html
     */
    QString localxdgdatadir() const;

    /**
     * @return @c $XDG_CONFIG_HOME
     * See also http://www.freedesktop.org/standards/basedir/draft/basedir-spec/basedir-spec.html
     */
    QString localxdgconfdir() const;

    /**
     * @return the path where type was installed to by kdelibs. This is an absolute path and only
     * one out of many search paths
     */
    static QString installPath(const char *type);

    /**
     * Checks for existence and accessability of a file or directory.
     * Faster than creating a QFileInfo first.
     * @param fullPath the path to check. IMPORTANT: must end with a slash if expected to be a directory
     *                 (and no slash for a file, obviously).
     * @return @c true if the directory exists, @c false otherwise
     */
    static bool exists(const QString &fullPath);

    /**
     * Expands all symbolic links and resolves references to
     * '/./', '/../' and extra  '/' characters in @p dirname
     * and returns the canonicalized absolute pathname.
     * The resulting path will have no symbolic link, '/./'
     * or '/../' components.
     */
    static QString realPath(const QString &dirname);

    /**
     * Expands all symbolic links and resolves references to
     * '/./', '/../' and extra  '/' characters in @p filename
     * and returns the canonicalized absolute pathname.
     * The resulting path will have no symbolic link, '/./'
     * or '/../' components.
     */
    static QString realFilePath(const QString &filename);

    /**
     * This function is just for convenience. It simply calls
     * instance->dirs()->\link KStandardDirs::findResource() findResource\endlink(type, filename).
     *
     * @param type   The type of the wanted resource, see KStandardDirs
     * @param filename   A relative filename of the resource
     * @param cData   The KComponentData object
     *
     * @return A full path to the filename specified in the second
     *         argument, or QString() if not found
     **/
    static QString locate( const char *type, const QString& filename, const KComponentData &cData = KGlobal::mainComponent() );

    /**
     * This function is much like locate. However it returns a
     * filename suitable for writing to. No check is made if the
     * specified @p filename actually exists. Missing directories
     * are created. If @p filename is only a directory, without a
     * specific file, @p filename must have a trailing slash.
     *
     * @param type   The type of the wanted resource, see KStandardDirs
     * @param filename   A relative filename of the resource
     * @param cData   The KComponentData object
     *
     * @return A full path to the filename specified in the second
     *         argument, or QString() if not found
     **/
    static QString locateLocal( const char *type, const QString& filename, const KComponentData &cData = KGlobal::mainComponent() );

    /**
     * This function is much like locate. No check is made if the
     * specified filename actually exists. Missing directories
     * are created if @p createDir is true. If @p filename is only
     * a directory, without a specific file, @p filename must have
     * a trailing slash.
     *
     * @param type   The type of the wanted resource, see KStandardDirs
     * @param filename   A relative filename of the resource
     * @param createDir  If @c true, missing directories are created,
     *        if @c false, no directory is created
     * @param cData   The KComponentData object
     *
     * @return A full path to the filename specified in the second
     *         argument, or QString() if not found
     **/
    static QString locateLocal( const char *type, const QString& filename, bool createDir, const KComponentData &cData = KGlobal::mainComponent() );

    /**
     * Check, if a file may be accessed in a given mode.
     * This is a wrapper around the access() system call.
     * checkAccess() calls access() with the given parameters.
     * If this is OK, checkAccess() returns true. If not, and W_OK
     * is part of mode, it is checked if there is write access to
     * the directory. If yes, checkAccess() returns true.
     * In all other cases checkAccess() returns false.
     *
     * Other than access() this function EXPLICITLY ignores non-existent
     * files if checking for write access.
     *
     * @param pathname The full path of the file you want to test
     * @param mode     The access mode, as in the access() system call.
     * @return Whether the access is allowed, true = Access allowed
     */
    static bool checkAccess(const QString& pathname, int mode);

private:
    // Disallow assignment and copy-construction
    KStandardDirs( const KStandardDirs& );
    KStandardDirs& operator= ( const KStandardDirs& );

    class KStandardDirsPrivate;
    KStandardDirsPrivate* const d;

    // Like their public counter parts but with an extra priority argument
    // If priority is true, the directory is added directly after
    // $KDEHOME/$XDG_DATA_HOME/$XDG_CONFIG_HOME
    void addPrefix( const QString& dir, bool priority );
    void addXdgConfigPrefix( const QString& dir, bool priority );
    void addXdgDataPrefix( const QString& dir, bool priority );
    void addKDEDefaults();

    void addResourcesFrom_krcdirs();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KStandardDirs::SearchOptions)

#endif // KSTANDARDDIRS_H
