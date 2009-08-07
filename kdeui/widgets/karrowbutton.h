/* This file is part of the KDE libraries
   Copyright (C) 2001, 2002 Frerich Raabe <raabe@kde.org>

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

#ifndef karrowbutton_h
#define karrowbutton_h

#include <kdeui_export.h>
#include <QtGui/QPushButton>

class KArrowButtonPrivate;
/**
 * @deprecated Use a QToolButton with the arrowType property set instead
 *
 * @short Draws a button with an arrow.
 *
 * Draws a button which shows an arrow pointing into a certain direction. The
 * arrow's alignment on the button depends on the direction it's pointing to,
 * e.g. a left arrow is aligned at the left border, a upwards arrow at the top
 * border. This class honors the currently configured KStyle when drawing
 * the arrow.
 *
 * \image html karrowbutton.png "KDE Arrow Buttons"
 *
 * @author Frerich Raabe
 */
class KDEUI_EXPORT_DEPRECATED KArrowButton : public QPushButton
{
	Q_OBJECT
	/**
	 * Arrow type for this button, from Qt::ArrowType.
	 */
	Q_PROPERTY( int arrowType READ arrowTp WRITE setArrowTp )

	public:
		/**
		 * Constructs an arrow button.
		 *
		 * @param parent This button's parent
		 * @param arrow The direction the arrrow should be pointing in
		 */
		explicit KArrowButton(QWidget *parent = 0, Qt::ArrowType arrow = Qt::UpArrow);

		/**
		 * Destructor.
		 */
		virtual ~KArrowButton();

		/**
		 * Reimplemented from QPushButton.
		 */
		virtual QSize sizeHint() const;

		/**
		* Returns the arrow type
		*/
		Qt::ArrowType arrowType() const;

		/// hack for moc braindamage with enums, use arrowType() instead
		int arrowTp() const { return (int) arrowType(); }
		/// hack for moc braindamage with enums, use setArrowType() instead
		void setArrowTp( int tp ) { setArrowType( (Qt::ArrowType) tp ); }
	public Q_SLOTS:
		/**
		 * Defines in what direction the arrow is pointing to. Will repaint the
		 * button if necessary.
		 *
		 * @param a The direction this arrow should be pointing in
		 */
		void setArrowType(Qt::ArrowType a);

	protected:
		/**
		 * Reimplemented from QPushButton.
		 */
		virtual void paintEvent(QPaintEvent*);

	private:
		KArrowButtonPrivate * const d;
};

#endif // karrowbutton_h
