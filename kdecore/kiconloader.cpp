/* vi: ts=8 sts=4 sw=4
 *
 * $Id: kiconloader.cpp 693955 2007-07-29 16:28:09Z tyrerj $
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
#include <qptrlist.h>
#include <qintdict.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qiconset.h>
#include <qmovie.h>
#include <qbitmap.h>

#include <kapplication.h>
#include <kipc.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kinstance.h>

#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <sys/types.h>
#include <stdlib.h>	//for abs
#include <unistd.h>     //for readlink
#include <dirent.h>
#include <config.h>
#include <assert.h>

#ifdef HAVE_LIBART
#include "svgicons/ksvgiconengine.h"
#include "svgicons/ksvgiconpainter.h"
#endif

#include "kiconloader_p.h"

/*** KIconThemeNode: A node in the icon theme dependancy tree. ***/

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
    dbgString += "(";
    dbgString += theme->name();
    dbgString += ")";
}

void KIconThemeNode::queryIcons(QStringList *result,
				int size, KIcon::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIcons(size, context);
}

void KIconThemeNode::queryIconsByContext(QStringList *result,
				int size, KIcon::Context context) const
{
    // add the icons of this theme to it
    *result += theme->queryIconsByContext(size, context);
}

KIcon KIconThemeNode::findIcon(const QString& name, int size,
			       KIcon::MatchType match) const
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

static QValueList< KIconLoaderDebug > *kiconloaders;
#endif

/*** KIconLoader: the icon loader ***/

KIconLoader::KIconLoader(const QString& _appname, KStandardDirs *_dirs)
{
#ifdef KICONLOADER_CHECKS
    if( kiconloaders == NULL )
        kiconloaders = new QValueList< KIconLoaderDebug>();
    // check for the (very unlikely case) that new KIconLoader gets allocated
    // at exactly same address like some previous one
    for( QValueList< KIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         )
        {
        if( (*it).loader == this )
            it = kiconloaders->remove( it );
        else
            ++it;
        }
    kiconloaders->append( KIconLoaderDebug( this, _appname ));
#endif
    d = new KIconLoaderPrivate;
    d->q = this;
    d->mpGroups = 0L;
    d->imgDict.setAutoDelete(true);
    d->links.setAutoDelete(true);

    if (kapp) {
        kapp->addKipcEventMask(KIPC::IconChanged);
        QObject::connect(kapp, SIGNAL(updateIconLoaders()), d, SLOT(reconfigure()));
    }

    init( _appname, _dirs );
}

void KIconLoader::reconfigure( const QString& _appname, KStandardDirs *_dirs )
{
    d->links.clear();
    d->imgDict.clear();
    d->mThemesInTree.clear();
    d->lastImage.reset();
    d->lastImageKey = QString::null;
    delete [] d->mpGroups;

    init( _appname, _dirs );
}

void KIconLoader::init( const QString& _appname, KStandardDirs *_dirs )
{
    // If this is unequal to 0, the iconloader is initialized
    // successfully.
    d->mpThemeRoot = 0L;

    d->appname = _appname;
    d->extraDesktopIconsLoaded = false;
    d->delayedLoading = false;

    if (_dirs)
        d->mpDirs = _dirs;
    else
        d->mpDirs = KGlobal::dirs();

    QString appname = _appname;
    if (appname.isEmpty())
        appname = KGlobal::instance()->instanceName();

    // Add the default theme and its base themes to the theme tree
    KIconTheme *def = new KIconTheme(KIconTheme::current(), appname);
    if (!def->isValid())
    {
        delete def;
        // warn, as this is actually a small penalty hit
        kdDebug(264) << "Couldn't find current icon theme, falling back to default." << endl;
	def = new KIconTheme(KIconTheme::defaultThemeName(), appname);
        if (!def->isValid())
        {
            kdError(264) << "Error: standard icon theme"
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
    KConfigGroupSaver cs(config, "dummy");

    // loading config and default sizes
    d->mpGroups = new KIconGroup[(int) KIcon::LastGroup];
    for (KIcon::Group i=KIcon::FirstGroup; i<KIcon::LastGroup; i++)
    {
	if (groups[i] == 0L)
	    break;
	config->setGroup(QString::fromLatin1(groups[i]) + "Icons");
	d->mpGroups[i].size = config->readNumEntry("Size", 0);
	d->mpGroups[i].dblPixels = config->readBoolEntry("DoublePixels", false);
	if (QPixmap::defaultDepth()>8)
	    d->mpGroups[i].alphaBlending = config->readBoolEntry("AlphaBlending", true);
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
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    dirs += d->mpDirs->resourceDirs("xdgdata-pixmap");
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it)
	d->mpDirs->addResourceDir("appicon", *it);

#ifndef NDEBUG
    QString dbgString = "Theme tree: ";
    d->mpThemeRoot->printTree(dbgString);
    kdDebug(264) << dbgString << endl;
#endif
}

KIconLoader::~KIconLoader()
{
#ifdef KICONLOADER_CHECKS
    for( QValueList< KIconLoaderDebug >::Iterator it = kiconloaders->begin();
         it != kiconloaders->end();
         ++it )
        {
        if( (*it).loader == this )
            {
            (*it).valid = false;
            (*it).delete_bt = kdBacktrace();
            break;
            }
        }
#endif
    /* antlarr: There's no need to delete d->mpThemeRoot as it's already
       deleted when the elements of d->links are deleted */
    d->mpThemeRoot=0;
    delete[] d->mpGroups;
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
	d->links.append(n);
	addBaseThemes(n, appname);
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
	QStringList lst = dir.entryList("default.*", QDir::Dirs);
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

    static const QString &png_ext = KGlobal::staticQString(".png");
    static const QString &xpm_ext = KGlobal::staticQString(".xpm");
    if (ext == png_ext || ext == xpm_ext)
      extensionLength=4;
#ifdef HAVE_LIBART
    else
    {
	static const QString &svgz_ext = KGlobal::staticQString(".svgz");
	static const QString &svg_ext = KGlobal::staticQString(".svg");

	if (name.right(5) == svgz_ext)
	    extensionLength=5;
	else if (ext == svg_ext)
	    extensionLength=4;
    }
#endif

    if ( extensionLength > 0 )
    {
	return name.left(name.length() - extensionLength);
    }
    return name;
}

QString KIconLoader::removeIconExtensionInternal(const QString &name) const
{
    QString name_noext = removeIconExtension(name);

#ifndef NDEBUG
    if (name != name_noext)
    {
	kdDebug(264) << "Application " << KGlobal::instance()->instanceName()
		     << " loads icon " << name << " with extension." << endl;
    }
#endif

    return name_noext;
}

KIcon KIconLoader::findMatchingIcon(const QString& name, int size) const
{
    KIcon icon;

    const QString *ext[4];
    int count=0;
    static const QString &png_ext = KGlobal::staticQString(".png");
    ext[count++]=&png_ext;
#ifdef HAVE_LIBART
    static const QString &svgz_ext = KGlobal::staticQString(".svgz");
    ext[count++]=&svgz_ext;
    static const QString &svg_ext = KGlobal::staticQString(".svg");
    ext[count++]=&svg_ext;
#endif
    static const QString &xpm_ext = KGlobal::staticQString(".xpm");
    ext[count++]=&xpm_ext;

    /* JRT: To follow the XDG spec, the order in which we look for an
       icon 1s:

       png, svgz, svg, xpm exact match
       png, svgz, svg, xpm best match
       next theme in inheritance tree : png, svgz, svg, xpm exact match
                                        png, svgz, svg, xpm best match
       next theme in inheritance tree : png, svgz, svg, xpm exact match
                                        png, svgz, svg, xpm best match
       and so on

       */
    for ( KIconThemeNode *themeNode = d->links.first() ; themeNode ;
	themeNode = d->links.next() )
    {
	for (int i = 0 ; i < count ; i++)
	{
	    icon = themeNode->theme->iconPath(name + *ext[i], size, KIcon::MatchExact);
	    if (icon.isValid()) goto icon_found ;
	}

	for (int i = 0 ; i < count ; i++)
	{
	    icon = themeNode->theme->iconPath(name + *ext[i], size, KIcon::MatchBest);
	    if (icon.isValid()) goto icon_found;	
	}
    }
    icon_found:
    return icon;
}

inline QString KIconLoader::unknownIconPath( int size ) const
{
    static const QString &str_unknown = KGlobal::staticQString("unknown");

    KIcon icon = findMatchingIcon(str_unknown, size);
    if (!icon.isValid())
    {
        kdDebug(264) << "Warning: could not find \"Unknown\" icon for size = "
                     << size << endl;
        return QString::null;
    }
    return icon.path;
}

// Finds the absolute path to an icon.

QString KIconLoader::iconPath(const QString& _name, int group_or_size,
			      bool canReturnNull) const
{
    if (d->mpThemeRoot == 0L)
	return QString::null;

    if (!QDir::isRelativePath(_name))
	return _name;

    QString name = removeIconExtensionInternal( _name );

    QString path;
    if (group_or_size == KIcon::User)
    {
	static const QString &png_ext = KGlobal::staticQString(".png");
	static const QString &xpm_ext = KGlobal::staticQString(".xpm");
	path = d->mpDirs->findResource("appicon", name + png_ext);

#ifdef HAVE_LIBART
	static const QString &svgz_ext = KGlobal::staticQString(".svgz");
	static const QString &svg_ext = KGlobal::staticQString(".svg");
	if (path.isEmpty())
	    path = d->mpDirs->findResource("appicon", name + svgz_ext);
	if (path.isEmpty())
	   path = d->mpDirs->findResource("appicon", name + svg_ext);
#endif
	if (path.isEmpty())
	     path = d->mpDirs->findResource("appicon", name + xpm_ext);
	return path;
    }

    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << endl;
	return path;
    }

    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;

    if (_name.isEmpty()) {
        if (canReturnNull)
            return QString::null;
        else
            return unknownIconPath(size);
    }

    KIcon icon = findMatchingIcon(name, size);

    if (!icon.isValid())
    {
	// Try "User" group too.
	path = iconPath(name, KIcon::User, true);
	if (!path.isEmpty() || canReturnNull)
	    return path;

	if (canReturnNull)
	    return QString::null;
        else
            return unknownIconPath(size);
    }
    return icon.path;
}

QPixmap KIconLoader::loadIcon(const QString& _name, KIcon::Group group, int size,
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
       name = locateLocal("cache", name+".png");
    }
    if (!QDir::isRelativePath(name)) absolutePath=true;

    static const QString &str_unknown = KGlobal::staticQString("unknown");

    // Special case for "User" icons.
    if (group == KIcon::User)
    {
	key = "$kicou_";
        key += QString::number(size); key += '_';
	key += name;
	bool inCache = QPixmapCache::find(key, pix);
	if (inCache && (path_store == 0L))
	    return pix;

	QString path = (absolutePath) ? name :
			iconPath(name, KIcon::User, canReturnNull);
	if (path.isEmpty())
	{
	    if (canReturnNull)
		return pix;
	    // We don't know the desired size: use small
	    path = iconPath(str_unknown, KIcon::Small, true);
	    if (path.isEmpty())
	    {
		kdDebug(264) << "Warning: Cannot find \"unknown\" icon." << endl;
		return pix;
	    }
	}

	if (path_store != 0L)
	    *path_store = path;
	if (inCache)
	    return pix;
	QImage img(path);
	if (size != 0)
	    img=img.smoothScale(size,size);

	pix.convertFromImage(img);
	QPixmapCache::insert(key, pix);
	return pix;
    }

    // Regular case: Check parameters

    if ((group < -1) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	group = KIcon::Desktop;
    }

    int overlay = (state & KIcon::OverlayMask);
    state &= ~KIcon::OverlayMask;
    if ((state < 0) || (state >= KIcon::LastState))
    {
	kdDebug(264) << "Illegal icon state: " << state << endl;
	state = KIcon::DefaultState;
    }

    if (size == 0 && group < 0)
    {
	kdDebug(264) << "Neither size nor group specified!" << endl;
	group = KIcon::Desktop;
    }

    if (!absolutePath)
    {
        if (!canReturnNull && name.isEmpty())
            name = str_unknown;
        else
	    name = removeIconExtensionInternal(name);
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
	    key += QString::fromLatin1(":dblsize");
    } else
	key += QString::fromLatin1("noeffect");
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
        KIcon icon;
        if (absolutePath && !favIconOverlay)
        {
            icon.context=KIcon::Any;
            icon.type=KIcon::Scalable;
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
                    pix = loadIcon(name, KIcon::User, size, state, path_store, true);
                if (!pix.isNull() || canReturnNull) {
                    QPixmapCache::insert(key, pix);
                    return pix;
                }

                icon = findMatchingIcon(str_unknown, size);
                if (!icon.isValid())
                {
                    kdDebug(264)
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
	QString ext = icon.path.right(3).upper();
	if(ext != "SVG" && ext != "VGZ")
	{
	    img = new QImage(icon.path, ext.latin1());
	    if (img->isNull()) {
                delete img;
		return pix;
            }
	}
#ifdef HAVE_LIBART
	else
	{
	    // Special stuff for SVG icons
	    KSVGIconEngine *svgEngine = new KSVGIconEngine();

	    if(svgEngine->load(size, size, icon.path))
		img = svgEngine->painter()->image();
	    else
		img = new QImage();

	    delete svgEngine;
	}
#endif

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
	if ((overlay & KIcon::LockOverlay) &&
		((ovl = loadOverlay(theme->lockOverlay(), size)) != 0L))
	    KIconEffect::overlay(*img, *ovl);
	if ((overlay & KIcon::LinkOverlay) &&
		((ovl = loadOverlay(theme->linkOverlay(), size)) != 0L))
	    KIconEffect::overlay(*img, *ovl);
	if ((overlay & KIcon::ZipOverlay) &&
		((ovl = loadOverlay(theme->zipOverlay(), size)) != 0L))
	    KIconEffect::overlay(*img, *ovl);
	if ((overlay & KIcon::ShareOverlay) &&
	    ((ovl = loadOverlay(theme->shareOverlay(), size)) != 0L))
	  KIconEffect::overlay(*img, *ovl);
        if (overlay & KIcon::HiddenOverlay)
        {
	    if (img->depth() != 32)
	        *img = img->convertDepth(32);
            for (int y = 0; y < img->height(); y++)
            {
		QRgb *line = reinterpret_cast<QRgb *>(img->scanLine(y));
                for (int x = 0; x < img->width();  x++)
                    line[x] = (line[x] & 0x00ffffff) | (QMIN(0x80, qAlpha(line[x])) << 24);
	    }
	}
    }

    // Scale the icon and apply effects if necessary
    if (iconType == KIcon::Scalable && size != img->width())
    {
        *img = img->smoothScale(size, size);
    }
    if (iconType == KIcon::Threshold && size != img->width())
    {
	if ( abs(size-img->width())>iconThreshold )
	    *img = img->smoothScale(size, size);
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
        if( favIcon.depth() != 32 )
            favIcon = favIcon.convertDepth( 32 );
        if( img->depth() != 32 )
            *img = img->convertDepth( 32 );
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

    pix.convertFromImage(*img);

    delete img;

    QPixmapCache::insert(key, pix);
    return pix;
}

QImage *KIconLoader::loadOverlay(const QString &name, int size) const
{
    QString key = name + '_' + QString::number(size);
    QImage *image = d->imgDict.find(key);
    if (image != 0L)
	return image;

    KIcon icon = findMatchingIcon(name, size);
    if (!icon.isValid())
    {
	kdDebug(264) << "Overlay " << name << "not found." << endl;
	return 0L;
    }
    image = new QImage(icon.path);
    // In some cases (since size in findMatchingIcon() is more a hint than a
    // constraint) image->size can be != size. If so perform rescaling.
    if ( size != image->width() )
        *image = image->smoothScale( size, size );
    d->imgDict.insert(key, image);
    return image;
}



QMovie KIconLoader::loadMovie(const QString& name, KIcon::Group group, int size) const
{
    QString file = moviePath( name, group, size );
    if (file.isEmpty())
	return QMovie();
    int dirLen = file.findRev('/');
    QString icon = iconPath(name, size ? -size : group, true);
    if (!icon.isEmpty() && file.left(dirLen) != icon.left(dirLen))
	return QMovie();
    return QMovie(file);
}

QString KIconLoader::moviePath(const QString& name, KIcon::Group group, int size) const
{
    if (!d->mpGroups) return QString::null;

    if ( (group < -1 || group >= KIcon::LastGroup) && group != KIcon::User )
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	group = KIcon::Desktop;
    }
    if (size == 0 && group < 0)
    {
	kdDebug(264) << "Neither size nor group specified!" << endl;
	group = KIcon::Desktop;
    }

    QString file = name + ".mng";
    if (group == KIcon::User)
    {
	file = d->mpDirs->findResource("appicon", file);
    }
    else
    {
	if (size == 0)
	    size = d->mpGroups[group].size;

        KIcon icon;

	for ( KIconThemeNode *themeNode = d->links.first() ; themeNode ;
		themeNode = d->links.next() )
	{
	    icon = themeNode->theme->iconPath(file, size, KIcon::MatchExact);
	    if (icon.isValid()) goto icon_found ;

		icon = themeNode->theme->iconPath(file, size, KIcon::MatchBest);
	    if (icon.isValid()) goto icon_found ;
	}

	icon_found:
	file = icon.isValid() ? icon.path : QString::null;
    }
    return file;
}


QStringList KIconLoader::loadAnimated(const QString& name, KIcon::Group group, int size) const
{
    QStringList lst;

    if (!d->mpGroups) return lst;

    if ((group < -1) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	group = KIcon::Desktop;
    }
    if ((size == 0) && (group < 0))
    {
	kdDebug(264) << "Neither size nor group specified!" << endl;
	group = KIcon::Desktop;
    }

    QString file = name + "/0001";
    if (group == KIcon::User)
    {
	file = d->mpDirs->findResource("appicon", file + ".png");
    } else
    {
	if (size == 0)
	    size = d->mpGroups[group].size;
	KIcon icon = findMatchingIcon(file, size);
	file = icon.isValid() ? icon.path : QString::null;

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

int KIconLoader::currentSize(KIcon::Group group) const
{
    if (!d->mpGroups) return -1;

    if (group < 0 || group >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	return -1;
    }
    return d->mpGroups[group].size;
}

QStringList KIconLoader::queryIconsByDir( const QString& iconsDir ) const
{
  QDir dir(iconsDir);
  QStringList lst = dir.entryList("*.png;*.xpm", QDir::Files);
  QStringList result;
  QStringList::ConstIterator it;
  for (it=lst.begin(); it!=lst.end(); ++it)
    result += iconsDir + "/" + *it;
  return result;
}

QStringList KIconLoader::queryIconsByContext(int group_or_size,
					    KIcon::Context context) const
{
    QStringList result;
    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << endl;
	return result;
    }
    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;

    for ( KIconThemeNode *themeNode = d->links.first() ; themeNode ;
            themeNode = d->links.next() )
       themeNode->queryIconsByContext(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    QString name;
    QStringList res2, entries;
    QStringList::ConstIterator it;
    for (it=result.begin(); it!=result.end(); ++it)
    {
	int n = (*it).findRev('/');
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

QStringList KIconLoader::queryIcons(int group_or_size, KIcon::Context context) const
{
    QStringList result;
    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << endl;
	return result;
    }
    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;

    for ( KIconThemeNode *themeNode = d->links.first() ; themeNode ;
            themeNode = d->links.next() )
       themeNode->queryIcons(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    QString name;
    QStringList res2, entries;
    QStringList::ConstIterator it;
    for (it=result.begin(); it!=result.end(); ++it)
    {
	int n = (*it).findRev('/');
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
bool KIconLoader::hasContext(KIcon::Context context) const
{
    for ( KIconThemeNode *themeNode = d->links.first() ; themeNode ;
            themeNode = d->links.next() )
       if( themeNode->theme->hasContext( context ))
           return true;
    return false;
}

KIconEffect * KIconLoader::iconEffect() const
{
    return &d->mpEffect;
}

bool KIconLoader::alphaBlending(KIcon::Group group) const
{
    if (!d->mpGroups) return false;

    if (group < 0 || group >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group << endl;
	return false;
    }
    return d->mpGroups[group].alphaBlending;
}

QIconSet KIconLoader::loadIconSet(const QString& name, KIcon::Group group, int size, bool canReturnNull)
{
    return loadIconSet( name, group, size, canReturnNull, true );
}

QIconSet KIconLoader::loadIconSet(const QString& name, KIcon::Group group, int size)
{
    return loadIconSet( name, group, size, false );
}

/*** class for delayed icon loading for QIconSet ***/

class KIconFactory
    : public QIconFactory
    {
    public:
        KIconFactory( const QString& iconName_P, KIcon::Group group_P,
            int size_P, KIconLoader* loader_P );
        KIconFactory( const QString& iconName_P, KIcon::Group group_P,
            int size_P, KIconLoader* loader_P, bool canReturnNull );
        virtual QPixmap* createPixmap( const QIconSet&, QIconSet::Size, QIconSet::Mode, QIconSet::State );
    private:
        QString iconName;
        KIcon::Group group;
        int size;
        KIconLoader* loader;
        bool canReturnNull;
    };


QIconSet KIconLoader::loadIconSet( const QString& name, KIcon::Group g, int s,
    bool canReturnNull, bool immediateExistenceCheck)
{
    if ( !d->delayedLoading )
        return loadIconSetNonDelayed( name, g, s, canReturnNull );

    if (g < -1 || g > 6) {
        kdDebug() << "KIconLoader::loadIconSet " << name << " " << (int)g << " " << s << endl;
        qDebug("%s", kdBacktrace().latin1());
        abort();
    }

    if(canReturnNull && immediateExistenceCheck)
    { // we need to find out if the icon actually exists
        QPixmap pm = loadIcon( name, g, s, KIcon::DefaultState, NULL, true );
        if( pm.isNull())
            return QIconSet();

        QIconSet ret( pm );
        ret.installIconFactory( new KIconFactory( name, g, s, this ));
        return ret;
    }

    QIconSet ret;
    ret.installIconFactory( new KIconFactory( name, g, s, this, canReturnNull ));
    return ret;
}

QIconSet KIconLoader::loadIconSetNonDelayed( const QString& name,
                                             KIcon::Group g,
                                             int s, bool canReturnNull )
{
    QIconSet iconset;
    QPixmap tmp = loadIcon(name, g, s, KIcon::ActiveState, NULL, canReturnNull);
    iconset.setPixmap( tmp, QIconSet::Small, QIconSet::Active );
    // we don't use QIconSet's resizing anyway
    iconset.setPixmap( tmp, QIconSet::Large, QIconSet::Active );
    tmp = loadIcon(name, g, s, KIcon::DisabledState, NULL, canReturnNull);
    iconset.setPixmap( tmp, QIconSet::Small, QIconSet::Disabled );
    iconset.setPixmap( tmp, QIconSet::Large, QIconSet::Disabled );
    tmp = loadIcon(name, g, s, KIcon::DefaultState, NULL, canReturnNull);
    iconset.setPixmap( tmp, QIconSet::Small, QIconSet::Normal );
    iconset.setPixmap( tmp, QIconSet::Large, QIconSet::Normal );
    return iconset;
}

KIconFactory::KIconFactory( const QString& iconName_P, KIcon::Group group_P,
    int size_P, KIconLoader* loader_P )
    : iconName( iconName_P ), group( group_P ), size( size_P ), loader( loader_P )
{
    canReturnNull = false;
    setAutoDelete( true );
}

KIconFactory::KIconFactory( const QString& iconName_P, KIcon::Group group_P,
    int size_P, KIconLoader* loader_P, bool canReturnNull_P )
    : iconName( iconName_P ), group( group_P ), size( size_P ),
      loader( loader_P ), canReturnNull( canReturnNull_P)
{
    setAutoDelete( true );
}

QPixmap* KIconFactory::createPixmap( const QIconSet&, QIconSet::Size, QIconSet::Mode mode_P, QIconSet::State )
    {
#ifdef KICONLOADER_CHECKS
    bool found = false;
    for( QValueList< KIconLoaderDebug >::Iterator it = kiconloaders->begin();
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
                kdWarning() << "Using already destroyed KIconLoader for loading an icon!" << endl;
                kdWarning() << "Appname:" << (*it).appname << ", icon:" << iconName << endl;
                kdWarning() << "Deleted at:" << endl;
                kdWarning() << (*it).delete_bt << endl;
                kdWarning() << "Current:" << endl;
                kdWarning() << kdBacktrace() << endl;
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
        kdWarning() << "Using unknown KIconLoader for loading an icon!" << endl;
        kdWarning() << "Icon:" << iconName << endl;
        kdWarning() << kdBacktrace() << endl;
        abort();
        return NULL;
#endif
        }
#endif
    // QIconSet::Mode to KIcon::State conversion
    static const KIcon::States tbl[] = { KIcon::DefaultState, KIcon::DisabledState, KIcon::ActiveState };
    int state = KIcon::DefaultState;
    if( mode_P <= QIconSet::Active )
        state = tbl[ mode_P ];
    if( group >= 0 && state == KIcon::ActiveState )
    { // active and normal icon are usually the same
	if( loader->iconEffect()->fingerprint(group, KIcon::ActiveState )
            == loader->iconEffect()->fingerprint(group, KIcon::DefaultState ))
            return 0; // so let QIconSet simply duplicate it
    }
    // ignore passed size
    // ignore passed state (i.e. on/off)
    QPixmap pm = loader->loadIcon( iconName, group, size, state, 0, canReturnNull );
    return new QPixmap( pm );
    }

// Easy access functions

QPixmap DesktopIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::Desktop, force_size, state);
}

QPixmap DesktopIcon(const QString& name, KInstance *instance)
{
    return DesktopIcon(name, 0, KIcon::DefaultState, instance);
}

QIconSet DesktopIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, KIcon::Desktop, force_size );
}

QPixmap BarIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::Toolbar, force_size, state);
}

QPixmap BarIcon(const QString& name, KInstance *instance)
{
    return BarIcon(name, 0, KIcon::DefaultState, instance);
}

QIconSet BarIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, KIcon::Toolbar, force_size );
}

QPixmap SmallIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::Small, force_size, state);
}

QPixmap SmallIcon(const QString& name, KInstance *instance)
{
    return SmallIcon(name, 0, KIcon::DefaultState, instance);
}

QIconSet SmallIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, KIcon::Small, force_size );
}

QPixmap MainBarIcon(const QString& name, int force_size, int state,
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::MainToolbar, force_size, state);
}

QPixmap MainBarIcon(const QString& name, KInstance *instance)
{
    return MainBarIcon(name, 0, KIcon::DefaultState, instance);
}

QIconSet MainBarIconSet(const QString& name, int force_size, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, KIcon::MainToolbar, force_size );
}

QPixmap UserIcon(const QString& name, int state, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::User, 0, state);
}

QPixmap UserIcon(const QString& name, KInstance *instance)
{
    return UserIcon(name, KIcon::DefaultState, instance);
}

QIconSet UserIconSet(const QString& name, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIconSet( name, KIcon::User );
}

int IconSize(KIcon::Group group, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->currentSize(group);
}

QPixmap KIconLoader::unknown()
{
    QPixmap pix;
    if ( QPixmapCache::find("unknown", pix) )
            return pix;

    QString path = KGlobal::iconLoader()->iconPath("unknown", KIcon::Small, true);
    if (path.isEmpty())
    {
	kdDebug(264) << "Warning: Cannot find \"unknown\" icon." << endl;
	pix.resize(32,32);
    } else
    {
        pix.load(path);
        QPixmapCache::insert("unknown", pix);
    }

    return pix;
}

void KIconLoaderPrivate::reconfigure()
{
  q->reconfigure(appname, mpDirs);
}

#include "kiconloader_p.moc"
