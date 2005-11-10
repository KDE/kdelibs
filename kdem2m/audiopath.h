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
#ifndef KDEM2M_AUDIOPATH_H
#define KDEM2M_AUDIOPATH_H

#include <QObject>

namespace KDEM2M
{
	class AudioEffect;
	class AudioOutputBase;

	namespace Ifaces
	{
		class AudioPath;
	}

	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class AudioPath : public QObject
	{
		friend class MediaProducer;
		Q_OBJECT
		public:
			AudioPath( QObject* parent = 0 );
			~AudioPath();

			// Operations:
			bool insertEffect( AudioEffect* newEffect, AudioEffect* insertBefore = 0 );
			bool addOutput( AudioOutputBase* audioOutput );
			bool removeOutput( AudioOutputBase* audioOutput );

		protected:
			AudioPath( Ifaces::AudioPath* iface, QObject* parent );
			virtual bool aboutToDeleteIface();
			virtual Ifaces::AudioPath* createIface();
			virtual void setupIface();

		protected slots:
			// when the Factory wants to change the Backend the following slots are used
			void slotDeleteIface();
			void slotCreateIface();

		private:
			Ifaces::AudioPath* iface();
			Ifaces::AudioPath* m_iface;
			class Private;
			Private* d;
	};
} //namespace KDEM2M

// vim: sw=4 ts=4 tw=80 noet
#endif // KDEM2M_AUDIOPATH_H
