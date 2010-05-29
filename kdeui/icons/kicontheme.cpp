/* vi: ts=8 sts=4 sw=4
 *
 * kicontheme.cpp: Lowlevel icon theme handling.
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
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

#include "kicontheme.h"
#include "k3icon_p.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <QtGui/QAction>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QMap>
#include <QtCore/QSet>
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
#include <klocale.h>
#include <kde_file.h>

#include <kconfiggroup.h>

// The following define exists because the Qt SVG renderer needs
// to be improved. This will be removed soon. (ereslibre)
#undef KDE_QT_SVG_RENDERER_FIXED

class KIconTheme::KIconThemePrivate
{
public:
    QString example, screenshot;
    QString linkOverlay, lockOverlay, zipOverlay, shareOverlay;
    bool hidden;
    KSharedConfig::Ptr sharedConfig;

    int mDefSize[6];
    QList<int> mSizes[6];

    int mDepth;
    QString mDir, mName, mInternalName, mDesc;
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
    KIconThemeDir(const QString& basedir, const QString &themedir, const KConfigGroup &config);

    bool isValid() const { return mbValid; }
    QString iconPath(const QString& name) const;
    QStringList iconList() const;
    QString dir() const { return mBaseDirThemeDir; }

    KIconLoader::Context context() const { return mContext; }
    KIconLoader::Type type() const { return mType; }
    int size() const { return mSize; }
    int minSize() const { return mMinSize; }
    int maxSize() const { return mMaxSize; }
    int threshold() const { return mThreshold; }

private:
    bool mbValid;
    KIconLoader::Type mType;
    KIconLoader::Context mContext;
    int mSize, mMinSize, mMaxSize;
    int mThreshold;

    QString mBaseDirThemeDir;
};


/*** K3Icon ***/

K3Icon::K3Icon()
{
    size = 0;
}

K3Icon::~K3Icon()
{
}

bool K3Icon::isValid() const
{
    return size != 0;
}


/*** KIconTheme ***/

KIconTheme::KIconTheme(const QString& name, const QString& appName)
    :d(new KIconThemePrivate)
{

    d->mInternalName = name;

    QStringList icnlibs;
    QStringList::ConstIterator it, itDir;
    QStringList themeDirs;
    QString cDir;
    QSet<QString> addedDirs; // Used for avoiding duplicates.

    // Applications can have local additions to the global "locolor" and
    // "hicolor" icon themes. For these, the _global_ theme description
    // files are used..

    if (!appName.isEmpty() &&
       ( name == defaultThemeName() || name== "hicolor" || name == "locolor" ) ) {
        icnlibs = KGlobal::dirs()->resourceDirs("data");
        for (it=icnlibs.constBegin(); it!=icnlibs.constEnd(); ++it) {
            cDir = *it + appName + "/icons/" + name;
            if (QFile::exists( cDir )) {
                themeDirs += cDir + '/';
            }
        }
    }
    // Find the theme description file. These are always global.

    icnlibs = KGlobal::dirs()->resourceDirs("icon")
        << KGlobal::dirs()->resourceDirs("xdgdata-icon")
        << "/usr/share/pixmaps"
        // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
        << KGlobal::dirs()->resourceDirs("xdgdata-pixmap");
    for (it=icnlibs.constBegin(); it!=icnlibs.constEnd(); ++it) {
        cDir = *it + name + '/';
        if (KStandardDirs::exists(cDir)) {
            themeDirs += cDir;
            if (d->mDir.isEmpty() &&
                (KStandardDirs::exists( cDir + "index.desktop") || KStandardDirs::exists( cDir + "index.theme"))) {
                d->mDir = cDir;
            }
        }
    }

    if (d->mDir.isEmpty()) {
        kDebug(264) << "Icon theme " << name << " not found.\n";
        return;
    }

    QString fileName, mainSection;
    if (QFile::exists(d->mDir + "index.desktop")) {
        fileName = d->mDir + "index.desktop";
        mainSection="KDE Icon Theme";
    } else {
        fileName = d->mDir + "index.theme";
        mainSection="Icon Theme";
    }
    // Use KSharedConfig to avoid parsing the file many times, from each kinstance.
    // Need to keep a ref to it to make this useful
    d->sharedConfig = KSharedConfig::openConfig(fileName);

    KConfigGroup cfg(d->sharedConfig, mainSection);
    d->mName = cfg.readEntry("Name");
    d->mDesc = cfg.readEntry("Comment");
    d->mDepth = cfg.readEntry("DisplayDepth", 32);
    d->mInherits = cfg.readEntry("Inherits", QStringList());
    if (name != defaultThemeName()) {
        for (QStringList::Iterator it = d->mInherits.begin(); it != d->mInherits.end(); ++it) {
            if (*it == "default" || *it == "hicolor") {
                *it = defaultThemeName();
            }
        }
    }

    d->hidden = cfg.readEntry("Hidden", false);
    d->example = cfg.readPathEntry("Example", QString());
    d->screenshot = cfg.readPathEntry("ScreenShot", QString());

    const QStringList dirs = cfg.readPathEntry("Directories", QStringList());
    for (it=dirs.begin(); it!=dirs.end(); ++it) {
        KConfigGroup cg(d->sharedConfig, *it);
        for (itDir=themeDirs.constBegin(); itDir!=themeDirs.constEnd(); ++itDir) {
            const QString currentDir(*itDir + *it + '/');
            if (KStandardDirs::exists(currentDir) && !addedDirs.contains(currentDir)) {
                addedDirs.insert(currentDir);
                KIconThemeDir *dir = new KIconThemeDir(*itDir, *it, cg);
                if (!dir->isValid()) {
                    delete dir;
                }
                else {
                    d->mDirs.append(dir);
                }
            }
        }
    }

    // Expand available sizes for scalable icons to their full range
    int i;
    QMap<int,QList<int> > scIcons;
    foreach(KIconThemeDir *dir, d->mDirs) {
        if (!dir) {
            break;
        }
        if ((dir->type() == KIconLoader::Scalable) && !scIcons.contains(dir->size())) {
            QList<int> lst;
            for (i=dir->minSize(); i<=dir->maxSize(); i++) {
                lst += i;
            }
            scIcons[dir->size()] = lst;
        }
    }

    QStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";
    groups += "Panel";
    groups += "Dialog";
    const int defDefSizes[] = { 32, 22, 22, 16, 32, 32 };
    KConfigGroup cg(d->sharedConfig, mainSection);
    for (it=groups.constBegin(), i=0; it!=groups.constEnd(); ++it, i++) {
        d->mDefSize[i] = cg.readEntry(*it + "Default", defDefSizes[i]);
        const QList<int> lst = cg.readEntry(*it + "Sizes", QList<int>());
        QList<int> exp;
        QList<int>::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2) {
            if (scIcons.contains(*it2)) {
                exp += scIcons[*it2];
            } else {
                exp += *it2;
            }
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

QString KIconTheme::internalName() const
{
    return d->mInternalName;
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

int KIconTheme::defaultSize(KIconLoader::Group group) const
{
    if ((group < 0) || (group >= KIconLoader::LastGroup)) {
        kDebug(264) << "Illegal icon group: " << group << "\n";
        return -1;
    }
    return d->mDefSize[group];
}

QList<int> KIconTheme::querySizes(KIconLoader::Group group) const
{
    QList<int> empty;
    if ((group < 0) || (group >= KIconLoader::LastGroup)) {
        kDebug(264) << "Illegal icon group: " << group << "\n";
        return empty;
    }
    return d->mSizes[group];
}

QStringList KIconTheme::queryIcons(int size, KIconLoader::Context context) const
{
    KIconThemeDir *dir;

    // Try to find exact match
    QStringList result;
    for (int i=0; i<d->mDirs.size(); ++i) {
        dir = d->mDirs.at(i);
        if ((context != KIconLoader::Any) && (context != dir->context()))
            continue;
        if ((dir->type() == KIconLoader::Fixed) && (dir->size() == size)) {
            result += dir->iconList();
            continue;
        }
        if ((dir->type() == KIconLoader::Scalable) &&
            (size >= dir->minSize()) && (size <= dir->maxSize())) {
            result += dir->iconList();
            continue;
        }
        if ((dir->type() == KIconLoader::Threshold) &&
            (abs(size-dir->size())<dir->threshold())) {
            result+=dir->iconList();
        }
    }

    return result;

/*
    int delta = 1000, dw;

    // Find close match
    KIconThemeDir *best = 0L;
    for(int i=0; i<d->mDirs.size(); ++i) {
        dir = d->mDirs.at(i);
        if ((context != KIconLoader::Any) && (context != dir->context())) {
            continue;
        }
        dw = dir->size() - size;
        if ((dw > 6) || (abs(dw) >= abs(delta)))
            continue;
        delta = dw;
        best = dir;
    }
    if (best == 0L) {
        return QStringList();
    }

    return best->iconList();
    */
}

QStringList KIconTheme::queryIconsByContext(int size, KIconLoader::Context context) const
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

    for (int i=0;i<d->mDirs.size();++i) {
        dir = d->mDirs.at(i);
        if ((context != KIconLoader::Any) && (context != dir->context()))
            continue;
        dw = abs(dir->size() - size);
        iconlist[(dw<127)?dw:127]+=dir->iconList();
    }

    QStringList iconlistResult;
    for (int i=0; i<128; i++) iconlistResult+=iconlist[i];

    return iconlistResult;
}

bool KIconTheme::hasContext(KIconLoader::Context context) const
{
    foreach(KIconThemeDir *dir, d->mDirs) {
        if ((context == KIconLoader::Any) || (context == dir->context())) {
            return true;
        }
    }
    return false;
}

K3Icon KIconTheme::iconPath(const QString& name, int size, KIconLoader::MatchType match) const
{
    K3Icon icon;
    QString path;
    int delta = -INT_MAX;  // current icon size delta of 'icon'
    int dw = INT_MAX;      // icon size delta of current directory
    int dirSize = INT_MAX; // directory size of 'icon'
    KIconThemeDir *dir;

    const int dirCount = d->mDirs.size();

    // Search the directory that contains the icon which matches best to the requested
    // size. If there is no directory which matches exactly to the requested size, the
    // following criterias get applied:
    // - Take a directory having icons with a minimum difference to the requested size.
    // - Prefer directories that allow a downscaling even if the difference to
    //   the requested size is bigger than a directory where an upscaling is required.
    for (int i = 0; i < dirCount; ++i) {
        dir = d->mDirs.at(i);

        if (match == KIconLoader::MatchExact) {
            if ((dir->type() == KIconLoader::Fixed) && (dir->size() != size)) {
                continue;
            }
            if ((dir->type() == KIconLoader::Scalable) &&
                ((size < dir->minSize()) || (size > dir->maxSize()))) {
                continue;
            }
            if ((dir->type() == KIconLoader::Threshold) &&
                (abs(dir->size() - size) > dir->threshold())) {
                continue;
            }
        } else {
            // dw < 0 means need to scale up to get an icon of the requested size.
            // Upscaling should only be done if no larger icon is available.
            if (dir->type() == KIconLoader::Fixed) {
                dw = dir->size() - size;
            } else if (dir->type() == KIconLoader::Scalable) {
                if (size < dir->minSize()) {
                    dw = dir->minSize() - size;
                } else if (size > dir->maxSize()) {
                    dw = dir->maxSize() - size;
                } else {
                    dw = 0;
                }
            } else if (dir->type() == KIconLoader::Threshold) {
                if (size < dir->size() - dir->threshold()) {
                    dw = dir->size() - dir->threshold() - size;
                } else if (size > dir->size() + dir->threshold()) {
                    dw = dir->size() + dir->threshold() - size;
                } else {
                    dw = 0;
                }
            }
            // Usually if the delta (= 'dw') of the current directory is
            // not smaller than the delta (= 'delta') of the currently best
            // matching icon, this candidate can be skipped. But skipping
            // the candidate may only be done, if this does not imply
            // in an upscaling of the icon.
            if ((abs(dw) >= abs(delta)) &&
                ((dw < 0) || ((dw > 0) && (dir->size() < dirSize)))) {
                continue;
            }
        }

        path = dir->iconPath(name);
        if (path.isEmpty()) {
            continue;
        }
        icon.path = path;
// The following code has been commented out because the Qt SVG renderer needs
// to be improved. If you are going to change/remove some code from this part,
// please contact me before (ereslibre@kde.org), or kde-core-devel@kde.org. (ereslibre)
#ifdef KDE_QT_SVG_RENDERER_FIXED
        icon.size = size;
#else
        icon.size = dir->size();
#endif
        icon.type = dir->type();
        icon.threshold = dir->threshold();
        icon.context = dir->context();

        // if we got in MatchExact that far, we find no better
        if (match == KIconLoader::MatchExact) {
            return icon;
        }
        delta = dw;
        if (delta == 0) {
            return icon; // We won't find a better match anyway
        }
        dirSize = dir->size();
    }
    return icon;
}

// static
QString KIconTheme::current()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (!_theme->isEmpty()) {
        return *_theme;
    }

    KConfigGroup cg(KGlobal::config(), "Icons");
    *_theme = cg.readEntry("Theme", defaultThemeName());
    if ( *_theme == QLatin1String("hicolor") ) {
        *_theme = defaultThemeName();
    }
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
    if (!_theme_list->isEmpty()) {
        return *_theme_list;
    }

    const QStringList icnlibs = KGlobal::dirs()->resourceDirs("icon")
     << KGlobal::dirs()->resourceDirs("xdgdata-icon")
     << "/usr/share/pixmaps"
     // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
     << KGlobal::dirs()->resourceDirs("xdgdata-pixmap");

    QStringList::ConstIterator it;
    for (it=icnlibs.begin(); it!=icnlibs.end(); ++it) {
        QDir dir(*it);
        if (!dir.exists()) {
            continue;
        }
        const QStringList lst = dir.entryList(QDir::Dirs);
        QStringList::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2) {
            if ((*it2 == ".") || (*it2 == "..") || (*it2).startsWith(QLatin1String("default.")) ) {
                continue;
            }
            if (!KStandardDirs::exists(*it + *it2 + "/index.desktop") && !KStandardDirs::exists(*it + *it2 + "/index.theme")) {
                continue;
            }
            KIconTheme oink(*it2);
            if (!oink.isValid()) {
                continue;
            }

            if (!_theme_list->contains(*it2)) {
                _theme_list->append(*it2);
            }
        }
    }
    return *_theme_list;
}

// static
void KIconTheme::reconfigure()
{
    _theme->clear();
    _theme_list->clear();
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
        // FIXME: This code depends on Qt's action ordering.
        case TextEditor:
            enum { UndoAct, RedoAct, Separator1, CutAct, CopyAct, PasteAct, DeleteAct, ClearAct,
                   Separator2, SelectAllAct, NCountActs };

            if ( actions.count() < NCountActs ) {
                return;
            }

            actions[UndoAct]->setIcon( KIcon("edit-undo") );
            actions[RedoAct]->setIcon( KIcon("edit-redo") );
            actions[CutAct]->setIcon( KIcon("edit-cut") );
            actions[CopyAct]->setIcon( KIcon("edit-copy") );
            actions[PasteAct]->setIcon( KIcon("edit-paste") );
            actions[ClearAct]->setIcon( KIcon("edit-clear") );
            actions[DeleteAct]->setIcon( KIcon("edit-delete") );
            actions[SelectAllAct]->setIcon( KIcon("edit-select-all") );
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

KIconThemeDir::KIconThemeDir(const QString& basedir, const QString &themedir, const KConfigGroup &config)
{
    mbValid = false;
    mBaseDirThemeDir = basedir + themedir;
    
    mSize = config.readEntry("Size", 0);
    mMinSize = 1;    // just set the variables to something
    mMaxSize = 50;   // meaningful in case someone calls minSize or maxSize
    mType = KIconLoader::Fixed;

    if (mSize == 0) {
        return;
    }

    QString tmp = config.readEntry("Context");
    if (tmp == "Devices")
        mContext = KIconLoader::Device;
    else if (tmp == "MimeTypes")
        mContext = KIconLoader::MimeType;
    else if (tmp == "FileSystems")
        mContext = KIconLoader::FileSystem;
    else if (tmp == "Applications")
        mContext = KIconLoader::Application;
    else if (tmp == "Actions")
        mContext = KIconLoader::Action;
    else if (tmp == "Animations")
        mContext = KIconLoader::Animation;
    else if (tmp == "Categories")
        mContext = KIconLoader::Category;
    else if (tmp == "Emblems")
        mContext = KIconLoader::Emblem;
    else if (tmp == "Emotes")
        mContext = KIconLoader::Emote;
    else if (tmp == "International")
        mContext = KIconLoader::International;
    else if (tmp == "Places")
        mContext = KIconLoader::Place;
    else if (tmp == "Status")
        mContext = KIconLoader::StatusIcon;
    else if (tmp == "Stock") // invalid, but often present context, skip warning
        return;
    else {
        kDebug(264) << "Invalid Context=" << tmp << "line for icon theme: " << dir() << "\n";
        return;
    }
    tmp = config.readEntry("Type");
    if (tmp == "Fixed")
        mType = KIconLoader::Fixed;
    else if (tmp == "Scalable")
        mType = KIconLoader::Scalable;
    else if (tmp == "Threshold")
        mType = KIconLoader::Threshold;
    else {
        kDebug(264) << "Invalid Type=" << tmp << "line for icon theme: " << dir() << "\n";
        return;
    }
    if (mType == KIconLoader::Scalable) {
        mMinSize = config.readEntry("MinSize", mSize);
        mMaxSize = config.readEntry("MaxSize", mSize);
    } else if (mType == KIconLoader::Threshold) {
        mThreshold = config.readEntry("Threshold", 2);
    }
    mbValid = true;
}

QString KIconThemeDir::iconPath(const QString& name) const
{
    if (!mbValid) {
        return QString();
    }

    QString file = dir() + '/' + name;

    if (KDE::access(file, R_OK) == 0) {
        return KGlobal::hasLocale() ? KGlobal::locale()->localizedFilePath(file) : file;
    }

    return QString();
}

QStringList KIconThemeDir::iconList() const
{
    const QDir icondir = dir();

    const QStringList formats = QStringList() << "*.png" << "*.svg" << "*.svgz" << "*.xpm";
    const QStringList lst = icondir.entryList( formats, QDir::Files);

    QStringList result;
    QStringList::ConstIterator it;
    for (it=lst.begin(); it!=lst.end(); ++it) {
        result += dir() + '/' + *it;
    }
    return result;
}
