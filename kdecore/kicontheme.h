/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; it comes under the GNU Library General 
 * Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 *
 */

#ifndef __KIconTheme_h_Included__
#define __KIconTheme_h_Included__

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>

class KConfig;
class KIconThemeDir;
class KIconThemePrivate;

/**
 * One icon as found by KIconTheme.
 */
class KIcon
{
public:
    KIcon() { size = 0; }

    /** Return true if this icon is valid, false otherwise. */
    bool isValid() { return size != 0; }

    enum Context { Any, Action, Application, Device, FileSystem, MimeType };
    enum Types { Fixed, Scalable };
    enum MatchType { MatchExact, MatchBest };
    enum Group { Desktop, Kicker, Toolbar, Small, ListItem, LastGroup, User };

    /** The size in pixels of the icon. */
    int size;

    /** The context of the icon. */
    int context;

    /** The type of the icon: Fixed or Scalable. */
    int type;

    /** The full path of the icon. */
    QString path;
};


/**
 * Class to use/access icon themes in KDE. This class is used by the
 * iconloader but can be used by others too.
 */
class KIconTheme
{
public:
    /** Load an icon theme by name.  */
    KIconTheme(QString name);
    ~KIconTheme();

    /** The stylized name of the icont theme. */
    QString name() { return mName; }

    /** A description for the icon theme. */
    QString description() { return mDesc; }

    /** The themes this icon theme falls back on. */
    QStringList inherits() { return mInherits; }

    /** The mimimum display depth required for this theme. This can either
     * be 8 or 32 */
    int depth() { return mDepth; }

    /** The default size of this theme for a certain icon group.
     * @param group The icon group. See @ref #KIcon::Group.
     * @return The default size in pixels for the given icon group.
     */
    int defaultSize(int group);

    /** Query available sizes for a group. */
    QValueList<int> querySizes(int group);

    /** Query available icons for a size and context. */
    QStringList queryIcons(int size, int context = KIcon::Any); 

    /** Lookup an icon in the theme.
     * @param name The name of the icon, without extension.
     * @param size The desired size of the icon.
     * @param match The matching mode. KIcon::MatchExact returns an icon
     * only if matches exactly. KIcon::MatchBest returns the best matching
     * icon. 
     * @return A KIcon class that describes the icon. If an icon is found,
     * @ref #KIcon::isValid will return true, and false otherwise.
     */
    KIcon iconPath(QString name, int size, int match);

    /** List all icon themes installed on the system, global and local. */
    static QStringList list();

private:
    int mDefSize[8];
    QValueList<int> mSizes[8];

    int mDepth;
    QString mDir, mName, mDesc;
    QStringList mInherits;
    QList<KIconThemeDir> mDirs;
    KIconThemePrivate *d;
};

#endif
