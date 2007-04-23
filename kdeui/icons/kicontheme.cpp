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
 * kicontheme.cpp: Lowlevel icon theme handling.
 */

#include "kicontheme.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <QtGui/QAction>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QMap>
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>
#include <QtGui/QImage>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <kdebug.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfig.h>
#include <kcomponentdata.h>

#include <kconfiggroup.h>

class KIconTheme::KIconThemePrivate
{
public:
    QString example, screenshot;
    QString linkOverlay, lockOverlay, zipOverlay, shareOverlay;
    bool hidden;
    KSharedConfig::Ptr sharedConfig;

    int mDefSize[8];
    QList<int> mSizes[8];

    int mDepth;
    QString mDir, mName, mDesc;
    QStringList mInherits;
    QList<KIconThemeDir *> mDirs;
};
K_GLOBAL_STATIC(QString, _theme)
K_GLOBAL_STATIC(QStringList, _theme_list)

/**
 * A subdirectory in an icon theme.
 */
class KIconThemeDir
{
public:
    KIconThemeDir(const QString& dir, const KConfigGroup &config);

    bool isValid() const { return mbValid; }
    QString iconPath(const QString& name) const;
    QStringList iconList() const;
    QString dir() const { return mDir; }

    K3Icon::Context context() const { return mContext; }
    K3Icon::Type type() const { return mType; }
    int size() const { return mSize; }
    int minSize() const { return mMinSize; }
    int maxSize() const { return mMaxSize; }
    int threshold() const { return mThreshold; }

private:
    bool mbValid;
    K3Icon::Type mType;
    K3Icon::Context mContext;
    int mSize, mMinSize, mMaxSize;
    int mThreshold;

    QString mDir;
};


/*** KIconTheme ***/

KIconTheme::KIconTheme(const QString& name, const QString& appName)
    :d(new KIconThemePrivate)
{

    QStringList icnlibs;
    QStringList::ConstIterator it, itDir;
    QStringList themeDirs;
    QString cDir;

    // Applications can have local additions to the global "locolor" and
    // "hicolor" icon themes. For these, the _global_ theme description
    // files are used..

    if (!appName.isEmpty() &&
       ( name == defaultThemeName() || name== "hicolor" || name == "locolor" ) )
    {
	icnlibs = KGlobal::dirs()->resourceDirs("data");
	for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
	{
	    cDir = *it + appName + "/icons/" + name;
	    if (QFile::exists( cDir ))
		themeDirs += cDir + '/';
	}
    }
    // Find the theme description file. These are always global.

    icnlibs = KGlobal::dirs()->resourceDirs("icon")
        << KGlobal::dirs()->resourceDirs("xdgdata-icon")
        << "/usr/share/pixmaps"
        // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
        << KGlobal::dirs()->resourceDirs("xdgdata-pixmap");
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it)
    {
        cDir = *it + name + '/';
        if (KStandardDirs::exists(cDir))
        {
            themeDirs += cDir;
            if (d->mDir.isEmpty() &&
                (KStandardDirs::exists( cDir + "index.desktop") || KStandardDirs::exists( cDir + "index.theme")))
                d->mDir = cDir;
        }
    }

    if (d->mDir.isEmpty())
    {
        kDebug(264) << "Icon theme " << name << " not found.\n";
        return;
    }

    QString fileName, mainSection;
    if(QFile::exists(d->mDir + "index.desktop")) {
	fileName = d->mDir + "index.desktop";
	mainSection="KDE Icon Theme";
    } else {
	fileName = d->mDir + "index.theme";
	mainSection="Icon Theme";
    }
    // Use KSharedConfig to avoid parsing the file many times, from each kinstance.
    // Need to keep a ref to it to make this useful
    d->sharedConfig = KSharedConfig::openConfig( fileName );

    KConfigGroup cfg(d->sharedConfig, mainSection);
    d->mName = cfg.readEntry("Name");
    d->mDesc = cfg.readEntry("Comment");
    d->mDepth = cfg.readEntry("DisplayDepth", 32);
    d->mInherits = cfg.readEntry("Inherits", QStringList());
    if ( name != defaultThemeName() ) {
      for ( QStringList::Iterator it = d->mInherits.begin(); it != d->mInherits.end(); ++it ) {
         if ( *it == "default" || *it == "hicolor" ) {
             *it = defaultThemeName();
         }
      }
    }

    d->hidden = cfg.readEntry("Hidden", false);
    d->example = cfg.readPathEntry("Example");
    d->screenshot = cfg.readPathEntry("ScreenShot");
    d->linkOverlay = cfg.readEntry("LinkOverlay", "link");
    d->lockOverlay = cfg.readEntry("LockOverlay", "system-lock-screen");
    d->zipOverlay = cfg.readEntry("ZipOverlay", "zip");
    d->shareOverlay = cfg.readEntry("ShareOverlay","share");

    QStringList dirs = cfg.readPathListEntry("Directories");
    for (it=dirs.begin(); it!=dirs.end(); ++it)
    {
        KConfigGroup cg(d->sharedConfig, *it);
	for (itDir=themeDirs.begin(); itDir!=themeDirs.end(); ++itDir)
	{
	    if (KStandardDirs::exists(*itDir + *it + '/'))
	    {
	        KIconThemeDir *dir = new KIconThemeDir(*itDir + *it, cg);
	        if (!dir->isValid())
	        {
	            kDebug(264) << "Icon directory " << *itDir << " group " << *it << " not valid.\n";
	            delete dir;
	        }
	        else
	            d->mDirs.append(dir);
            }
        }
    }

    // Expand available sizes for scalable icons to their full range
    int i;
    QMap<int,QList<int> > scIcons;
    foreach(KIconThemeDir *dir, d->mDirs)
    {
        if(!dir) break;
        if ((dir->type() == K3Icon::Scalable) && !scIcons.contains(dir->size()))
        {
            QList<int> lst;
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
    KConfigGroup cg(d->sharedConfig, mainSection);
    for (it=groups.begin(), i=0; it!=groups.end(); ++it, i++)
    {
        d->mDefSize[i] = cg.readEntry(*it + "Default", defDefSizes[i]);
        QList<int> exp, lst = cg.readEntry(*it + "Sizes", QList<int>());
        QList<int>::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2)
        {
            if (scIcons.contains(*it2))
                exp += scIcons[*it2];
            else
                exp += *it2;
        }
        d->mSizes[i] = exp;
    }

}

KIconTheme::~KIconTheme()
{
    qDeleteAll(d->mDirs);
    delete d;
}

QString KIconTheme::name() const
{
    return d->mName;
}

QString KIconTheme::description() const
{
    return d->mDesc;
}

QString KIconTheme::example() const
{ 
    return d->example;
}

QString KIconTheme::screenshot() const
{ 
    return d->screenshot;
}

QString KIconTheme::linkOverlay() const
{ 
    return d->linkOverlay;
}

QString KIconTheme::lockOverlay() const
{ 
    
    return d->lockOverlay;
}

QString KIconTheme::zipOverlay() const
{ 
    return d->zipOverlay;
}

QString KIconTheme::shareOverlay() const
{ 
    return d->shareOverlay;
}

QString KIconTheme::dir() const
{
    return d->mDir;
}

QStringList KIconTheme::inherits() const
{
    return d->mInherits;
}

bool KIconTheme::isValid() const
{
    return !d->mDirs.isEmpty();
}

bool KIconTheme::isHidden() const
{
    return d->hidden;
}

int KIconTheme::depth() const
{
    return d->mDepth;
}

int KIconTheme::defaultSize(K3Icon::Group group) const
{
    if ((group < 0) || (group >= K3Icon::LastGroup))
    {
        kDebug(264) << "Illegal icon group: " << group << "\n";
        return -1;
    }
    return d->mDefSize[group];
}

QList<int> KIconTheme::querySizes(K3Icon::Group group) const
{
    QList<int> empty;
    if ((group < 0) || (group >= K3Icon::LastGroup))
    {
        kDebug(264) << "Illegal icon group: " << group << "\n";
        return empty;
    }
    return d->mSizes[group];
}

QStringList KIconTheme::queryIcons(int size, K3Icon::Context context) const
{
    int delta = 1000, dw;

    KIconThemeDir *dir;

    // Try to find exact match
    QStringList result;
    for(int i=0; i<d->mDirs.size(); ++i)
    {
        dir = d->mDirs.at(i);
        if ((context != K3Icon::Any) && (context != dir->context()))
            continue;
        if ((dir->type() == K3Icon::Fixed) && (dir->size() == size))
        {
            result += dir->iconList();
            continue;
        }
        if ((dir->type() == K3Icon::Scalable) &&
            (size >= dir->minSize()) && (size <= dir->maxSize()))
        {
            result += dir->iconList();
            continue;
        }
	if ((dir->type() == K3Icon::Threshold) &&
            (abs(size-dir->size())<dir->threshold()))
            result+=dir->iconList();
    }

    return result;

    // Find close match
    KIconThemeDir *best = 0L;
    for(int i=0; i<d->mDirs.size(); ++i)
    {
        dir = d->mDirs.at(i);
        if ((context != K3Icon::Any) && (context != dir->context()))
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

QStringList KIconTheme::queryIconsByContext(int size, K3Icon::Context context) const
{
    int dw;
    KIconThemeDir *dir;

    // We want all the icons for a given context, but we prefer icons
    // of size size . Note that this may (will) include duplicate icons
    //QStringList iconlist[34]; // 33 == 48-16+1
    QStringList iconlist[128]; // 33 == 48-16+1
    // Usually, only the 0, 6 (22-16), 10 (32-22), 16 (48-32 or 32-16),
    // 26 (48-22) and 32 (48-16) will be used, but who knows if someone
    // will make icon themes with different icon sizes.

    for(int i=0;i<d->mDirs.size();++i)
    {
        dir = d->mDirs.at(i);
        if ((context != K3Icon::Any) && (context != dir->context()))
            continue;
        dw = abs(dir->size() - size);
        iconlist[(dw<127)?dw:127]+=dir->iconList();
    }

    QStringList iconlistResult;
    for (int i=0; i<128; i++) iconlistResult+=iconlist[i];

    return iconlistResult;
}

bool KIconTheme::hasContext(K3Icon::Context context) const
{
    foreach(KIconThemeDir *dir, d->mDirs)
        if ((context == K3Icon::Any) || (context == dir->context()))
            return true;
    return false;
}

K3Icon KIconTheme::iconPath(const QString& name, int size, K3Icon::MatchType match) const
{
    K3Icon icon;
    QString path;
    int delta = -1000, dw;
    KIconThemeDir *dir;

    dw = 1000; // shut up, gcc

    for(int i=0;i<d->mDirs.size();++i)
    {
        dir = d->mDirs.at(i);

        if (match == K3Icon::MatchExact)
        {
            if ((dir->type() == K3Icon::Fixed) && (dir->size() != size))
                continue;
            if ((dir->type() == K3Icon::Scalable) &&
                ((size < dir->minSize()) || (size > dir->maxSize())))
              continue;
            if ((dir->type() == K3Icon::Threshold) &&
		(abs(dir->size()-size) > dir->threshold()))
                continue;
        } else
        {
          // dw < 0 means need to scale up to get an icon of the requested size
          if (dir->type() == K3Icon::Fixed)
          {
            dw = dir->size() - size;
          } else if (dir->type() == K3Icon::Scalable)
          {
            if (size < dir->minSize())
              dw = dir->minSize() - size;
            else if (size > dir->maxSize())
              dw = dir->maxSize() - size;
            else
              dw = 0;
          } else if (dir->type() == K3Icon::Threshold)
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
        if (match == K3Icon::MatchExact)
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
QString KIconTheme::current()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (_theme != 0)
        return *_theme;

    KConfigGroup cg(KGlobal::config(), "Icons");
    *_theme = cg.readEntry("Theme", defaultThemeName());
    if ( *_theme == QLatin1String("hicolor") ) *_theme = defaultThemeName();
/*    if (_theme->isEmpty())
    {
        if (QPixmap::defaultDepth() > 8)
            *_theme = defaultThemeName();
        else
            *_theme = QLatin1String("locolor");
    }*/
    return *_theme;
}

// static
QStringList KIconTheme::list()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (_theme_list != 0)
        return *_theme_list;

    QStringList icnlibs = KGlobal::dirs()->resourceDirs("icon")
     << KGlobal::dirs()->resourceDirs("xdgdata-icon")
     << "/usr/share/pixmaps"
     // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
     << KGlobal::dirs()->resourceDirs("xdgdata-pixmap");

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
    _theme.reinit();
    _theme_list.reinit();
    
}

// static
QString KIconTheme::defaultThemeName()
{
    return QLatin1String("oxygen");
}

void KIconTheme::assignIconsToContextMenu( ContextMenus type,
                                           QList<QAction*> actions )
{
    switch (type) {
        case TextEditor:
            enum { UndoAct, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };

            if ( actions.count() < NCountActs ) {
                return;
            }

            actions[UndoAct]->setIcon( KIcon("edit-undo") );
            actions[RedoAct]->setIcon( KIcon("edit-redo") );
            actions[CutAct]->setIcon( KIcon("edit-cut") );
            actions[CopyAct]->setIcon( KIcon("edit-copy") );
            actions[PasteAct]->setIcon( KIcon("edit-paste") );
            actions[ClearAct]->setIcon( KIcon("edit-clear") );
            break;

        case ReadOnlyText:
            if ( actions.count() < 1 ) {
                return;
            }

            actions[0]->setIcon( KIcon("edit-copy") );
            break;
    }
}

/*** KIconThemeDir ***/

KIconThemeDir::KIconThemeDir(const QString& dir, const KConfigGroup &config)
{
    mbValid = false;
    mDir = dir;
    mSize = config.readEntry("Size", 0);
    mMinSize = 1;    // just set the variables to something
    mMaxSize = 50;   // meaningful in case someone calls minSize or maxSize
    mType = K3Icon::Fixed;

    if (mSize == 0)
        return;

    QString tmp = config.readEntry("Context");
    if (tmp == "Devices")
        mContext = K3Icon::Device;
    else if (tmp == "MimeTypes")
        mContext = K3Icon::MimeType;
    else if (tmp == "FileSystems")
        mContext = K3Icon::FileSystem;
    else if (tmp == "Applications")
        mContext = K3Icon::Application;
    else if (tmp == "Actions")
        mContext = K3Icon::Action;
    else if (tmp == "Animations")
        mContext = K3Icon::Animation;
    else if (tmp == "Categories")
        mContext = K3Icon::Category;
    else if (tmp == "Emblems")
        mContext = K3Icon::Emblem;
    else if (tmp == "Emotes")
        mContext = K3Icon::Emote;
    else if (tmp == "International")
        mContext = K3Icon::International;
    else if (tmp == "Places")
        mContext = K3Icon::Place;
    else if (tmp == "Status")
        mContext = K3Icon::StatusIcon;
    else {
        kDebug(264) << "Invalid Context= line for icon theme: " << mDir << "\n";
        return;
    }
    tmp = config.readEntry("Type");
    if (tmp == "Fixed")
        mType = K3Icon::Fixed;
    else if (tmp == "Scalable")
        mType = K3Icon::Scalable;
    else if (tmp == "Threshold")
        mType = K3Icon::Threshold;
    else {
        kDebug(264) << "Invalid Type= line for icon theme: " << mDir << "\n";
        return;
    }
    if (mType == K3Icon::Scalable)
    {
        mMinSize = config.readEntry("MinSize", mSize);
        mMaxSize = config.readEntry("MaxSize", mSize);
    } else if (mType == K3Icon::Threshold)
	mThreshold = config.readEntry("Threshold", 2);
    mbValid = true;
}

QString KIconThemeDir::iconPath(const QString& name) const
{
    if (!mbValid)
        return QString();
    QString file = mDir + '/' + name;

    if (access(QFile::encodeName(file), R_OK) == 0)
        return file;

    return QString();
}

QStringList KIconThemeDir::iconList() const
{
    QDir dir(mDir);

    QStringList formats;
    formats << "*.png" << "*.svg" << "*.svgz" << "*.xpm";
    QStringList lst = dir.entryList( formats, QDir::Files);

    QStringList result;
    QStringList::ConstIterator it;
    for (it=lst.begin(); it!=lst.end(); ++it)
        result += mDir + '/' + *it;
    return result;
}
