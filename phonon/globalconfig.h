/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include <QObject>
#include <ksharedconfig.h>
#include "phononnamespace.h"

#ifndef PHONON_GLOBALCONFIG_H
#define PHONON_GLOBALCONFIG_H

namespace Phonon
{
	class GlobalConfig : public QObject
	{
		Q_OBJECT
		public:
			GlobalConfig( QObject *parent = 0 );
			~GlobalConfig();

			QList<int> audioOutputDeviceListFor( Phonon::Category category ) const;
			int audioOutputDeviceFor( Phonon::Category category ) const;

                        QList<int> audioCaptureDeviceList() const;
                        int audioCaptureDevice() const;

                Q_SIGNALS:
			void audioOutputDeviceConfigChanged();

		private:
			KSharedConfig::Ptr m_config;
	};
} // namespace Phonon
#endif // PHONON_GLOBALCONFIG_H
