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
#ifndef Kdem2m_VIDEOPATH_H
#define Kdem2m_VIDEOPATH_H

#include <QObject>
#include <kdelibs_export.h>

namespace Kdem2m
{
	class VideoEffect;
	class VideoOutputBase;

	namespace Ifaces
	{
		class VideoPath;
	}

	class KDEM2M_EXPORT VideoPath : public QObject
	{
		friend class MediaProducer;
		Q_OBJECT
		public:
			VideoPath( QObject* parent = 0 );
			~VideoPath();

			// Operations:
			bool insertEffect( VideoEffect* newEffect, VideoEffect* insertBefore = 0 );
			bool addOutput( VideoOutputBase* videoOutput );
			bool removeOutput( VideoOutputBase* videoOutput );

		protected:
			VideoPath( Ifaces::VideoPath* iface, QObject* parent );
			virtual bool aboutToDeleteIface();
			virtual Ifaces::VideoPath* createIface();
			virtual void setupIface();

		protected slots:
			// when the Factory wants to change the Backend the following slots are used
			void slotDeleteIface();
			void slotCreateIface();

		private:
			Ifaces::VideoPath* iface();
			Ifaces::VideoPath* m_iface;
			class Private;
			Private* d;
	};
} //namespace Kdem2m

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_VIDEOPATH_H
