/* vi: ts=8 sts=4 sw=4
 *
 * $Id: kicontheme.cpp 601485 2006-11-03 12:50:30Z lunakl $
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kicontheme.cpp: Lowlevel icon theme handling.
 */

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <config.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kinstance.h>

#include "kicontheme.h"

class KIconThemePrivate
{
public:
    QString example, screenshot;
    QString linkOverlay, lockOverlay, zipOverlay, shareOverlay;
    bool hidden;
    KSharedConfig::Ptr sharedConfig;
};

/**
 * A subdirectory in an icon theme.
 */
class KIconThemeDir
{
public:
    KIconThemeDir(const QString& dir, const KConfigBase *config);

    bool isValid() const { return mbValid; }
    QString iconPath(const QString& name) const;
    QStringList iconList() const;
    QString dir() const { return mDir; }

    KIcon::Context context() const { return mContext; }
    KIcon::Type type() const { return mType; }
    int size() const { return mSize; }
    int minSize() const { return mMinSize; }
    int maxSize() const { return mMaxSize; }
    int threshold() const { return mThreshold; }

private:
    bool mbValid;
    KIcon::Type mType;
    KIcon::Context mContext;
    int mSize, mMinSize, mMaxSize;
    int mThreshold;

    QString mDir;
};


/*** KIconTheme ***/

KIconTheme::KIconTheme(const QString& name, const QString& appName)
{
    d = new KIconThemePrivate;

    QStringList icnlibs;
    QStringList::ConstIterator it, itDir;
    QStringList themeDirs;
    QString cDir;

    // Applications can have local additions to the global "locolor" and
    // "hicolor" icon themes. For these, the _global_ theme description
    // files are used..

    if (!appName.isEmpty() &&
       ( name == "crystalsvg" || name== "hicolor" || name == "locolor" ) )
    {
	icnlibs = KGlobal::dirs()->resourceDirs("data");
	for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
	{
	    cDir = *it + appName + "/icons/" + name;
	    if (QFile::exists( cDir ))
		themeDirs += cDir + "/";
	}
    }
    // Find the theme description file. These are always global.

    icnlibs = KGlobal::dirs()->resourceDirs("icon");
    icnlibs += KGlobal::dirs()->resourceDirs("xdgdata-icon");
    icnlibs += "/usr/share/pixmaps";
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    icnlibs += KGlobal::dirs()->resourceDirs("xdgdata-pixmap");
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
    {
        cDir = *it + name + "/";
        if (KStandardDirs::exists(cDir))
        {
            themeDirs += cDir;
	    if (mDir.isEmpty()
		    && (KStandardDirs::exists( cDir + "index.desktop") || KStandardDirs::exists( cDir + "index.theme")))
		mDir = cDir;
        }
    }

    if (mDir.isEmpty())
    {
        kdDebug(264) << "Icon theme " << name << " not found.\n";
        return;
    }

    QString fileName, mainSection;
    if(QFile::exists(mDir + "index.desktop")) {
	fileName = mDir + "index.desktop";
	mainSection="KDE Icon Theme";
    } else {
	fileName = mDir + "index.theme";
	mainSection="Icon Theme";
    }
    // Use KSharedConfig to avoid parsing the file many times, from each kinstance.
    // Need to keep a ref to it to make this useful
    d->sharedConfig = KSharedConfig::openConfig( fileName, true /*readonly*/, false /*useKDEGlobals*/ );
    KConfig& cfg = *d->sharedConfig;
    //was: KSimpleConfig cfg(fileName);

    cfg.setGroup(mainSection);
    mName = cfg.readEntry("Name");
    mDesc = cfg.readEntry("Comment");
    mDepth = cfg.readNumEntry("DisplayDepth", 32);
    mInherits = cfg.readListEntry("Inherits");
    if ( name != "crystalsvg" )
      for ( QStringList::Iterator it = mInherits.begin(); it != mInherits.end(); ++it )
         if ( *it == "default" || *it == "hicolor" ) *it="crystalsvg";

    d->hidden = cfg.readBoolEntry("Hidden", false);
    d->example = cfg.readPathEntry("Example");
    d->screenshot = cfg.readPathEntry("ScreenShot");
    d->linkOverlay = cfg.readEntry("LinkOverlay", "link");
    d->lockOverlay = cfg.readEntry("LockOverlay", "lock");
    d->zipOverlay = cfg.readEntry("ZipOverlay", "zip");
    d->shareOverlay = cfg.readEntry("ShareOverlay","share");

    QStringList dirs = cfg.readPathListEntry("Directories");
    mDirs.setAutoDelete(true);
    for (it=dirs.begin(); it!=dirs.end(); ++it)
    {
	cfg.setGroup(*it);
	for (itDir=themeDirs.begin(); itDir!=themeDirs.end(); ++itDir)
	{
	    if (KStandardDirs::exists(*itDir + *it + "/"))
	    {
	        KIconThemeDir *dir = new KIconThemeDir(*itDir + *it, &cfg);
	        if (!dir->isValid())
	        {
	            kdDebug(264) << "Icon directory " << *itDir << " group " << *it << " not valid.\n";
	            delete dir;
	        }
	        else
	            mDirs.append(dir);
            }
        }
    }

    // Expand available sizes for scalable icons to their full range
    int i;
    QMap<int,QValueList<int> > scIcons;
    for (KIconThemeDir *dir=mDirs.first(); dir!=0L; dir=mDirs.next())
    {
        if ((dir->type() == KIcon::Scalable) && !scIcons.contains(dir->size()))
        {
            QValueList<int> lst;
            for (i=dir->minSize(); i<=dir->maxSize(); i++)
                lst += i;
            scIcons[dir->size()] = lst;
        }
    }

    QStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";
    groups += "Panel";
    const int defDefSizes[] = { 32, 22, 22, 16, 32 };
    cfg.setGroup(mainSection);
    for (it=groups.begin(), i=0; it!=groups.end(); ++it, i++)
    {
        mDefSize[i] = cfg.readNumEntry(*it + "Default", defDefSizes[i]);
        QValueList<int> exp, lst = cfg.readIntListEntry(*it + "Sizes");
        QValueList<int>::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2)
        {
            if (scIcons.contains(*it2))
                exp += scIcons[*it2];
            else
                exp += *it2;
        }
        mSizes[i] = exp;
    }

}

KIconTheme::~KIconTheme()
{
    delete d;
}

bool KIconTheme::isValid() const
{
    return !mDirs.isEmpty();
}

bool KIconTheme::isHidden() const
{
    return d->hidden;
}

QString KIconTheme::example() const { return d->example; }
QString KIconTheme::screenshot() const { return d->screenshot; }
QString KIconTheme::linkOverlay() const { return d->linkOverlay; }
QString KIconTheme::lockOverlay() const { return d->lockOverlay; }
QString KIconTheme::zipOverlay() const { return d->zipOverlay; }
QString KIconTheme::shareOverlay() const { return d->shareOverlay; }

int KIconTheme::defaultSize(KIcon::Group group) const
{
    if ((group < 0) || (group >= KIcon::LastGroup))
    {
        kdDebug(264) << "Illegal icon group: " << group << "\n";
        return -1;
    }
    return mDefSize[group];
}

QValueList<int> KIconTheme::querySizes(KIcon::Group group) const
{
    QValueList<int> empty;
    if ((group < 0) || (group >= KIcon::LastGroup))
    {
        kdDebug(264) << "Illegal icon group: " << group << "\n";
        return empty;
    }
    return mSizes[group];
}

QStringList KIconTheme::queryIcons(int size, KIcon::Context context) const
{
    int delta = 1000, dw;

    QPtrListIterator<KIconThemeDir> dirs(mDirs);
    KIconThemeDir *dir;

    // Try to find exact match
    QStringList result;
    for ( ; dirs.current(); ++dirs)
    {
        dir = dirs.current();
        if ((context != KIcon::Any) && (context != dir->context()))
            continue;
        if ((dir->type() == KIcon::Fixed) && (dir->size() == size))
        {
            result += dir->iconList();
            continue;
        }
        if ((dir->type() == KIcon::Scalable) &&
            (size >= dir->minSize()) && (size <= dir->maxSize()))
        {
            result += dir->iconList();
            continue;
        }
	if ((dir->type() == KIcon::Threshold) &&
            (abs(size-dir->size())<dir->threshold()))
            result+=dir->iconList();
    }

    return result;

    dirs.toFirst();

    // Find close match
    KIconThemeDir *best = 0L;
    for ( ; dirs.current(); ++dirs)
    {
        dir = dirs.current();
        if ((context != KIcon::Any) && (context != dir->context()))
            continue;
        dw = dir->size() - size;
        if ((dw > 6) || (abs(dw) >= abs(delta)))
            continue;
        delta = dw;
        best = dir;
    }
    if (best == 0L)
        return QStringList();

    return best->iconList();
}

QStringList KIconTheme::queryIconsByContext(int size, KIcon::Context context) const
{
    QPtrListIterator<KIconThemeDir> dirs(mDirs);
    int dw;
    KIconThemeDir *dir;

    // We want all the icons for a given context, but we prefer icons
    // of size size . Note that this may (will) include duplicate icons
    //QStringList iconlist[34]; // 33 == 48-16+1
    QStringList iconlist[128]; // 33 == 48-16+1
    // Usually, only the 0, 6 (22-16), 10 (32-22), 16 (48-32 or 32-16),
    // 26 (48-22) and 32 (48-16) will be used, but who knows if someone
    // will make icon themes with different icon sizes.

    for ( ; dirs.current(); ++dirs)
    {
        dir = dirs.current();
        if ((context != KIcon::Any) && (context != dir->context()))
            continue;
        dw = abs(dir->size() - size);
        iconlist[(dw<127)?dw:127]+=dir->iconList();
    }

    QStringList iconlistResult;
    for (int i=0; i<128; i++) iconlistResult+=iconlist[i];

    return iconlistResult;
}

bool KIconTheme::hasContext(KIcon::Context context) const
{
    QPtrListIterator<KIconThemeDir> dirs(mDirs);
    KIconThemeDir *dir;

    for ( ; dirs.current(); ++dirs)
    {
        dir = dirs.current();
        if ((context == KIcon::Any) || (context == dir->context()))
            return true;
    }
    return false;
}

KIcon KIconTheme::iconPath(const QString& name, int size, KIcon::MatchType match) const
{
    KIcon icon;
    QString path;
    int delta = -1000, dw;
    KIconThemeDir *dir;

    dw = 1000; // shut up, gcc
    QPtrListIterator<KIconThemeDir> dirs(mDirs);
    for ( ; dirs.current(); ++dirs)
    {
        dir = dirs.current();

        if (match == KIcon::MatchExact)
        {
            if ((dir->type() == KIcon::Fixed) && (dir->size() != size))
                continue;
            if ((dir->type() == KIcon::Scalable) &&
                ((size < dir->minSize()) || (size > dir->maxSize())))
              continue;
            if ((dir->type() == KIcon::Threshold) &&
		(abs(dir->size()-size) > dir->threshold()))
                continue;
        } else
        {
          // dw < 0 means need to scale up to get an icon of the requested size
          if (dir->type() == KIcon::Fixed)
          {
            dw = dir->size() - size;
          } else if (dir->type() == KIcon::Scalable)
          {
            if (size < dir->minSize())
              dw = dir->minSize() - size;
            else if (size > dir->maxSize())
              dw = dir->maxSize() - size;
            else
              dw = 0;
          } else if (dir->type() == KIcon::Threshold)
          {
            if (size < dir->size() - dir->threshold())
              dw = dir->size() - dir->threshold() - size;
            else if (size > dir->size() + dir->threshold())
              dw = dir->size() + dir->threshold() - size;
            else
              dw = 0;
          }
          /* Skip this if we've found a closer one, unless
             it's a downscale, and we only had upscales befores.
             This is to avoid scaling up unless we have to,
             since that looks very ugly */
          if ((abs(dw) >= abs(delta)) ||
              (delta > 0 && dw < 0))
            continue;
        }

        path = dir->iconPath(name);
        if (path.isEmpty())
            continue;
        icon.path = path;
        icon.size = dir->size();
        icon.type = dir->type();
	icon.threshold = dir->threshold();
        icon.context = dir->context();

        // if we got in MatchExact that far, we find no better
        if (match == KIcon::MatchExact)
            return icon;
	else
        {
	    delta = dw;
	    if (delta==0) return icon; // We won't find a better match anyway
        }
    }
    return icon;
}

// static
QString *KIconTheme::_theme = 0L;

// static
QStringList *KIconTheme::_theme_list = 0L;

// static
QString KIconTheme::current()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (_theme != 0L)
        return *_theme;

    _theme = new QString();
    KConfig *config = KGlobal::config();
    KConfigGroupSaver saver(config, "Icons");
    *_theme = config->readEntry("Theme",defaultThemeName());
    if ( *_theme == QString::fromLatin1("hicolor") ) *_theme = defaultThemeName();
/*    if (_theme->isEmpty())
    {
        if (QPixmap::defaultDepth() > 8)
            *_theme = defaultThemeName();
        else
            *_theme = QString::fromLatin1("locolor");
    }*/
    return *_theme;
}

// static
QStringList KIconTheme::list()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (_theme_list != 0L)
        return *_theme_list;

    _theme_list = new QStringList();
    QStringList icnlibs = KGlobal::dirs()->resourceDirs("icon");
    icnlibs += (KGlobal::dirs()->resourceDirs("xdgdata-icon"));
    icnlibs += "/usr/share/pixmaps";
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    icnlibs += KGlobal::dirs()->resourceDirs("xdgdata-pixmap");
    QStringList::ConstIterator it;
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
    {
        QDir dir(*it);
        if (!dir.exists())
            continue;
        QStringList lst = dir.entryList(QDir::Dirs);
        QStringList::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2)
        {
            if ((*it2 == ".") || (*it2 == "..") || (*it2).startsWith("default.") )
                continue;
            if (!KStandardDirs::exists(*it + *it2 + "/index.desktop") && !KStandardDirs::exists(*it + *it2 + "/index.theme"))
                continue;
		KIconTheme oink(*it2);
	    if (!oink.isValid()) continue;

	    if (!_theme_list->contains(*it2))
                _theme_list->append(*it2);
        }
    }
    return *_theme_list;
}

// static
void KIconTheme::reconfigure()
{
    delete _theme;
    _theme=0L;
    delete _theme_list;
    _theme_list=0L;
}

// static
QString KIconTheme::defaultThemeName()
{
    return QString::fromLatin1("crystalsvg");
}

/*** KIconThemeDir ***/

KIconThemeDir::KIconThemeDir(const QString& dir, const KConfigBase *config)
{
    mbValid = false;
    mDir = dir;
    mSize = config->readNumEntry("Size");
    mMinSize = 1;    // just set the variables to something
    mMaxSize = 50;   // meaningful in case someone calls minSize or maxSize
    mType = KIcon::Fixed;

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
    else if (tmp == "Animations")
        mContext = KIcon::Animation;
    else if (tmp == "Categories")
        mContext = KIcon::Category;
    else if (tmp == "Emblems")
        mContext = KIcon::Emblem;
    else if (tmp == "Emotes")
        mContext = KIcon::Emote;
    else if (tmp == "International")
        mContext = KIcon::International;
    else if (tmp == "Places")
        mContext = KIcon::Place;
    else if (tmp == "Status")
        mContext = KIcon::StatusIcon;
    else {
        kdDebug(264) << "Invalid Context= line for icon theme: " << mDir << "\n";
        return;
    }
    tmp = config->readEntry("Type");
    if (tmp == "Fixed")
        mType = KIcon::Fixed;
    else if (tmp == "Scalable")
        mType = KIcon::Scalable;
    else if (tmp == "Threshold")
        mType = KIcon::Threshold;
    else {
        kdDebug(264) << "Invalid Type= line for icon theme: " <<  mDir << "\n";
        return;
    }
    if (mType == KIcon::Scalable)
    {
        mMinSize = config->readNumEntry("MinSize", mSize);
        mMaxSize = config->readNumEntry("MaxSize", mSize);
    } else if (mType == KIcon::Threshold)
	mThreshold = config->readNumEntry("Threshold", 2);
    mbValid = true;
}

QString KIconThemeDir::iconPath(const QString& name) const
{
    if (!mbValid)
        return QString::null;
    QString file = mDir + "/" + name;

    if (access(QFile::encodeName(file), R_OK) == 0)
        return file;

    return QString::null;
}

QStringList KIconThemeDir::iconList() const
{
    QDir dir(mDir);
#ifdef HAVE_LIBART
    QStringList lst = dir.entryList("*.png;*.svg;*.svgz;*.xpm", QDir::Files);
#else
    QStringList lst = dir.entryList("*.png;*.xpm", QDir::Files);
#endif
    QStringList result;
    QStringList::ConstIterator it;
    for (it=lst.begin(); it!=lst.end(); ++it)
        result += mDir + "/" + *it;
    return result;
}
