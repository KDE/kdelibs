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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/
#ifndef Kdem2m_VIDEOOUTPUTBASE_H
#define Kdem2m_VIDEOOUTPUTBASE_H

#include <QObject>

class QString;

namespace Kdem2m
{
	namespace Ifaces
	{
		class VideoOutputBase;
	}

	class VideoOutputBase : public QObject
	{
		friend class VideoPath;
		Q_OBJECT
		public:
			VideoOutputBase( QObject* parent = 0 );
			~VideoOutputBase();

			// Attributes Getters:
			QString name() const;

		public slots:
			// Attributes Setters:
			void setName( const QString& newName );

		protected:
			VideoOutputBase( Ifaces::VideoOutputBase* iface, QObject* parent );
			virtual bool aboutToDeleteIface();
			virtual Ifaces::VideoOutputBase* createIface();
			virtual void setupIface();

		protected slots:
			// when the Factory wants to change the Backend the following slots are used
			void slotDeleteIface();
			void slotCreateIface();

		private:
			Ifaces::VideoOutputBase* iface();
			Ifaces::VideoOutputBase* m_iface;
			class Private;
			Private* d;
	};
} //namespace Kdem2m

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_VIDEOOUTPUTBASE_H
