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
#include <time.h>


#define KDE_ICONCACHE_NAME "kde-icon-cache"
#define KDE_ICONCACHE_VERSION 0x000100


class KIconCache::Private
{
public:
    Private(KIconCache* _q)
    {
        q = _q;
        mUpdatesCheckedTime = 0;
    }
    bool themeDirsChanged()
    {
        if (q->existingIconThemeDirs(mThemeNames) != mThemeDirs ||
            q->mostRecentMTime(mThemeDirs) != mThemesMTime) {
            kDebug(264) << "Theme directory has been modified";
            return true;
        } else {
            return false;
        }
    }
    void checkForThemeUpdates()
    {
        if (!q->isEnabled()) {
            return;
        }
        // Don't check more often than every 5 secs
        const quint32 now = ::time(0);
        if (now < mUpdatesCheckedTime + 5) {
            return;
        }

        mUpdatesCheckedTime = now;
        // Perhaps another process has already checked for updates in last 5 secs
        const QFileInfo fi(mUpdatesFile);
        if (fi.exists() && fi.lastModified().toTime_t() + 5 > now) {
            return;
        }
        // Check for theme updates
        if (themeDirsChanged()) {
            // Update themes info and discard the cache
            mThemeDirs = q->existingIconThemeDirs(mThemeNames);
            mThemesMTime = q->mostRecentMTime(mThemeDirs);
            q->discard();
        }
        // Update timestamp file
        QFile f(mUpdatesFile);
        f.open(QIODevice::WriteOnly);
    }

    KIconCache* q;

    qint32 mDefaultIconSize[6];
    QStringList mThemeNames;
    QSet<QString> mThemeDirs;
    quint32 mThemesMTime;
    QString mUpdatesFile;
    quint32 mUpdatesCheckedTime;

    QString* mLoadPath;
    QString mSavePath;
};


KIconCache::KIconCache()
    : KPixmapCache(KDE_ICONCACHE_NAME), d(new Private(this))
{
    d->mUpdatesFile  = KGlobal::dirs()->locateLocal("cache", "kpc/"KDE_ICONCACHE_NAME".updated");
    // Set limit to 10 MB
    setCacheLimit(10 * 1024);
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

    // Load version
    quint32 version;
    stream >> version;
    if (version != KDE_ICONCACHE_VERSION) {
        kDebug(264) << "Obsolete iconcache version" << version << "will recreate";
        return false;
    }

    // Load default sizes of icons
    for (int i = 0; i < 6; i++) {
        stream >> d->mDefaultIconSize[i];
    }


    stream >> d->mThemeNames;
    stream >> d->mThemeDirs;
    // TODO: use KPixmapCache's timestamp instead
    stream >> d->mThemesMTime;

    if (stream.status() != QDataStream::Ok) {
        kWarning() << "Failed to read index file's header";
        recreateCacheFiles();
        return false;
    }

    // Make sure at least one theme was read
    if (!d->mThemeNames.count()) {
        kDebug(264) << "Empty themes list";
        return false;
    }

    // Make sure the theme dirs haven't changed
    if (d->themeDirsChanged()) {
        return false;
    }
    d->mUpdatesCheckedTime= ::time(0);

    return true;
}

void KIconCache::writeCustomIndexHeader(QDataStream& stream)
{
    setValid(false);

    stream << (quint32)KDE_ICONCACHE_VERSION;

    for (int i = 0; i < 6; i++) {
        stream << d->mDefaultIconSize[i];
    }

    // Save iconthemes info
    stream << d->mThemeNames;
    stream << d->mThemeDirs;
    stream << d->mThemesMTime;

    // Cache is valid if header was successfully written and we actually have
    //  the icontheme name(s)
    if (stream.status() == QDataStream::Ok && d->mThemeNames.count()) {
        setValid(true);
    }
}

QSet<QString> KIconCache::existingIconThemeDirs(const QStringList& themeNames) const
{
    // Find all possible icontheme dirs
    // This has been taken from kicontheme.cpp
    const QStringList icondirs = KGlobal::dirs()->resourceDirs("icon")
            << KGlobal::dirs()->resourceDirs("xdgdata-icon")
            << "/usr/share/pixmaps"
            // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
            << KGlobal::dirs()->resourceDirs("xdgdata-pixmap");

    // Check which of theme actually contain existing dir of one of the
    //  given themes
    QSet<QString> dirs;
    for (QStringList::ConstIterator it = icondirs.begin(); it != icondirs.end(); ++it) {
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
    foreach (const QString &dir, dirNames) {
        unsigned int mtime = QFileInfo(dir).lastModified().toTime_t();
        if (timestamp < mtime) {
            timestamp = mtime;
        }
    }

    return timestamp;
}

int KIconCache::defaultIconSize(KIconLoader::Group group) const
{
    if ((group < 0) || (group >= KIconLoader::LastGroup))
    {
        kDebug(264) << "Illegal icon group:" << group;
        return -1;
    }
    return d->mDefaultIconSize[group];
}

void KIconCache::setThemeInfo(const QList<KIconTheme*>& themes)
{
    if (themes.isEmpty()) {
        for (KIconLoader::Group i = KIconLoader::FirstGroup; i < KIconLoader::LastGroup; i++) {
            d->mDefaultIconSize[i] = 0;
        }
        return;
    }
    // This as to be done always, even if the cache itself is disabled
    for (KIconLoader::Group i = KIconLoader::FirstGroup; i < KIconLoader::LastGroup; i++) {
        d->mDefaultIconSize[i] = themes.first()->defaultSize(i);
    }

    if (!isEnabled()) {
        return;
    }
    setValid(false);

    // Save internal names and dirs of all themes
    d->mThemeNames.clear();
    foreach (KIconTheme* theme, themes) {
        d->mThemeNames.append(theme->internalName());
    }
    d->mThemeDirs = existingIconThemeDirs(d->mThemeNames);
    d->mThemesMTime = mostRecentMTime(d->mThemeDirs);
    d->mUpdatesCheckedTime= ::time(0);

    // Recreate the cache
    recreateCacheFiles();
}

bool KIconCache::find(const QString& key, QPixmap& pix, QString* path)
{
    d->checkForThemeUpdates();

    d->mLoadPath = path;
    // We can use QPixmapCache only if we don't need the path
    setUseQPixmapCache(!path);
    //kDebug(264) << "use QPC = " << useQPixmapCache();
    bool ret = find(key, pix);
    d->mLoadPath = 0;
    return ret;
}

void KIconCache::insert(const QString& key, const QPixmap& pix, const QString& path)
{
    d->mSavePath = path;
    insert(key, pix);
    d->mSavePath.clear();
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

