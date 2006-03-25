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
#ifndef Phonon_FAKE_AVCAPTURECAPTURE_H
#define Phonon_FAKE_AVCAPTURECAPTURE_H

#include "abstractmediaproducer.h"
#include "../../ifaces/avcapture.h"

namespace Phonon
{
namespace Fake
{
	class AvCapture : public AbstractMediaProducer, virtual public Ifaces::AvCapture
	{
		Q_OBJECT
		public:
			AvCapture( QObject* parent );
			virtual ~AvCapture();

			virtual int audioCaptureDevice() const;
			virtual int setAudioCaptureDevice( int index );
			virtual int videoCaptureDevice() const;
			virtual int setVideoCaptureDevice( int index );

		private:
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_AVCAPTURECAPTURE_H
