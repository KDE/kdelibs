/* vi: ts=8 sts=4 sw=4
 *
 * kiconloader.cpp: An icon loader for KDE with theming functionality.
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *               2010 Michael Pyne <mpyne@kde.org>
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

#include <sys/types.h>
#include <stdlib.h>     //for abs
#include <unistd.h>     //for readlink
#include <dirent.h>
#include <assert.h>

#include <QtCore/QCache>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QBuffer>
#include <QtCore/QDataStream>
#include <QtCore/QByteArray>
#include <QtCore/QStringBuilder> // % operator for QString
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>
#include <QImage>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>

// kdecore
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kshareddatacache.h>
#include <ksharedconfig.h>
#include <QtDBus/QDBusConnection>
#include <QDBusMessage>

// kdeui
#include "kicontheme.h"
#include "kiconeffect.h"

// Used to make cache keys for icons with no group. Result type is QString*
Q_GLOBAL_STATIC_WITH_ARGS(QString, NULL_EFFECT_FINGERPRINT, (QString::fromLatin1("noeffect")))

/**
 * Checks for relative paths quickly on UNIX-alikes, slowly on everything else.
 */
static bool pathIsRelative(const QString &path)
{
#ifdef Q_OS_UNIX
    return (!path.isEmpty() && path[0] != QChar('/'));
#else
    return QDir::isRelativePath(path);
#endif
}

/**
 * Holds a QPixmap for this process, along with its associated path on disk.
 */
struct PixmapWithPath
{
    QPixmap pixmap;
    QString path;
};

/*** KIconThemeNode: A node in the icon theme dependancy tree. ***/

class KIconThemeNode
{
public:

    KIconThemeNode(KIconTheme *_theme);
    ~KIconThemeNode();

    void queryIcons(QStringList *lst, int size, KIconLoader::Context context) const;
    void queryIconsByContext(QStringList *lst, int size, KIconLoader::Context context) const;
    QString findIcon(const QString& name, int size, KIconLoader::MatchType match) const;
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

QString KIconThemeNode::findIcon(const QString& name, int size,
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
        qDeleteAll(links);
        delete[] mpGroups;
        delete mIconCache;
    }

    /**
     * @internal
     */
    void init(const QString& _appname, const QStringList& extraSearchPaths = QStringList());

    /**
     * @internal
     */
    bool initIconThemes();

    /**
     * @internal
     * tries to find an icon with the name. It tries some extension and
     * match strategies
     */
    QString findMatchingIcon(const QString& name, int size) const;

    /**
     * @internal
     * tries to find an icon with the name.
     * This is one layer above findMatchingIcon -- it also implements generic fallbacks
     * such as generic icons for mimetypes.
     */
    QString findMatchingIconWithGenericFallbacks(const QString& name, int size) const;

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
     * Adds all the default themes from other desktops at the end of
     * the list of icon themes.
     */
    void addExtraDesktopThemes();

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

    /**
     * @internal
     * Used with KIconLoader::loadIcon to convert the given name, size, group,
     * and icon state information to valid states. All parameters except the
     * name can be modified as well to be valid.
     */
    void normalizeIconMetadata(KIconLoader::Group &group, int &size, int &state) const;

    /**
     * @internal
     * Used with KIconLoader::loadIcon to get a base key name from the given
     * icon metadata. Ensure the metadata is normalized first.
     */
    QString makeCacheKey(const QString &name, KIconLoader::Group group, const QStringList &overlays,
                         int size, int state) const;

    /**
     * @internal
     * Creates the QImage for @p path, using SVG rendering as appropriate.
     * @p size is only used for scalable images, but if non-zero non-scalable
     * images will be resized anyways.
     */
    QImage createIconImage(const QString &path, int size = 0);

    /**
     * @internal
     * Adds an QPixmap with its associated path to the shared icon cache.
     */
    void insertCachedPixmapWithPath(const QString &key, const QPixmap &data, const QString &path);

    /**
     * @internal
     * Retrieves the path and pixmap of the given key from the shared
     * icon cache.
     */
    bool findCachedPixmapWithPath(const QString &key, QPixmap &data, QString &path);

    /**
     * Find the given file in the search paths.
     */
    QString locate(const QString& fileName);

    /**
     * @internal
     * React to a global icon theme change
     */
    void _k_refreshIcons(int group);

    KIconLoader *const q;

    QStringList mThemesInTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    QStringList searchPaths;
    KIconEffect mpEffect;
    QList<KIconThemeNode *> links;

    // This shares the icons across all processes
    KSharedDataCache* mIconCache;

    // This caches rendered QPixmaps in just this process.
    QCache<QString, PixmapWithPath> mPixmapCache;

    bool extraDesktopIconsLoaded :1;
    // lazy loading: initIconThemes() is only needed when the "links" list is needed
    // mIconThemeInited is used inside initIconThemes() to init only once
    bool mIconThemeInited :1;
    QString appname;

    void drawOverlays(const KIconLoader *loader, KIconLoader::Group group, int state, QPixmap& pix, const QStringList& overlays);
};

class KIconLoaderGlobalData : public QObject
{
    Q_OBJECT

public:
    KIconLoaderGlobalData() {
        const QStringList genericIconsFiles = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "mime/generic-icons");
        //qDebug() << genericIconsFiles;
        Q_FOREACH(const QString& file, genericIconsFiles) {
            parseGenericIconsFiles(file);
        }

        QDBusConnection::sessionBus().connect( QString(), "/KIconLoader", "org.kde.KIconLoader",
                                                   "iconChanged", this, SIGNAL(iconChanged(int)) );
    }

    void emitChange(KIconLoader::Group group) {
        QDBusMessage message = QDBusMessage::createSignal("/KIconLoader", "org.kde.KIconLoader", "iconChanged" );
        message.setArguments(QList<QVariant>() << int(group));
        QDBusConnection::sessionBus().send(message);
    }

    QString genericIconFor(const QString& icon) const {
        return m_genericIcons.value(icon);
    }

Q_SIGNALS:
    void iconChanged(int group);

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
            //qDebug() << mimeIcon << "->" << genericIcon;
        }
    }
}

Q_GLOBAL_STATIC(KIconLoaderGlobalData, s_globalData)

void KIconLoaderPrivate::drawOverlays(const KIconLoader *iconLoader, KIconLoader::Group group, int state, QPixmap& pix, const QStringList& overlays)
{
    if (overlays.isEmpty()) {
        return;
    }

    const int width = pix.size().width();
    const int height = pix.size().height();
    const int iconSize = qMin(width, height);
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
            startPoint = QPoint(2, height - overlaySize - 2);
            break;
        case 1:
            // bottom right corner
            startPoint = QPoint(width - overlaySize - 2,
                                height - overlaySize - 2);
            break;
        case 2:
            // top right corner
            startPoint = QPoint(width - overlaySize - 2, 2);
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

void KIconLoaderPrivate::_k_refreshIcons(int group)
{
    q->newIconLoader();
    emit q->iconChanged(group);
}

KIconLoader::KIconLoader(const QString& _appname, const QStringList& extraSearchPaths, QObject* parent)
    : QObject(parent)
{
    setObjectName(_appname);
    d = new KIconLoaderPrivate(this);

    connect(s_globalData, SIGNAL(iconChanged(int)), SLOT(_k_refreshIcons(int)));
    d->init(_appname, extraSearchPaths);
}

void KIconLoader::reconfigure(const QString& _appname, const QStringList& extraSearchPaths)
{
    d->mIconCache->clear();
    delete d;
    d = new KIconLoaderPrivate(this);
    d->init(_appname, extraSearchPaths);
}

void KIconLoaderPrivate::init(const QString& _appname, const QStringList& extraSearchPaths)
{
    extraDesktopIconsLoaded=false;
    mIconThemeInited = false;
    mpThemeRoot = 0;

    searchPaths = extraSearchPaths;

    appname = _appname;
    if (appname.isEmpty())
        appname = QCoreApplication::applicationName();

    // Initialize icon cache
    mIconCache = new KSharedDataCache("icon-cache", 10 * 1024 * 1024);
    // Cost here is number of pixels, not size. So this is actually a bit
    // smaller.
    mPixmapCache.setMaxCost(10 * 1024 * 1024);

    // These have to match the order in kicontheme.h
    static const char * const groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", "Panel", "Dialog", 0L };
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    // loading config and default sizes
    initIconThemes();
    KIconTheme *defaultSizesTheme = links.empty() ? 0 : links.first()->theme;
    mpGroups = new KIconGroup[(int) KIconLoader::LastGroup];
    for (KIconLoader::Group i = KIconLoader::FirstGroup; i < KIconLoader::LastGroup; ++i) {
        if (groups[i] == 0L) {
            break;
        }

        KConfigGroup cg(config, QLatin1String(groups[i]) + "Icons");
        mpGroups[i].size = cg.readEntry("Size", 0);
        if (QPixmap::defaultDepth() > 8) {
            mpGroups[i].alphaBlending = cg.readEntry("AlphaBlending", true);
        } else {
            mpGroups[i].alphaBlending = false;
        }

        if (!mpGroups[i].size && defaultSizesTheme) {
            mpGroups[i].size = defaultSizesTheme->defaultSize(i);
        }
    }
}

bool KIconLoaderPrivate::initIconThemes()
{
    if (mIconThemeInited) {
        // If mpThemeRoot isn't 0 then initing has succeeded
        return (mpThemeRoot != 0);
    }
    //qDebug();
    mIconThemeInited = true;

    // Add the default theme and its base themes to the theme tree
    KIconTheme *def = new KIconTheme(KIconTheme::current(), appname);
    if (!def->isValid())
    {
        delete def;
        // warn, as this is actually a small penalty hit
        qDebug() << "Couldn't find current icon theme, falling back to default.";
        def = new KIconTheme(KIconTheme::defaultThemeName(), appname);
        if (!def->isValid())
        {
            qWarning() << "Error: standard icon theme" << KIconTheme::defaultThemeName() << "not found!";
            delete def;
            return false;
        }
    }
    mpThemeRoot = new KIconThemeNode(def);
    mThemesInTree.append(def->internalName());
    links.append(mpThemeRoot);
    addBaseThemes(mpThemeRoot, appname);

    // Insert application specific themes at the top.
    searchPaths.append(appname + "/pics");

    // Add legacy icon dirs.
    searchPaths.append("icons"); // was xdgdata-icon in KStandardDirs
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    searchPaths.append("pixmaps"); // was xdgdata-pixmaps in KStandardDirs

#ifndef NDEBUG
    QString dbgString = "Theme tree: ";
    mpThemeRoot->printTree(dbgString);
    qDebug() << dbgString;
#endif

    return true;
}

KIconLoader::~KIconLoader()
{
    delete d;
}

QStringList KIconLoader::searchPaths() const
{
    return d->searchPaths;
}

void KIconLoader::addAppDir(const QString& appname)
{
    d->initIconThemes();

    d->searchPaths.append(appname + "/pics");
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

void KIconLoaderPrivate::addExtraDesktopThemes()
{
    if ( extraDesktopIconsLoaded ) return;

    initIconThemes();

    QStringList list;
    const QStringList icnlibs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "icons", QStandardPaths::LocateDirectory);
    QStringList::ConstIterator it;
    char buf[1000];
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
    {
        QDir dir(*it);
        if (!dir.exists())
            continue;
        const QStringList lst = dir.entryList(QStringList( "default.*" ), QDir::Dirs);
        QStringList::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2)
        {
            if (!QFile::exists(*it + *it2 + "/index.desktop")
                && !QFile::exists(*it + *it2 + "/index.theme"))
                continue;
            const int r = readlink(QFile::encodeName(*it + *it2) , buf, sizeof(buf)-1);
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
        addThemeByName(*it, "");
    }

    extraDesktopIconsLoaded=true;

}

void KIconLoader::drawOverlays(const QStringList &overlays, QPixmap &pixmap, KIconLoader::Group group, int state) const
{
    d->drawOverlays(this, group, state, pixmap, overlays);
}

QString KIconLoaderPrivate::removeIconExtension(const QString &name) const
{
    if (name.endsWith(QLatin1String(".png"))
        || name.endsWith(QLatin1String(".xpm"))
        || name.endsWith(QLatin1String(".svg"))) {
        return name.left(name.length() - 4);
    } else if (name.endsWith(QLatin1String(".svgz"))) {
        return name.left(name.length() - 5);
    }

    return name;
}

void KIconLoaderPrivate::normalizeIconMetadata(KIconLoader::Group &group, int &size, int &state) const
{
    if ((state < 0) || (state >= KIconLoader::LastState))
    {
        qWarning() << "Illegal icon state:" << state;
        state = KIconLoader::DefaultState;
    }

    if (size < 0) {
        size = 0;
    }

    // For "User" icons, bail early since the size should be based on the size on disk,
    // which we've already checked.
    if (group == KIconLoader::User) {
        return;
    }

    if ((group < -1) || (group >= KIconLoader::LastGroup))
    {
        qWarning() << "Illegal icon group:" << group;
        group = KIconLoader::Desktop;
    }

    // If size == 0, use default size for the specified group.
    if (size == 0)
    {
        if (group < 0)
        {
            qWarning() << "Neither size nor group specified!";
            group = KIconLoader::Desktop;
        }
        size = mpGroups[group].size;
    }
}

QString KIconLoaderPrivate::makeCacheKey(const QString &name, KIconLoader::Group group,
                                         const QStringList &overlays, int size, int state) const
{
    // The KSharedDataCache is shared so add some namespacing. The following code
    // uses QStringBuilder (new in Qt 4.6)

    return (group == KIconLoader::User
                  ? QLatin1Literal("$kicou_")
                  : QLatin1Literal("$kico_"))
           % name
           % QLatin1Char('_')
           % QString::number(size)
           % QLatin1Char('_')
           % overlays.join("_")
           % ( group >= 0 ? mpEffect.fingerprint(group, state)
                          : *NULL_EFFECT_FINGERPRINT());
}

QImage KIconLoaderPrivate::createIconImage(const QString &path, int size)
{
    QImageReader reader(path);

    if (!reader.canRead()) {
        return QImage();
    }

    if (size != 0) {
        reader.setScaledSize(QSize(size, size));
    }

    return reader.read();
}

void KIconLoaderPrivate::insertCachedPixmapWithPath(
    const QString &key,
    const QPixmap &data,
    const QString &path = QString())
{
    // Even if the pixmap is null, we add it to the caches so that we record
    // the fact that whatever icon led to us getting a null pixmap doesn't
    // exist.

    QBuffer output;
    output.open(QIODevice::WriteOnly);

    QDataStream outputStream(&output);
    outputStream.setVersion(QDataStream::Qt_4_6);

    outputStream << path;

    // Convert the QPixmap to PNG. This is actually done by Qt's own operator.
    outputStream << data;

    output.close();

    // The byte array contained in the QBuffer is what we want in the cache.
    mIconCache->insert(key, output.buffer());

    // Also insert the object into our process-local cache for even more
    // speed.
    PixmapWithPath *pixmapPath = new PixmapWithPath;
    pixmapPath->pixmap = data;
    pixmapPath->path = path;

    mPixmapCache.insert(key, pixmapPath, data.width() * data.height() + 1);
}

bool KIconLoaderPrivate::findCachedPixmapWithPath(const QString &key, QPixmap &data, QString &path)
{
    // If the pixmap is present in our local process cache, use that since we
    // don't need to decompress and upload it to the X server/graphics card.
    const PixmapWithPath *pixmapPath = mPixmapCache.object(key);
    if (pixmapPath) {
        path = pixmapPath->path;
        data = pixmapPath->pixmap;

        return true;
    }

    // Otherwise try to find it in our shared memory cache since that will
    // be quicker than the disk, especially for SVGs.
    QByteArray result;

    if (!mIconCache->find(key, &result) || result.isEmpty()) {
        return false;
    }

    QBuffer buffer;
    buffer.setBuffer(&result);
    buffer.open(QIODevice::ReadOnly);

    QDataStream inputStream(&buffer);
    inputStream.setVersion(QDataStream::Qt_4_6);

    QString tempPath;
    inputStream >> tempPath;

    if (inputStream.status() == QDataStream::Ok) {
        QPixmap tempPixmap;
        inputStream >> tempPixmap;

        if (inputStream.status() == QDataStream::Ok) {
            data = tempPixmap;
            path = tempPath;

            // Since we're here we didn't have a QPixmap cache entry, add one now.
            PixmapWithPath *newPixmapWithPath = new PixmapWithPath;
            newPixmapWithPath->pixmap = data;
            newPixmapWithPath->path = path;

            mPixmapCache.insert(key, newPixmapWithPath, data.width() * data.height() + 1);

            return true;
        }
    }

    return false;
}

QString KIconLoaderPrivate::findMatchingIconWithGenericFallbacks(const QString& name, int size) const
{
    QString path = findMatchingIcon(name, size);
    if (!path.isEmpty())
        return path;

    const QString genericIcon = s_globalData()->genericIconFor(name);
    if (!genericIcon.isEmpty()) {
        path = findMatchingIcon(genericIcon, size);
    }
    return path;
}

QString KIconLoaderPrivate::findMatchingIcon(const QString& name, int size) const
{
    const_cast<KIconLoaderPrivate*>(this)->initIconThemes();

    QString path;

    const char * const ext[4] = { ".png", ".svgz", ".svg", ".xpm" };
    bool genericFallback = name.endsWith(QLatin1String("-x-generic"));

    // Do two passes through themeNodes.
    //
    // The first pass looks for an exact match in each themeNode one after the other.
    // If one is found and it is an app icon then return that icon.
    //
    // In the next pass (assuming the first pass failed), it looks for exact matches
    // and then generic fallbacks in each themeNode one after the other
    //
    // The reasoning is that application icons should always match exactly, all other
    // icons may fallback. Since we do not know what the context is here when we start
    // looking for it, we can only go by the path found.
    foreach (KIconThemeNode *themeNode, links) {
        for (int i = 0 ; i < 4 ; i++) {
            path = themeNode->theme->iconPath(name + ext[i], size, KIconLoader::MatchExact);
            if (!path.isEmpty()) {
                return path;
            }

            path = themeNode->theme->iconPath(name + ext[i], size, KIconLoader::MatchBest);
            if (!path.isEmpty()) {
                return path;
            }
        }

        if (!path.isEmpty() && path.contains("/apps/")) {
            return path;
        }
    }

    foreach (KIconThemeNode *themeNode, links) {
        QString currentName = name;

        while (!currentName.isEmpty()) {
            //qDebug() << "Looking up" << currentName;

            for (int i = 0 ; i < 4 ; i++)
            {
                path = themeNode->theme->iconPath(currentName + ext[i], size, KIconLoader::MatchExact);
                if (!path.isEmpty())
                    return path;

                path = themeNode->theme->iconPath(currentName + ext[i], size, KIconLoader::MatchBest);
                if (!path.isEmpty())
                    return path;
            }
            //qDebug() << "Looking up" << currentName;

            if (genericFallback) {
                // we already tested the base name
                break;
            }

            int rindex = currentName.lastIndexOf('-');
            if (rindex > 1) { // > 1 so that we don't split x-content or x-epoc
                currentName.truncate(rindex);

                if (currentName.endsWith(QLatin1String("-x")))
                    currentName.chop(2);
            } else {
                // From update-mime-database.c
                static const QSet<QString> mediaTypes = QSet<QString>()
                    << "text" << "application" << "image" << "audio"
                    << "inode" << "video" << "message" << "model" << "multipart"
                    << "x-content" << "x-epoc";
                // Shared-mime-info spec says:
                // "If [generic-icon] is not specified then the mimetype is used to generate the
                // generic icon by using the top-level media type (e.g. "video" in "video/ogg")
                // and appending "-x-generic" (i.e. "video-x-generic" in the previous example)."
                if (mediaTypes.contains(currentName)) {
                    currentName += QLatin1String("-x-generic");
                    genericFallback = true;
                } else {
                    break;
                }
            }
        }
    }
    return path;
}

inline QString KIconLoaderPrivate::unknownIconPath( int size ) const
{
    QString path = findMatchingIcon(QLatin1String("unknown"), size);
    if (path.isEmpty())
    {
        qDebug() << "Warning: could not find \"unknown\" icon for size" << size;
        return QString();
    }
    return path;
}

QString KIconLoaderPrivate::locate(const QString& fileName)
{
    Q_FOREACH(const QString& dir, searchPaths) {
        const QString path = dir + '/' + fileName;
        if (QDir(dir).isAbsolute()) {
            if (QFile::exists(path))
                return path;
        } else {
            const QString fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, path);
            if (!fullPath.isEmpty())
                return fullPath;
        }
    }
    return QString();
}

// Finds the absolute path to an icon.

QString KIconLoader::iconPath(const QString& _name, int group_or_size,
                              bool canReturnNull) const
{
    if (!d->initIconThemes()) {
        return QString();
    }

    if (_name.isEmpty() || !pathIsRelative(_name))
    {
        // we have either an absolute path or nothing to work with
        return _name;
    }

    QString name = d->removeIconExtension( _name );

    QString path;
    if (group_or_size == KIconLoader::User)
    {
        path = d->locate(name + QLatin1String(".png"));
        if (path.isEmpty())
            path = d->locate(name + QLatin1String(".svgz"));
        if (path.isEmpty())
            path = d->locate(name + QLatin1String(".svg"));
        if (path.isEmpty())
            path = d->locate(name + QLatin1String(".xpm"));
        return path;
    }

    if (group_or_size >= KIconLoader::LastGroup)
    {
        qDebug() << "Illegal icon group:" << group_or_size;
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

    path = d->findMatchingIconWithGenericFallbacks(name, size);

    if (path.isEmpty())
    {
        // Try "User" group too.
        path = iconPath(name, KIconLoader::User, true);
        if (!path.isEmpty() || canReturnNull)
            return path;

        return d->unknownIconPath(size);
    }
    return path;
}

QPixmap KIconLoader::loadMimeTypeIcon( const QString& _iconName, KIconLoader::Group group, int size,
                                       int state, const QStringList& overlays, QString *path_store ) const
{
    QString iconName = _iconName;
    const int slashindex = iconName.indexOf(QLatin1Char('/'));
    if (slashindex != -1) {
        iconName[slashindex] = QLatin1Char('-');
    }

    if ( !d->extraDesktopIconsLoaded )
    {
        const QPixmap pixmap = loadIcon( iconName, group, size, state, overlays, path_store, true );
        if (!pixmap.isNull() ) {
            return pixmap;
        }
        d->addExtraDesktopThemes();
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
    bool favIconOverlay = false;

    if (size < 0 || _name.isEmpty())
        return QPixmap();

    /*
     * This method works in a kind of pipeline, with the following steps:
     * 1. Sanity checks.
     * 2. Convert _name, group, size, etc. to a key name.
     * 3. Check if the key is already cached.
     * 4. If not, initialize the theme and find/load the icon.
     * 4a Apply overlays
     * 4b Re-add to cache.
     */

    // Special case for absolute path icons.
    if (name.startsWith(QLatin1String("favicons/")))
    {
       favIconOverlay = true;
       name = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + '/' + name + ".png";
    }

    bool absolutePath = !pathIsRelative(name);
    if (!absolutePath) {
        name = d->removeIconExtension(name);
    }

    // Don't bother looking for an icon with no name.
    if (name.isEmpty()) {
        return QPixmap();
    }

    // May modify group, size, or state. This function puts them into sane
    // states.
    d->normalizeIconMetadata(group, size, state);

    // See if the image is already cached.
    QString key = d->makeCacheKey(name, group, overlays, size, state);
    QPixmap pix;
    bool iconWasUnknown = false;
    QString path;

    // icon.path would be empty for "unknown" icons, which should be searched for
    // anew each time.
    if (d->findCachedPixmapWithPath(key, pix, path) && !path.isEmpty()) {
        if (path_store) {
            *path_store = path;
        }

        return pix;
    }

    // Image is not cached... go find it and apply effects.
    if (!d->initIconThemes()) {
        return QPixmap();
    }

    favIconOverlay = favIconOverlay && size > 22;

    // First we look for non-User icons. If we don't find one we'd search in
    // the User space anyways...
    if (group != KIconLoader::User) {
        if (absolutePath && !favIconOverlay)
        {
            path = name;
        }
        else
        {
            path = d->findMatchingIconWithGenericFallbacks(favIconOverlay ? QString("text-html") : name, size);
        }
    }

    if (path.isEmpty()) {
        // We do have a "User" icon, or we couldn't find the non-User one.
        path = (absolutePath) ? name :
                                iconPath(name, KIconLoader::User, canReturnNull);
    }

    // Still can't find it? Use "unknown" if we can't return null.
    // We keep going in the function so we can ensure this result gets cached.
    if (path.isEmpty() && !canReturnNull) {
        path = d->unknownIconPath(size);
        iconWasUnknown = true;
    }

    QImage img;
    if (!path.isEmpty()) {
        img = d->createIconImage(path, size);
    }

    if (group >= 0)
    {
        img = d->mpEffect.apply(img, group, state);
    }

    if (favIconOverlay)
    {
        QImage favIcon(name, "PNG");
        if (!favIcon.isNull()) // if favIcon not there yet, don't try to blend it
        {
            QPainter p(&img);

            // Align the favicon overlay
            QRect r(favIcon.rect());
            r.moveBottomRight(img.rect().bottomRight());
            r.adjust(-1, -1, -1, -1); // Move off edge

            // Blend favIcon over img.
            p.drawImage(r, favIcon);
        }
    }

    pix = QPixmap::fromImage(img);

    // TODO: If we make a loadIcon that returns the image we can convert
    // drawOverlays to use the image instead of pixmaps as well so we don't
    // have to transfer so much to the graphics card.
    d->drawOverlays(this, group, state, pix, overlays);

    // Don't add the path to our unknown icon to the cache, only cache the
    // actual image.
    if (iconWasUnknown) {
        path.clear();
    }

    d->insertCachedPixmapWithPath(key, pix, path);

    if (path_store) {
        *path_store = path;
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
        qDebug() << "Illegal icon group:" << group;
        group = KIconLoader::Desktop;
    }
    if (size == 0 && group < 0)
    {
        qDebug() << "Neither size nor group specified!";
        group = KIconLoader::Desktop;
    }

    QString file = name + ".mng";
    if (group == KIconLoader::User)
    {
        file = d->locate(file);
    }
    else
    {
        if (size == 0)
            size = d->mpGroups[group].size;

        QString path;

        foreach(KIconThemeNode *themeNode, d->links)
        {
            path = themeNode->theme->iconPath(file, size, KIconLoader::MatchExact);
            if (!path.isEmpty())
                break;
        }

        if (path.isEmpty())
        {
            foreach(KIconThemeNode *themeNode, d->links)
            {
                path = themeNode->theme->iconPath(file, size, KIconLoader::MatchBest);
                if (!path.isEmpty())
                    break;
            }
        }

        file = path;
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
        qDebug() << "Illegal icon group: " << group;
        group = KIconLoader::Desktop;
    }
    if ((size == 0) && (group < 0))
    {
        qDebug() << "Neither size nor group specified!";
        group = KIconLoader::Desktop;
    }

    QString file = name + "/0001";
    if (group == KIconLoader::User)
    {
        file = d->locate(file + ".png");
    } else
    {
        if (size == 0)
            size = d->mpGroups[group].size;
        file = d->findMatchingIcon(file, size);
    }
    if (file.isEmpty())
        return lst;

    QString path = file.left(file.length()-8);
    QDir dir(QFile::encodeName(path));
    if(!dir.exists())
        return lst;

    foreach (const QString &entry, dir.entryList()) {
        if(!(entry.left(4)).toUInt())
            continue;

        lst += path + entry;
    }
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
        qDebug() << "Illegal icon group:" << group;
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
        qDebug() << "Illegal icon group:" << group_or_size;
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
        qDebug() << "Illegal icon group:" << group_or_size;
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
    d->initIconThemes();

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
        qDebug() << "Illegal icon group:" << group;
        return false;
    }
    return d->mpGroups[group].alphaBlending;
}

// deprecated
#ifndef KDE_NO_DEPRECATED
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
#endif

// Easy access functions

QPixmap DesktopIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::Desktop, force_size, state, overlays);
}

// deprecated
#ifndef KDE_NO_DEPRECATED
QIcon DesktopIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet(name, KIconLoader::Desktop, force_size);
}
#endif

QPixmap BarIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::Toolbar, force_size, state, overlays);
}

// deprecated
#ifndef KDE_NO_DEPRECATED
QIcon BarIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::Toolbar, force_size );
}
#endif

QPixmap SmallIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::Small, force_size, state, overlays);
}

// deprecated
#ifndef KDE_NO_DEPRECATED
QIcon SmallIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::Small, force_size );
}
#endif

QPixmap MainBarIcon(const QString& name, int force_size, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::MainToolbar, force_size, state, overlays);
}

// deprecated
#ifndef KDE_NO_DEPRECATED
QIcon MainBarIconSet(const QString& name, int force_size)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::MainToolbar, force_size );
}
#endif

QPixmap UserIcon(const QString& name, int state, const QStringList &overlays)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIcon(name, KIconLoader::User, 0, state, overlays);
}

// deprecated
#ifndef KDE_NO_DEPRECATED
QIcon UserIconSet(const QString& name)
{
    KIconLoader *loader = KIconLoader::global();
    return loader->loadIconSet( name, KIconLoader::User );
}
#endif

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
        qDebug() << "Warning: Cannot find \"unknown\" icon.";
        pix = QPixmap(32,32);
    } else
    {
        pix.load(path);
        QPixmapCache::insert("unknown", pix); //krazy:exclude=iconnames
    }

    return pix;
}

/*** the global icon loader ***/
Q_GLOBAL_STATIC(KIconLoader, globalIconLoader)

KIconLoader *KIconLoader::global()
{
    return globalIconLoader();
}

#ifndef KDE_NO_DEPRECATED
void KIconLoader::newIconLoader()
{
    if ( global() == this) {
        KIconTheme::reconfigure();
    }

    reconfigure(objectName());
    emit iconLoaderSettingsChanged();
}
#endif

void KIconLoader::emitChange(KIconLoader::Group g)
{
    s_globalData->emitChange(g);
}

#include <kiconengine_p.h>
QIcon KDE::icon(const QString& iconName, KIconLoader* iconLoader)
{
    return QIcon(new KIconEngine(iconName, iconLoader ? iconLoader : KIconLoader::global()));
}

QIcon KDE::icon(const QString& iconName, const QStringList& overlays, KIconLoader* iconLoader)
{
    return QIcon(new KIconEngine(iconName, iconLoader ? iconLoader : KIconLoader::global(), overlays));
}

#include "kiconloader.moc"
#include "moc_kiconloader.moc"
