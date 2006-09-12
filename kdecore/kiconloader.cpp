/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kiconloader.cpp: An icon loader for KDE with theming functionality.
 */

#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qicon.h>
#include <qbitmap.h>
#include <QHash>
#include <QPainter>
#include <QMovie>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kinstance.h>
#include <ksvgrenderer.h>

#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <sys/types.h>
#include <stdlib.h>	//for abs
#include <unistd.h>     //for readlink
#include <dirent.h>
#include <assert.h>

/*** KIconThemeNode: A node in the icon theme dependancy tree. ***/

class KIconThemeNode
{
public:

    KIconThemeNode(KIconTheme *_theme);
    ~KIconThemeNode();

    void queryIcons(QStringList *lst, int size, K3Icon::Context context) const;
    void queryIconsByContext(QStringList *lst, int size, K3Icon::Context context) const;
    K3Icon findIcon(const QString& name, int size, K3Icon::MatchType match) const;
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
				int size, K3Icon::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIcons(size, context);
}

void KIconThemeNode::queryIconsByContext(QStringList *result,
				int size, K3Icon::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIconsByContext(size, context);
}

K3Icon KIconThemeNode::findIcon(const QString& name, int size,
			       K3Icon::MatchType match) const
{
    return theme->iconPath(name, size, match);
}


/*** KIconGroup: Icon type description. ***/

struct KIconGroup
{
    int size;
    bool dblPixels;
    bool alphaBlending;
};


/*** d pointer for KIconLoader. ***/

struct KIconLoaderPrivate
{
    QStringList mThemesInTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconEffect mpEffect;
    QHash<QString, QImage*> imgDict;
    QImage lastImage; // last loaded image without effect applied
    QString lastImageKey; // key for icon without effect
    int lastIconType; // see K3Icon::type
    int lastIconThreshold; // see K3Icon::threshold
    QList<KIconThemeNode *> links;
    bool extraDesktopIconsLoaded :1;
    bool delayedLoading :1;
};

#define KICONLOADER_CHECKS
#ifdef KICONLOADER_CHECKS
// Keep a list of recently created and destroyed KIconLoader instances in order
// to detect bugs like #68528.
struct KIconLoaderDebug
    {
    KIconLoaderDebug( KIconLoader* l, const QString& a )
        : loader( l ), appname( a ), valid( true )
        {}
    KIconLoaderDebug() {}; // this QValueList feature annoys me
    KIconLoader* loader;
    QString appname;
    bool valid;
    QString delete_bt;
    };

static QList< KIconLoaderDebug > *kiconloaders;
#endif

/*** KIconLoader: the icon loader ***/

KIconLoader::KIconLoader(const QString& _appname, KStandardDirs *_dirs)
{
#ifdef KICONLOADER_CHECKS
    if( kiconloaders == NULL )
        kiconloaders = new QList< KIconLoaderDebug>();
    // check for the (very unlikely case) that new KIconLoader gets allocated
    // at exactly same address like some previous one
    for( QList< KIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         )
        {
        if( (*it).loader == this )
            it = kiconloaders->erase( it );
        else
            ++it;
        }
    kiconloaders->append( KIconLoaderDebug( this, _appname ));
#endif
    init( _appname, _dirs );
}

void KIconLoader::reconfigure( const QString& _appname, KStandardDirs *_dirs )
{
    delete d;
    init( _appname, _dirs );
}

void KIconLoader::init( const QString& _appname, KStandardDirs *_dirs )
{
    d = new KIconLoaderPrivate;
    d->extraDesktopIconsLoaded=false;
    d->delayedLoading=false;

    if (_dirs)
	d->mpDirs = _dirs;
    else
	d->mpDirs = KGlobal::dirs();

    // If this is unequal to 0, the iconloader is initialized
    // successfully.
    d->mpThemeRoot = 0L;

    QString appname = _appname;
    if (appname.isEmpty())
	appname = KGlobal::instance()->instanceName();

    // Add the default theme and its base themes to the theme tree
    KIconTheme *def = new KIconTheme(KIconTheme::current(), appname);
    if (!def->isValid())
    {
	delete def;
        // warn, as this is actually a small penalty hit
        kDebug(264) << "Couldn't find current icon theme, falling back to default." << endl;
	def = new KIconTheme(KIconTheme::defaultThemeName(), appname);
        if (!def->isValid())
        {
            kError(264) << "Error: standard icon theme"
                         << " \"" << KIconTheme::defaultThemeName() << "\" "
                         << " not found!" << endl;
            d->mpGroups=0L;
            return;
        }
    }
    d->mpThemeRoot = new KIconThemeNode(def);
    d->links.append(d->mpThemeRoot);
    d->mThemesInTree += KIconTheme::current();
    addBaseThemes(d->mpThemeRoot, appname);

    // These have to match the order in kicontheme.h
    static const char * const groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", "Panel", 0L };
    KConfig *config = KGlobal::config();

    // loading config and default sizes
    d->mpGroups = new KIconGroup[(int) K3Icon::LastGroup];
    for (K3Icon::Group i=K3Icon::FirstGroup; i<K3Icon::LastGroup; i++)
    {
	if (groups[i] == 0L)
	    break;

	KConfigGroup cg(config, QLatin1String(groups[i]) + "Icons");
	d->mpGroups[i].size = cg.readEntry("Size", 0);
	d->mpGroups[i].dblPixels = cg.readEntry("DoublePixels", false);
	if (QPixmap::defaultDepth()>8)
	    d->mpGroups[i].alphaBlending = cg.readEntry("AlphaBlending", true);
	else
	    d->mpGroups[i].alphaBlending = false;

	if (!d->mpGroups[i].size)
	    d->mpGroups[i].size = d->mpThemeRoot->theme->defaultSize(i);
    }

    // Insert application specific themes at the top.
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/pics/");
    // ################## KDE4: consider removing the toolbar directory
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/toolbar/");

    // Add legacy icon dirs.
    QStringList dirs;
    dirs += d->mpDirs->resourceDirs("icon");
    dirs += d->mpDirs->resourceDirs("pixmap");
    dirs += d->mpDirs->resourceDirs("xdgdata-icon");
    dirs += "/usr/share/pixmaps";
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it)
	d->mpDirs->addResourceDir("appicon", *it);

#ifndef NDEBUG
    QString dbgString = "Theme tree: ";
    d->mpThemeRoot->printTree(dbgString);
    kDebug(264) << dbgString << endl;
#endif
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
    /* antlarr: There's no need to delete d->mpThemeRoot as it's already
       deleted when the elements of d->links are deleted */
    d->mpThemeRoot=0;
    delete[] d->mpGroups;
    qDeleteAll(d->imgDict);
    qDeleteAll(d->links);
    delete d;
}

void KIconLoader::enableDelayedIconSetLoading( bool enable )
{
    d->delayedLoading = enable;
}

bool KIconLoader::isDelayedIconSetLoadingEnabled() const
{
    return d->delayedLoading;
}

void KIconLoader::addAppDir(const QString& appname)
{
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/pics/");
    // ################## KDE4: consider removing the toolbar directory
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/toolbar/");
    addAppThemes(appname);
}

void KIconLoader::addAppThemes(const QString& appname)
{
    if ( KIconTheme::current() != KIconTheme::defaultThemeName() )
    {
        KIconTheme *def = new KIconTheme(KIconTheme::current(), appname);
        if (def->isValid())
        {
            KIconThemeNode* node = new KIconThemeNode(def);
            d->links.append(node);
            addBaseThemes(node, appname);
        }
        else
            delete def;
    }

    KIconTheme *def = new KIconTheme(KIconTheme::defaultThemeName(), appname);
    KIconThemeNode* node = new KIconThemeNode(def);
    d->links.append(node);
    addBaseThemes(node, appname);
}

void KIconLoader::addBaseThemes(KIconThemeNode *node, const QString &appname)
{
    QStringList lst = node->theme->inherits();
    QStringList::ConstIterator it;

    for (it=lst.begin(); it!=lst.end(); ++it)
    {
	if( d->mThemesInTree.contains(*it) && (*it) != "hicolor")
	    continue;
	KIconTheme *theme = new KIconTheme(*it,appname);
	if (!theme->isValid()) {
	    delete theme;
	    continue;
	}
        KIconThemeNode *n = new KIconThemeNode(theme);
	d->mThemesInTree.append(*it);
	addBaseThemes(n, appname);
	d->links.append(n);
    }
}

void KIconLoader::addExtraDesktopThemes()
{
    if ( d->extraDesktopIconsLoaded ) return;

    QStringList list;
    QStringList icnlibs = KGlobal::dirs()->resourceDirs("icon");
    QStringList::ConstIterator it;
    char buf[1000];
    int r;
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
    {
	QDir dir(*it);
	if (!dir.exists())
	    continue;
	QStringList lst = dir.entryList(QStringList( "default.*" ), QDir::Dirs);
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
	      QDir dir2( buf );
	      QString themeName=dir2.dirName();

	      if (!list.contains(themeName))
		list.append(themeName);
	    }
	}
    }

    for (it=list.begin(); it!=list.end(); ++it)
    {
	if ( d->mThemesInTree.contains(*it) )
		continue;
	if ( *it == QString("default.kde") ) continue;

	KIconTheme *def = new KIconTheme( *it, "" );
	KIconThemeNode* node = new KIconThemeNode(def);
	d->mThemesInTree.append(*it);
	d->links.append(node);
	addBaseThemes(node, "" );
    }

    d->extraDesktopIconsLoaded=true;

}

bool KIconLoader::extraDesktopThemesAdded() const
{
    return d->extraDesktopIconsLoaded;
}

QString KIconLoader::removeIconExtension(const QString &name) const
{
    int extensionLength=0;

    QString ext = name.right(4);

    if (ext == ".png" || ext == ".xpm" )
      extensionLength=4;
    else
    {
	static const QString &svgz_ext = KGlobal::staticQString(".svgz");
	if (name.endsWith(svgz_ext))
	    extensionLength=5;
	else if (ext == ".svg")
	    extensionLength=4;
    }

    if ( extensionLength > 0 )
    {
#ifndef NDEBUG
	kDebug(264) << "Application " << KGlobal::instance()->instanceName()
                     << " loads icon " << name << " with extension." << endl;
#endif

	return name.left(name.length() - extensionLength);
    }
    return name;
}


K3Icon KIconLoader::findMatchingIcon(const QString& name, int size) const
{
    K3Icon icon;

    const char * const ext[4] = { ".png", "svgz", ".svg", ".xpm" };

    /* antlarr: Multiple inheritance is a broken concept on icon themes, so
       the next code doesn't support it on purpose because in fact, it was
       never supported at all. This makes the order in which we look for an
       icon as:

       png, svgz, svg, xpm exact match
       next theme in inheritance tree : png, svgz, svg, xpm exact match
       next theme in inheritance tree : png, svgz, svg, xpm exact match
       and so on

       And if the icon couldn't be found then it tries best match in the same
       order.

       */
    foreach(KIconThemeNode *themeNode, d->links)
    {
	for (int i = 0 ; i < 4 ; i++)
	{
	    icon = themeNode->theme->iconPath(name + ext[i], size, K3Icon::MatchExact);
	    if (icon.isValid())
		return icon;
	}

    }

    foreach(KIconThemeNode *themeNode, d->links)
    {
	for (int i = 0 ; i < 4 ; i++)
	{
	    icon = themeNode->theme->iconPath(name + ext[i], size, K3Icon::MatchBest);
	    if (icon.isValid())
		return icon;
	}

    }

    return icon;
}

inline QString KIconLoader::unknownIconPath( int size ) const
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
    if (d->mpThemeRoot == 0L)
	return QString();

    if (!QDir::isRelativePath(_name))
	return _name;

    QString name = removeIconExtension( _name );

    QString path;
    if (group_or_size == K3Icon::User)
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

    if (group_or_size >= K3Icon::LastGroup)
    {
	kDebug(264) << "Illegal icon group: " << group_or_size << endl;
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
            return unknownIconPath(size);
    }

    K3Icon icon = findMatchingIcon(name, size);

    if (!icon.isValid())
    {
	// Try "User" group too.
	path = iconPath(name, K3Icon::User, true);
	if (!path.isEmpty() || canReturnNull)
	    return path;

        return unknownIconPath(size);
    }
    return icon.path;
}

QPixmap KIconLoader::loadMimeTypeIcon( const QString& iconName, K3Icon::Group group, int size,
                                       int state, QString *path_store ) const
{
    if ( !d->extraDesktopIconsLoaded )
    {
        QPixmap pixmap = loadIcon( iconName, group, size, state, path_store, true );
        if (!pixmap.isNull() ) return pixmap;
        const_cast<KIconLoader *>(this)->addExtraDesktopThemes();
    }
    return loadIcon( iconName, group, size, state, path_store, false );
}

QPixmap KIconLoader::loadIcon(const QString& _name, K3Icon::Group group, int size,
                              int state, QString *path_store, bool canReturnNull) const
{
    QString name = _name;
    QPixmap pix;
    QString key;
    bool absolutePath=false, favIconOverlay=false;

    if (d->mpThemeRoot == 0L)
	return pix;

    // Special case for absolute path icons.
    if (name.startsWith("favicons/"))
    {
       favIconOverlay = true;
       name = KStandardDirs::locateLocal("cache", name+".png");
    }
    if (!QDir::isRelativePath(name)) absolutePath=true;

    static const QString &str_unknown = KGlobal::staticQString("unknown");

    // Special case for "User" icons.
    if (group == K3Icon::User)
    {
	key = "$kicou_";
        key += QString::number(size); key += '_';
	key += name;
	bool inCache = QPixmapCache::find(key, pix);
	if (inCache && (path_store == 0L))
	    return pix;

	QString path = (absolutePath) ? name :
			iconPath(name, K3Icon::User, canReturnNull);
	if (path.isEmpty())
	{
	    if (canReturnNull)
		return pix;
	    // We don't know the desired size: use small
	    path = iconPath(str_unknown, K3Icon::Small, true);
	    if (path.isEmpty())
	    {
		kDebug(264) << "Warning: Cannot find \"unknown\" icon." << endl;
		return pix;
	    }
	}

	if (path_store != 0L)
	    *path_store = path;
	if (inCache)
	    return pix;
	QImage img(path);
	if (size != 0)
	    img=img.scaled(size,size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	pix = QPixmap::fromImage(img);
	QPixmapCache::insert(key, pix);
	return pix;
    }

    // Regular case: Check parameters

    if ((group < -1) || (group >= K3Icon::LastGroup))
    {
	kDebug(264) << "Illegal icon group: " << group << endl;
	group = K3Icon::Desktop;
    }

    int overlay = (state & K3Icon::OverlayMask);
    state &= ~K3Icon::OverlayMask;
    if ((state < 0) || (state >= K3Icon::LastState))
    {
	kDebug(264) << "Illegal icon state: " << state << endl;
	state = K3Icon::DefaultState;
    }

    if (size == 0 && group < 0)
    {
	kDebug(264) << "Neither size nor group specified!" << endl;
	group = K3Icon::Desktop;
    }

    if (!absolutePath)
    {
        if (!canReturnNull && name.isEmpty())
            name = str_unknown;
        else
	    name = removeIconExtension(name);
    }

    // If size == 0, use default size for the specified group.
    if (size == 0)
    {
	size = d->mpGroups[group].size;
    }
    favIconOverlay = favIconOverlay && size > 22;

    // Generate a unique cache key for the icon.

    key = "$kico_";
    key += name; key += '_';
    key += QString::number(size); key += '_';

    QString overlayStr = QString::number( overlay );

    QString noEffectKey = key + '_' + overlayStr;

    if (group >= 0)
    {
	key += d->mpEffect.fingerprint(group, state);
	if (d->mpGroups[group].dblPixels)
	    key += QLatin1String(":dblsize");
    } else
	key += QLatin1String("noeffect");
    key += '_';
    key += overlayStr;

    // Is the icon in the cache?
    bool inCache = QPixmapCache::find(key, pix);
    if (inCache && (path_store == 0L))
	return pix;

    QImage *img = 0;
    int iconType;
    int iconThreshold;

    if ( ( path_store != 0L ) ||
         noEffectKey != d->lastImageKey )
    {
        // No? load it.
        K3Icon icon;
        if (absolutePath && !favIconOverlay)
        {
            icon.context=K3Icon::Any;
            icon.type=K3Icon::Scalable;
            icon.path=name;
        }
        else
        {
            if (!name.isEmpty())
                icon = findMatchingIcon(favIconOverlay ? QString("www") : name, size);

            if (!icon.isValid())
            {
                // Try "User" icon too. Some apps expect this.
                if (!name.isEmpty())
                    pix = loadIcon(name, K3Icon::User, size, state, path_store, true);
                if (!pix.isNull() || canReturnNull)
                    return pix;

                icon = findMatchingIcon(str_unknown, size);
                if (!icon.isValid())
                {
                    kDebug(264)
                        << "Warning: could not find \"Unknown\" icon for size = "
                        << size << endl;
                    return pix;
                }
            }
        }

        if (path_store != 0L)
            *path_store = icon.path;
        if (inCache)
            return pix;

	// Use the extension as the format. Works for XPM and PNG, but not for SVG
	QString ext = icon.path.right(3).toUpper();
	if(ext != "SVG" && ext != "VGZ")
	{
	    img = new QImage(icon.path, ext.toLatin1());
	    if (img->isNull()) {
                delete img;
		return pix;
            }
	}
	else
	{
	    // Special stuff for SVG icons
            KSvgRenderer renderer(icon.path);
            if (renderer.isValid()) {
                img = new QImage(size, size, QImage::Format_ARGB32_Premultiplied);
                img->fill(0);
                QPainter p(img);
                renderer.render(&p);
            } else
                return pix;
	}

        iconType = icon.type;
        iconThreshold = icon.threshold;

        d->lastImage = img->copy();
        d->lastImageKey = noEffectKey;
        d->lastIconType = iconType;
        d->lastIconThreshold = iconThreshold;
    }
    else
    {
        img = new QImage( d->lastImage.copy() );
        iconType = d->lastIconType;
        iconThreshold = d->lastIconThreshold;
    }

    // Blend in all overlays
    if (overlay)
    {
	QImage *ovl;
	KIconTheme *theme = d->mpThemeRoot->theme;
	if ((overlay & K3Icon::LockOverlay) &&
		((ovl = loadOverlay(theme->lockOverlay(), size)) != 0L))
	    KIconEffect::overlay(*img, *ovl);
	if ((overlay & K3Icon::LinkOverlay) &&
		((ovl = loadOverlay(theme->linkOverlay(), size)) != 0L))
	    KIconEffect::overlay(*img, *ovl);
	if ((overlay & K3Icon::ZipOverlay) &&
		((ovl = loadOverlay(theme->zipOverlay(), size)) != 0L))
	    KIconEffect::overlay(*img, *ovl);
	if ((overlay & K3Icon::ShareOverlay) &&
	    ((ovl = loadOverlay(theme->shareOverlay(), size)) != 0L))
	  KIconEffect::overlay(*img, *ovl);
        if (overlay & K3Icon::HiddenOverlay)
        {
	    *img = img->convertToFormat(QImage::Format_ARGB32);
            for (int y = 0; y < img->height(); y++)
            {
		QRgb* line = reinterpret_cast<QRgb *>(img->scanLine(y));
                for (int x = 0; x < img->width();  x++)
                    line[x] = (line[x] & 0x00ffffff) | (qMin(0x80, qAlpha(line[x])) << 24);
	    }
	}
    }

    // Scale the icon and apply effects if necessary
    if (iconType == K3Icon::Scalable && size != img->width())
    {
        *img = img->scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    if (iconType == K3Icon::Threshold && size != img->width())
    {
	if ( abs(size-img->width())>iconThreshold )
            *img = img->scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    if (group >= 0 && d->mpGroups[group].dblPixels)
    {
	*img = d->mpEffect.doublePixels(*img);
    }
    if (group >= 0)
    {
	*img = d->mpEffect.apply(*img, group, state);
    }

    if (favIconOverlay)
    {
        QImage favIcon(name, "PNG");
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

    pix = QPixmap::fromImage(*img);

    delete img;

    QPixmapCache::insert(key, pix);
    return pix;
}

QImage *KIconLoader::loadOverlay(const QString &name, int size) const
{
    QString key = name + '_' + QString::number(size);
    QImage *image = 0L;
    if(d->imgDict.contains(key)) image = d->imgDict.value(key);
    if (image != 0L)
	return image;

    K3Icon icon = findMatchingIcon(name, size);
    if (!icon.isValid())
    {
	kDebug(264) << "Overlay " << name << "not found." << endl;
	return 0L;
    }
    image = new QImage(icon.path);
    // In some cases (since size in findMatchingIcon() is more a hint than a
    // constraint) image->size can be != size. If so perform rescaling.
    if ( size != image->width() )
        *image = image->scaled( size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    d->imgDict.insert(key, image);
    return image;
}



QMovie *KIconLoader::loadMovie(const QString& name, K3Icon::Group group, int size, QObject *parent) const
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

QString KIconLoader::moviePath(const QString& name, K3Icon::Group group, int size) const
{
    if (!d->mpGroups) return QString();

    if ( (group < -1 || group >= K3Icon::LastGroup) && group != K3Icon::User )
    {
	kDebug(264) << "Illegal icon group: " << group << endl;
	group = K3Icon::Desktop;
    }
    if (size == 0 && group < 0)
    {
	kDebug(264) << "Neither size nor group specified!" << endl;
	group = K3Icon::Desktop;
    }

    QString file = name + ".mng";
    if (group == K3Icon::User)
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
	    icon = themeNode->theme->iconPath(file, size, K3Icon::MatchExact);
	    if (icon.isValid())
		break;
	}

	if ( !icon.isValid() )
	{
    	foreach(KIconThemeNode *themeNode, d->links)
	    {
		icon = themeNode->theme->iconPath(file, size, K3Icon::MatchBest);
		if (icon.isValid())
		    break;
	    }
	}

	file = icon.isValid() ? icon.path : QString();
    }
    return file;
}


QStringList KIconLoader::loadAnimated(const QString& name, K3Icon::Group group, int size) const
{
    QStringList lst;

    if (!d->mpGroups) return lst;

    if ((group < -1) || (group >= K3Icon::LastGroup))
    {
	kDebug(264) << "Illegal icon group: " << group << endl;
	group = K3Icon::Desktop;
    }
    if ((size == 0) && (group < 0))
    {
	kDebug(264) << "Neither size nor group specified!" << endl;
	group = K3Icon::Desktop;
    }

    QString file = name + "/0001";
    if (group == K3Icon::User)
    {
	file = d->mpDirs->findResource("appicon", file + ".png");
    } else
    {
	if (size == 0)
	    size = d->mpGroups[group].size;
	K3Icon icon = findMatchingIcon(file, size);
	file = icon.isValid() ? icon.path : QString();

    }
    if (file.isEmpty())
	return lst;

    QString path = file.left(file.length()-8);
    DIR* dp = opendir( QFile::encodeName(path) );
    if(!dp)
        return lst;

    struct dirent* ep;
    while( ( ep = readdir( dp ) ) != 0L )
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
    if (d->mpThemeRoot) return d->mpThemeRoot->theme;
    return 0L;
}

int KIconLoader::currentSize(K3Icon::Group group) const
{
    if (!d->mpGroups) return -1;

    if (group < 0 || group >= K3Icon::LastGroup)
    {
	kDebug(264) << "Illegal icon group: " << group << endl;
	return -1;
    }
    return d->mpGroups[group].size;
}

QStringList KIconLoader::queryIconsByDir( const QString& iconsDir ) const
{
  QDir dir(iconsDir);
  QStringList formats;
  formats << "*.png" << "*.xpm";
  QStringList lst = dir.entryList(formats, QDir::Files);
  QStringList result;
  QStringList::ConstIterator it;
  for (it=lst.begin(); it!=lst.end(); ++it)
    result += iconsDir + '/' + *it;
  return result;
}

QStringList KIconLoader::queryIconsByContext(int group_or_size,
					    K3Icon::Context context) const
{
    QStringList result;
    if (group_or_size >= K3Icon::LastGroup)
    {
	kDebug(264) << "Illegal icon group: " << group_or_size << endl;
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
    for (it=result.begin(); it!=result.end(); ++it)
    {
	int n = (*it).lastIndexOf('/');
	if (n == -1)
	    name = *it;
	else
	    name = (*it).mid(n+1);
	name = removeIconExtension(name);
	if (!entries.contains(name))
	{
	    entries += name;
	    res2 += *it;
	}
    }
    return res2;

}

QStringList KIconLoader::queryIcons(int group_or_size, K3Icon::Context context) const
{
    QStringList result;
    if (group_or_size >= K3Icon::LastGroup)
    {
	kDebug(264) << "Illegal icon group: " << group_or_size << endl;
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
    for (it=result.begin(); it!=result.end(); ++it)
    {
	int n = (*it).lastIndexOf('/');
	if (n == -1)
	    name = *it;
	else
	    name = (*it).mid(n+1);
	name = removeIconExtension(name);
	if (!entries.contains(name))
	{
	    entries += name;
	    res2 += *it;
	}
    }
    return res2;
}

// used by KIconDialog to find out which contexts to offer in a combobox
bool KIconLoader::hasContext(K3Icon::Context context) const
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

bool KIconLoader::alphaBlending(K3Icon::Group group) const
{
    if (!d->mpGroups) return false;

    if (group < 0 || group >= K3Icon::LastGroup)
    {
	kDebug(264) << "Illegal icon group: " << group << endl;
	return false;
    }
    return d->mpGroups[group].alphaBlending;
}

/*** class for delayed icon loading for QIconSet ***/

#ifdef __GNUC__
    #warning "Delayed loading not ported - cullmann"
#endif

#ifdef DELAYED_LOADING_PORTED

class KIconFactory
    : public QIconFactory
    {
    public:
        KIconFactory( const QString& iconName_P, K3Icon::Group group_P,
            int size_P, KIconLoader* loader_P );
        virtual QPixmap* createPixmap( const QIcon&, QIcon::Size, QIcon::Mode, QIcon::State );
    private:
        QString iconName;
        K3Icon::Group group;
        int size;
        KIconLoader* loader;
    };

#endif

QIcon KIconLoader::loadIconSet( const QString& name, K3Icon::Group g, int s,
    bool canReturnNull)
{
#ifdef DELAYED_LOADING_PORTED
    if ( !d->delayedLoading )
#endif
        return loadIconSetNonDelayed( name, g, s, canReturnNull );

#ifdef DELAYED_LOADING_PORTED
    if (g < -1 || g > 6) {
        kDebug() << "KIconLoader::loadIconSet " << name << " " << (int)g << " " << s << endl;
        qDebug("%s", qPrintable(kBacktrace());
        abort();
    }

    if(canReturnNull)
    { // we need to find out if the icon actually exists
        QPixmap pm = loadIcon( name, g, s, K3Icon::DefaultState, NULL, true );
        if( pm.isNull())
            return QIcon();

        QIcon ret( pm );
        ret.installIconFactory( new KIconFactory( name, g, s, this ));
        return ret;
    }

    QIcon ret;
    ret.installIconFactory( new KIconFactory( name, g, s, this ));
    return ret;
#endif
}

QIcon KIconLoader::loadIconSetNonDelayed( const QString& name,
                                             K3Icon::Group g,
                                             int s, bool canReturnNull )
{
    QIcon iconset;
    QPixmap tmp = loadIcon(name, g, s, K3Icon::ActiveState, NULL, canReturnNull);
    iconset.addPixmap( tmp, QIcon::Active, QIcon::On );
    // we don't use QIconSet's resizing anyway
    tmp = loadIcon(name, g, s, K3Icon::DisabledState, NULL, canReturnNull);
    iconset.addPixmap( tmp, QIcon::Disabled, QIcon::On );
    tmp = loadIcon(name, g, s, K3Icon::DefaultState, NULL, canReturnNull);
    iconset.addPixmap( tmp, QIcon::Normal, QIcon::On );
    return iconset;
}

#ifdef DELAYED_LOADING_PORTED

KIconFactory::KIconFactory( const QString& iconName_P, K3Icon::Group group_P,
    int size_P, KIconLoader* loader_P )
    : iconName( iconName_P ), group( group_P ), size( size_P ), loader( loader_P )
{
    setAutoDelete( true );
}

QPixmap* KIconFactory::createPixmap( const QIcon&, QIcon::Size, QIcon::Mode mode_P, QIcon::State )
    {
#ifdef KICONLOADER_CHECKS
    bool found = false;
    for( QList< KIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         ++it )
        {
        if( (*it).loader == loader )
            {
            found = true;
            if( !(*it).valid )
                {
#ifdef NDEBUG
                loader = KGlobal::iconLoader();
                iconName = "no_way_man_you_will_get_broken_icon";
#else
                kWarning() << "Using already destroyed KIconLoader for loading an icon!" << endl;
                kWarning() << "Appname:" << (*it).appname << ", icon:" << iconName << endl;
                kWarning() << "Deleted at:" << endl;
                kWarning() << (*it).delete_bt << endl;
                kWarning() << "Current:" << endl;
                kWarning() << kBacktrace() << endl;
                abort();
                return NULL;
#endif
                }
            break;
            }
        }
    if( !found )
        {
#ifdef NDEBUG
        loader = KGlobal::iconLoader();
        iconName = "no_way_man_you_will_get_broken_icon";
#else
        kWarning() << "Using unknown KIconLoader for loading an icon!" << endl;
        kWarning() << "Icon:" << iconName << endl;
        kWarning() << kBacktrace() << endl;
        abort();
        return NULL;
#endif
        }
#endif
    // QIconSet::Mode to K3Icon::State conversion
    static const K3Icon::States tbl[] = { K3Icon::DefaultState, K3Icon::DisabledState, K3Icon::ActiveState };
    int state = K3Icon::DefaultState;
    if( mode_P <= QIcon::Active )
        state = tbl[ mode_P ];
    if( group >= 0 && state == K3Icon::ActiveState )
    { // active and normal icon are usually the same
	if( loader->iconEffect()->fingerprint(group, K3Icon::ActiveState )
            == loader->iconEffect()->fingerprint(group, K3Icon::DefaultState ))
            return 0; // so let QIconSet simply duplicate it
    }
    // ignore passed size
    // ignore passed state (i.e. on/off)
    QPixmap pm = loader->loadIcon( iconName, group, size, state );
    return new QPixmap( pm );
    }

#endif

// Easy access functions

QPixmap DesktopIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, K3Icon::Desktop, force_size, state);
}

QPixmap DesktopIcon(const QString& name, KInstance *instance)
{
    return DesktopIcon(name, 0, K3Icon::DefaultState, instance);
}

QIcon DesktopIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, K3Icon::Desktop, force_size );
}

QPixmap BarIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, K3Icon::Toolbar, force_size, state);
}

QPixmap BarIcon(const QString& name, KInstance *instance)
{
    return BarIcon(name, 0, K3Icon::DefaultState, instance);
}

QIcon BarIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, K3Icon::Toolbar, force_size );
}

QPixmap SmallIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, K3Icon::Small, force_size, state);
}

QPixmap SmallIcon(const QString& name, KInstance *instance)
{
    return SmallIcon(name, 0, K3Icon::DefaultState, instance);
}

QIcon SmallIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, K3Icon::Small, force_size );
}

QPixmap MainBarIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, K3Icon::MainToolbar, force_size, state);
}

QPixmap MainBarIcon(const QString& name, KInstance *instance)
{
    return MainBarIcon(name, 0, K3Icon::DefaultState, instance);
}

QIcon MainBarIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, K3Icon::MainToolbar, force_size );
}

QPixmap UserIcon(const QString& name, int state, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, K3Icon::User, 0, state);
}

QPixmap UserIcon(const QString& name, KInstance *instance)
{
    return UserIcon(name, K3Icon::DefaultState, instance);
}

QIcon UserIconSet(const QString& name, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, K3Icon::User );
}

int IconSize(K3Icon::Group group, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->currentSize(group);
}

QPixmap KIconLoader::unknown()
{
    QPixmap pix;
    if ( QPixmapCache::find("unknown", pix) )
            return pix;

    QString path = KGlobal::iconLoader()->iconPath("unknown", K3Icon::Small, true);
    if (path.isEmpty())
    {
	kDebug(264) << "Warning: Cannot find \"unknown\" icon." << endl;
	pix = QPixmap(32,32);
    } else
    {
        pix.load(path);
        QPixmapCache::insert("unknown", pix);
    }

    return pix;
}
