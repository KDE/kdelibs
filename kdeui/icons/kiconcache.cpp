/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
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

#include "kiconcache.h"

#include <QtCore/QString>
#include <QtGui/QPixmap>
#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <sys/file.h>


#define KDE_ICONCACHE_NAME "kde-icon-cache"

class KIconCache::Private
{
public:
    qint32 mDefaultIconSize[6];
    QList<KIconTheme*> mThemes;

    QString* mLoadPath;
    QString mSavePath;
};


KIconCache::KIconCache()
    : KPixmapCache(KDE_ICONCACHE_NAME), d(new Private)
{
}

KIconCache::~KIconCache()
{
    delete d;
}

void KIconCache::deleteCache()
{
    KPixmapCache::deleteCache(KDE_ICONCACHE_NAME);
}

bool KIconCache::loadCustomIndexHeader(QDataStream& stream)
{
    if (stream.atEnd()) {
        return false;
    }

    // Load default sizes of icons
    for (int i = 0; i < 6; i++) {
        stream >> d->mDefaultIconSize[i];
    }


    QStringList themeNames;
    stream >> themeNames;
    QSet<QString> themeDirs;
    stream >> themeDirs;
    // TODO: use KPixmapCache's timestamp instead
    quint32 themesMTime;
    stream >> themesMTime;

    if (stream.status() != QDataStream::Ok) {
        kWarning() << k_funcinfo << "Failed to read index file's header" << endl;
        recreateCacheFiles();
        return false;
    }

    // Make sure at least one theme was read
    if (!themeNames.count()) {
        kDebug() << k_funcinfo << "Empty themes list" << endl;
        return false;
    }

    // Make sure the theme dirs haven't changed
    if (existingIconThemeDirs(themeNames) != themeDirs ||
            mostRecentMTime(themeDirs) != themesMTime) {
        kDebug() << k_funcinfo << "Theme directory has been modified" << endl;
        return false;
    }

    return true;
}

void KIconCache::writeCustomIndexHeader(QDataStream& stream)
{
    setValid(false);

    for (int i = 0; i < 6; i++) {
        stream << d->mDefaultIconSize[i];
    }

    //Save internal names of all themes
    QStringList themeNames;
    foreach (KIconTheme* theme, d->mThemes) {
        themeNames.append(theme->internalName());
    }
    stream << themeNames;
    // Save all existing dirs and the most recent mtime of them
    QSet<QString> themeDirs = existingIconThemeDirs(themeNames);
    stream << themeDirs;
    stream << (quint32)mostRecentMTime(themeDirs);

    // Cache is valid if header was successfully written and we actually have
    //  the icontheme name(s)
    if (stream.status() == QDataStream::Ok && themeNames.count()) {
        setValid(true);
    }
}

QSet<QString> KIconCache::existingIconThemeDirs(const QStringList& themeNames) const
{
    // Find all possible icontheme dirs
    // This has been taken from kicontheme.cpp
    QStringList icondirs = KGlobal::dirs()->resourceDirs("icon")
            << KGlobal::dirs()->resourceDirs("xdgdata-icon")
            << "/usr/share/pixmaps"
            // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
            << KGlobal::dirs()->resourceDirs("xdgdata-pixmap");

    // Check which of theme actually contain existing dir of one of the
    //  given themes
    QSet<QString> dirs;
    for (QStringList::Iterator it = icondirs.begin(); it != icondirs.end(); ++it) {
        QStringList::ConstIterator themeIt;
        for (themeIt = themeNames.begin(); themeIt != themeNames.end(); ++themeIt) {
            QString dirName = *it + *themeIt + '/';
            if (KStandardDirs::exists(dirName)) {
                dirs.insert(dirName);
            }
        }
    }

    return dirs;
}

unsigned int KIconCache::mostRecentMTime(const QSet<QString>& dirNames) const
{
    unsigned int timestamp = 0;
    foreach (QString dir, dirNames) {
        unsigned int mtime = QFileInfo(dir).lastModified().toTime_t();
        if (timestamp < mtime) {
            timestamp = mtime;
        }
    }

    return timestamp;
}

int KIconCache::defaultIconSize(K3Icon::Group group) const
{
    if ((group < 0) || (group >= K3Icon::LastGroup))
    {
        kDebug(264) << "Illegal icon group: " << group << "\n";
        return -1;
    }
    return d->mDefaultIconSize[group];
}

void KIconCache::setThemeInfo(const QList<KIconTheme*>& themes)
{
    if (!isEnabled()) {
        return;
    }
    setValid(false);

    // Load relevant data from the icon theme
    for (K3Icon::Group i = K3Icon::FirstGroup; i < K3Icon::LastGroup; i++) {
        d->mDefaultIconSize[i] = themes.first()->defaultSize(i);
    }
    d->mThemes = themes;
    recreateCacheFiles();
}

bool KIconCache::find(const QString& key, QPixmap& pix, QString* path)
{
    d->mLoadPath = path;
    // We can use QPixmapCache only if we don't need the path
    setUseQPixmapCache(!path);
    //kDebug() << k_funcinfo << "use QPC = " << useQPixmapCache() << endl;
    bool ret = find(key, pix);
    d->mLoadPath = 0;
    return ret;
}

void KIconCache::insert(const QString& key, const QPixmap& pix, const QString& path)
{
    d->mSavePath = path;
    insert(key, pix);
    d->mSavePath = QString();
}

bool KIconCache::find(const QString& key, QPixmap& pix)
{
    // TODO: make sure that cache is still valid
    return KPixmapCache::find(key, pix);
}

void KIconCache::insert(const QString& key, const QPixmap& pix)
{
    // TODO: make sure that cache is still valid
    KPixmapCache::insert(key, pix);
}

bool KIconCache::loadCustomData(QDataStream& stream)
{
    QString path;
    stream >> path;
    if (d->mLoadPath) {
        *d->mLoadPath = path;
    }

    return true;
}

bool KIconCache::writeCustomData(QDataStream& stream)
{
    stream << d->mSavePath;
    return true;
}

