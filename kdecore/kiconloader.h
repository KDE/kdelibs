/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; it comes under the GNU Library General 
 * Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __KIconLoader_h_Included__
#define __KIconLoader_h_Included__

#include <qstring.h>
#include <qlist.h>
#include <qmap.h>
#include <qpixmap.h>

#include "kglobal.h"
#include "kinstance.h"
#include "kicontheme.h"

class KIconGroup;
class KIconThemeNode;
class KConfig;
class KIconLoaderPrivate;
class KStandardDirs;


/**
 * Iconloader for KDE.
 *
 * KIconLoader will load the current icon theme and all its base themes. 
 * Icons will be searched in any of these themes. Additionally, it caches 
 * icons and applies effects according the the user's preferences.
 *
 * In KDE, icons are loaded by "Group". An icon group is a location on the
 * screen where icons are being used. Standard groups are: Desktop, Kicker, 
 * Toolbar, Small and ListItem. For each group, the user determines what size 
 * of icons are to be used.
 *
 * Experts can also load icons by size. This is not recommended because this
 * won't honour the user's preferences. When loading an icon by size, you
 * might want to know what sizes are available. This can be done by querying
 * the current icon theme, as returned by @ref #theme.
 */
class KIconLoader
{
public:

    // Source compatibility issue
    enum Size { Small=0, Medium, Large, Default };

    /**
     * Construct the iconloader.
     * @param appname Add the local directories of this application to the
     * icon search path. This can be used for loading icons which are not 
     * part of a theme. These icons can be loaded using the "User" group in 
     * @ref #loadIcon or @ref #iconPath. The default argument adds the local
     * directories of the current application.
     */
    KIconLoader(QString appname=QString::null);

    /** Cleanup */
    ~KIconLoader();

    /**
     * Load an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, the best match is returned.
     * The only case where no icon is returned, is when there's no icon with
     * the requested @em name.
     * @param name The name of the icon, without extension.
     * @param group_or_size If positive, this will find the named icon for
     * the group @em group_or_size. The group will specify the size and
     * effects to be applied. If negative, this will load an icon of size 
     * -@em group_or_size.
     * @param path_store If not null, the path of the icon is stored here.
     * @param canReturnNull Can return a null pixmap?
     */
    QPixmap loadIcon(QString name, int group_or_size = Default /* SCI */, 
	    QString *path_store=0L, bool canReturnNull=true);

    /**
     * Returns the path of an icon. See @ref #loadIcon.
     */
    QString iconPath(QString name, int group_or_size, 
	    bool canReturnNull=true);

    /**
     * Query all available icons for a specific group, having a specific
     * context.
     * @param group_or_size The icon group or size. See @ref #loadIcon.
     * @param context The icon context.
     */
    QStringList queryIcons(int group_or_size, int context=KIcon::Any);

    /**
     * Return the current size for an icon group.
     */
    int currentSize(int group);

    /**
     * Returns a pointer to the current theme. Can be used to query
     * available and default sizes for groups.
     */
    const KIconTheme *theme();


private:
    KIcon iconPath2(QString name, int size);
    KIcon iconPath2(QString name, int size, int match, KIconThemeNode *node);

    void addIconTheme(KIconTheme *theme, KIconThemeNode *node);
    void addIcons(QStringList *lst, int size, int context, KIconThemeNode *node);
    void printThemeTree(KIconThemeNode *node);

    QString mTheme;
    QStringList mThemeList, mThemeTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconLoaderPrivate *d;
};

/** Load a desktop icon.  */
QPixmap DesktopIcon(QString name, KInstance *instance=KGlobal::instance());

/** Load a kicker icon.  */
QPixmap KickerIcon(QString name, KInstance *instance=KGlobal::instance());

/** Load a toolbar icon.  */
QPixmap BarIcon(QString name, KInstance *instance=KGlobal::instance());

/** SCI: Load a toolbar icon.  */
QPixmap BarIcon(QString name, int size, KInstance *instance=KGlobal::instance());

/** Load a small icon.  */
QPixmap SmallIcon(QString name, KInstance *instance=KGlobal::instance());

/** Load a listitem icon.  */
QPixmap ListIcon(QString name, KInstance *instance=KGlobal::instance());

/** Load a user icon. */
QPixmap UserIcon(QString name, KInstance *instance=KGlobal::instance());

/** Returns the current icon size for a specific group.  */
int IconSize(int group, KInstance *instance=KGlobal::instance());

#endif // __KIconLoader_h_Included__
