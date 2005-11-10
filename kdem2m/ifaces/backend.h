/*  This file is part of the KDE project
    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

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

#ifndef Kdem2m_IFACES_BACKEND_H
#define Kdem2m_IFACES_BACKEND_H

#include <QObject>
#include <kmimetype.h>
class KURL;

namespace Kdem2m
{
namespace Ifaces
{
	class MediaObject;
	class SoundcardCapture;
	class ByteStream;

	class AudioPath;
	class AudioEffect;
	class AudioOutputBase;
	class AudioOutput;
	class AudioDataOutput;
	class AudioFftOutput;

	class VideoPath;
	class VideoEffect;
	class VideoOutputBase;

	/**
	 * \brief Base class for all KDE Multimedia Backends
	 *
	 * This class provides the interface for Multimedia Backends. Use it to get
	 * a pointer to a new Player or VideoWidget.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class Backend : public QObject
	{
		Q_OBJECT
		public:
			Backend( QObject* parent ) : QObject( parent ) {}
			virtual ~Backend() {}

			virtual MediaObject*      createMediaObject( QObject* parent ) = 0;
			virtual SoundcardCapture* createSoundcardCapture( QObject* parent ) = 0;
			virtual ByteStream*       createByteStream( QObject* parent ) = 0;

			virtual AudioPath*        createAudioPath( QObject* parent ) = 0;
			virtual AudioEffect*      createAudioEffect( QObject* parent ) = 0;
			virtual AudioOutput*      createAudioOutput( QObject* parent ) = 0;
			virtual AudioDataOutput*  createAudioDataOutput( QObject* parent ) = 0;
			virtual AudioFftOutput*   createAudioFftOutput( QObject* parent ) = 0;

			virtual VideoPath*        createVideoPath( QObject* parent ) = 0;
			virtual VideoEffect*      createVideoEffect( QObject* parent ) = 0;
			virtual VideoOutputBase*  createVideoOutputBase( QObject* parent ) = 0;

			virtual bool supportsVideo() const = 0;
			virtual bool supportsOSD() const = 0;
			virtual bool supportsSubtitles() const = 0;
			virtual const KMimeType::List& knownMimeTypes() const = 0;
			virtual const QStringList& availableSoundcardCaptureTypes() const = 0;
			virtual const QStringList& availableAudioEffects() const = 0;
			virtual const QStringList& availableVideoEffects() const = 0;

		private:
			class Private;
			Private * d;
	};
}} // namespace Kdem2m::Ifaces

// vim: sw=4 ts=4 noet tw=80
#endif // Kdem2m_IFACES_BACKEND_H
