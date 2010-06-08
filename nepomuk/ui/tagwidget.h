/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2010 Sebastian Trueg <trueg@kde.org>
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

#ifndef _NEPOMUK_TAG_WIDGET_H_
#define _NEPOMUK_TAG_WIDGET_H_

#include "nepomuk_export.h"

#include <QtGui/QWidget>

namespace Nepomuk {
    class Resource;
    class Tag;
    class TagWidgetPrivate;

    /**
     * \class TagWidget tagwidget.h Nepomuk/TagWidget
     *
     * \brief Allows to change a selection of tags.
     *
     * TagWidget provides a simple GUI interface to assign tags.
     * It has two basic modes:
     * \li If resources are set via setTaggedResource() or setTaggedResources()
     * the changes in the tag selection are automatically assigned to the
     * selected resources.
     * \li If no resources have been set the widget simply emits the selectionChanged()
     * signal.
     *
     * \author Sebastian Trueg <trueg@kde.org>
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

        /**
         * Constructor
         */
        TagWidget( QWidget* parent = 0 );

        /**
         * Destructor
         */
        ~TagWidget();

        /**
         * \return The resources that are supposed to be tagged or an empty
         * list if none have been set.
         */
        QList<Resource> taggedResources() const;

        /**
         * \deprecated use selectedTags() instead
         */
        KDE_DEPRECATED QList<Tag> assignedTags() const;

        /**
         * The list of selected tags.
         *
         * \return The list of all tags that are currently selected. In case
         * resources to be tagged have been selected this list matches the
         * tags assigned to the resources.
         *
         * \sa setTaggedResource, taggedResource, Resource::getTags
         *
         * \since 4.5
         */
        QList<Nepomuk::Tag> selectedTags() const;

        /**
         * By default the TagWidget shows the most often used tags in the
         * main window and allows access to all tags via an additional button.
         * This is the maximum number of tags that should be shown unless they
         * are selected.
         *
         * \sa setMaxTagsShown()
         *
         * \since 4.5
         */
        int maxTagsShown() const;

        /**
         * The alignment of the tags in the widget.
         *
         * \since 4.5
         */
        Qt::Alignment alignment() const;

        /**
         * Flags to configure the widget.
         *
         * \since 4.5
         */
        enum ModeFlag {
            /**
             * The mini mode which tries to
             * display the tags on the least
             * space possible.
             */
            MiniMode = 0x1,

            /**
             * The standard mode which is used by default
             * provides a set of check boxes for the most
             * frequently used tags.
             */
            StandardMode = 0x2,

            /**
             * Read only mode which prevents the changing
             * of tags by the user.
             */
            ReadOnly = 0x4,

            /**
             * Disable the clicking of the tags. This will
             * also disable the emitting of the tagClicked()
             * signal.
             */
            DisableTagClicking = 0x8
        };
        Q_DECLARE_FLAGS( ModeFlags, ModeFlag )

        /**
         * Flags the widget is configured with.
         *
         * \sa setModeFlags()
         *
         * \since 4.5
         */
        ModeFlags modeFlags() const;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever a tag is clicked.
         */
        void tagClicked( Nepomuk::Tag );

        /**
         * Emitted whenever the selection of tags changes.
         *
         * \since 4.5
         */
        void selectionChanged( const QList<Nepomuk::Tag>& tags );

    public Q_SLOTS:
        /**
         * Set the Resource to be tagged. The assigned tags will be loaded
         * instantly.
         */
        void setTaggedResource( const Resource& resource );

        /**
         * Set the resources to be tagged. If the list of resources is
         * empty TagWidget will only emit the selectionChanged() signal.
         */
        void setTaggedResources( const QList<Resource>& resources );

        /**
         * \deprecated use setSelectedTags() instead
         */
        KDE_DEPRECATED void setAssignedTags( const QList<Nepomuk::Tag>& tags );

        /**
         * Set the list of selected tags. In case resources have been
         * set via setTaggedResource() or setTaggedResources() their
         * list of tags is changed automatically.
         *
         * \since 4.5
         */
        void setSelectedTags( const QList<Nepomuk::Tag>& tags );

        /**
         * By default the TagWidget shows the most often used tags in the
         * main window and allows access to all tags via an additional button.
         *
         * The number of tags that are shown by default can be changed.
         *
         * \param max The maximum number of tags that should be shown in the
         * main window. Set to 0 for no limit. Be aware that more tags might be
         * shown since selected tags are always shown.
         *
         * \since 4.5
         */
        void setMaxTagsShown( int max );

        /**
         * Set the alignment to use. Only horizontal alignment flags make a
         * difference.
         *
         * \since 4.5
         */
        void setAlignment( Qt::Alignment alignment );

        /**
         * Set flags to change the behaviour and look of the tag widget.
         *
         * \since 4.5
         */
        void setModeFlags( ModeFlags flags );

    private Q_SLOTS:
        void slotShowAll();
        void slotTagUpdateDone();
        void slotTagStateChanged( const Nepomuk::Tag&, int );

    private:
        TagWidgetPrivate* const d;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Nepomuk::TagWidget::ModeFlags )

#endif
