/* vi: ts=8 sts=4 sw=4
 *
 * kiconloader.cpp: An icon loader for KDE with theming functionality.
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kiconloader.h"
#include "kicontheme.h"
#include "kiconeffect.h"
#include "kiconcache.h"
#include "k3icon_p.h"

#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>
#include <QtGui/QImage>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QIcon>
#include <QtGui/QBitmap>
#include <QHash>
#include <QPainter>
#include <QMovie>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kcomponentdata.h>
#include <ksvgrenderer.h>
#include <kde_file.h>

#include <sys/types.h>
#include <stdlib.h>     //for abs
#include <unistd.h>     //for readlink
#include <dirent.h>
#include <assert.h>
#include <kconfiggroup.h>

// Qt implements Tiny SVG specification. This specification does not cover important elements
// that are pretty globally used on our icons, like blurring (and other filters). TT seems to have
// no interest in supporting the full SVG specification (it would be slower, even with JS, CSS
// support...). So, we have no chance for now. Let's disable svg rendering unconditionally.
// (ereslibre)
#undef KDE_QT_SVG_RENDERER_FIXED

//#define NO_LAZYLOAD_ICONTHEME


/*** KIconThemeNode: A node in the icon theme dependancy tree. ***/

class KIconThemeNode
{
public:

    KIconThemeNode(KIconTheme *_theme);
    ~KIconThemeNode();

    void queryIcons(QStringList *lst, int size, KIconLoader::Context context) const;
    void queryIconsByContext(QStringList *lst, int size, KIconLoader::Context context) const;
    K3Icon findIcon(const QString& name, int size, KIconLoader::MatchType match) const;
    void printTree(QString& dbgString) const;

    KIconTheme *theme;
};

KIconThemeNode::KIconThemeNode(KIconTheme *_theme)
{
    theme = _theme;
}

KIconThemeNode::~KIconThemeNode()
{
    delete theme;
}

void KIconThemeNode::printTree(QString& dbgString) const
{
    /* This method doesn't have much sense anymore, so maybe it should
       be removed in the (near?) future */
    dbgString += '(';
    dbgString += theme->name();
    dbgString += ')';
}

void KIconThemeNode::queryIcons(QStringList *result,
                                int size, KIconLoader::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIcons(size, context);
}

void KIconThemeNode::queryIconsByContext(QStringList *result,
                                int size, KIconLoader::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIconsByContext(size, context);
}

K3Icon KIconThemeNode::findIcon(const QString& name, int size,
                               KIconLoader::MatchType match) const
{
    return theme->iconPath(name, size, match);
}


/*** KIconGroup: Icon type description. ***/

struct KIconGroup
{
    int size;
    bool alphaBlending;
};


static const int MAX_SVG_RENDERERS = 100;
/*** d pointer for KIconLoader. ***/
class KIconLoaderPrivate
{
public:
    KIconLoaderPrivate(KIconLoader *q)
        : q(q)
        , mpGroups(0)
        , mIconCache(0)
    {
    }

    ~KIconLoaderPrivate()
    {
        /* antlarr: There's no need to delete d->mpThemeRoot as it's already
        deleted when the elements of d->links are deleted */
        qDeleteAll(imgDict);
        qDeleteAll(links);
        qDeleteAll(svgRenderers);
        delete[] mpGroups;
        delete mIconCache;
    }

    /**
     * @internal
     */
    void init( const QString& _appname, KStandardDirs *_dirs );

    /**
     * @internal
     */
    bool initIconThemes();

    /**
     * @internal
     * tries to find an icon with the name. It tries some extension and
     * match strategies
     */
    K3Icon findMatchingIcon(const QString& name, int size) const;

    /**
     * @internal
     * tries to find an icon with the name.
     * This is one layer above findMatchingIcon -- it also implements generic fallbacks
     * such as generic icons for mimetypes.
     */
    K3Icon findMatchingIconWithGenericFallbacks(const QString& name, int size) const;

    /**
     * @internal
     * Adds themes installed in the application's directory.
     **/
    void addAppThemes(const QString& appname);

    /**
     * @internal
     * Adds all themes that are part of this node and the themes
     * below (the fallbacks of the theme) into the tree.
     */
    void addBaseThemes(KIconThemeNode *node, const QString &appname);

    /**
     * @internal
     * Recursively adds all themes that are specified in the "Inherits"
     * property of the given theme into the tree.
     */
    void addInheritedThemes(KIconThemeNode *node, const QString &appname);

    /**
     * @internal
     * Creates a KIconThemeNode out of a theme name, and adds this theme
     * as well as all its inherited themes into the tree. Themes that already
     * exist in the tree will be ignored and not added twice.
     */
    void addThemeByName(const QString &themename, const QString &appname);

    /**
     * @internal
     * return the path for the unknown icon in that size
     */
    QString unknownIconPath( int size ) const;

    /**
     * Checks if name ends in one of the supported icon formats (i.e. .png)
     * and returns the name without the extension if it does.
     */
    QString removeIconExtension(const QString &name) const;

    KIconLoader *q;

    QStringList mThemesInTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconEffect mpEffect;
    QHash<QString, QImage*> imgDict;
    QImage lastImage; // last loaded image without effect applied
    QString lastImageKey; // key for icon without effect
    int lastIconType; // see KIconLoader::type
    int lastIconThreshold; // see KIconLoader::threshold
    QList<KIconThemeNode *> links;
    QHash<QString, KSvgRenderer*> svgRenderers;
    KIconCache* mIconCache;
    bool extraDesktopIconsLoaded :1;
    // lazy loading: initIconThemes() is only needed when the "links" list is needed
    // mIconThemeInited is used inside initIconThemes() to init only once
    bool mIconThemeInited :1;
    bool lastWasUnknown :1; // last loaded image was the unknown image
    QString appname;

    void drawOverlays(const KIconLoader *loader, KIconLoader::Group group, int state, QPixmap& pix, const QStringList& overlays);
};

class KIconLoaderGlobalData
{
public:
    KIconLoaderGlobalData() {
        const QStringList genericIconsFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "generic-icons");
        //kDebug() << genericIconsFiles;
        Q_FOREACH(const QString& file, genericIconsFiles) {
            parseGenericIconsFiles(file);
        }
    }

    QString genericIconFor(const QString& icon) const {
        return m_genericIcons.value(icon);
    }

private:
    void parseGenericIconsFiles(const QString& fileName);
    QHash<QString, QString> m_genericIcons;
};

void KIconLoaderGlobalData::parseGenericIconsFiles(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("ISO 8859-1");
        while (!stream.atEnd()) {
            const QString line = stream.readLine();
            if (line.isEmpty() || line[0] == '#')
                continue;
            const int pos = line.indexOf(':');
            if (pos == -1) // syntax error
                continue;
            QString mimeIcon = line.left(pos);
            const int slashindex = mimeIcon.indexOf(QLatin1Char('/'));
            if (slashindex != -1) {
                mimeIcon[slashindex] = QLatin1Char('-');
            }

            const QString genericIcon = line.mid(pos+1);
            m_genericIcons.insert(mimeIcon, genericIcon);
            //kDebug(264) << mimeIcon << "->" << genericIcon;
        }
    }
}
K_GLOBAL_STATIC(KIconLoaderGlobalData, s_globalData)

void KIconLoaderPrivate::drawOverlays(const KIconLoader *iconLoader, KIconLoader::Group group, int state, QPixmap& pix, const QStringList& overlays)
{
    if (overlays.isEmpty()) {
        return;
    }

    const int iconSize = pix.size().width();
    int overlaySize;

    if (iconSize < 32) {
        overlaySize = 8;
    } else if (iconSize <= 48) {
        overlaySize = 16;
    } else if (iconSize <= 96) {
        overlaySize = 22;
    } else if (iconSize < 256) {
        overlaySize = 32;
    } else {
        overlaySize = 64;
    }

    QPainter painter(&pix);

    int count = 0;
    foreach (const QString& overlay, overlays) {
        // Ensure empty strings fill up a emblem spot
        // Needed when you have several emblems to ensure they're always painted
        // at the same place, even if one is not here
        if (overlay.isEmpty()) {
            ++count;
            continue;
        }

        //TODO: should we pass in the kstate? it results in a slower
        //      path, and perhaps emblems should remain in the default state
        //      anyways?
        const QPixmap pixmap = iconLoader->loadIcon(overlay, group, overlaySize, state, QStringList(), 0, true);

        if (pixmap.isNull()) {
            continue;
        }

        QPoint startPoint;
        switch (count) {
        case 0:
            // bottom left corner
            startPoint = QPoint(2, iconSize - overlaySize - 2);
            break;
        case 1:
            // bottom right corner
            startPoint = QPoint(iconSize - overlaySize - 2,
                                iconSize - overlaySize - 2);
            break;
        case 2:
            // top right corner
            startPoint = QPoint(iconSize - overlaySize - 2, 2);
            break;
        case 3:
            // top left corner
            startPoint = QPoint(2, 2);
            break;
        }

        painter.drawPixmap(startPoint, pixmap);

        ++count;
        if (count > 3) {
            break;
        }
    }
}

#define KICONLOADER_CHECKS
#ifdef KICONLOADER_CHECKS
// Keep a list of recently created and destroyed KIconLoader instances in order
// to detect bugs like #68528.
struct KIconLoaderDebug
{
    KIconLoaderDebug( KIconLoader* l, const QString& a )
        : loader( l ), appname( a ), valid( true )
        {}
    KIconLoader* loader;
    QString appname;
    bool valid;
    QString delete_bt;
};

static QList< KIconLoaderDebug > *kiconloaders;

static void registerIconLoader( KIconLoader* iconloader, const QString& _appname )
{
    if( kiconloaders == NULL )
        kiconloaders = new QList< KIconLoaderDebug>();
    // check for the (very unlikely case) that new KIconLoader gets allocated
    // at exactly same address like some previous one
    for( QList< KIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         )
        {
        if( (*it).loader == iconloader )
            it = kiconloaders->erase( it );
        else
            ++it;
        }
    kiconloaders->append( KIconLoaderDebug( iconloader, _appname ));
}

#endif

KIconLoader::KIconLoader(const QString& _appname, KStandardDirs *_dirs, QObject* parent)
    : QObject(parent)
{
    setObjectName(_appname);
    d = new KIconLoaderPrivate(this);

    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
            this, SLOT(newIconLoader()));
#ifdef KICONLOADER_CHECKS
    registerIconLoader(this, _appname);
#endif
    d->init( _appname, _dirs );
}

KIconLoader::KIconLoader(const KComponentData &componentData, QObject* parent)
    : QObject(parent)
{
    setObjectName(componentData.componentName());
    d = new KIconLoaderPrivate(this);

    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
            this, SLOT(newIconLoader()));
#ifdef KICONLOADER_CHECKS
    registerIconLoader(this, componentData.componentName());
#endif
    d->init(componentData.componentName(), componentData.dirs());
}

void KIconLoader::reconfigure( const QString& _appname, KStandardDirs *_dirs )
{
    delete d;
    d = new KIconLoaderPrivate(this);
    d->init( _appname, _dirs );
}

void KIconLoaderPrivate::init( const QString& _appname, KStandardDirs *_dirs )
{
    extraDesktopIconsLoaded=false;
    mIconThemeInited = false;
    mpThemeRoot = 0;

    if (_dirs)
        mpDirs = _dirs;
    else
        mpDirs = KGlobal::dirs();

    appname = _appname;
    if (appname.isEmpty())
        appname = KGlobal::mainComponent().componentName();

    // Initialize icon cache
    mIconCache = new KIconCache;
    if (!mIconCache->isValid()) {
        initIconThemes();
        QList<KIconTheme*> allThemes;
        foreach (KIconThemeNode* node, links) {
            allThemes.append(node->theme);
        }
        mIconCache->setThemeInfo(allThemes);
    }

    // These have to match the order in kicontheme.h
    static const char * const groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", "Panel", "Dialog", 0L };
    KSharedConfig::Ptr config = KGlobal::config();

    // loading config and default sizes
    mpGroups = new KIconGroup[(int) KIconLoader::LastGroup];
    for (KIconLoader::Group i=KIconLoader::FirstGroup; i<KIconLoader::LastGroup; i++)
    {
        if (groups[i] == 0L)
            break;

        KConfigGroup cg(config, QLatin1String(groups[i]) + "Icons");
        mpGroups[i].size = cg.readEntry("Size", 0);
        if (QPixmap::defaultDepth()>8)
            mpGroups[i].alphaBlending = cg.readEntry("AlphaBlending", true);
        else
            mpGroups[i].alphaBlending = false;

        if (!mpGroups[i].size)
            mpGroups[i].size = mIconCache->defaultIconSize(i);
    }

#ifdef NO_LAZYLOAD_ICONTHEME
    initIconThemes();
#endif
}

bool KIconLoaderPrivate::initIconThemes()
{
    if (mIconThemeInited) {
        // If mpThemeRoot isn't 0 then initing has succeeded
        return (mpThemeRoot != 0);
    }
    //kDebug(264);
    mIconThemeInited = true;

    // Add the default theme and its base themes to the theme tree
    KIconTheme *def = new KIconTheme(KIconTheme::current(), appname);
    if (!def->isValid())
    {
        delete def;
        // warn, as this is actually a small penalty hit
        kDebug(264) << "Couldn't find current icon theme, falling back to default.";
        def = new KIconTheme(KIconTheme::defaultThemeName(), appname);
        if (!def->isValid())
        {
            kError(264) << "Error: standard icon theme" << KIconTheme::defaultThemeName() << "not found!" << endl;
            delete def;
            return false;
        }
    }
    mpThemeRoot = new KIconThemeNode(def);
    mThemesInTree.append(def->internalName());
    links.append(mpThemeRoot);
    addBaseThemes(mpThemeRoot, appname);

    // Insert application specific themes at the top.
    mpDirs->addResourceType("appicon", "data", appname + "/pics/");
    // ################## KDE5: consider removing the toolbar directory
    mpDirs->addResourceType("appicon", "data", appname + "/toolbar/");

    // Add legacy icon dirs.
    QStringList dirs;
    dirs += mpDirs->resourceDirs("icon");
    dirs += mpDirs->resourceDirs("pixmap");
    dirs += mpDirs->resourceDirs("xdgdata-icon");
    dirs += "/usr/share/pixmaps";
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    dirs += mpDirs->resourceDirs("xdgdata-pixmap");
    for (QStringList::ConstIterator it = dirs.constBegin(); it != dirs.constEnd(); ++it)
        mpDirs->addResourceDir("appicon", *it);

#ifndef NDEBUG
    QString dbgString = "Theme tree: ";
    mpThemeRoot->printTree(dbgString);
    kDebug(264) << dbgString;
#endif

    return true;
}

KIconLoader::~KIconLoader()
{
#ifdef KICONLOADER_CHECKS
    for( QList< KIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         ++it )
        {
        if( (*it).loader == this )
            {
            (*it).valid = false;
            (*it).delete_bt = kBacktrace();
            break;
            }
        }
#endif
    delete d;
}

void KIconLoader::addAppDir(const QString& appname)
{
    d->initIconThemes();

    d->mpDirs->addResourceType("appicon", "data", appname + "/pics/");
    // ################## KDE5: consider removing the toolbar directory
    d->mpDirs->addResourceType("appicon", "data", appname + "/toolbar/");
    d->addAppThemes(appname);
}

void KIconLoaderPrivate::addAppThemes(const QString& appname)
{
    initIconThemes();

    KIconTheme *def = new KIconTheme(KIconTheme::current(), appname);
    if (!def->isValid()) {
        delete def;
        def = new KIconTheme(KIconTheme::defaultThemeName(), appname);
    }
    KIconThemeNode* node = new KIconThemeNode(def);
    bool addedToLinks = false;

    if (!mThemesInTree.contains(node->theme->internalName())) {
        mThemesInTree.append(node->theme->internalName());
        links.append(node);
        addedToLinks = true;
    }
    addBaseThemes(node, appname);

    if (!addedToLinks) {
        // Nodes in links are being deleted later - this one needs manual care.
        delete node;
    }
}

void KIconLoaderPrivate::addBaseThemes(KIconThemeNode *node, const QString &appname)
{
    // Quote from the icon theme specification:
    //   The lookup is done first in the current theme, and then recursively
    //   in each of the current theme's parents, and finally in the
    //   default theme called "hicolor" (implementations may add more
    //   default themes before "hicolor", but "hicolor" must be last).
    //
    // So we first make sure that all inherited themes are added, then we
    // add the KDE default theme as fallback for all icons that might not be
    // present in an inherited theme, and hicolor goes last.

    addInheritedThemes(node, appname);
    addThemeByName(KIconTheme::defaultThemeName(), appname);
    addThemeByName("hicolor", appname);
}

void KIconLoaderPrivate::addInheritedThemes(KIconThemeNode *node, const QString &appname)
{
    const QStringList lst = node->theme->inherits();

    for (QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it) {
        if ((*it) == "hicolor") {
          // The icon theme spec says that "hicolor" must be the very last
          // of all inherited themes, so don't add it here but at the very end
          // of addBaseThemes().
          continue;
        }
        addThemeByName(*it, appname);
    }
}

void KIconLoaderPrivate::addThemeByName(const QString &themename, const QString &appname)
{
    if (mThemesInTree.contains(themename + appname)) {
        return;
    }
    KIconTheme *theme = new KIconTheme(themename, appname);
    if (!theme->isValid()) {
        delete theme;
        return;
    }
    KIconThemeNode *n = new KIconThemeNode(theme);
    mThemesInTree.append(themename + appname);
    links.append(n);
    addInheritedThemes(n, appname);
}

void KIconLoader::addExtraDesktopThemes()
{
    if ( d->extraDesktopIconsLoaded ) return;

    d->initIconThemes();

    QStringList list;
    const QStringList icnlibs = KGlobal::dirs()->resourceDirs("icon");
    QStringList::ConstIterator it;
    char buf[1000];
    int r;
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
    {
        QDir dir(*it);
        if (!dir.exists())
            continue;
        const QStringList lst = dir.entryList(QStringList( "default.*" ), QDir::Dirs);
        QStringList::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2)
        {
            if (!KStandardDirs::exists(*it + *it2 + "/index.desktop")
                && !KStandardDirs::exists(*it + *it2 + "/index.theme"))
                continue;
            r=readlink( QFile::encodeName(*it + *it2) , buf, sizeof(buf)-1);
            if ( r>0 )
            {
                buf[r]=0;
                const QDir dir2( buf );
                QString themeName=dir2.dirName();

                if (!list.contains(themeName))
                    list.append(themeName);
            }
        }
    }

    for (it = list.constBegin(); it != list.constEnd(); ++it)
    {
        // Don't add the KDE defaults once more, we have them anyways.
        if (*it == QLatin1String("default.kde")
            || *it == QLatin1String("default.kde4")) {
            continue;
        }
        d->addThemeByName(*it, "");
    }

    d->extraDesktopIconsLoaded=true;

}

bool KIconLoader::extraDesktopThemesAdded() const
{
    return d->extraDesktopIconsLoaded;
}

QString KIconLoaderPrivate::removeIconExtension(const QString &name) const
{
    if (name.endsWith(".png") || name.endsWith(".xpm") || name.endsWith(".svg")) {
        return name.left(name.length() - 4);
    } else if (name.endsWith(".svgz")) {
        return name.left(name.length() - 5);
    }

    return name;
}


K3Icon KIconLoaderPrivate::findMatchingIconWithGenericFallbacks(const QString& name, int size) const
{
    K3Icon icon = findMatchingIcon(name, size);
    if (icon.isValid())
        return icon;

    const QString genericIcon = s_globalData->genericIconFor(name);
    if (!genericIcon.isEmpty()) {
        icon = findMatchingIcon(genericIcon, size);
        if (icon.isValid())
            return icon;
    }

    // From update-mime-database.c
    static const char* const media_types[] = {
        "text", "application", "image", "audio",
        "inode", "video", "message", "model", "multipart",
        "x-content", "x-epoc"
    };
    // Shared-mime-info spec says:
    // "If [generic-icon] is not specified then the mimetype is used to generate the
    // generic icon by using the top-level media type (e.g. "video" in "video/ogg")
    // and appending "-x-generic" (i.e. "video-x-generic" in the previous example)."
    for (uint i = 0 ; i < sizeof(media_types)/sizeof(*media_types) ; i++) {
        if (name.startsWith(QLatin1String(media_types[i]))) {
            icon = findMatchingIcon(QString::fromLatin1(media_types[i]) + "-x-generic", size);
            if (icon.isValid())
                return icon;
            break;
        }
    }
    // not found
    return icon;
}

K3Icon KIconLoaderPrivate::findMatchingIcon(const QString& name, int size) const
{
    const_cast<KIconLoaderPrivate*>(this)->initIconThemes();

    K3Icon icon;

// The following code has been commented out because the Qt SVG renderer needs
// to be improved. If you are going to change/remove some code from this part,
// please contact me before (ereslibre@kde.org), or kde-core-devel@kde.org. (ereslibre)
#ifdef KDE_QT_SVG_RENDERER_FIXED
    const char * ext1[4] = { ".png", ".svgz", ".svg", ".xpm" };
    const char * ext2[4] = { ".svgz", ".svg", ".png", ".xpm" };
    const char ** ext;

    if (size == KIconLoader::SizeSmall ||
        size == KIconLoader::SizeSmallMedium ||
        size == KIconLoader::SizeMedium ||
        size == KIconLoader::SizeLarge ||
        size == KIconLoader::SizeHuge ||
        size == KIconLoader::SizeEnormous)
    {
        ext = ext1; // size is standard, give preference to PNG over SVG when searching
    }
    else
    {
        ext = ext2; // size is non-standard, give preference to SVG over PNG when searching
    }

    /* If size parameter is a standard one, that means:

           - KIconLoader::SizeSmall
           - KIconLoader::SizeSmallMedium
           - KIconLoader::SizeMedium
           - KIconLoader::SizeLarge
           - KIconLoader::SizeHuge
           - KIconLoader::SizeEnormous

       To follow the XDG icon theme and icon naming specifications,
       the order in which we look for an icon is:

       png, svgz, svg, xpm exact match
       png, svgz, svg, xpm best match
       less specific fallback in this theme: png, svgz, svg, xpm exact match
                                             png, svgz, svg, xpm best match
       even less specific fallback in this theme: [same order]
       (...)

       next theme in inheritance tree: png, svgz, svg, xpm exact match
                                       png, svgz, svg, xpm best match
       less specific fallbacks in this next theme
       (...)

       next theme in inheritance tree: png, svgz, svg, xpm exact match
                                       png, svgz, svg, xpm best match
       less specific fallbacks in this next theme
       (...)

       and so on.

       If size parameter is a non-standard one, then we give more preference to
       SVG format since drawing SVG's gives better quality and despite being
       slower than resizing a PNG image, the cases where non-standard sizes are
       asked are very rare. For non-standard sizes what we have is:

       svgz, svg, png, xpm exact match
       svgz, svg, png, xpm best match
       less specific fallback in this theme: svgz, svg, png, xpm exact match
                                             svgz, svg, png, xpm best match
       even less specific fallback in this theme: [same order]
       (...)

       next theme in inheritance tree: svgz, svg, png, xpm exact match
                                       svgz, svg, png, xpm best match
       less specific fallbacks in this next theme
       (...)

       next theme in inheritance tree: svgz, svg, png, xpm exact match
                                       svgz, svg, png, xpm best match
       less specific fallbacks in this next theme
       (...)

       and so on.
       */
#else
    const char * const ext[4] = { ".png", ".svgz", ".svg", ".xpm" };
#endif

    foreach(KIconThemeNode *themeNode, links)
    {
        QStringList nameParts = name.split('-');
        QString currentName = name;

        while (!nameParts.isEmpty())
        {

            //kDebug(264) << "Looking up" << currentName;

// The following code has been commented out because the Qt SVG renderer needs
// to be improved. If you are going to change/remove some code from this part,
// please contact me before (ereslibre@kde.org), or kde-core-devel@kde.org. (ereslibre)
#ifdef KDE_QT_SVG_RENDERER_FIXED
            for (int i = 0 ; i < 4 ; i++)
            {
                icon = themeNode->theme->iconPath(currentName + ext[i], size, KIconLoader::MatchExact);
                if (icon.isValid())
                    return icon;
            }

            for (int i = 0 ; i < 4 ; i++)
            {
                icon = themeNode->theme->iconPath(currentName + ext[i], size, KIconLoader::MatchBest);
                if (icon.isValid())
                    return icon;
            }
#else
            for (int i = 0 ; i < 4 ; i++)
            {
                icon = themeNode->theme->iconPath(currentName + ext[i], size, KIconLoader::MatchExact);
                if (icon.isValid())
                    return icon;

                icon = themeNode->theme->iconPath(currentName + ext[i], size, KIconLoader::MatchBest);
                if (icon.isValid())
                    return icon;
            }
#endif

            nameParts.removeLast();
            currentName = nameParts.join("-");
        }
    }
    return icon;
}

inline QString KIconLoaderPrivate::unknownIconPath( int size ) const
{
    static const QString &str_unknown = KGlobal::staticQString("unknown");

    K3Icon icon = findMatchingIcon(str_unknown, size);
    if (!icon.isValid())
    {
        kDebug(264) << "Warning: could not find \"Unknown\" icon for size = "
                     << size << endl;
        return QString();
    }
    return icon.path;
}

// Finds the absolute path to an icon.

QString KIconLoader::iconPath(const QString& _name, int group_or_size,
                              bool canReturnNull) const
{
    if (!d->initIconThemes()) {
        return QString();
    }

    if (_name.isEmpty()
#ifdef Q_OS_WIN
        || (_name.length() > 1 &&
            (_name[0].isLetter() && _name[1] == QLatin1Char(':') ||
             _name[0] == '/'     && _name[1] == '/' ||
             _name[0] == '\\'    && _name[1] == '\\')))
#else
        || _name[0] == '/')
#endif
    {
        // we have either an absolute path or nothing to work with
        return _name;
    }

    QString name = d->removeIconExtension( _name );

    QString path;
    if (group_or_size == KIconLoader::User)
    {
        static const QString &png_ext = KGlobal::staticQString(".png");
        static const QString &xpm_ext = KGlobal::staticQString(".xpm");
        path = d->mpDirs->findResource("appicon", name + png_ext);

        static const QString &svgz_ext = KGlobal::staticQString(".svgz");
        static const QString &svg_ext = KGlobal::staticQString(".svg");
        if (path.isEmpty())
            path = d->mpDirs->findResource("appicon", name + svgz_ext);
        if (path.isEmpty())
            path = d->mpDirs->findResource("appicon", name + svg_ext);
        if (path.isEmpty())
            path = d->mpDirs->findResource("appicon", name + xpm_ext);
        return path;
    }

    if (group_or_size >= KIconLoader::LastGroup)
    {
        kDebug(264) << "Illegal icon group: " << group_or_size;
        return path;
    }

    int size;
    if (group_or_size >= 0)
        size = d->mpGroups[group_or_size].size;
    else
        size = -group_or_size;

    if (_name.isEmpty()) {
        if (canReturnNull)
            return QString();
        else
            return d->unknownIconPath(size);
    }

    K3Icon icon = d->findMatchingIconWithGenericFallbacks(name, size);

    if (!icon.isValid())
    {
        // Try "User" group too.
        path = iconPath(name, KIconLoader::User, true);
        if (!path.isEmpty() || canReturnNull)
            return path;

        return d->unknownIconPath(size);
    }
    return icon.path;
}

QPixmap KIconLoader::loadMimeTypeIcon( const QString& iconName, KIconLoader::Group group, int size,
                                       int state, const QStringList& overlays, QString *path_store ) const
{
    if ( !d->extraDesktopIconsLoaded )
    {
        const QPixmap pixmap = loadIcon( iconName, group, size, state, overlays, path_store, true );
        if (!pixmap.isNull() ) {
            return pixmap;
        }
        const_cast<KIconLoader *>(this)->addExtraDesktopThemes();
    }
    const QPixmap pixmap = loadIcon(iconName, group, size, state, overlays, path_store, true);
    if (pixmap.isNull()) {
        // Icon not found, fallback to application/octet-stream
        return loadIcon("application-octet-stream", group, size, state, overlays, path_store, false);
    }
    return pixmap;
}

QPixmap KIconLoader::loadIcon(const QString& _name, KIconLoader::Group group, int size,
                              int state, const QStringList& overlays,
                              QString *path_store, bool canReturnNull) const
{
    QString name = _name;
    QString path;
    QPixmap pix;
    bool unknownIcon = false;
    bool absolutePath = false;
    bool favIconOverlay = false;

    if (size < 0)
        return pix;

    // Special case for absolute path icons.
    if (name.startsWith("favicons/"))
    {
       favIconOverlay = true;
       name = KStandardDirs::locateLocal("cache", name+".png");
    }

    if (!name.isEmpty()
#ifdef Q_WS_WIN
        && !QDir::isRelativePath(name))
#else
        && name[0] == '/')
#endif
    {
        absolutePath = true;
    }

    static const QString &str_unknown = KGlobal::staticQString("unknown");

    // Special case for "User" icons.
    if (group == KIconLoader::User)
    {
        QString key;
        key.reserve(200);
        key.append("$kicou_");
        key.append(name).append('_').append(QString::number(size));
        key.append(overlays.join("_")); // krazy:exclude=doublequote_chars

        if (d->mIconCache->find(key, pix, path_store)) {
            //kDebug(264) << "KIL: " << "found the icon from KIC";
            if (!pix.isNull() || canReturnNull) {
                return pix;
            } else if (_name != str_unknown) {
                return loadIcon(str_unknown, group, size, state,
                            overlays, path_store, canReturnNull);
            }
        }
        if (!d->initIconThemes()) {
            return pix;  // null pixmap
        }

        path = (absolutePath) ? name :
                        iconPath(name, KIconLoader::User, canReturnNull);
        if (path.isEmpty())
        {
            d->mIconCache->insert(key, pix); //insert the null icon so we know it's null
            if (!canReturnNull) {
#ifndef NDEBUG
                kWarning(264) << "No such icon" << _name;
#endif
                unknownIcon = true;
            } else {
                return pix; // null pixmap
            }
            // We don't know the desired size: use small
            path = iconPath(str_unknown, KIconLoader::Small, true);
            if (path.isEmpty())
            {
                kWarning(264) << "Warning: Cannot find \"unknown\" icon.";
                return pix;
            }
        }

        if (path_store != 0L)
            *path_store = path;
        //if (inCache)
        //    return pix;
        QImage img(path);
        if (size != 0)
            img=img.scaled(size,size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        pix = QPixmap::fromImage(img);
        d->drawOverlays(this, KIconLoader::Desktop, state, pix, overlays);
        if (!unknownIcon)
            d->mIconCache->insert(key, pix, path);
        return pix;
    }

    // Regular case: Check parameters

    if ((group < -1) || (group >= KIconLoader::LastGroup))
    {
        kDebug(264) << "Illegal icon group: " << group;
        group = KIconLoader::Desktop;
    }

    if ((state < 0) || (state >= KIconLoader::LastState))
    {
        kDebug(264) << "Illegal icon state: " << state;
        state = KIconLoader::DefaultState;
    }

    if (size == 0 && group < 0)
    {
        kDebug(264) << "Neither size nor group specified!";
        group = KIconLoader::Desktop;
    }

    if (!absolutePath)
    {
        if (!canReturnNull && name.isEmpty())
            name = str_unknown;
        else
            name = d->removeIconExtension(name);
    }

    // If size == 0, use default size for the specified group.
    if (size == 0)
    {
        size = d->mpGroups[group].size;
    }
    favIconOverlay = favIconOverlay && size > 22;

    // Generate a unique cache key for the icon.

    QString key;
    key.reserve(100);
    key.append("$kico_");
    key.append(name).append('_').append(QString::number(size));

    QString overlayKey = overlays.join("_"); // krazy:exclude=doublequote_chars
    QString noEffectKey = key + overlayKey;

    if (group >= 0)
    {
        key.append(d->mpEffect.fingerprint(group, state));
    } else {
        key.append(QLatin1String("noeffect"));
    }
    key.append(overlayKey);

    // Is the icon in the cache?
    if (d->mIconCache->find(key, pix, path_store)) {
        //kDebug() << "KIL: " << "found icon from KIC";
        if (!pix.isNull() || canReturnNull) {
            return pix;
        } else if (_name != str_unknown) {
            return loadIcon(str_unknown, group, size, state,
                            overlays, path_store, canReturnNull);
        }
    }
    if (!d->initIconThemes()) {
        return pix; // null pixmap
    }

    QImage *img = 0;
    int iconType;
    int iconThreshold;

    if ( ( path_store != 0 ) ||
         ( noEffectKey != d->lastImageKey ) ||
         ( d->lastWasUnknown && canReturnNull ) )
    {
        // Not in cache and not the same as the last requested icon -> load it.
        K3Icon icon;
        if (absolutePath && !favIconOverlay)
        {
            icon.context=KIconLoader::Any;
            icon.type=KIconLoader::Scalable;
            icon.path=name;
        }
        else
        {
            if (!name.isEmpty())
                icon = d->findMatchingIconWithGenericFallbacks(favIconOverlay ? QString("text-html") : name, size);

            if (!icon.isValid())
            {
                // Try "User" icon too. Some apps expect this.
                if (!name.isEmpty()) {
                    pix = loadIcon(name, KIconLoader::User, size, state, overlays, path_store, true);
                }
                d->mIconCache->insert(key, pix, path);
                if (!pix.isNull() || canReturnNull) {
                    return pix;
                }
#ifndef NDEBUG
                kWarning(264) << "No such icon" << _name;
#endif
                unknownIcon = true;
                icon = d->findMatchingIcon(str_unknown, size);
                if (!icon.isValid())
                {
                    kDebug(264)
                        << "Warning: could not find \"Unknown\" icon for size = "
                        << size << endl;
                    return pix;
                }
            }
        }

        if (path_store != 0)
            *path_store = icon.path;

        // Use the extension as the format. Works for XPM and PNG, but not for SVG
        QString ext = icon.path.right(3).toUpper();
        if(ext != "SVG" && ext != "VGZ")
        {
            img = new QImage(icon.path, ext.toLatin1());
            if (img->isNull()) {
                if (!unknownIcon)
                    d->mIconCache->insert(key, pix, path);
                delete img;
                return pix;
            }
        }
        else
        {
            KSvgRenderer *renderer = d->svgRenderers[icon.path];
            if (!renderer) {
                renderer = new KSvgRenderer(icon.path);
                if (renderer->isValid()) {
                    if (d->svgRenderers.count() >= MAX_SVG_RENDERERS) {
                        QList<QString> keys = d->svgRenderers.keys();
                        for (int i = 0; i < MAX_SVG_RENDERERS/2; ++i) {
                            KSvgRenderer *oldRenderer = d->svgRenderers.take(keys[i]);
                            delete oldRenderer;
                        }
                    }
                    d->svgRenderers.insert(icon.path, renderer);
                }
            }
            // Special stuff for SVG icons

            if (renderer && renderer->isValid()) {
                img = new QImage(size, size, QImage::Format_ARGB32_Premultiplied);
                img->fill(0);
                QPainter p(img);
                renderer->render(&p);
            } else {
                delete renderer;
                if (!unknownIcon)
                    d->mIconCache->insert(key, pix, path);
                return pix;
            }
        }

        iconType = icon.type;
        iconThreshold = icon.threshold;
        path = icon.path;

        d->lastImage = img->copy();
        d->lastImageKey = noEffectKey;
        d->lastIconType = iconType;
        d->lastIconThreshold = iconThreshold;
        d->lastWasUnknown = unknownIcon;
    }
    else
    {
        img = new QImage( d->lastImage.copy() );
        iconType = d->lastIconType;
        iconThreshold = d->lastIconThreshold;
        unknownIcon = d->lastWasUnknown;
    }

    // Scale the icon and apply effects if necessary
#ifndef KDE_QT_SVG_RENDERER_FIXED
    // The following code needs to be removed after the SVG rendering has been
    // fixed (please take a look at the comment above). Please do not remove the
    // #if condition as it marks what needs to be removed (ereslibre)
    if (iconType == KIconLoader::Scalable && size != img->width())
    {
        *img = img->scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
#endif
    if (iconType == KIconLoader::Threshold && size != img->width())
    {
        const int sizeDiff = size - img->width();
        if (sizeDiff < 0 || sizeDiff > iconThreshold)
            *img = img->scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    if (group >= 0)
    {
        *img = d->mpEffect.apply(*img, group, state);
    }

    if (favIconOverlay)
    {
        QImage favIcon(name, "PNG");
        if (!favIcon.isNull()) // if favIcon not there yet, don't try to blend it
        {
            // Blend favIcon over img.
            // FIXME: This code should be updated to use modern QPainter
            // features.
            int x = img->width() - favIcon.width() - 1,
                y = img->height() - favIcon.height() - 1;
            favIcon = favIcon.convertToFormat(QImage::Format_ARGB32);
            *img = img->convertToFormat(QImage::Format_ARGB32);
            for( int line = 0;
                 line < favIcon.height();
                 ++line )
            {
                QRgb* fpos = reinterpret_cast< QRgb* >( favIcon.scanLine( line ));
                QRgb* ipos = reinterpret_cast< QRgb* >( img->scanLine( line + y )) + x;
                for( int i = 0;
                     i < favIcon.width();
                     ++i, ++fpos, ++ipos )
                    *ipos = qRgba( ( qRed( *ipos ) * ( 255 - qAlpha( *fpos )) + qRed( *fpos ) * qAlpha( *fpos )) / 255,
                                   ( qGreen( *ipos ) * ( 255 - qAlpha( *fpos )) + qGreen( *fpos ) * qAlpha( *fpos )) / 255,
                                   ( qBlue( *ipos ) * ( 255 - qAlpha( *fpos )) + qBlue( *fpos ) * qAlpha( *fpos )) / 255,
                                   ( qAlpha( *ipos ) * ( 255 - qAlpha( *fpos )) + qAlpha( *fpos ) * qAlpha( *fpos )) / 255 );
            }
        }
    }

    pix = QPixmap::fromImage(*img);

    d->drawOverlays(this, group, state, pix, overlays);

    delete img;

    if (!unknownIcon)
    {
        d->mIconCache->insert(key, pix, path);
    }
    return pix;
}

QMovie *KIconLoader::loadMovie(const QString& name, KIconLoader::Group group, int size, QObject *parent) const
{
    QString file = moviePath( name, group, size );
    if (file.isEmpty())
        return 0;
    int dirLen = file.lastIndexOf('/');
    QString icon = iconPath(name, size ? -size : group, true);
    if (!icon.isEmpty() && file.left(dirLen) != icon.left(dirLen))
        return 0;
    QMovie *movie = new QMovie(file, QByteArray(), parent);
    if (!movie->isValid())
    {
        delete movie;
        return 0;
    }
    return movie;
}

QString KIconLoader::moviePath(const QString& name, KIconLoader::Group group, int size) const
{
    if (!d->mpGroups) return QString();

    d->initIconThemes();

    if ( (group < -1 || group >= KIconLoader::LastGroup) && group != KIconLoader::User )
    {
        kDebug(264) << "Illegal icon group: " << group;
        group = KIconLoader::Desktop;
    }
    if (size == 0 && group < 0)
    {
        kDebug(264) << "Neither size nor group specified!";
        group = KIconLoader::Desktop;
    }

    QString file = name + ".mng";
    if (group == KIconLoader::User)
    {
        file = d->mpDirs->findResource("appicon", file);
    }
    else
    {
        if (size == 0)
            size = d->mpGroups[group].size;

        K3Icon icon;

        foreach(KIconThemeNode *themeNode, d->links)
        {
            icon = themeNode->theme->iconPath(file, size, KIconLoader::MatchExact);
            if (icon.isValid())
                break;
        }

        if ( !icon.isValid() )
        {
            foreach(KIconThemeNode *themeNode, d->links)
            {
                icon = themeNode->theme->iconPath(file, size, KIconLoader::MatchBest);
                if (icon.isValid())
                    break;
            }
        }

        file = icon.isValid() ? icon.path : QString();
    }
    return file;
}


QStringList KIconLoader::loadAnimated(const QString& name, KIconLoader::Group group, int size) const
{
    QStringList lst;

    if (!d->mpGroups) return lst;

    d->initIconThemes();

    if ((group < -1) || (group >= KIconLoader::LastGroup))
    {
        kDebug(264) << "Illegal icon group: " << group;
        group = KIconLoader::Desktop;
    }
    if ((size == 0) && (group < 0))
    {
        kDebug(264) << "Neither size nor group specified!";
        group = KIconLoader::Desktop;
    }

    QString file = name + "/0001";
    if (group == KIconLoader::User)
    {
        file = d->mpDirs->findResource("appicon", file + ".png");
    } else
    {
        if (size == 0)
            size = d->mpGroups[group].size;
        K3Icon icon = d->findMatchingIcon(file, size);
        file = icon.isValid() ? icon.path : QString();

    }
    if (file.isEmpty())
        return lst;

    QString path = file.left(file.length()-8);
    DIR* dp = opendir( QFile::encodeName(path) );
    if(!dp)
        return lst;

    KDE_struct_dirent* ep;
    while( ( ep = KDE_readdir( dp ) ) != 0L )
    {
        QString fn(QFile::decodeName(ep->d_name));
        if(!(fn.left(4)).toUInt())
            continue;

        lst += path + fn;
    }
    closedir ( dp );
    lst.sort();
    return lst;
}

KIconTheme *KIconLoader::theme() const
{
    d->initIconThemes();
    if (d->mpThemeRoot) return d->mpThemeRoot->theme;
    return 0L;
}

int KIconLoader::currentSize(KIconLoader::Group group) const
{
    if (!d->mpGroups) return -1;

    if (group < 0 || group >= KIconLoader::LastGroup)
    {
        kDebug(264) << "Illegal icon group: " << group;
        return -1;
    }
    return d->mpGroups[group].size;
}

QStringList KIconLoader::queryIconsByDir( const QString& iconsDir ) const
{
    const QDir dir(iconsDir);
    const QStringList formats = QStringList() << "*.png" << "*.xpm" << "*.svg" << "*.svgz";
    const QStringList lst = dir.entryList(formats, QDir::Files);
    QStringList result;
    QStringList::ConstIterator it;
    for (it=lst.begin(); it!=lst.end(); ++it)
        result += iconsDir + '/' + *it;
    return result;
}

QStringList KIconLoader::queryIconsByContext(int group_or_size,
                                             KIconLoader::Context context) const
{
    d->initIconThemes();

    QStringList result;
    if (group_or_size >= KIconLoader::LastGroup)
    {
        kDebug(264) << "Illegal icon group: " << group_or_size;
        return result;
    }
    int size;
    if (group_or_size >= 0)
        size = d->mpGroups[group_or_size].size;
    else
        size = -group_or_size;

    foreach(KIconThemeNode *themeNode, d->links)
       themeNode->queryIconsByContext(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    QString name;
    QStringList res2, entries;
    QStringList::ConstIterator it;
    for (it=result.constBegin(); it!=result.constEnd(); ++it)
    {
        int n = (*it).lastIndexOf('/');
        if (n == -1)
            name = *it;
        else
            name = (*it).mid(n+1);
        name = d->removeIconExtension(name);
        if (!entries.contains(name))
        {
            entries += name;
            res2 += *it;
        }
    }
    return res2;

}

QStringList KIconLoader::queryIcons(int group_or_size, KIconLoader::Context context) const
{
    d->initIconThemes();

    QStringList result;
    if (group_or_size >= KIconLoader::LastGroup)
    {
        kDebug(264) << "Illegal icon group: " << group_or_size;
        return result;
    }
    int size;
    if (group_or_size >= 0)
        size = d->mpGroups[group_or_size].size;
    else
        size = -group_or_size;

    foreach(KIconThemeNode *themeNode, d->links)
       themeNode->queryIcons(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    QString name;
    QStringList res2, entries;
    QStringList::ConstIterator it;
    for (it=result.constBegin(); it!=result.constEnd(); ++it)
    {
        int n = (*it).lastIndexOf('/');
        if (n == -1)
            name = *it;
        else
            name = (*it).mid(n+1);
        name = d->removeIconExtension(name);
        if (!entries.contains(name))
        {
            entries += name;
            res2 += *it;
        }
    }
    return res2;
}

// used by KIconDialog to find out which contexts to offer in a combobox
bool KIconLoader::hasContext(KIconLoader::Context context) const
{
    foreach(KIconThemeNode *themeNode, d->links)
       if( themeNode->theme->hasContext( context ))
           return true;
    return false;
}

KIconEffect * KIconLoader::iconEffect() const
{
    return &d->mpEffect;
}

bool KIconLoader::alphaBlending(KIconLoader::Group group) const
{
    if (!d->mpGroups) return false;

    if (group < 0 || group >= KIconLoader::LastGroup)
    {
        kDebug(264) << "Illegal icon group: " << group;
        return false;
    }
    return d->mpGroups[group].alphaBlending;
}

// deprecated
QIcon KIconLoader::loadIconSet( const QString& name, KIconLoader::Group g, int s,
                                bool canReturnNull )
{
    QIcon iconset;
    QPixmap tmp = loadIcon(name, g, s, KIconLoader::ActiveState, QStringList(), NULL, canReturnNull);
    iconset.addPixmap( tmp, QIcon::Active, QIcon::On );
    // we don't use QIconSet's resizing anyway
    tmp = loadIcon(name, g, s, KIconLoader::DisabledState, QStringList(), NULL, canReturnNull);
    iconset.addPixmap( tmp, QIcon::Disabled, QIcon::On );
    tmp = loadIcon(name, g, s, KIconLoader::DefaultState, QStringList(), NULL, canReturnNull);
    iconset.addPixmap( tmp, QIcon::Normal, QIcon::On );
    return iconset;
}

// Easy access functions

QPixmap DesktopIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::Desktop, force_size, state, overlays);
}

// deprecated
QIcon DesktopIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet(name, KIconLoader::Desktop, force_size);
}

QPixmap BarIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::Toolbar, force_size, state, overlays);
}

// deprecated
QIcon BarIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::Toolbar, force_size );
}

QPixmap SmallIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::Small, force_size, state, overlays);
}

// deprecated
QIcon SmallIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::Small, force_size );
}

QPixmap MainBarIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::MainToolbar, force_size, state, overlays);
}

// deprecated
QIcon MainBarIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::MainToolbar, force_size );
}

QPixmap UserIcon(const QString& name, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::User, 0, state, overlays);
}

// deprecated
QIcon UserIconSet(const QString& name)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::User );
}

int IconSize(KIconLoader::Group group)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->currentSize(group);
}

QPixmap KIconLoader::unknown()
{
    QPixmap pix;
    if ( QPixmapCache::find("unknown", pix) ) //krazy:exclude=iconnames
            return pix;

    QString path = global()->iconPath("unknown", KIconLoader::Small, true); //krazy:exclude=iconnames
    if (path.isEmpty())
    {
        kDebug(264) << "Warning: Cannot find \"unknown\" icon.";
        pix = QPixmap(32,32);
    } else
    {
        pix.load(path);
        QPixmapCache::insert("unknown", pix); //krazy:exclude=iconnames
    }

    return pix;
}

/*** the global icon loader ***/
K_GLOBAL_STATIC_WITH_ARGS(KIconLoader, globalIconLoader, (KGlobal::mainComponent(), 0))

KIconLoader *KIconLoader::global()
{
    return globalIconLoader;
}

void KIconLoader::newIconLoader()
{
    if ( global() == this) {
        KIconTheme::reconfigure();
    }

    reconfigure( objectName(), d->mpDirs );
}

#include "kiconloader.moc"

