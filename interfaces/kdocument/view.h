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
#ifndef _KDOCUMENT_VIEW_
#define _KDOCUMENT_VIEW_

#include <qwidget.h>
#include <kxmlguiclient.h>

namespace KDocument {

	/**
	 * A widget with actions representing a document.
	 *
	 * A View represents the document's content. All views of the same
	 * document are synchronized.
	 */
	class KDE_EXPORT View : public QWidget, public KXMLGUIClient {
		Q_OBJECT
		public:
			/**
			 * Constructor.
			 *
			 * Create a new view with parent widget @p parent.
			 * @param parent parent widget
			 */
			View(QWidget *parent):QWidget(parent),KXMLGUIClient() {}
			/**
			 * Virtual destructor.
			 */
			virtual ~View() {}
		signals:
			/**
			 * This signal is emitted whenever the @p view got the focus.
			 * @param view view that emitted the signal
			 * @see focusOut()
			 */
			void focusIn ( KDocument::View *view );

			/**
			 * This signal is emitted whenever the @p view lost the focus.
			 * @param view view that emitted the signal
			 * @see focusIn()
			 */
			void focusOut ( KDocument::View *view );

			/**
			 * This signal is emitted whenever the @p view sends an
			 * information with text @p message.
			 * @param view view that emitted the signal
			 * @param message the information message
			 */
			void informationMessage ( KDocument::View *view, const QString &message );
	};
}

#endif

// kate: space-indent off; indent-width 2; tab-width 4; replace-tabs off;
