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

#include "kicontheme.h"
#include "kiconloader.h"


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
    int defOldSize;
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
    KConfig *config = KGlobal::config();

    // SCI (Source compatibility issue)
    config->setGroup("KDE");
    QString tmp = config->readEntry("KDEIconStyle");
    if (tmp == "Large")
	d->defOldSize = 48;
    else if (tmp == "Small")
	d->defOldSize = 16;
    else
	d->defOldSize = 32;

    mpDirs = KGlobal::dirs();
    config->setGroup("Icons");
    mTheme = config->readEntry("Theme");
    mpGroups = new KIconGroup[12];

    QStringList groups;
    groups += "Desktop";
    groups += "Kicker";
    groups += "Toolbar";
    groups += "Small";
    groups += "ListItem";

    int i;
    QStringList::ConstIterator it;
    for (it=groups.begin(), i=0; it!=groups.end(); it++, i++)
    {
	mpGroups[i].size = config->readNumEntry(*it + "Size");
	mpGroups[i].dblPixels = config->readBoolEntry(*it + "DoublePixels");
    }

    mThemeList = KIconTheme::list();
    if (!mThemeList.contains("hicolor") || !mThemeList.contains("locolor"))
    {
	kdDebug(264) << "Error: standard icon themes: \"locolor\" and "
	                "hicolor not found!\nBig trouble!\n";
	return;
    }
    
    if (!mThemeList.contains(mTheme))
    {
	if (QPixmap::defaultDepth() > 8)
	    mTheme = "hicolor";
	else
	    mTheme = "locolor";
    }
    KIconTheme *root = new KIconTheme(mTheme);
    if ((root->depth() == 32) && (QPixmap::defaultDepth() < 16))
    {
	kdDebug(264) << "Using theme locolor instead of " << mTheme 
		     << " because display depth is too small.\n";
	mTheme = "locolor";
    }
    mpThemeRoot = new KIconThemeNode;
    mThemeTree += mTheme;
    addIconTheme(root, mpThemeRoot);

    d->dbgString = "Theme tree: ";
    printThemeTree(mpThemeRoot);
    kdDebug(264) << d->dbgString << "\n";

    // Fix default icon sizes
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	if (mpGroups[i].size == 0)
	    mpGroups[i].size = root->defaultSize(i);
    }

    // Add application specific directories for special Group = User
    if (appname.isEmpty()) 
	appname = KGlobal::instance()->instanceName();
    mpDirs->addResourceType("usricon", KStandardDirs::kde_default("data") +
		appname + "/pics/");
    mpDirs->addResourceType("usricon", KStandardDirs::kde_default("data") +
		appname + "/toolbar/");
}

KIconLoader::~KIconLoader()
{
    delete d;
    delete mpThemeRoot;
    delete[] mpGroups;
}

void KIconLoader::addUserDir(QString appname)
{
    mpDirs->addResourceType("usricon", KStandardDirs::kde_default("data") +
		appname + "/pics/");
    mpDirs->addResourceType("usricon", KStandardDirs::kde_default("data") +
		appname + "/toolbar/");
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
    bool sci = false;  // Source compatibility with older apps
    switch (group_or_size)
    {
    case KIconLoader::Small:
	group_or_size = -16;
	sci = true;
	break;
    case KIconLoader::Medium:
	group_or_size = -32;
	sci = true;
	break;
    case KIconLoader::Large:
	group_or_size = -48;
	sci = true;
	break;
    case KIconLoader::Default:
	group_or_size = -d->defOldSize;
	sci = true;
	break;
    case KIcon::User:
	path = mpDirs->findResource("usricon", name + ".png");
	if (path.isEmpty())
	     path = mpDirs->findResource("usricon", name + ".xpm");
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
    icon = iconPath2(name, size, KIcon::MatchExact, mpThemeRoot);
    if (!icon.isValid())
	icon = iconPath2(name, size, KIcon::MatchBest, mpThemeRoot);
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

QPixmap KIconLoader::loadIcon(QString name, int group_or_size,
	QString *path_store, bool canReturnNull)
{
    QPixmap pix;
    if (mpThemeRoot == 0L)
	return pix;
    if (name.at(0) == '/')
    {
	pix.load(name);
	return pix;
    }

    QString ext = name.right(4);
    if ((ext == ".png") || (ext == ".xpm"))
    {
	kdDebug(264) << "Application " << KGlobal::instance()->instanceName()
		<< " loads icon with extension.\n";
	name = name.left(name.length() - 4);
    }

    QString path;
    bool sci = false;
    switch (group_or_size)
    {
    case KIconLoader::Small:
	group_or_size = -16;
	sci = true;
	break;
    case KIconLoader::Medium:
	group_or_size = -32;
	sci = true;
	break;
    case KIconLoader::Large:
	group_or_size = -48;
	sci = true;
	break;
    case KIconLoader::Default:
	group_or_size = -d->defOldSize;
	sci = true;
	break;
    case KIcon::User:
	path = iconPath(name, KIcon::User, canReturnNull);
	if (path.isEmpty())
	    return pix;
	pix.load(path);
	if (path_store != 0L)
	    *path_store = path;
	kdDebug(264) << "User icon resolved to " << path << "\n";
	return pix;
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
    QString key = "$kico_";
    key += name;
    key += "_";
    key += QString().setNum((int) size);
    bool inCache = QPixmapCache::find(key, pix);
    if (inCache && (path_store == 0L))
	return pix;

    if (sci)
    {
	kdDebug(264) << "Application " << KGlobal::instance()->instanceName() 
		<< " requests to load legacy icon " << name << " of size "
		<< size << "\n";
    }

    KIcon icon = iconPath2(name, size);
    if (!icon.isValid())
    {
	// Try "User" icon too.
	pix = loadIcon(name, KIcon::User, path_store, canReturnNull);
	if (!pix.isNull() || canReturnNull)
	    return pix;

	if (canReturnNull)
	    return pix;
	icon = iconPath2("unknown", size);
	if (!icon.isValid())
	{
	    kdDebug(264) << "Warning: could not find \"Unknown\" icon for size = "
		         << size << "\n";
	    return pix;
	}
    }
    if (sci)
	kdDebug(264) << "Icon resolved to " << icon.path << "\n";

    if (path_store != 0L)
	*path_store = icon.path;
    if (inCache)
	return pix;

    QImage img(icon.path);
    if (img.isNull())
	return pix;
    int newsize = img.width();
    if (icon.type == KIcon::Scalable)
	newsize = size;
    if (group_or_size >= 0)
    {
	if (mpGroups[group_or_size].dblPixels)
	    newsize *= 2;
    }
    if (newsize != img.width())
	img = img.smoothScale(newsize, newsize);

    pix.convertFromImage(img);
    QPixmapCache::insert(key, pix);
    return pix;
}

const KIconTheme *KIconLoader::theme()
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

QPixmap DesktopIcon(QString name, KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, KIcon::Desktop);
}

QPixmap KickerIcon(QString name, KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, KIcon::Kicker);
}

QPixmap BarIcon(QString name, KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, KIcon::Toolbar);
}

// SCI
QPixmap BarIcon(QString name, int size, KInstance *instace)
{
    KIconLoader *loader = instace->iconLoader();
    return loader->loadIcon(name, size);
}

QPixmap SmallIcon(QString name, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::Small);
}

QPixmap ListIcon(QString name, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::ListItem);
}

QPixmap UserIcon(QString name, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->loadIcon(name, KIcon::User);
}

int IconSize(int group, KInstance *instance)
{
    KIconLoader *loader = instance->iconLoader();
    return loader->currentSize(group);
}

