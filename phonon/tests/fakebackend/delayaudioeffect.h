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

#ifndef PHONON_FAKE_DELAYAUDIOEFFECT_H
#define PHONON_FAKE_DELAYAUDIOEFFECT_H

#include <QQueue>
#include "effectinterface.h"

namespace Phonon
{
namespace Fake
{
	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class DelayAudioEffect : public EffectInterface
	{
		public:
			DelayAudioEffect();
			~DelayAudioEffect();

			virtual QVariant value( int parameterId ) const;
			virtual void setValue( int parameterId, QVariant newValue );
			virtual void processBuffer( QVector<float>& buffer );

		private:
			QQueue<float> m_delayBuffer;
			float m_feedback, m_level;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // PHONON_FAKE_DELAYAUDIOEFFECT_H
