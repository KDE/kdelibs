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

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <QAction>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QPixmap>
#include <QPixmapCache>
#include <QImage>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QDebug>

#include <klocalizedstring.h> // KLocalizedString::localizedFilePath. Need such functionality in, hmm, QLocale? QStandardPaths?

#include <ksharedconfig.h>
#include <kconfig.h>

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
Q_GLOBAL_STATIC(QString, _theme)
Q_GLOBAL_STATIC(QStringList, _theme_list)

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


KIconTheme::KIconTheme(const QString& name, const QString& appName)
    :d(new KIconThemePrivate)
{

    d->mInternalName = name;

    QStringList themeDirs;
    QSet<QString> addedDirs; // Used for avoiding duplicates.

    // Applications can have local additions to the global "locolor" and
    // "hicolor" icon themes. For these, the _global_ theme description
    // files are used..

    if (!appName.isEmpty() &&
       ( name == defaultThemeName() || name== "hicolor" || name == "locolor" ) ) {
        const QStringList icnlibs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (QStringList::ConstIterator it=icnlibs.constBegin(); it!=icnlibs.constEnd(); ++it) {
            const QString cDir = *it + '/' + appName + "/icons/" + name;
            if (QFile::exists( cDir )) {
                themeDirs += cDir + '/';
            }
        }
    }
    // Find the theme description file. These are always global.

    QStringList icnlibs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "icons", QStandardPaths::LocateDirectory);
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    icnlibs += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "pixmaps", QStandardPaths::LocateDirectory);

    QString fileName, mainSection;
    for (QStringList::ConstIterator it=icnlibs.constBegin(); it!=icnlibs.constEnd(); ++it) {
        const QString cDir = *it + '/' + name + '/';
        if (QDir(cDir).exists()) {
            themeDirs += cDir;
            if (d->mDir.isEmpty()) {
                if (QFile::exists(cDir + "index.theme")) {
                    d->mDir = cDir;
                    fileName = d->mDir + "index.theme";
                    mainSection = "Icon Theme";
                } else if (QFile::exists(cDir + "index.desktop")) {
                    d->mDir = cDir;
                    fileName = d->mDir + "index.desktop";
                    mainSection = "KDE Icon Theme";
                }
            }
        }
    }

    if (d->mDir.isEmpty()) {
        qWarning() << "Icon theme" << name << "not found.";
        return;
    }

    // Use KSharedConfig to avoid parsing the file many times, from each component.
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
    for (QStringList::ConstIterator it=dirs.begin(); it!=dirs.end(); ++it) {
        KConfigGroup cg(d->sharedConfig, *it);
        for (QStringList::ConstIterator itDir=themeDirs.constBegin(); itDir!=themeDirs.constEnd(); ++itDir) {
            const QString currentDir(*itDir + *it + '/');
            if (!addedDirs.contains(currentDir) && QDir(currentDir).exists()) {
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
    QMap<int,QList<int> > scIcons;
    foreach(KIconThemeDir *dir, d->mDirs) {
        if (!dir) {
            break;
        }
        if ((dir->type() == KIconLoader::Scalable) && !scIcons.contains(dir->size())) {
            QList<int> lst;
            for (int i=dir->minSize(); i<=dir->maxSize(); ++i) {
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
    for (int i = 0; i < groups.size(); ++i) {
        const QString group = groups.at(i);
        d->mDefSize[i] = cg.readEntry(group + "Default", defDefSizes[i]);
        const QList<int> lst = cg.readEntry(group + "Sizes", QList<int>());
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
        qWarning() << "Illegal icon group: " << group;
        return -1;
    }
    return d->mDefSize[group];
}

QList<int> KIconTheme::querySizes(KIconLoader::Group group) const
{
    QList<int> empty;
    if ((group < 0) || (group >= KIconLoader::LastGroup)) {
        qWarning() << "Illegal icon group: " << group;
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

QString KIconTheme::iconPath(const QString& name, int size, KIconLoader::MatchType match) const
{
    QString path;
    int delta = -INT_MAX;  // current icon size delta of 'icon'
    int dw = INT_MAX;      // icon size delta of current directory
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
            // in an upscaling of the icon (it is OK to use a directory with
            // smaller icons that what we've already found, however).
            if ((abs(dw) >= abs(delta)) && ((dw < 0) || (delta > 0))) {
                continue;
            }
        }

        path = dir->iconPath(name);
        if (path.isEmpty()) {
            continue;
        }

        // if we got in MatchExact that far, we find no better
        if (match == KIconLoader::MatchExact) {
            return path;
        }
        delta = dw;
        if (delta == 0) {
            return path; // We won't find a better match anyway
        }
    }
    return path;
}

// static
QString KIconTheme::current()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (!_theme()->isEmpty()) {
        return *_theme();
    }

    KConfigGroup cg(KSharedConfig::openConfig(), "Icons");
    *_theme() = cg.readEntry("Theme", defaultThemeName());
    if ( *_theme() == QLatin1String("hicolor") ) {
        *_theme() = defaultThemeName();
    }
/*    if (_theme->isEmpty())
    {
        if (QPixmap::defaultDepth() > 8)
            *_theme = defaultThemeName();
        else
            *_theme = QLatin1String("locolor");
    }*/
    return *_theme();
}

// static
QStringList KIconTheme::list()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (!_theme_list()->isEmpty()) {
        return *_theme_list();
    }

    QStringList icnlibs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "icons", QStandardPaths::LocateDirectory);
    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    icnlibs += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "pixmaps", QStandardPaths::LocateDirectory);

    Q_FOREACH(const QString& it, icnlibs) {
        QDir dir(it);
        const QStringList lst = dir.entryList(QDir::Dirs);
        QStringList::ConstIterator it2;
        for (it2=lst.begin(); it2!=lst.end(); ++it2) {
            if ((*it2 == ".") || (*it2 == "..") || (*it2).startsWith(QLatin1String("default.")) ) {
                continue;
            }
            if (!QFile::exists(it + *it2 + "/index.desktop") && !QFile::exists(it + *it2 + "/index.theme")) {
                continue;
            }
            KIconTheme oink(*it2);
            if (!oink.isValid()) {
                continue;
            }

            if (!_theme_list()->contains(*it2)) {
                _theme_list()->append(*it2);
            }
        }
    }
    return *_theme_list();
}

// static
void KIconTheme::reconfigure()
{
    _theme()->clear();
    _theme_list()->clear();
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

            actions[UndoAct]->setIcon( QIcon::fromTheme("edit-undo") );
            actions[RedoAct]->setIcon( QIcon::fromTheme("edit-redo") );
            actions[CutAct]->setIcon( QIcon::fromTheme("edit-cut") );
            actions[CopyAct]->setIcon( QIcon::fromTheme("edit-copy") );
            actions[PasteAct]->setIcon( QIcon::fromTheme("edit-paste") );
            actions[ClearAct]->setIcon( QIcon::fromTheme("edit-clear") );
            actions[DeleteAct]->setIcon( QIcon::fromTheme("edit-delete") );
            actions[SelectAllAct]->setIcon( QIcon::fromTheme("edit-select-all") );
            break;

        case ReadOnlyText:
            if ( actions.count() < 1 ) {
                return;
            }

            actions[0]->setIcon( QIcon::fromTheme("edit-copy") );
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
        qWarning() << "Invalid Context=" << tmp << "line for icon theme: " << dir();
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
        qWarning() << "Invalid Type=" << tmp << "line for icon theme: " << dir();
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

    if (QFile::exists(file)) {
        return KLocalizedString::localizedFilePath(file);
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
