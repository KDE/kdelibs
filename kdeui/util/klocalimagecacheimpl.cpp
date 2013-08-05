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

#include "klocalimagecacheimpl.h"

#include <QtCore/QBuffer>
#include <QtCore/QCache>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>

#include <QImage>
#include <QPixmap>

/**
 * This is a QObject subclass so we can catch the signal that the application is about
 * to close and properly release any QPixmaps we have cached.
 */
class KLocalImageCacheImplementation::Private : public QObject
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

KLocalImageCacheImplementation::KLocalImageCacheImplementation(unsigned defaultCacheSize)
    : d(new Private)
{
    // Use at least 16 KiB for the pixmap cache
    d->pixmapCache.setMaxCost(qMax(defaultCacheSize / 8, (unsigned int) 16384));
}

KLocalImageCacheImplementation::~KLocalImageCacheImplementation()
{
    delete d;
}

void KLocalImageCacheImplementation::updateModifiedTime()
{
    d->timestamp = QDateTime::currentDateTime();
}

QByteArray KLocalImageCacheImplementation::serializeImage(const QImage &image) const
{
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    image.save(&buffer, "PNG");
    return buffer.buffer();
}

bool KLocalImageCacheImplementation::insertLocalPixmap(const QString &key, const QPixmap &pixmap) const
{
    return d->insertPixmap(key, new QPixmap(pixmap));
}

bool KLocalImageCacheImplementation::findLocalPixmap(const QString &key, QPixmap *destination) const
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

    return false;
}

void KLocalImageCacheImplementation::clearLocalCache()
{
    d->pixmapCache.clear();
}

QDateTime KLocalImageCacheImplementation::lastModifiedTime() const
{
    return d->timestamp;
}

bool KLocalImageCacheImplementation::pixmapCaching() const
{
    return d->enablePixmapCaching;
}

void KLocalImageCacheImplementation::setPixmapCaching(bool enable)
{
    if (enable != d->enablePixmapCaching) {
        d->enablePixmapCaching = enable;
        if (!enable) {
            d->pixmapCache.clear();
        }
    }
}

int KLocalImageCacheImplementation::pixmapCacheLimit() const
{
    return d->pixmapCache.maxCost();
}

void KLocalImageCacheImplementation::setPixmapCacheLimit(int size)
{
    d->pixmapCache.setMaxCost(size);
}

#include "klocalimagecacheimpl.moc"
