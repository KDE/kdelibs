/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef Kdem2m_TEMPLATE_H
#define Kdem2m_TEMPLATE_H

#include <QObject>

namespace Kdem2m
{
	namespace Ifaces
	{
		class Template;
	}

	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class Template : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * Create a new Template.
			 *
			 * @param parent The QObject parent.
			 */
			Template( QObject* parent = 0 );

			~Template();

		protected:
			virtual bool aboutToDeleteIface();
			virtual Ifaces::Template* createIface();

		private slots:
			void slotDeleteIface();
			void slotCreateIface();

		private:
			Ifaces::Template* iface();
			Ifaces::Template* m_iface;
			class Private;
			Private * d;
	};
} //namespace Kdem2m

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_TEMPLATE_H
