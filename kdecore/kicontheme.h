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
 * @see KIconEffect
 * @see KIconTheme
 * @see KIconLoader
 */
class KIcon
{
public:
    KIcon() { size = 0; }

    /**
     * Return true if this icon is valid, false otherwise.
     */
    bool isValid() const { return size != 0; }

    /**
     * Defines the context of the icon.
     */
    enum Context { 
      Any, ///< Some icon with unknown purpose.
      Action, ///< An action icon (e.g. 'save', 'print').
      Application, ///< An icon that represents an application.
      Device, ///< An icon that represents a device.
      FileSystem, ///< An icon that represents a file system.
      MimeType ///< An icon that represents a mime type (or file type).
    };

    /**
     * The type of the icon.
     */
    enum Type { 
      Fixed, ///< Fixed-size icon.
      Scalable, ///< Scalable-size icon.
      Threshold ///< A threshold icon.
    };

    /**
     * The type of a match.
     */
    enum MatchType { 
      MatchExact, ///< Only try to find an exact match.
      MatchBest   ///< Take the best match if there is no exact match.

    };

    // if you add a group here, make sure to change the config reading in
    // KIconLoader too
    /**
     * The group an an icon.
     */
    enum Group { NoGroup=-1, Desktop=0, FirstGroup=Desktop, Toolbar,
        MainToolbar, Small, Panel, LastGroup, User };

    /**
     * These are the standard sizes for icons.
     */
    enum StdSizes {
        /// small icons for menu entries
        SizeSmall=16,
        /// medium sized icons for the desktop
        SizeMedium=32,
        /// large sized icons for the panel
        SizeLarge=48 };

    /**
     * Defines the possible states of an icon.
     */
    enum States { DefaultState, ///< The default state.
		  ActiveState,  ///< Icon is active.
		  DisabledState, ///< Icon is disabled.
		  LastState      ///< Last state (last constant)
    };

    /**
     * This defines an overlay, a semi-transparent image that is 
     * projected onto the icon. They are used to show that the file
     * represented by the icon is, for example, locked, zipped or hidden.
     */
    enum Overlays { 
      LockOverlay=0x100, ///< a file is locked
      ZipOverlay=0x200,  ///< a file is zipped
      LinkOverlay=0x400, ///< a fileis a link
      HiddenOverlay=0x800, ///< a file is hidden
      ShareOverlay=0x1000, ///< a file is shared
      OverlayMask = ~0xff 
    };

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
 * @see KIconLoader
 */
class KIconTheme
{
public:
    /**
     * Load an icon theme by name.
     * @param name the name of the theme (e.g. "hicolor" or "keramik")
     * @param appName the name of the application. Can be null. This argument
     *        allows applications to have themed application icons.
     */
    KIconTheme(const QString& name, const QString& appName=QString::null);
    ~KIconTheme();

    /**
     * The stylized name of the icon theme.
     * @return the (human-readable) name of the theme
     */
    QString name() const { return mName; }

    /**
     * A description for the icon theme.
     * @return a human-readable description of the theme, QString::null 
     *         if there is none
     */
    QString description() const { return mDesc; }

    /**
     * Return the name of the "example" icon. This can be used to 
     * present the theme to the user.
     * @return the name of the example icon, QString::null if there is none
     */
    QString example() const;

    /**
     * Return the name of the screenshot.
     * @return the name of the screenshot, QString::null if there is none
     */
    QString screenshot() const;

    /**
     * Returns the name of this theme's link overlay.
     * @return the name of the link overlay
     */
    QString linkOverlay() const;

    /**
     * Returns the name of this theme's zip overlay.
     * @return the name of the zip overlay
     */
    QString zipOverlay() const;

    /**
     * Returns the name of this theme's lock overlay.
     * @return the name of the lock overlay
     */
    QString lockOverlay() const;

    /** 
     * Returns the name of this theme's share overlay. 
     * @return the name of the share overlay
     * @since 3.1
     */
    QString shareOverlay () const;

    /**
     * Returns the toplevel theme directory.
     * @return the directory of the theme
     */
    QString dir() const { return mDir; }

    /**
     * The themes this icon theme falls back on.
     * @return a list of icon themes that are used as fall-backs
     */
    QStringList inherits() const { return mInherits; }

    /**
     * The icon theme exists?
     * @return true if the icon theme is valid
     */
    bool isValid() const;

    /**
     * The minimum display depth required for this theme. This can either
     * be 8 or 32.
     * @return the minimum bpp (8 or 32)
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
     * @param group The icon group. See @ref #KIcon::Group.
     * @return a list of available sized for the given group
     */
    QValueList<int> querySizes(KIcon::Group group) const;

    /**
     * Query available icons for a size and context.
     * @param size the size of the icons
     * @param context the context of the icons
     * @return the list of icon names
     */
    QStringList queryIcons(int size, KIcon::Context context = KIcon::Any) const;

    /**
     * Query available icons for a context and preferred size.
     * @param size the size of the icons
     * @param context the context of the icons
     * @return the list of icon names
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
     * @see KIcon::isValid will return true, and false otherwise.
     */
    KIcon iconPath(const QString& name, int size, KIcon::MatchType match) const;

    /**
     * List all icon themes installed on the system, global and local.
     * @return the list of all icon themes
     */
    static QStringList list();

    /**
     * Returns the current icon theme.
     * @return the name of the current theme
     */
    static QString current();

    /**
     * Reconfigure the theme.
     */
    static void reconfigure();

    /**
     * Returns the default icon theme.
     * @return the name of the default theme name
     * @since 3.1
     */
    static QString defaultThemeName();

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
