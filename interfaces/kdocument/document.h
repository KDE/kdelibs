/* This file is part of the KDE libraries
   Copyright (C) 2005 Joseph Wenninger <jowenn@kde.org>

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
#ifndef _KDOCUMENT_DOCUMENT_
#define _KDOCUMENT_DOCUMENT_

#include <kparts/part.h>

/**
 * Namespace for document/view architectures.
 *
 * This is a very basic abstraction for applications and libraries that need
 * documents to support several views.
 */
namespace KDocument {
	class View;

	/**
	 * A KParts document that provides several views.
	 *
	 * <b>Introduction</b>\n
	 *
	 * A document can have any number of views or no view, all representing
	 * the same data. A view represents the document's content - usually
	 * graphically. Every document has a unique number accessible with
	 * documentNumber(). Create a new view with createView() and get a list
	 * of all views with views().
	 *
	 * @see KParts::ReadWritePart
	 */
	class KDE_EXPORT Document : public KParts::ReadWritePart {
		Q_OBJECT
		public:
			/**
			 * Constructor.
			 *
			 * Create a new document with @p parent.
			 * @param parent parent object
			 */
			Document(QObject *parent);

			/**
			 * Virtual destructor.
			 */
			virtual ~Document() {}

			/**
			 * Create a new view attached to @p parent.
			 * @param parent parent widget
			 * @return the new view
			 */
			virtual View *createView ( QWidget *parent ) = 0;

			/**
			 * Get a list of all views.
			 * @return all document views
			 * @see createView()
			 */
			virtual const QList<View*> &views () = 0;

			/**
			 * Get the global unique number of this document in your application.
			 * @return document number
			 */
			int documentNumber () const;

		private:
			/**
			 * Private d-pointer
			 */
			class Private;
			class Private *d;
			/**
			 * document number
			 */
			int m_documentNumber;
	};
}


#endif

// kate: space-indent off; indent-width 2; tab-width 4; replace-tabs off;
