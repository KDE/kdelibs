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
 * kiconloader.cpp: An icon loader for KDE with theming functionality.
 */

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <kdebug.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kinstance.h>

#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <cassert>

/*
 * A node in the icon theme dependancy tree.
 */
class KIconThemeNode
{
public:
    KIconThemeNode() { links.setAutoDelete(true); }
    KIconTheme *theme;
    QList<KIconThemeNode> links;

    void queryIcons(QStringList *lst, int size, int context) const;

};

void KIconThemeNode::queryIcons(QStringList *result, int size, int context) const
{
  // add the icons of this theme to it
  *result += theme->queryIcons(size, context);
  QListIterator<KIconThemeNode> it(links);
  for (; it.current(); ++it) // recursivly going down
    it.current()->queryIcons(result, size, context);
}

/*
 * Icon type description.
 */
struct KIconGroup
{
    int size;
    bool dblPixels;
};

/*
 * d pointer for KIconLoader
 */
struct KIconLoaderPrivate
{
    QStringList extensions;

    QStringList mThemeList, mThemeTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconEffect mpEffect;
};

// for debugging
static void printThemeTree(KIconThemeNode *node, QString& dbgString);

/*** KIconLoader: the icon loader ***/
KIconLoader::KIconLoader(const QString& _appname)
{
    d = new KIconLoaderPrivate;

    // This is the order we want to look at if no extension is
    // given. ".xpm" should die soon
    d->extensions += ".png";
    d->extensions += ".xpm";

    d->mpDirs = KGlobal::dirs();

    // get a list of available theme dirs in /icons
    d->mThemeList = KIconTheme::list();
    if (!d->mThemeList.contains("hicolor") || !d->mThemeList.contains("locolor"))
    {
	// If someone removed locolor/hicolor without replacing them, better
	// jump out of the program and let him fix that
	kdFatal(264) << "Error: standard icon themes: \"locolor\" and "
		     << "hicolor not found!\n"; // passing two lines won't work
	return;
    }

    // get the default theme or the one written to config, e.g. "hicolor"
    // the default varies depending on the color depth
    KIconTheme *root = new KIconTheme(KIconTheme::current());

    // Add global themes to the theme tree.
    d->mpThemeRoot = new KIconThemeNode;
    d->mThemeTree += root->name();
    addIconTheme(root, d->mpThemeRoot);

    // this are the (absolute) dirs we're going to look at when we get an unthemed icon
    // we're looking into everything else first though
    QStringList dirs = d->mpDirs->resourceDirs("icon") + d->mpDirs->resourceDirs("pixmap");
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); it++)
      d->mpDirs->addResourceDir("appicon", *it);

    QString dbgString = "Theme tree: ";
    printThemeTree(d->mpThemeRoot, dbgString);
    kdDebug(264) << dbgString << endl;

    // relying on LastGroup's value becomes dangerous as soon
    // as someone is changing KIcon's enum to be bases of 10 ;/
    d->mpGroups = new KIconGroup[KIcon::LastGroup];

    // these are the currently known groups and they have to match the order in
    // kicontheme.h
    const char *groups[] = { "Desktop", "Toolbar", "MainToolbar", 0, "Small", 0 };
    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs(config, "dummy"); // we need it to reset it later

    // loading config and default sizes
    bool isnull = false;
    for (int i = 0; i < KIcon::LastGroup; i++) {
      assert(!isnull);
      if (groups[i] == 0) {
	isnull = true;
	// we continue here to catch the assert above if
	// someone added a group in KIcon without adding a group
	// here
	continue;
      }
      config->setGroup(QString::fromLatin1(groups[i]) + "Icons");
      d->mpGroups[i].size = config->readNumEntry("Size", 0);
      if (d->mpGroups[i].size) // no config set
	  d->mpGroups[i].size = d->mpThemeRoot->theme->defaultSize(i);
      d->mpGroups[i].dblPixels = config->readBoolEntry("DoublePixels", false);
    }

    // by now everything should be setup, it's time to do the
    // real things
    QString appname = _appname;

    // Insert application specific ones at the top.
    if (appname.isEmpty())
	appname = KGlobal::instance()->instanceName();
    addAppDir(appname);
}

KIconLoader::~KIconLoader()
{
    // the ...Private destructor could destruct
    // them too, but then this would be its only task :/
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

void KIconLoader::addIconTheme(KIconTheme *theme, KIconThemeNode *node)
{
    node->theme = theme;
    QStringList lst = node->theme->inherits();
    QStringList::ConstIterator it;
    for (it=lst.begin(); it!=lst.end(); it++)
    {
	if (!d->mThemeList.contains(*it) || d->mThemeTree.contains(*it))
	    continue;
	KIconThemeNode *n = new KIconThemeNode;
	d->mThemeTree += *it;
	addIconTheme(new KIconTheme(*it), n);
	node->links.append(n);
    }
}

void KIconLoader::addAppThemes(const QString& appname)
{
    KIconThemeNode *node = new KIconThemeNode;
    KIconThemeNode *top = node, *tmp;
    KIconTheme *theme;

    node->theme = 0L;
    if (QPixmap::defaultDepth() > 8)
    {
	theme = new KIconTheme("hicolor", appname);
	if (theme->isValid())
	    node->theme = theme;
	else
	    delete theme;
    }

    theme = new KIconTheme("locolor", appname);
    if (theme->isValid())
    {
	if (node->theme != 0L)
	{
	    tmp = new KIconThemeNode;
	    node->links.append(tmp);
	    node = tmp;
	}
	node->theme = theme;
    } else
	delete theme;

    if (node->theme != 0L)
    {
	top->links.append(d->mpThemeRoot);
	d->mpThemeRoot = top;
    }
}

static void printThemeTree(KIconThemeNode *node, QString& dbgString)
{
    dbgString += "(";
    dbgString += node->theme->name();
    bool first;
    KIconThemeNode *n;
    for (first=true, n=node->links.first(); n; n=node->links.next(), first=false)
    {
	if (first) dbgString += ": ";
	else dbgString += ", ";
	printThemeTree(n, dbgString);
    }
    dbgString += ")";
}

QString KIconLoader::iconPath(const QString& _name, int group_or_size,
	bool canReturnNull) const
{
    if (_name.at(0) == '/')
	return _name;

    QString name = _name;
    QString ext = name.right(4);

    if ((ext == ".png") || (ext == ".xpm"))
    {
	kdDebug(264) << "Application " << KGlobal::instance()->instanceName()
		<< " loads icon with extension.\n";
	name = name.left(name.length() - 4);
    }

    QString path;
    if (group_or_size == KIcon::User)
    {
	path = d->mpDirs->findResource("appicon", name + ".png");
	if (path.isEmpty())
	     path = d->mpDirs->findResource("appicon", name + ".xpm");
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
    KIcon icon = iconPath2(name, size);
    if (!icon.isValid())
    {
	// Try "User" group too.
	path = iconPath(name, KIcon::User, true);
	if (!path.isEmpty() || canReturnNull)
	    return path;

	if (canReturnNull)
	    return QString::null;
	icon = iconPath2("unknown", size);
	if (!icon.isValid())
	{
	    kdDebug(264) << "Warning: could not find \"Unknown\" icon for size = "
		         << size << "\n";
	    return QString::null;
	}
    }
    return icon.path;
}

KIcon KIconLoader::iconPath2(const QString& name, int size) const
{
    KIcon icon;
    QStringList::ConstIterator it;
    for (it=d->extensions.begin(); it!=d->extensions.end(); it++)
    {
	icon = iconPath2(name + *it, size, KIcon::MatchExact, d->mpThemeRoot);
	if (icon.isValid())
	    break;
	icon = iconPath2(name + *it, size, KIcon::MatchBest, d->mpThemeRoot);
	if (icon.isValid())
	    break;
    }
    return icon;
}

KIcon KIconLoader::iconPath2(const QString& name, int size,
			     int match, KIconThemeNode *node) const
{
    KIcon icon;
    icon = node->theme->iconPath(name, size, match);
    if (icon.isValid())
	return icon;
    KIconThemeNode *n;
    for (n=node->links.first(); n!=0L; n=node->links.next())
    {
	icon = iconPath2(name, size, match, n);
	if (icon.isValid())
	    break;
    }
    return icon;
}

QPixmap KIconLoader::loadIcon(const QString& _name, int group, int size,
	int state, QString *path_store, bool canReturnNull) const
{
    QString name = _name;
    QPixmap pix;

    QString key;
    /** if the name is an absolute path, we know everything
	and just store it in the cache to have it faster next
	time it's asked for
    */
    if (name.at(0) == '/')
    {
	if (path_store != 0L)
	    *path_store = name;
	key = "$kicoa_";
	key += name;
	if (QPixmapCache::find(key, pix))
	    return pix;
	pix.load(name);
	QPixmapCache::insert(key, pix);
	return pix;
    }

    // Special case for "User" icons.
    if (group == KIcon::User)
    {
	key = "$kicou_";
	key += name;
	bool inCache = QPixmapCache::find(key, pix);
	if (inCache && (path_store == 0L))
	    return pix;
	QString path = iconPath(name, KIcon::User, canReturnNull);
	if (path.isEmpty())
	{
	    if (canReturnNull)
		return pix;
	    // We don't know the desired size: use small
	    path = iconPath("unknown", KIcon::Small, true);
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

    if ((group < -1) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	group = 0;
    }
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
    QString ext = name.right(4);
    if ((ext == ".png") || (ext == ".xpm"))
    {
	kdDebug(264) << "Application " << KGlobal::instance()->instanceName()
		<< " loads icon with extension.\n";
	name = name.left(name.length() - 4);
    }

    if (size == 0)
    {
	size = d->mpGroups[group].size;
    }

    // Generate a unique cache key for the icon.
    key = "$kico_";
    key += name; key += "_";
    key += QString().setNum(size); key += "_";
    if (group >= 0)
    {
	key += d->mpEffect.fingerprint(group, state);
	if (d->mpGroups[group].dblPixels)
	    key += QString::fromLatin1(":dblsize");
    } else
	key += QString::fromLatin1("noeffect");

    bool inCache = QPixmapCache::find(key, pix);
    if (inCache && (path_store == 0L))
	return pix;

    KIcon icon = iconPath2(name, size);
    if (!icon.isValid())
    {
	// Try "User" icon too.
	pix = loadIcon(name, KIcon::User, size, state, path_store, true);
	if (!pix.isNull() || canReturnNull)
	    return pix;

	icon = iconPath2("unknown", size);
	if (!icon.isValid())
	{
	    kdDebug(264) << "Warning: could not find \"Unknown\" icon for size = "
		         << size << "\n";
	    return pix;
	}
    }

    if (path_store != 0L)
	*path_store = icon.path;
    if (inCache)
	return pix;

    QImage img(icon.path);
    if (img.isNull())
	return pix;

    // Scale the icon and apply effects if necessary
    if ((icon.type == KIcon::Scalable) && (size != img.width()))
    {
	img = img.smoothScale(size, size);
    }
    if ((group >= 0) && d->mpGroups[group].dblPixels)
    {
	img = d->mpEffect.doublePixels(img);
    }
    if (group >= 0)
    {
	img = d->mpEffect.apply(img, group, state);
    }

    pix.convertFromImage(img);
    QPixmapCache::insert(key, pix);
    return pix;
}

QStringList KIconLoader::loadAnimated(const QString& name, int group, int size) const
{
    QStringList lst;

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
	KIcon icon = iconPath2(file, size);
	file = icon.isValid() ? icon.path : QString::null;
    }
    if (file.isEmpty())
	return lst;
    lst += file;

    int i=1;
    QString fmt = file.left(file.length() - 8) + "%04d.png";
    file.sprintf(fmt.latin1(), i);
    QFileInfo fi(file);
    while (fi.exists())
    {
	lst += file;
	file.sprintf(fmt.latin1(), ++i);
	fi.setFile(file);
    }
    return lst;
}

KIconTheme *KIconLoader::theme()
{
    return d->mpThemeRoot->theme;
}

int KIconLoader::currentSize(int group)
{
    if ((group < 0) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return -1;
    }
    return d->mpGroups[group].size;
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

// Easy access functions

QPixmap DesktopIcon(const QString& name, int force_size, int state,
	KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, KIcon::Desktop, force_size, state);
}

QPixmap DesktopIcon(const QString& name, KInstance *instance)
{
    return DesktopIcon(name, 0, KIcon::DefaultState, instance);
}

QPixmap BarIcon(const QString& name, int force_size, int state,
	KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, KIcon::Toolbar, force_size, state);
}

QPixmap BarIcon(const QString& name, KInstance *instance)
{
    return BarIcon(name, 0, KIcon::DefaultState, instance);
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

QPixmap UserIcon(const QString& name, int state, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::User, 0, state);
}

QPixmap UserIcon(const QString& name, KInstance *instance)
{
    return UserIcon(name, KIcon::DefaultState, instance);
}

int IconSize(int group, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->currentSize(group);
}

