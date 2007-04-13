/*
   $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>
 
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

#ifndef KMETADATA_TAG_CLOUD_H
#define KMETADATA_TAG_CLOUD_H

#include <kmetadata/kmetadata_export.h>

#include "ktagcloudwidget.h"

namespace Nepomuk {
    namespace KMetaData {
	class Tag;

	/**
	 * Tag cloud widget that displays all tags stored in the
	 * local KMetaData store.
	 */
	class KMETADATA_EXPORT TagCloud : public KTagCloudWidget
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
		 * regularly to keep in sync with the local KMetaData store.
		 *
		 * \sa updateTags
		 */
		void setAutoUpdate( bool enable );

	    signals:
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
		Private* d;
	    };
    }
}

#endif
