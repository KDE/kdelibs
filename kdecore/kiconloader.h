/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __KIconLoader_h_Included__
#define __KIconLoader_h_Included__

#include <qstring.h>
#include <qpixmap.h>
#include <qiconset.h>

// Grmbl, X headers.....
#ifdef Status
#define KIconLoaderXStatus Status
#undef Status
#endif
#include <qmovie.h>
#ifdef KIconLoaderXStatus
#define Status int
#undef KIconLoaderXStatus
#endif

#include <kglobal.h>
#include <kinstance.h>
#include <kicontheme.h>

class KIconGroup;
class KIconThemeNode;
class KConfig;
class KIconLoaderPrivate;
class KStandardDirs;
class KIconEffect;


/**
 * Iconloader for KDE.
 *
 * KIconLoader will load the current icon theme and all its base themes.
 * Icons will be searched in any of these themes. Additionally, it caches
 * icons and applies effects according the the user's preferences.
 *
 * In KDE, it is encouraged to load icons by "Group". An icon group is a
 * location on the screen where icons are being used. Standard groups are:
 * Desktop, Toolbar, MainToolbar, Small and Panel. Each group has some
 * centrally configured properties bound to it, including the icon size
 * and effects. This makes it possible to offer a consistent icon look in
 * all KDE applications.
 *
 * The standard groups are defined below.
 *
 * @li KIcon::Desktop: Icons in the iconview of konqueror, kdesktop and similar apps.
 * @li KIcon::Toolbar: Icons in toolbars.
 * @li KIcon::MainToolbar: Icons in the main toolbars.
 * @li KIcon::Small: Various small (typical 16x16) places: titlebars, listviews
 * and menu entries.
 * @li KIcon::Panel: Icons in kicker's panel
 *
 * The icons are stored on disk in an icon theme or in a standalone
 * directory. The icon theme directories contain multiple sizes and/or
 * depths for the same icon. The iconloader will load the correct one based
 * on the icon group and the current theme. Icon themes are stored globally
 * in share/icons, or, application specific in share/apps/$appdir/icons.
 *
 * The standalone directories contain just one version of an icon. The
 * directories that are searched are: $appdir/pics and $appdir/toolbar.
 * Icons in these directories can be loaded by using the special group
 * "User".
 *
 */
class KIconLoader
{
public:

    /**
     * Constructs an iconloader.
     * @param appname Add the data directories of this application to the
     * icon search path for the "User" group. The default argument adds the
     * directories of the current application.
     * @param the KStandardDirs object to use. If null the global one is used
     */
    KIconLoader(const QString& appname=QString::null, KStandardDirs *dirs = 0);

    /**
     * Cleanup
     */
    ~KIconLoader();

    /**
     * Adds @p appname to the list of application specific directories.
     * @param appname The application name.
     */
    void addAppDir(const QString& appname);

    /**
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * @p canReturnNull parameter.
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See @ref KIcon::StdSizes.
     * @param state The icon state: @p DefaultState, @p ActiveState or
     * @p DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     * @param path_store If not null, the path of the icon is stored here.
     * @param canReturnNull Can return a null pixmap? If false, the
     * "unknown" pixmap is returned when no appropriate icon has been found.
     * @return the QPixmap. Can be null when not found, depending on
     *         @p canReturnNull.
     */
    QPixmap loadIcon(const QString& name, KIcon::Group group, int size=0,
		     int state=KIcon::DefaultState, QString *path_store=0L,
		     bool canReturnNull=false) const;

    /**
     * Creates an icon set, that will do on-demand loading of the icon.
     * Loading itself is done by calling @ref loadIcon .
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See @ref KIcon::StdSizes.
     * @param canReturnNull Can return a null iconset? If false, iconset
     * containing the "unknown" pixmap is returned when no appropriate icon has
     * been found.
     * @return the icon set. Can be null when not found, depending on
     *          @p canReturnNull.
     * @since 3.1
     */
    QIconSet loadIconSet(const QString& name, KIcon::Group group, int size,
                         bool canReturnNull);

    // KDE4 merge as (const QString&,KIcon::Group,int=0,bool=false);
    /**
     * Creates an icon set, that will do on-demand loading of the icon.
     * Loading itself is done by calling @ref loadIcon .
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See @ref KIcon::StdSizes.
     * @return the icon set. Can be null when not found
     */
    QIconSet loadIconSet(const QString& name, KIcon::Group group, int size=0);

    /**
     * Returns the path of an icon.
     * @param name The name of the icon, without extension. If an absolute 
     * path is supplied for this parameter, iconPath will return it 
     * directly.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @p group_or_size. If negative, search
     * icons whose size is - @p group_or_size.
     *             See @ref KIcon::Group and @ref KIcon::StdSizes
     * @param canReturnNull Can return a null string? If not, a path to the
     *                      "unknown" icon will be returned.
     * @return the path of an icon, can be null or the "unknown" icon when
     *         not found, depending on @p canReturnNull.
     */
    QString iconPath(const QString& name, int group_or_size,
		     bool canReturnNull=false) const;

    /**
     * Loads an animated icon.
     * @param name The name of the icon.
     * @param group The icon group. See @ref loadIcon().
     * @param size Override the default size for @p group.
     *             See @ref KIcon::StdSizes.
     * @return A QMovie object. Can be null if not found.
     */
    QMovie loadMovie(const QString& name, KIcon::Group group, int size=0) const;

    /**
     * Returns the path to an animated icon.
     * @param name The name of the icon.
     * @param group The icon group. See @ref loadIcon().
     * @param size Override the default size for @p group.
     *             See @ref KIcon::StdSizes.
     * @return the full path to the movie, ready to be passed to QMovie's constructor.
     * Empty string if not found.
     */
    QString moviePath(const QString& name, KIcon::Group group, int size=0) const;

    /**
     * Loads an animated icon. In the future, this will be replaced by a
     * routine which loads an MNG animation and returns a QMovie.
     * @param name The name of the icon.
     * @param group The icon group. See @ref loadIcon().
     * @param size Override the default size for @p group.
     *             See @ref KIcon::StdSizes.
     * @return A QStringList containing the absolute path of all the frames
     * making up the animation.
     */
    QStringList loadAnimated(const QString& name, KIcon::Group group, int size=0) const;

    /**
     * Queries all available icons for a specific group, having a specific
     * context.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @p group_or_size. If negative, search
     * icons whose size is - @p group_or_size.
     *             See @ref KIcon::Group and @ref KIcon::StdSizes
     * @param context The icon context.
     * @return a list of all icons
     */
    QStringList queryIcons(int group_or_size, KIcon::Context context=KIcon::Any) const;

    /**
     * Queries all available icons for a specific context.
     * @param group_or_size The icon preferred group or size. If available
     * at this group or size, those icons will be returned, in other case,
     * icons of undefined size will be returned. Positive numbers are groups,
     * negative numbers are negated sizes. See @ref KIcon::Group and
     *    @ref KIcon::StdSizes
     * @param context The icon context.
     * @return A QStringList containing the icon names
     * available for that context
     */
    QStringList queryIconsByContext(int group_or_size,
				    KIcon::Context context=KIcon::Any) const;

    /**
     * Returns a list of all icons (*.png or *.xpm extension) in the
     * given directory.
     * @param iconsDir the directory to search in
     * @return A QStringList containing the icon paths
     * @since 3.1
     */
    QStringList queryIconsByDir( const QString& iconsDir ) const;

    /**
     * Returns the current size of the group.
     * @param group the group to check.
     * @return the current size for an icon group.
     */
    int currentSize(KIcon::Group group) const;

    /**
     * Returns a pointer to the current theme. Can be used to query
     * available and default sizes for groups.
     * @return a pointer to the current theme. 0 if no theme set.
     */
    KIconTheme *theme() const;

    /**
     * Returns a pointer to the KIconEffect object used by the icon loader.
     * @return the KIconEffect.
     */
    KIconEffect *iconEffect() const;

    /**
     * Called by KInstance::newIconLoader to reconfigure the icon loader.
     * @param _appname the new application name
     * @param _dirs the new standard directories. If 0, the directories
     *              from @ref KGlobal will be taken.
     */
    void reconfigure( const QString& _appname, KStandardDirs *_dirs );

    /**
     * Returns the unknown icon. An icon that is used when no other icon
     * can be found.
     * @return the unknown pixmap
     */
    static QPixmap unknown();

    /**
     * Returns if the user wants to use blend the icons with the background
     *  using the alpha channel information for a given group.
     * @param group the group to check
     * @return true if alpha blending is desired
     */
    bool alphaBlending( KIcon::Group group ) const;

 private:
    /**
     * @internal
     */
    void init( const QString& _appname, KStandardDirs *_dirs );

    /**
     * @internal
     * tries to find an icon with the name. It tries some extension and
     * match strategies
     */
    KIcon findMatchingIcon(const QString& name, int size) const;

    /**
     * @internal
     * Loads and caches an overlay.
     */
     QImage *loadOverlay(const QString& name, int size) const;

    /**
     * @internal
     * Adds themes installed in the application's directory.
     **/
    void addAppThemes(const QString& appname);

    /**
     * Adds all themes that are part of this node and the themes
     * below (the fallbacks of the theme) in the tree.
     * @internal
     */
    void addBaseThemes(KIconThemeNode *node, const QString &appname);

    /**
     * Adds all the default themes from other desktops at the end of
     * the list of icon themes.
     * @internal
     * @since 3.1
     */
    void addExtraDesktopThemes();

    /**
     * @internal
     * return the path for the unknown icon in that size
     * @since 3.1
     */
    QString unknownIconPath( int size ) const;

    /**
     * Checks if name ends in one of the supported icon formats (i.e. .png)
     * and returns the name without the extension if it does.
     * @since 3.1
     */
    QString removeIconExtension(const QString &name) const;

    // @internal the data object
    KIconLoaderPrivate *d;
};

/**
 * Load a desktop icon.
 */
QPixmap DesktopIcon(const QString& name, int size=0,
		    int state=KIcon::DefaultState,
		    KInstance *instance=KGlobal::instance());

/**
 * Load a desktop icon.
 */
QPixmap DesktopIcon(const QString& name, KInstance *instance);

/**
 * Load a desktop icon, and apply the necessary effects to get an IconSet.
 */
QIconSet DesktopIconSet(const QString& name, int size=0,
		    KInstance *instance=KGlobal::instance());

/**
 * Load a toolbar icon.
 */
QPixmap BarIcon(const QString& name, int size=0, int state=KIcon::DefaultState,
	KInstance *instance=KGlobal::instance());

/**
 * Load a toolbar icon.
 */
QPixmap BarIcon(const QString& name, KInstance *instance);

/**
 * Load a toolbar icon, and apply the neccesary effects to get an IconSet.
 */
QIconSet BarIconSet(const QString& name, int size=0,
		    KInstance *instance=KGlobal::instance());

/**
 * Load a small icon.
 */
QPixmap SmallIcon(const QString& name, int size=0,
		  int state=KIcon::DefaultState,
		  KInstance *instance=KGlobal::instance());

/**
 * Load a small icon.
 */
QPixmap SmallIcon(const QString& name, KInstance *instance);

/**
 * Load a small icon, and apply the neccesary effects to get an IconSet.
 */
QIconSet SmallIconSet(const QString& name, int size=0,
		    KInstance *instance=KGlobal::instance());

/**
 * Load a main toolbar icon.
 */
QPixmap MainBarIcon(const QString& name, int size=0,
		    int state=KIcon::DefaultState,
		    KInstance *instance=KGlobal::instance());

/**
 * Load a main toolbar icon.
 */
QPixmap MainBarIcon(const QString& name, KInstance *instance);

/**
 * Load a main toolbar icon, and apply the effects to get an IconSet.
 */
QIconSet MainBarIconSet(const QString& name, int size=0,
		    KInstance *instance=KGlobal::instance());

/**
 * Load a user icon.
 */
QPixmap UserIcon(const QString& name, int state=KIcon::DefaultState,
	KInstance *instance=KGlobal::instance());

/**
 * Load a user icon.
 */
QPixmap UserIcon(const QString& name, KInstance *instance);

/**
 * Load a user icon, and apply the effects to get an IconSet.
 */
QIconSet UserIconSet(const QString& name,
	KInstance *instance=KGlobal::instance());

/**
 * Returns the current icon size for a specific group.
 */
int IconSize(KIcon::Group group, KInstance *instance=KGlobal::instance());

#endif // __KIconLoader_h_Included__
