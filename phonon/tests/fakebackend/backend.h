/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef Phonon_FAKE_BACKEND_H
#define Phonon_FAKE_BACKEND_H

#include <kdelibs_export.h>
#include <QList>
#include <QPointer>
#include <QStringList>

class KUrl;

namespace Phonon
{
namespace Fake
{
	class AudioOutput;

	class PHONON_FAKE_EXPORT Backend : public QObject
	{
		Q_OBJECT
		public:
			Backend( QObject* parent, const QStringList& args );
			virtual ~Backend();

			Q_INVOKABLE QObject*      createMediaObject( QObject* parent );
			Q_INVOKABLE QObject*       createMediaQueue( QObject* parent );
			Q_INVOKABLE QObject*        createAvCapture( QObject* parent );
			Q_INVOKABLE QObject*       createByteStream( QObject* parent );

			Q_INVOKABLE QObject*        createAudioPath( QObject* parent );
			Q_INVOKABLE QObject*      createAudioEffect( int effectId, QObject* parent );
			Q_INVOKABLE QObject* createVolumeFaderEffect( QObject* parent );
			Q_INVOKABLE QObject*      createAudioOutput( QObject* parent );
			Q_INVOKABLE QObject*  createAudioDataOutput( QObject* parent );
			Q_INVOKABLE QObject*    createVisualization( QObject* parent );

			Q_INVOKABLE QObject*        createVideoPath( QObject* parent );
			Q_INVOKABLE QObject*      createVideoEffect( int effectId, QObject* parent );
			Q_INVOKABLE QObject* createBrightnessControl( QObject* parent );
			Q_INVOKABLE QObject*  createVideoDataOutput( QObject* parent );

			Q_INVOKABLE bool supportsVideo() const;
			Q_INVOKABLE bool supportsOSD() const;
			Q_INVOKABLE bool supportsFourcc( quint32 fourcc ) const;
			Q_INVOKABLE bool supportsSubtitles() const;
			Q_INVOKABLE QStringList knownMimeTypes() const;

			void freeSoundcardDevices();

			Q_INVOKABLE QSet<int> audioOutputDeviceIndexes() const;
			Q_INVOKABLE QString audioOutputDeviceName( int index ) const;
			Q_INVOKABLE QString audioOutputDeviceDescription( int index ) const;

			Q_INVOKABLE QSet<int> audioCaptureDeviceIndexes() const;
			Q_INVOKABLE QString audioCaptureDeviceName( int index ) const;
			Q_INVOKABLE QString audioCaptureDeviceDescription( int index ) const;
			Q_INVOKABLE qint32 audioCaptureDeviceVideoIndex( int index ) const;

			Q_INVOKABLE QSet<int> videoOutputDeviceIndexes() const;
			Q_INVOKABLE QString videoOutputDeviceName( int index ) const;
			Q_INVOKABLE QString videoOutputDeviceDescription( int index ) const;

			Q_INVOKABLE QSet<int> videoCaptureDeviceIndexes() const;
			Q_INVOKABLE QString videoCaptureDeviceName( int index ) const;
			Q_INVOKABLE QString videoCaptureDeviceDescription( int index ) const;
			Q_INVOKABLE qint32 videoCaptureDeviceAudioIndex( int index ) const;

			Q_INVOKABLE QSet<int> visualizationIndexes() const;
			Q_INVOKABLE QString visualizationName( int index ) const;
			Q_INVOKABLE QString visualizationDescription( int index ) const;

			Q_INVOKABLE QSet<int> audioEffectIndexes() const;
			Q_INVOKABLE QString audioEffectName( int index ) const;
			Q_INVOKABLE QString audioEffectDescription( int index ) const;

			Q_INVOKABLE QSet<int> videoEffectIndexes() const;
			Q_INVOKABLE QString videoEffectName( int index ) const;
			Q_INVOKABLE QString videoEffectDescription( int index ) const;

			Q_INVOKABLE QSet<int> audioCodecIndexes() const;
			Q_INVOKABLE QString audioCodecName( int index ) const;
			Q_INVOKABLE QString audioCodecDescription( int index ) const;

			Q_INVOKABLE QSet<int> videoCodecIndexes() const;
			Q_INVOKABLE QString videoCodecName( int index ) const;
			Q_INVOKABLE QString videoCodecDescription( int index ) const;

			Q_INVOKABLE QSet<int> containerFormatIndexes() const;
			Q_INVOKABLE QString containerFormatName( int index ) const;
			Q_INVOKABLE QString containerFormatDescription( int index ) const;

			Q_INVOKABLE char const* uiLibrary() const;
			// Q_INVOKABLE char* uiSymbol() const;

		private:
			QStringList m_supportedMimeTypes;
			QList<QPointer<AudioOutput> > m_audioOutputs;
	};
}} // namespace Phonon::Ifaces

// vim: sw=4 ts=4 noet tw=80
#endif // Phonon_FAKE_BACKEND_H
