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

#include <QObject>
#include <kdelibs_export.h>
class KFileItemList;
class QAbstractItemView;
class KDirModel;
class KMimeTypeResolverPrivate;

/**
 * This class implements the "delayed-mimetype-determination" feature,
 * for directory views.
 *
 * It determines the mimetypes of the icons in the background, but giving
 * preferrence to the visible icons.
 *
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
    explicit KMimeTypeResolver(QAbstractItemView* view, KDirModel* model);

    ~KMimeTypeResolver();

Q_SIGNALS:
    // void mimeTypeDeterminationFinished();

private Q_SLOTS:
    void slotNewItems( const KFileItemList& );

    /**
     * Connected to the timer
     */
    void slotProcessMimeIcons();

private:
    KMimeTypeResolverPrivate *const d;
};

#endif /* KMIMETYPERESOLVER_H */

