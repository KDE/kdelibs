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

#ifndef _KMETADATA_TAG_WIDGET_H_
#define _KMETADATA_TAG_WIDGET_H_

#include "nepomuk_export.h"

#include <QtGui/QWidget>

namespace Nepomuk {
    class Resource;
    class Tag;

    /**
     * TagWidget provides a simple GUI interface to assign tags.
     * It consists of a single text line displaying the assigned
     * tags and a menu to change the tags.
     */
    class NEPOMUK_EXPORT TagWidget : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Creates a new TagWidget for resource. The assigned tags are loaded
         * instantly.
         */
        explicit TagWidget( const Resource& resource, QWidget* parent = 0 );
        TagWidget( QWidget* parent = 0 );
        ~TagWidget();

        /**
         * \return The resources that are supposed to be tagged or an empty
         * list if none have been set.
         */
        QList<Resource> taggedResources() const;

        /**
         * \return The list of all tags that are assigned to the currently set
         * resource or an empty list if no resource has been set.
         *
         * \sa setTaggedResource, taggedResource, Resource::getTags
         */
        QList<Tag> assignedTags() const;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever a tag is clicked.
         */
        void tagClicked( Tag );

    public Q_SLOTS:
        /**
         * Set the Resource to be tagged. The assigned tags will be loaded
         * instantly.
         */
        void setTaggedResource( const Resource& resource );

        void setTaggedResources( const QList<Resource>& resources );

        /**
         * Set the list of tags to be assigned to the configured resource.
         * If no resource has been set this method does nothing.
         *
         * \sa setTaggedResource
         */
        void setAssignedTags( const QList<Tag>& tags );

    private Q_SLOTS:
        void fillTagMenu();
        void createTag();
        void updateAssignedTagsFromMenu();
        void slotTagUpdateDone();
        void slotTagClicked( const QString& text );

    private:
        class Private;
        Private* const d;
    };
}

#endif
