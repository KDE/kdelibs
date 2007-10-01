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

#ifndef KICONCACHE_H
#define KICONCACHE_H

#include <kdeui_export.h>

#include <QtCore/QList>
#include <QtCore/QSet>
#include <kicontheme.h>
#include <kpixmapcache.h>

class QString;
class QStringList;
class QPixmap;
class QDataStream;


/**
 * Icon cache for KDE.
 *
 * Note that this is the KDE-wide cache for storing icons and should only be
 *  used by KIconLoader. If you want to cache your own pixmaps you should look
 *  at KPixmapCache instead.
 */
class KDEUI_EXPORT KIconCache : public KPixmapCache
{
public:
    /**
     * Constucts the icon cache object.
     **/
    explicit KIconCache();
    virtual ~KIconCache();


    bool find(const QString& key, QPixmap& pix, QString* path);
    void insert(const QString& key, const QPixmap& pix, const QString& path);

    virtual bool find(const QString& key, QPixmap& pix);
    virtual void insert(const QString& key, const QPixmap& pix);

    /**
     * Deletes the icon cache.
     **/
    static void deleteCache();

    /**
     * The default size of current theme for a certain icon group.
     * @param group The icon group. See KIconLoader::Group.
     * @return The default size in pixels for the given icon group.
     */
    int defaultIconSize(KIconLoader::Group group) const;

    void setThemeInfo(const QList<KIconTheme*>& themes);


protected:
    virtual bool loadCustomIndexHeader(QDataStream& stream);
    virtual void writeCustomIndexHeader(QDataStream& stream);

    virtual bool loadCustomData(QDataStream& stream);
    virtual bool writeCustomData(QDataStream& stream);

    QSet<QString> existingIconThemeDirs(const QStringList& themeNames) const;
    unsigned int mostRecentMTime(const QSet<QString>& dirNames) const;

private:
    class Private;
    friend class Private;
    Private * const d;
};

#endif // KICONCACHE_H
