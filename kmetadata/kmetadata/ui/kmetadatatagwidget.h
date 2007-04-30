/*
   $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 
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

#ifndef _KMETADATA_TAG_WIDGET_H_
#define _KMETADATA_TAG_WIDGET_H_

#include <kmetadata/kmetadata_export.h>

#include <QtGui/QWidget>

namespace Nepomuk {
    namespace KMetaData {

	class Resource;
	class Tag;

        /**
	 * TagWidget provides a simple GUI interface to assign tags.
	 * It consists of a single text line displaying the assigned 
	 * tags and a menu to change the tags.
	 */
	class KMETADATA_EXPORT TagWidget : public QWidget
	    {
		Q_OBJECT
		
	    public:
                /**
		 * Creates a new TagWidget for resource. The assigned tags are loaded
		 * instantly.
		 */
		TagWidget( const Resource& resource, QWidget* parent = 0 );
		TagWidget( QWidget* parent = 0 );
		~TagWidget();

		/**
		 * \return The resource that is supposed to be tagged or an invalid
		 * Resource if none has been set.
		 */
		Resource taggedResource() const;

                /**
		 * \return The list of all tags that are assigned to the currently set 
		 * resource or an empty list if no resource has been set.
		 *
		 * \sa setTaggedResource, taggedResource, Resource::getTags
		 */
		QList<Tag> assignedTags() const;

	    signals:
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

		/**
		 * Set the list of tags to be assigned to the configured resource.
		 * If no resource has been set this method does nothing.
		 *
		 * \sa setTaggedResource
		 */
		void setAssignedTags( const QList<Tag>& tags );

	    private Q_SLOTS:
		void slotShowTagMenu();

	    private:
		class Private;
		Private* const d;
	    };
    }
}

#endif
