/*
 * This file is part of the KDE project.
 * Copyright © 2010 Michael Pyne <mpyne@kde.org>
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

#include "kimagecache.h"

#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtCore/QBuffer>
#include <QtCore/QCache>
#include <QtCore/QCoreApplication>

#include <time.h>

/**
 * This is a QObject subclass so we can catch the signal that the application is about
 * to close and properly release any QPixmaps we have cached.
 */
class KImageCache::Private : public QObject
{
    Q_OBJECT

    public:
    Private()
        : timestamp(::time(0))
        , enablePixmapCaching(true)
    {
        QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                         this, SLOT(clearPixmaps()));
    }

    public slots:
    void clearPixmaps()
    {
        pixmapCache.clear();
    }

    public:
    time_t timestamp;

    /**
     * This is used to cache pixmaps as they are inserted, instead of always
     * converting to image data and storing that in shared memory.
     */
    QCache<QString, QPixmap> pixmapCache;

    bool enablePixmapCaching;
};

KImageCache::KImageCache(const QString &cacheName,
                         unsigned defaultCacheSize,
                         unsigned expectedItemSize)
    : KSharedDataCache(cacheName, defaultCacheSize, expectedItemSize)
    , d(new Private)
{
    d->pixmapCache.setMaxCost(defaultCacheSize);
}

bool KImageCache::insertImage(const QString &key, const QImage &image)
{
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    image.save(&buffer, "PNG");

    if (this->insert(key, buffer.buffer())) {
        d->timestamp = time(0);
        return true;
    }

    return false;
}

bool KImageCache::insertPixmap(const QString &key, const QPixmap &pixmap)
{
    if (d->enablePixmapCaching) {
        d->pixmapCache.insert(key, new QPixmap(pixmap),
                              pixmap.width() * pixmap.height());
    }

    // One thing to think about is only inserting things to the shared cache
    // that are frequently used. But that would require tracking the use count
    // in our local cache too, which I think is probably too much work.

    return insertImage(key, pixmap.toImage());
}

QImage KImageCache::findImage(const QString &key) const
{
    QByteArray cachedData = this->find(key);
    if (cachedData.isNull()) {
        return QImage();
    }

    QImage imageResult;
    imageResult.loadFromData(cachedData, "PNG");
    return imageResult;
}

QPixmap KImageCache::findPixmap(const QString &key) const
{
    if (d->enablePixmapCaching) {
        QPixmap *cachedPixmap = d->pixmapCache.object(key);
        if (cachedPixmap) {
            return QPixmap(*cachedPixmap);
        }
    }

    QByteArray cachedData = this->find(key);
    if (cachedData.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapResult;
    pixmapResult.loadFromData(cachedData, "PNG");

    d->pixmapCache.insert(key, new QPixmap(pixmapResult),
                          pixmapResult.height() * pixmapResult.width());
    return pixmapResult;
}

void KImageCache::clear()
{
    d->pixmapCache.clear();
    KSharedDataCache::clear();
}

time_t KImageCache::lastModifiedTime() const
{
    return d->timestamp;
}

bool KImageCache::pixmapCaching() const
{
    return d->enablePixmapCaching;
}

void KImageCache::setPixmapCaching(bool enable)
{
    if (enable != d->enablePixmapCaching) {
        d->enablePixmapCaching = enable;
        if (!enable) {
            d->pixmapCache.clear();
        }
    }
}

#include "kimagecache.moc"
