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
 *
 */

#ifndef __KIconTheme_h_Included__
#define __KIconTheme_h_Included__

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>

class KConfig;
class KIconThemeDir;

class KIconThemePrivate;

class KIconPrivate;

/**
 * One icon as found by KIconTheme. Also serves as a namespace containing
 * icon related constants.
 */
class KIcon
{
public:
    KIcon() { size = 0; }

    /**
     * Return true if this icon is valid, false otherwise.
     */
    bool isValid() const { return size != 0; }

    enum Context { Any, Action, Application, Device, FileSystem, MimeType };
    enum Type { Fixed, Scalable, Threshold };
    enum MatchType { MatchExact, MatchBest };
    // if you add a group here, make sure to change the config reading in
    // KIconLoader too
    enum Group { NoGroup=-1, Desktop=0, FirstGroup=Desktop, Toolbar,
        MainToolbar, Small, Panel, LastGroup, User };

    enum StdSizes {
        /// small icons for menu entries
        SizeSmall=16,
        /// medium sized icons for the desktop
        SizeMedium=32,
        /// large sized icons for the panel
        SizeLarge=48 };

    enum States { DefaultState, ActiveState, DisabledState, LastState };
    enum Overlays { LockOverlay=0x100, ZipOverlay=0x200, LinkOverlay=0x400,
            HiddenOverlay=0x800, ShareOverlay=0x1000, OverlayMask = ~0xff };

    /**
     * The size in pixels of the icon.
     */
    int size;

    /**
     * The context of the icon.
     */
    Context context;

    /**
     * The type of the icon: Fixed, Scalable or Threshold.
     **/
    Type type;

    /**
     * The threshold in case type == Threshold
     */
    int threshold;

    /**
     * The full path of the icon.
     */
    QString path;

private:
    KIconPrivate *d;
};

inline KIcon::Group& operator++(KIcon::Group& group) { group = static_cast<KIcon::Group>(group+1); return group; }
inline KIcon::Group operator++(KIcon::Group& group,int) { KIcon::Group ret = group; ++group; return ret; }

/**
 * Class to use/access icon themes in KDE. This class is used by the
 * iconloader but can be used by others too.
 */
class KIconTheme
{
public:
    /**
     * Load an icon theme by name.
     */
    KIconTheme(const QString& name, const QString& appName=QString::null);
    ~KIconTheme();

    /**
     * The stylized name of the icon theme.
     */
    QString name() const { return mName; }

    /**
     * A description for the icon theme.
     */
    QString description() const { return mDesc; }

    /**
     * Return the name of the "example" icon.
     */
    QString example() const;

    /**
     * Return the name of the screenshot.
     */
    QString screenshot() const;

    /**
     * Returns the name of this theme's link overlay.
     */
    QString linkOverlay() const;

    /**
     * Returns the name of this theme's zip overlay.
     */
    QString zipOverlay() const;

    /**
     * Returns the name of this theme's lock overlay.
     */
    QString lockOverlay() const;

    /** Returns the name of this theme's share overlay. */
    QString shareOverlay () const;

    /**
     * Returns the toplevel theme directory.
     */
    QString dir() const { return mDir; }

    /**
     * The themes this icon theme falls back on.
     */
    QStringList inherits() const { return mInherits; }

    /**
     * The icon theme exists?
     */
    bool isValid() const;

    /**
     * The minimum display depth required for this theme. This can either
     * be 8 or 32.
     */
    int depth() const { return mDepth; }

    /**
     * The default size of this theme for a certain icon group.
     * @param group The icon group. See @ref #KIcon::Group.
     * @return The default size in pixels for the given icon group.
     */
    int defaultSize(KIcon::Group group) const;

    /**
     * Query available sizes for a group.
     */
    QValueList<int> querySizes(KIcon::Group group) const;

    /**
     * Query available icons for a size and context.
     */
    QStringList queryIcons(int size, KIcon::Context context = KIcon::Any) const;

    /**
     * Query available icons for a context and preferred size.
     */
    QStringList queryIconsByContext(int size, KIcon::Context context = KIcon::Any) const;


    /**
     * Lookup an icon in the theme.
     * @param name The name of the icon, without extension.
     * @param size The desired size of the icon.
     * @param match The matching mode. KIcon::MatchExact returns an icon
     * only if matches exactly. KIcon::MatchBest returns the best matching
     * icon.
     * @return A KIcon class that describes the icon. If an icon is found,
     * @ref #KIcon::isValid will return true, and false otherwise.
     */
    KIcon iconPath(const QString& name, int size, KIcon::MatchType match) const;

    /**
     * List all icon themes installed on the system, global and local.
     */
    static QStringList list();

    /**
     * Returns the current icon theme.
     */
    static QString current();

    static void reconfigure();

private:
    int mDefSize[8];
    QValueList<int> mSizes[8];

    int mDepth;
    QString mDir, mName, mDesc;
    QStringList mInherits;
    QPtrList<KIconThemeDir> mDirs;
    KIconThemePrivate *d;

    static QString *_theme;
    static QStringList *_theme_list;
};

#endif
