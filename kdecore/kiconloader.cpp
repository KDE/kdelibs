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
 * kiconloader.cpp: An icon loader for KDE with theming functionality.
 */

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qintdict.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qiconset.h>

#include <kdebug.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kinstance.h>

#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <sys/types.h>
#include <stdlib.h>	//for abs
#include <dirent.h>


/*** KIconThemeNode: A node in the icon theme dependancy tree. ***/

class KIconThemeNode
{
public:

    KIconThemeNode(KIconTheme *_theme);
    ~KIconThemeNode();

    void queryIcons(QStringList *lst, int size, int context) const;
    void queryIconsByContext(QStringList *lst, int size, int context) const;
    KIcon findIcon(const QString& name, int size, int match) const;
    void printTree(QString& dbgString) const;

    KIconTheme *theme;
    QList<KIconThemeNode> links;
};

KIconThemeNode::KIconThemeNode(KIconTheme *_theme)
{
    links.setAutoDelete(true);
    theme = _theme;
}

KIconThemeNode::~KIconThemeNode()
{
    delete theme;
}

void KIconThemeNode::printTree(QString& dbgString) const
{
    dbgString += "(";
    dbgString += theme->name();
    bool first;
    QListIterator<KIconThemeNode> it(links);
    for (first=true ; it.current(); ++it, first=false)
    {
	if (first) dbgString += ": ";
	else dbgString += ", ";
	it.current()->printTree(dbgString);
    }
    dbgString += ")";
}

void KIconThemeNode::queryIcons(QStringList *result,
				int size, int context) const
{
    // add the icons of this theme to it
    *result += theme->queryIcons(size, context);
    QListIterator<KIconThemeNode> it(links);
    for (; it.current(); ++it) // recursivly going down
	it.current()->queryIcons(result, size, context);
}

void KIconThemeNode::queryIconsByContext(QStringList *result,
				int size, int context) const
{
    // add the icons of this theme to it
    *result += theme->queryIconsByContext(size, context);
    QListIterator<KIconThemeNode> it(links);
    for (; it.current(); ++it) // recursivly going down
	it.current()->queryIconsByContext(result, size, context);
}

KIcon KIconThemeNode::findIcon(const QString& name, int size,
			       int match) const
{
    KIcon icon;
    icon = theme->iconPath(name, size, match);
    if (icon.isValid())
	return icon;

    QListIterator <KIconThemeNode> it(links);
    for (; it.current(); ++it)
    {
	icon = it.current()->findIcon(name, size, match);
	if (icon.isValid())
	    break;
    }
    return icon;
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
    QStringList mThemeList;
    QStringList mThemesInTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconEffect mpEffect;
    QDict<QImage> imgDict;
    QIntDict<QImage> imgCache;
    QImage lastImage; // last loaded image without effect applied
    QString lastImageKey; // key for icon without effect
    bool lastIconType; // see KIcon::type
    bool lastIconThreshold; // see KIcon::threshold
};

/*** KIconLoader: the icon loader ***/

KIconLoader::KIconLoader(const QString& _appname, KStandardDirs *_dirs)
{
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
    d->imgDict.setAutoDelete( true );
    d->imgCache.setAutoDelete( true );

    if (_dirs)
	d->mpDirs = _dirs;
    else
	d->mpDirs = KGlobal::dirs();

    // If this is unequal to 0, the iconloader is initialized
    // successfully.
    d->mpThemeRoot = 0L;

    // Check installed themes.
    d->mThemeList = KIconTheme::list();
    if (!d->mThemeList.contains("hicolor"))
    {
        kdError(264) << "Error: standard icon theme "
                     << "\"hicolor\" not found!\n";
        d->mpGroups=0L;

        return;
    }

    QString appname = _appname;
    if (appname.isEmpty())
	appname = KGlobal::instance()->instanceName();

    // Add the default theme and its base themes to the theme tree
    KIconTheme *def = new KIconTheme(KIconTheme::current(), appname);
    if (!def->isValid())
    {
	delete def;
	def = new KIconTheme(QString::fromLatin1("hicolor"), appname);
    }
    d->mpThemeRoot = new KIconThemeNode(def);
    d->mThemesInTree += KIconTheme::current();
    addBaseThemes(d->mpThemeRoot, appname);

    // These have to match the order in kicontheme.h
    const char *groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", "Panel", 0L };
    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs(config, "dummy");

    // loading config and default sizes
    d->mpGroups = new KIconGroup[(int) KIcon::LastGroup];
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	if (groups[i] == 0L)
	    break;
	config->setGroup(QString::fromLatin1(groups[i]) + "Icons");
	d->mpGroups[i].size = config->readNumEntry("Size", 0);
	d->mpGroups[i].dblPixels = config->readBoolEntry("DoublePixels", false);
	if (QPixmap::defaultDepth()>8)
	    d->mpGroups[i].alphaBlending = config->readBoolEntry("AlphaBlending", false);
	else
	    d->mpGroups[i].alphaBlending = false;

	if (!d->mpGroups[i].size)
	    d->mpGroups[i].size = d->mpThemeRoot->theme->defaultSize(i);
    }

    // Insert application specific themes at the top.
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/pics/");
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/toolbar/");

    // Add legacy icon dirs.
    QStringList dirs;
    dirs += d->mpDirs->resourceDirs("icon");
    dirs += d->mpDirs->resourceDirs("pixmap");
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); it++)
	d->mpDirs->addResourceDir("appicon", *it);

#ifndef NDEBUG
    QString dbgString = "Theme tree: ";
    d->mpThemeRoot->printTree(dbgString);
    kdDebug(264) << dbgString << endl;
#endif
}

KIconLoader::~KIconLoader()
{
    delete d->mpThemeRoot;
    delete[] d->mpGroups;
    delete d;
}

void KIconLoader::addAppDir(const QString& appname)
{
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/pics/");
    d->mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/toolbar/");
    addAppThemes(appname);
}

void KIconLoader::addAppThemes(const QString& appname)
{
    KIconThemeNode *node = 0L;
    KIconTheme *theme = 0L;

    theme = new KIconTheme("hicolor", appname);
    if (theme->isValid())
	node = new KIconThemeNode(theme);
    else
	delete theme;

    if (node)
    {
	node->links.append(d->mpThemeRoot);
	d->mpThemeRoot = node;
    }

}

void KIconLoader::addBaseThemes(KIconThemeNode *node, const QString &appname)
{
    QStringList lst = node->theme->inherits();
    QStringList::ConstIterator it;

    for (it=lst.begin(); it!=lst.end(); it++)
    {
	if (!d->mThemeList.contains(*it) || d->mThemesInTree.contains(*it))
	    continue;
	KIconTheme *theme = new KIconTheme(*it,appname);
	if (!theme->isValid()) {
	    delete theme;
	    continue;
	}
        KIconThemeNode *n = new KIconThemeNode(theme);
	d->mThemesInTree.append(*it);
	addBaseThemes(n, appname);
	node->links.append(n);
    }
}


KIcon KIconLoader::findMatchingIcon(const QString& name, int size) const
{
    KIcon icon;
    static const QString &png_ext = KGlobal::staticQString(".png");
    icon = d->mpThemeRoot->findIcon(name + png_ext, size, KIcon::MatchExact);
    if (icon.isValid())
      return icon;
    icon = d->mpThemeRoot->findIcon(name + png_ext, size, KIcon::MatchBest);
    if (icon.isValid())
      return icon;

    static const QString &xpm_ext = KGlobal::staticQString(".xpm");
    icon = d->mpThemeRoot->findIcon(name + xpm_ext, size, KIcon::MatchExact);
    if (icon.isValid())
      return icon;
    icon = d->mpThemeRoot->findIcon(name + xpm_ext, size, KIcon::MatchBest);
    if (icon.isValid())
      return icon;

    return icon;
}


// Finds the absolute path to an icon.

QString KIconLoader::iconPath(const QString& _name, int group_or_size,
			      bool canReturnNull) const
{
    if (d->mpThemeRoot == 0L)
	return QString::null;

    if (_name.at(0) == '/')
	return _name;

    QString name = _name;
    QString ext = name.right(4);

    static const QString &png_ext = KGlobal::staticQString(".png");
    static const QString &xpm_ext = KGlobal::staticQString(".xpm");
    if (ext == png_ext || ext == xpm_ext)
    {
#ifndef NDEBUG
	kdDebug(264) << "Application " << KGlobal::instance()->instanceName()
                     << " loads icon " << name << " with extension.\n";
#endif
	name = name.left(name.length() - 4);
    }

    QString path;
    if (group_or_size == KIcon::User)
    {
	path = d->mpDirs->findResource("appicon", name + png_ext);
	if (path.isEmpty())
	     path = d->mpDirs->findResource("appicon", name + xpm_ext);
	return path;
    }
    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << "\n";
	return path;
    }

    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;
    KIcon icon = findMatchingIcon(name, size);

    if (!icon.isValid())
    {
	// Try "User" group too.
	path = iconPath(name, KIcon::User, true);
	if (!path.isEmpty() || canReturnNull)
	    return path;

	if (canReturnNull)
	    return QString::null;
	icon = findMatchingIcon("unknown", size);
	if (!icon.isValid())
	{
	    kdDebug(264) << "Warning: could not find \"Unknown\" icon for size = "
		         << size << "\n";
	    return QString::null;
	}
    }
    return icon.path;
}

QPixmap KIconLoader::loadIcon(const QString& _name, int group, int size,
	int state, QString *path_store, bool canReturnNull) const
{
    QString name = _name;
    QPixmap pix;
    QString key;
    bool absolutePath=false;

    if (d->mpThemeRoot == 0L)
	return pix;

    // Special case for absolute path icons.
    if (name.at(0) == '/') absolutePath=true;

    static const QString &str_unknown = KGlobal::staticQString("unknown");

    // Special case for "User" icons.
    if (group == KIcon::User)
    {
	key = "$kicou_";
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
		kdDebug(264) << "Warning: Cannot find \"unknown\" icon.\n";
		return pix;
	    }
	}

	if (path_store != 0L)
	    *path_store = path;
	if (inCache)
	    return pix;
	pix.load(path);
	QPixmapCache::insert(key, pix);
	return pix;
    }

    // Regular case: Check parameters

    if ((group < -1) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	group = 0;
    }

    int overlay = (state & KIcon::OverlayMask);
    state &= ~KIcon::OverlayMask;
    if ((state < 0) || (state >= KIcon::LastState))
    {
	kdDebug(264) << "Illegal icon state: " << state << "\n";
	state = 0;
    }

    if ((size == 0) && (group < 0))
    {
	kdDebug(264) << "Neither size nor group specified!\n";
	group = 0;
    }

    if (!absolutePath)
    {
        static const QString &png_ext = KGlobal::staticQString(".png");
        static const QString &xpm_ext = KGlobal::staticQString(".xpm");

        if (!canReturnNull && name.isEmpty())
            name = str_unknown;
        else
        {
	    QString ext = name.right(4);
	    if (ext == png_ext || ext == xpm_ext)
	    {
#ifndef NDEBUG
	        kdDebug(264) << "Application "
		    << KGlobal::instance()->instanceName()
		    << " loads icon " << name << " with extension.\n";
#endif
	        name = name.left(name.length() - 4);
            }
	}
    }

    // If size == 0, use default size for the specified group.
    if (size == 0)
    {
	size = d->mpGroups[group].size;
    }

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
        if (absolutePath)
        {
            icon.context=KIcon::Any;
            icon.type=KIcon::Scalable;
            icon.path=name;
        }
        else
        {
            icon = findMatchingIcon(name, size);
            if (!icon.isValid())
            {
                // Try "User" icon too. Some apps expect this.
                pix = loadIcon(name, KIcon::User, size, state, path_store, true);
                if (!pix.isNull() || canReturnNull)
                    return pix;

                icon = findMatchingIcon(str_unknown, size);
                if (!icon.isValid())
                {
                    kdDebug(264)
                        << "Warning: could not find \"Unknown\" icon for size = "
                        << size << "\n";
                    return pix;
                }
            }
        }

        if (path_store != 0L)
            *path_store = icon.path;
        if (inCache)
            return pix;

        img=new QImage(icon.path);
        if (img->isNull())
            return pix;

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
        if (overlay & KIcon::HiddenOverlay)
            for (int y = 0; y < img->height(); y++)
            {
		Q_UINT32 *line = reinterpret_cast<Q_UINT32 *>(img->scanLine(y));
                for (int x = 0; x < img->width();  x++)
                    line[x] = (line[x] & 0x00ffffff) | (QMIN(0x80, qAlpha(line[x])) << 24);
	    }
    }

    // Scale the icon and apply effects if necessary
    if ((iconType == KIcon::Scalable) && (size != img->width()))
    {
	*img = img->smoothScale(size, size);
    }
    if ((iconType == KIcon::Threshold) && (size != img->width()))
    {
	if ( abs(size-img->width())>iconThreshold )
	    *img = img->smoothScale(size, size);
    }
    if ((group >= 0) && d->mpGroups[group].dblPixels)
    {
	*img = d->mpEffect.doublePixels(*img);
    }
    if (group >= 0)
    {
	*img = d->mpEffect.apply(*img, group, state);
    }

    pix.convertFromImage(*img);

    // We only insert the image in the cache if alphaBlending is
    // being used
    if (d->mpGroups[group].alphaBlending)
	d->imgCache.insert(pix.serialNumber(), img);
    else
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
	kdDebug(264) << "Overlay " << name << "not found.\n";
	return 0L;
    }
    image = new QImage(icon.path);
    d->imgDict.insert(key, image);
    return image;
}

QStringList KIconLoader::loadAnimated(const QString& name, int group, int size) const
{
    QStringList lst;

    if (!d->mpGroups) return lst;

    if ((group < -1) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	group = 0;
    }
    if ((size == 0) && (group < 0))
    {
	kdDebug(264) << "Neither size nor group specified!\n";
	group = 0;
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

KIconTheme *KIconLoader::theme()
{
    if (d->mpThemeRoot) return d->mpThemeRoot->theme;
    return 0L;
}

int KIconLoader::currentSize(int group)
{
    if (!d->mpGroups) return -1;

    if ((group < 0) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return -1;
    }
    return d->mpGroups[group].size;
}

QStringList KIconLoader::queryIconsByContext(int group_or_size,
					    int context) const
{
    QStringList result;
    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << "\n";
	return result;
    }
    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;
    d->mpThemeRoot->queryIconsByContext(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    QString name;
    QStringList res2, entries;
    QStringList::ConstIterator it;
    for (it=result.begin(); it!=result.end(); it++)
    {
	int n = (*it).findRev('/');
	if (n == -1)
	    name = *it;
	else
	    name = (*it).mid(n+1);
	if (!entries.contains(name))
	{
	    entries += name;
	    res2 += *it;
	}
    }
    return res2;

}

QStringList KIconLoader::queryIcons(int group_or_size, int context) const
{
    QStringList result;
    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << "\n";
	return result;
    }
    int size;
    if (group_or_size >= 0)
	size = d->mpGroups[group_or_size].size;
    else
	size = -group_or_size;
    d->mpThemeRoot->queryIcons(&result, size, context);

    // Eliminate duplicate entries (same icon in different directories)
    QString name;
    QStringList res2, entries;
    QStringList::ConstIterator it;
    for (it=result.begin(); it!=result.end(); it++)
    {
	int n = (*it).findRev('/');
	if (n == -1)
	    name = *it;
	else
	    name = (*it).mid(n+1);
	if (!entries.contains(name))
	{
	    entries += name;
	    res2 += *it;
	}
    }
    return res2;
}

KIconEffect * KIconLoader::iconEffect()
{
    return &d->mpEffect;
}

bool KIconLoader::alphaBlending(int group) const
{
    if (!d->mpGroups) return -1;

    if ((group < 0) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return -1;
    }
    return d->mpGroups[group].alphaBlending;
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
    QIconSet iconset;
    iconset.setPixmap(
	DesktopIcon(name, force_size, KIcon::ActiveState, instance) ,
	QIconSet::Automatic, QIconSet::Active );
    iconset.setPixmap(
	DesktopIcon(name, force_size, KIcon::DisabledState, instance) ,
	QIconSet::Automatic, QIconSet::Disabled );
    iconset.setPixmap(
	DesktopIcon(name, force_size, KIcon::DefaultState, instance) ,
	QIconSet::Automatic, QIconSet::Normal );
    return iconset;
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
    QIconSet iconset;
    iconset.setPixmap(
	BarIcon(name, force_size, KIcon::ActiveState, instance) ,
	QIconSet::Automatic, QIconSet::Active );
    iconset.setPixmap(
	BarIcon(name, force_size, KIcon::DisabledState, instance) ,
	QIconSet::Automatic, QIconSet::Disabled );
    iconset.setPixmap(
	BarIcon(name, force_size, KIcon::DefaultState, instance) ,
	QIconSet::Automatic, QIconSet::Normal );
    return iconset;
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
    QIconSet iconset;
    iconset.setPixmap(
	SmallIcon(name, force_size, KIcon::ActiveState, instance) ,
	QIconSet::Automatic, QIconSet::Active );
    iconset.setPixmap(
	SmallIcon(name, force_size, KIcon::DisabledState, instance) ,
	QIconSet::Automatic, QIconSet::Disabled );
    iconset.setPixmap(
	SmallIcon(name, force_size, KIcon::DefaultState, instance) ,
	QIconSet::Automatic, QIconSet::Normal );
    return iconset;
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
    QIconSet iconset;
    iconset.setPixmap(
	MainBarIcon(name, force_size, KIcon::ActiveState, instance) ,
	QIconSet::Automatic, QIconSet::Active );
    iconset.setPixmap(
	MainBarIcon(name, force_size, KIcon::DisabledState, instance) ,
	QIconSet::Automatic, QIconSet::Disabled );
    iconset.setPixmap(
	MainBarIcon(name, force_size, KIcon::DefaultState, instance) ,
	QIconSet::Automatic, QIconSet::Normal );
    return iconset;
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
    QIconSet iconset;
    iconset.setPixmap(
	UserIcon(name, KIcon::ActiveState, instance) ,
	QIconSet::Automatic, QIconSet::Active );
    iconset.setPixmap(
	UserIcon(name, KIcon::DisabledState, instance) ,
	QIconSet::Automatic, QIconSet::Disabled );
    iconset.setPixmap(
	UserIcon(name, KIcon::DefaultState, instance) ,
	QIconSet::Automatic, QIconSet::Normal );
    return iconset;
}

int IconSize(int group, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->currentSize(group);
}

QImage *KIconLoader::image(int key) const
{
    return d->imgCache.find(key);
}

QPixmap KIconLoader::unknown()
{
    QPixmap pix;
    if ( QPixmapCache::find("unknown", pix) )
            return pix;

    QString path = KGlobal::iconLoader()->iconPath("unknown", KIcon::Small, true);
    if (path.isEmpty())
    {
	kdDebug(264) << "Warning: Cannot find \"unknown\" icon.\n";
	pix.resize(32,32);
    } else
    {
        pix.load(path);
        QPixmapCache::insert("unknown", pix);
    }

    return pix;
}
