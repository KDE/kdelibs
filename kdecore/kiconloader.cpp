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


/*
 * A node in the icon theme dependancy tree.
 */
class KIconThemeNode
{
public:
    KIconThemeNode() { links.setAutoDelete(true); }
    KIconTheme *theme;
    QList<KIconThemeNode> links;
};

/*
 * d pointer for KIconLoader
 */
struct KIconLoaderPrivate
{
    QString dbgString;
    QStringList appDirs;
    QStringList extensions;
};

/*
 * Icon type description.
 */
struct KIconGroup
{
    int size;
    bool dblPixels;
};


/*** KIconLoader: the icon loader ***/

KIconLoader::KIconLoader(QString appname)
{
    d = new KIconLoaderPrivate;
    mpEffect = new KIconEffect;
    mpDirs = KGlobal::dirs();
    mpGroups = new KIconGroup[8];
    KConfig *config = KGlobal::config();

    d->extensions += ".png";
    d->extensions += ".xpm";

    QStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";

    int i;
    QStringList::ConstIterator it;
    for (it=groups.begin(), i=0; it!=groups.end(); it++, i++)
    {
	config->setGroup(*it + "Icons");
	mpGroups[i].size = config->readNumEntry("Size");
	mpGroups[i].dblPixels = config->readBoolEntry("DoublePixels");
    }

    mThemeList = KIconTheme::list();
    if (!mThemeList.contains("hicolor") || !mThemeList.contains("locolor"))
    {
	kdDebug(264) << "Error: standard icon themes: \"locolor\" and "
	                "hicolor not found!\nBig trouble!\n";
	return;
    }

    mTheme = KIconTheme::current();
    KIconTheme *root = new KIconTheme(mTheme);
    if ((root->depth() == 32) && !(QPixmap::defaultDepth() > 8))
    {
	kdDebug(264) << "Using theme locolor instead of " << mTheme 
		     << " because display depth is too small.\n";
	mTheme = "locolor";
    }

    // Add global themes to the theme tree.
    mpThemeRoot = new KIconThemeNode;
    mThemeTree += mTheme;
    addIconTheme(root, mpThemeRoot);

    // Insert application specific ones at the top.
    if (appname.isEmpty()) 
	appname = KGlobal::instance()->instanceName();
    addAppDir(appname);

    d->dbgString = "Theme tree: ";
    printThemeTree(mpThemeRoot);
    kdDebug(264) << d->dbgString << "\n";

    // Fix default icon sizes
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	if (mpGroups[i].size == 0)
	    mpGroups[i].size = mpThemeRoot->theme->defaultSize(i);
    }
}

KIconLoader::~KIconLoader()
{
    delete d;
    delete mpThemeRoot;
    delete[] mpGroups;
}

void KIconLoader::addAppDir(QString appname)
{
    if (d->appDirs.contains(appname))
	return;
    d->appDirs += appname;

    mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/pics/");
    mpDirs->addResourceType("appicon", KStandardDirs::kde_default("data") +
		appname + "/toolbar/");
    addAppThemes(appname);
}
    
void KIconLoader::addIconTheme(KIconTheme *theme, KIconThemeNode *node)
{
    node->theme = theme;
    QStringList lst = node->theme->inherits();
    QStringList::Iterator it;
    for (it=lst.begin(); it!=lst.end(); it++)
    {
	if (!mThemeList.contains(*it) || mThemeTree.contains(*it))
	    continue;
	KIconThemeNode *n = new KIconThemeNode;
	mThemeTree += *it;
	addIconTheme(new KIconTheme(*it), n);
	node->links.append(n);
    }
}

void KIconLoader::addIcons(QStringList *result, int size, int context,
	KIconThemeNode *node)
{
    *result += node->theme->queryIcons(size, context);
    KIconThemeNode *n;
    for (n=node->links.first(); n!=0L; n=node->links.next())
    {
	addIcons(result, size, context, n);
    }
}

void KIconLoader::addAppThemes(QString appname)
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
	top->links.append(mpThemeRoot);
	mpThemeRoot = top;
    }
}

void KIconLoader::printThemeTree(KIconThemeNode *node)
{
    d->dbgString += "(";
    d->dbgString += node->theme->name();
    bool first;
    KIconThemeNode *n;
    for (first=true, n=node->links.first(); n; n=node->links.next(), first=false)
    {
	if (first) d->dbgString += ": ";
	else d->dbgString += ", ";
	printThemeTree(n);
    }
    d->dbgString += ")";
}

QString KIconLoader::iconPath(QString name, int group_or_size,
	bool canReturnNull)
{
    if (name.at(0) == '/')
	return name;
    if (mpThemeRoot == 0L)
	return QString::null;

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
	path = mpDirs->findResource("appicon", name + ".png");
	if (path.isEmpty())
	     path = mpDirs->findResource("appicon", name + ".xpm");
	return path;
    }
    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << "\n";
	return path;
    }

    int size;
    if (group_or_size >= 0)
	size = mpGroups[group_or_size].size;
    else
	size = -group_or_size;
    KIcon icon = iconPath2(name, size);
    if (!icon.isValid())
    {
	// Try "User" group too.
	path = iconPath(name, KIcon::User, canReturnNull);
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

KIcon KIconLoader::iconPath2(QString name, int size)
{
    KIcon icon;
    QStringList::Iterator it;
    for (it=d->extensions.begin(); it!=d->extensions.end(); it++)
    {
	icon = iconPath2(name + *it, size, KIcon::MatchExact, mpThemeRoot);
	if (icon.isValid())
	    break;
	icon = iconPath2(name + *it, size, KIcon::MatchBest, mpThemeRoot);
	if (icon.isValid())
	    break;
    }
    return icon;
}

KIcon KIconLoader::iconPath2(QString name, int size, 
	int match, KIconThemeNode *node)
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

QPixmap KIconLoader::loadIcon(QString name, int group, int size,
	int state, QString *path_store, bool canReturnNull)
{
    QPixmap pix;
    if (mpThemeRoot == 0L)
	return pix;

    QString key;
    if (name.at(0) == '/')
    {
	if (path_store != 0L)
	    *path_store = name;
	key = "$kico_a_";
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
	key = "$kico_u_";
	key += name;
	bool inCache = QPixmapCache::find(key, pix);
	if (inCache && (path_store == 0L))
	    return pix;
	QString path = iconPath(name, KIcon::User, canReturnNull);
	if (path.isEmpty())
	    return pix;
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
	size = mpGroups[group].size;
    }

    key = "$kico_";
    key += QString().setNum(size); key += "_";
    key += QString().setNum(state); key += "-";
    key += name;
    bool inCache = QPixmapCache::find(key, pix);
    if (inCache && (path_store == 0L))
	return pix;

    KIcon icon = iconPath2(name, size);
    if (!icon.isValid())
    {
	// Try "User" icon too.
	pix = loadIcon(name, KIcon::User, size, state, path_store, canReturnNull);
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
    if ((group >= 0) && mpGroups[group].dblPixels)
    {
	img = mpEffect->doublePixels(img);
    }
    if (group >= 0)
    {
	img = mpEffect->apply(img, group, state);
    }

    pix.convertFromImage(img);
    QPixmapCache::insert(key, pix);
    return pix;
}

KIconTheme *KIconLoader::theme()
{
    if (mpThemeRoot == 0L)
	return 0L;
    return mpThemeRoot->theme;
}
    
int KIconLoader::currentSize(int group)
{
    if ((group < 0) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return -1;
    }
    return mpGroups[group].size;
}

QStringList KIconLoader::queryIcons(int group_or_size, int context)
{
    QStringList result;
    if (group_or_size >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group_or_size << "\n";
	return result;
    }
    int size;
    if (group_or_size >= 0)
	size = mpGroups[group_or_size].size;
    else
	size = -group_or_size;
    addIcons(&result, size, context, mpThemeRoot);

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

QPixmap DesktopIcon(QString name, int force_size, int state, 
	KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, KIcon::Desktop, force_size, state);
}

QPixmap DesktopIcon(QString name, KInstance *instance)
{
    return DesktopIcon(name, 0, KIcon::DefaultState, instance);
}

QPixmap BarIcon(QString name, int force_size, int state, 
	KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, KIcon::Toolbar, force_size, state);
}

QPixmap BarIcon(QString name, KInstance *instance)
{
    return BarIcon(name, 0, KIcon::DefaultState, instance);
}

QPixmap SmallIcon(QString name, int force_size, int state, 
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::Small, force_size, state);
}

QPixmap SmallIcon(QString name, KInstance *instance)
{
    return SmallIcon(name, 0, KIcon::DefaultState, instance);
}

QPixmap MainBarIcon(QString name, int force_size, int state, 
	KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::MainToolbar, force_size, state);
}

QPixmap MainBarIcon(QString name, KInstance *instance)
{
    return MainBarIcon(name, 0, KIcon::DefaultState, instance);
}

QPixmap UserIcon(QString name, int state, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::User, 0, state);
}

QPixmap UserIcon(QString name, KInstance *instance)
{
    return UserIcon(name, KIcon::DefaultState, instance);
}

int IconSize(int group, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->currentSize(group);
}

