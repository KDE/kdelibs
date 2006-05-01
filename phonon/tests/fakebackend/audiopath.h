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
#ifndef Phonon_FAKE_AUDIOPATH_H
#define Phonon_FAKE_AUDIOPATH_H

#include <QObject>
#include "../../ifaces/audiopath.h"
#include <QList>

namespace Phonon
{
namespace Fake
{
	class AudioEffect;
	class AbstractAudioOutput;

	class AudioPath : public QObject, virtual public Ifaces::AudioPath
	{
		Q_OBJECT
		public:
			AudioPath( QObject* parent );
			virtual ~AudioPath();

			virtual int availableChannels() const;
			virtual QString channelName( int ) const;
			virtual bool selectChannel( int );
			virtual int selectedChannel() const;

			virtual bool addOutput( Ifaces::AbstractAudioOutput* audioOutput );
			virtual bool removeOutput( Ifaces::AbstractAudioOutput* audioOutput );
			virtual bool insertEffect( Ifaces::AudioEffect* newEffect, Ifaces::AudioEffect* insertBefore = 0 );
			virtual bool removeEffect( Ifaces::AudioEffect* effect );

			// fake specific
			void processBuffer( const QVector<float>& buffer );

		public:
			virtual QObject* qobject() { return this; }
			virtual const QObject* qobject() const { return this; }

		private:
			QList<AudioEffect*> m_effects;
			QList<AbstractAudioOutput*> m_outputs;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_AUDIOPATH_H
