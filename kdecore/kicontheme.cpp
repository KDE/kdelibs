/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; it comes under the GNU Library General 
 * Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 *
 * kicontheme.cpp: Icon theme handling.
 */

#include <stdlib.h>

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


/**
 * A subdirectory in an icon theme.
 */
class KIconThemeDir
{
public:
    KIconThemeDir(QString dir, KConfigBase *config);

    QString iconPath(QString name);
    QStringList iconList();

    int context() { return mContext; }
    int type() { return mType; }
    int size() { return mSize; }
    int minSize() { return mMinSize; }
    int maxSize() { return mMaxSize; }

private:
    bool mbValid;
    int mType, mSize, mContext;
    int mMinSize, mMaxSize;

    QString mDir;
    QStringList mExtensions;
};


/*** KIconTheme ***/

KIconTheme::KIconTheme(QString name)
{
    QStringList icnlibs = KGlobal::dirs()->resourceDirs("icon");
    QStringList::ConstIterator it;
    for (it=icnlibs.begin(); it!=icnlibs.end(); it++)
    {
	QFileInfo fi(*it + name + "/index.desktop");
	if (fi.exists())
	    break;
    }
    if (it == icnlibs.end())
    {
	kdDebug(264) << "Icon theme " << name << " not found.\n";
	return;
    }
    mDir = *it + name + "/";
    KConfig *cfg = new KSimpleConfig(mDir + "index.desktop");
    cfg->setGroup("KDE Icon Theme");
    mName = cfg->readEntry("Name");
    mDesc = cfg->readEntry("Description");
    mDepth = cfg->readNumEntry("DisplayDepth", 32);
    mInherits = cfg->readListEntry("Inherits");
    
    int i;
    QStringList groups;
    groups += "Desktop";
    groups += "Kicker";
    groups += "Toolbar";
    groups += "Small";
    groups += "ListItem";
    for (it=groups.begin(), i=0; it!=groups.end(); it++, i++)
    {
	mDefSize[i] = cfg->readNumEntry(*it + "Default", 32);
	mSizes[i] = cfg->readIntListEntry(*it + "Sizes");
    }

    QStringList dirs = cfg->readListEntry("Directories");
    mDirs.setAutoDelete(true);
    for (it=dirs.begin(); it!=dirs.end(); it++)
    {
	cfg->setGroup(*it);
	mDirs.append(new KIconThemeDir(mDir + *it, cfg));
    }
    delete cfg;
}

KIconTheme::~KIconTheme()
{
}

int KIconTheme::defaultSize(int group)
{
    if ((group < 0) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return -1;
    }
    return mDefSize[group];
}

QValueList<int> KIconTheme::querySizes(int group)
{
    QValueList<int> empty;
    if ((group < 0) || (group >= KIcon::LastGroup))
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return empty;
    }
    return mSizes[group];
}
	
QStringList KIconTheme::queryIcons(int size, int context)
{
    int delta = 1000, dw;
    KIconThemeDir *dir;

    // Try to find exact match
    for (dir=mDirs.first(); dir!=0L; dir=mDirs.next())
    {
	if ((context != KIcon::Any) && (context != dir->context()))
	    continue;
	if ((dir->type() == KIcon::Fixed) && (dir->size() == size))
	    return dir->iconList();
	if ((dir->type() == KIcon::Scalable) && 
		(size >= dir->minSize()) && (size <= dir->maxSize()))
	    return dir->iconList();
    }

    // Find best match
    KIconThemeDir *best = 0L;
    for (dir=mDirs.first(); dir!=0L; dir=mDirs.next())
    {
	if ((context != KIcon::Any) && (context != dir->context()))
	    continue;
	dw = abs(dir->size() - size);
	if (dw >= delta)
	    continue;
	delta = dw;
	best = dir;
    }
    if (best == 0L)
	return QStringList();
    return best->iconList();
}

KIcon KIconTheme::iconPath(QString name, int size, int match)
{
    KIcon icon;
    QString path;
    int delta = 1000, dw;
    KIconThemeDir *dir;
    for (dir=mDirs.first(); dir!=0L; dir=mDirs.next())
    {
	if (match == KIcon::MatchExact) 
	{
	    if ((dir->type() == KIcon::Fixed) && (dir->size() != size))
		continue;
	    if ((dir->type() == KIcon::Scalable) && 
		    ((size < dir->minSize()) || (size > dir->maxSize())))
		continue;
	} else
	{
	    dw = abs(dir->size() - size);
	    if (dw >= delta)
		continue;
	}

	path = dir->iconPath(name);
	if (path.isEmpty())
	    continue;
	icon.path = path;
	icon.size = dir->size();
	icon.type = dir->type();
	icon.context = dir->context();

	if (match == KIcon::MatchExact)
	    break;
	else
	    delta = dw;
    }
    return icon;
}


// static
QStringList KIconTheme::list()
{
    QStringList result;
    QStringList icnlibs = KGlobal::dirs()->resourceDirs("icon");
    QStringList::ConstIterator it;
    for (it=icnlibs.begin(); it!=icnlibs.end(); it++)
    {
        QDir dir(*it);
        if (!dir.exists())
            continue;
        QStringList lst = dir.entryList(QDir::Dirs);
        QStringList::ConstIterator it2;
        QFileInfo fi;
        for (it2=lst.begin(); it2!=lst.end(); it2++)
        {
            if ((*it2 == ".") || (*it2 == ".."))
                continue;
            fi.setFile(*it + *it2 + "/index.desktop");
            if (!fi.exists())
                continue;
            if (!result.contains(*it2))
		result += *it2;
        }
    }     
    return result;
}


/*** KIconThemeDir ***/

KIconThemeDir::KIconThemeDir(QString dir, KConfigBase *config)
{
    mbValid = false;
    mDir = dir;
    mExtensions += ".png";
    mExtensions += ".xpm";
    mSize = config->readNumEntry("Size");
    if (mSize == 0)
	return;

    QString tmp = config->readEntry("Context");
    if (tmp == "Devices")
	mContext = KIcon::Device;
    else if (tmp == "MimeTypes")
	mContext = KIcon::MimeType;
    else if (tmp == "FileSystems")
	mContext = KIcon::FileSystem;
    else if (tmp == "Applications")
	mContext = KIcon::Application;
    else if (tmp == "Actions")
	mContext = KIcon::Action;
    else {
	kdDebug(264) << "Invalid Context= line for icon theme: " << mDir << "\n";
	return;
    }
    tmp = config->readEntry("Type");
    if (tmp == "Fixed")
	mType = KIcon::Fixed;
    else if (tmp == "Scalable")
	mType = KIcon::Scalable;
    else {
	kdDebug(264) << "Invalid Type= line for icon theme: " <<  mDir << "\n";
	return;
    }
    if (mType == KIcon::Scalable)
    {
	mMinSize = config->readNumEntry("MinSize", 49);
	mMaxSize = config->readNumEntry("MaxSize", 128);
    }
    mbValid = true;
}

QString KIconThemeDir::iconPath(QString name)
{
    if (!mbValid)
	return QString::null;
    QFileInfo fi;
    QString file;
    QStringList::Iterator it;
    for (it=mExtensions.begin(); it!=mExtensions.end(); it++)
    {
	file = mDir + "/" + name + *it;
	fi.setFile(file);
	if (fi.exists())
	    break;
    }
    return fi.exists() ? file : QString::null;
}

QStringList KIconThemeDir::iconList()
{
    QDir dir(mDir);
    QStringList lst = dir.entryList("*.png;*.xpm", QDir::Files);
    QStringList result;
    QStringList::ConstIterator it;
    for (it=lst.begin(); it!=lst.end(); it++)
	result += mDir + "/" + *it;
    return result;
}
