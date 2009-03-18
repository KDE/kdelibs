/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KMIMETYPERESOLVER_H
#define KMIMETYPERESOLVER_H

#include <QtCore/QObject>
#include <kio/kio_export.h>
class QModelIndex;
class QAbstractItemView;
class QAbstractProxyModel;
class KDirModel;
class KMimeTypeResolverPrivate;
class KAbstractViewAdapter;

/**
 * This class implements the "delayed-mimetype-determination" feature,
 * for directory views.
 *
 * It determines the mimetypes of the icons in the background, but giving
 * preferrence to the visible icons.
 *
 * @deprecated since 4.3, use KFilePreviewGenerator instead (from libkfile),
 * which can do both delayed-mimetype-determination and delayed-preview-determination
 * (actually, it rather does one or the other), and with better performance
 * (it batches the updates rather than doing them one by one)
 */
class KIO_EXPORT KMimeTypeResolver : public QObject
{
    Q_OBJECT
public:
    /**
     * The mimetype resolver is made a child of the view.
     * It is OK to have one resolver per view, and have those views share a common model;
     * this way the mimetypes will only be determined once per item in the model, which saves time.
     *
     * WARNING: if you call KDirModel::setDirLister, do it before creating the KMimeTypeResolver
     * If this is a problem, tell me and I'll add a signal...
     */
    KDE_DEPRECATED KMimeTypeResolver(QAbstractItemView* view, KDirModel* model);

    /**
     * This constructor should be used when the view uses a proxy model instead of a KDirModel.
     * The source model must be a KDirModel however, and the above warning applies
     * to this constructor as well.
     */
    KDE_DEPRECATED KMimeTypeResolver(QAbstractItemView* view, QAbstractProxyModel* model);

    /**
     * @internal
     */
    explicit KDE_DEPRECATED KMimeTypeResolver(KAbstractViewAdapter* adapter);

    ~KMimeTypeResolver();

private:
    KMimeTypeResolverPrivate *const d;

    Q_PRIVATE_SLOT( d, void _k_slotRowsInserted(const QModelIndex&,int,int) )
    Q_PRIVATE_SLOT( d, void _k_slotViewportAdjusted() )
    Q_PRIVATE_SLOT( d, void _k_slotProcessMimeIcons() )
};

#endif /* KMIMETYPERESOLVER_H */

