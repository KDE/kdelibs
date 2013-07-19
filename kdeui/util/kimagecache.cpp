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

#include <QPixmap>
#include <QImage>
#include <QtCore/QBuffer>
#include <QtCore/QCache>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>

/**
 * This is a QObject subclass so we can catch the signal that the application is about
 * to close and properly release any QPixmaps we have cached.
 */
class KImageCache::Private : public QObject
{
    Q_OBJECT

    public:
    Private(QObject *parent = 0)
        : QObject(parent)
        , timestamp(QDateTime::currentDateTime())
        , enablePixmapCaching(true)
    {
        QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                         this, SLOT(clearPixmaps()));
    }

    /**
     * Inserts a pixmap into the pixmap cache if the pixmap cache is enabled, with
     * weighting based on image size and bit depth.
     */
    bool insertPixmap(const QString &key, QPixmap *pixmap)
    {
        if (enablePixmapCaching && pixmap && !pixmap->isNull()) {
            // "cost" parameter is based on both image size and depth to make cost
            // based on size in bytes instead of area on-screen.
            return pixmapCache.insert(key, pixmap,
                pixmap->width() * pixmap->height() * pixmap->depth() / 8);
        }

        return false;
    }

    public Q_SLOTS:
    void clearPixmaps()
    {
        pixmapCache.clear();
    }

    public:
    QDateTime timestamp;

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
    // Use at least 16 KiB for the pixmap cache
    d->pixmapCache.setMaxCost(qMax(defaultCacheSize / 8, (unsigned int) 16384));
}

KImageCache::~KImageCache()
{
    delete d;
}

bool KImageCache::insertImage(const QString &key, const QImage &image)
{
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    image.save(&buffer, "PNG");

    if (this->insert(key, buffer.buffer())) {
        d->timestamp = QDateTime::currentDateTime();
        return true;
    }

    return false;
}

bool KImageCache::insertPixmap(const QString &key, const QPixmap &pixmap)
{
    d->insertPixmap(key, new QPixmap(pixmap));

    // One thing to think about is only inserting things to the shared cache
    // that are frequently used. But that would require tracking the use count
    // in our local cache too, which I think is probably too much work.

    return insertImage(key, pixmap.toImage());
}

bool KImageCache::findImage(const QString &key, QImage *destination) const
{
    QByteArray cachedData;
    if (!this->find(key, &cachedData) || cachedData.isNull()) {
        return false;
    }

    if (destination) {
        destination->loadFromData(cachedData, "PNG");
    }

    return true;
}

bool KImageCache::findPixmap(const QString &key, QPixmap *destination) const
{
    if (d->enablePixmapCaching) {
        QPixmap *cachedPixmap = d->pixmapCache.object(key);
        if (cachedPixmap) {
            if (destination) {
                *destination = *cachedPixmap;
            }

            return true;
        }
    }

    QByteArray cachedData;
    if (!this->find(key, &cachedData) || cachedData.isNull()) {
        return false;
    }

    if (destination) {
        destination->loadFromData(cachedData, "PNG");

        // Manually re-insert to pixmap cache if we'll be using this one.
        d->insertPixmap(key, new QPixmap(*destination));
    }

    return true;
}

void KImageCache::clear()
{
    d->pixmapCache.clear();
    KSharedDataCache::clear();
}

QDateTime KImageCache::lastModifiedTime() const
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

int KImageCache::pixmapCacheLimit() const
{
    return d->pixmapCache.maxCost();
}

void KImageCache::setPixmapCacheLimit(int size)
{
    d->pixmapCache.setMaxCost(size);
}

#include "kimagecache.moc"
