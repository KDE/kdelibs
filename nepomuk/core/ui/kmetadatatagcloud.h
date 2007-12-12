/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef KMETADATATAGCLOUD_H
#define KMETADATATAGCLOUD_H

#include <nepomuk/nepomuk_export.h>

#include "ktagcloudwidget.h"

namespace Nepomuk {
    class Tag;

    /**
     * Tag cloud widget that displays all tags stored in the
     * local Nepomuk store.
     */
    class NEPOMUK_EXPORT TagCloud : public KTagCloudWidget
    {
        Q_OBJECT

    public:
        /**
         * Create a new TagCloud widget.
         * The widget will be populated immediately.
         */
        TagCloud( QWidget* parent = 0 );
        ~TagCloud();

        /**
         * \return true if auto updating is enabled (the default).
         *
         * \sa setAutoUpdate
         */
        bool autoUpdate() const;

    public Q_SLOTS:
        /**
         * Update the tag cloud, ie. reload all tags from the store
         * and recalculate their importance.
         *
         * There is no need to call this unless auto updating is
         * disabled.
         *
         * \sa setAutoUpdate
         */
        void updateTags();

        /**
         * If auto updating is enabled the tag cloud is updated
         * regularly to keep in sync with the local Nepomuk store.
         *
         * \sa updateTags
         */
        void setAutoUpdate( bool enable );

    Q_SIGNALS:
        /**
         * This signal is emitted whenever a tag is clicked.
         *
         * \sa KTagCloudWidget::tagClicked
         */
        void tagClicked( Tag );

    private Q_SLOTS:
        void slotTagClicked( const QString& tag );

    private:
        class Private;
        Private* const d;
    };
}

#endif // KMETADATATAGCLOUD_H
