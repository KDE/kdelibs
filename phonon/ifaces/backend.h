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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef Phonon_IFACES_BACKEND_H
#define Phonon_IFACES_BACKEND_H

#include <QObject>
#include <kmimetype.h>
#include <kdelibs_export.h>

class KUrl;

namespace Phonon
{
/**
 * \short The interfaces that need to be implemented by a backend.
 *
 * As a Phonon backend developer you'll have to implement the interfaces in this
 * namespace. Start with an implementation of Backend returning 0 for
 * all create methods. Then go on with MediaObject, AudioPath and AudioOutput
 * (and add them to the create methods in Backend). With those three classes
 * your backend can already do basic playback of audio files.
 *
 * \see Phonon::Ui::Ifaces
 * \author Matthias Kretz <kretz@kde.org>
 */
namespace Ifaces
{
	class MediaObject;
	class AvCapture;
	class ByteStream;

	class AudioPath;
	class AudioEffect;
	class VolumeFaderEffect;
	class AudioOutput;
	class AudioDataOutput;

	class VideoPath;
	class VideoEffect;

	/**
	 * \brief Central class for all Phonon backends.
	 *
	 * Exactly one instance of this class is created for the Phonon fronted
	 * class which utilizes it for Phonon::BackendCapabilities and to create all the
	 * other backend classes.
	 *
	 * \remarks
	 * If you need new factory-methods for the backend:
	 * - add a new Phonon::Ifaces::BackendV2 interface that inherits
	 *   Phonon::Ifaces::Backend and in Phonon::Factory check whether you can
	 *   qobject_cast<Phonon::Ifaces::BackendV2*>( d->backend ). If the cast succeeds
	 *   you have a backend that implements your new virtual functions.
	 * - simply add the new Phonon::Ifaces classes to libphononifaces and backends
	 *   can then implement those interfaces and instead of implementing
	 *   Ifaces::Backend implementing Ifaces::BackendV2.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONIFACES_EXPORT Backend : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * Standard QObject constructor.
			 */
			Backend( QObject* parent = 0 );
			virtual ~Backend();

			/**
			 * Creates an instance of MediaObject
			 */
			virtual MediaObject*      createMediaObject( QObject* parent ) = 0;

			/**
			 * Creates an instance of AvCapture
			 */
			virtual AvCapture*        createAvCapture( QObject* parent ) = 0;

			/**
			 * Creates an instance of ByteStream
			 */
			virtual ByteStream*       createByteStream( QObject* parent ) = 0;

			/**
			 * Creates an instance of AudioPath
			 */
			virtual AudioPath*        createAudioPath( QObject* parent ) = 0;

			/**
			 * Creates an instance of AudioEffect
			 */
			virtual AudioEffect*      createAudioEffect( QObject* parent ) = 0;

			/**
			 * Creates an instance of VolumeFaderEffect
			 */
			virtual VolumeFaderEffect* createVolumeFaderEffect( QObject* parent ) = 0;

			/**
			 * Creates an instance of AudioOutput
			 */
			virtual AudioOutput*      createAudioOutput( QObject* parent ) = 0;

			/**
			 * Creates an instance of AudioDataOutput
			 */
			virtual AudioDataOutput*  createAudioDataOutput( QObject* parent ) = 0;

			/**
			 * Creates an instance of VideoPath
			 */
			virtual VideoPath*        createVideoPath( QObject* parent ) = 0;

			/**
			 * Creates an instance of VideoEffect
			 */
			virtual VideoEffect*      createVideoEffect( QObject* parent ) = 0;

			/**
			 * Tells whether the backend is audio only or can handle video files
			 * and display of videos.
			 */
			virtual bool supportsVideo() const = 0;

			/**
			 * Tells whether the backend implements the OSD interfaces.
			 *
			 * \todo there's no interfaces for that at this point
			 */
			virtual bool supportsOSD() const = 0;

			/**
			 * Tells whether the backend supports subtitle rendering on the
			 * video output.
			 *
			 * \todo there's no interfaces for that at this point
			 */
			virtual bool supportsSubtitles() const = 0;

			/**
			 * Lists the MIME types the backend can read and decode.
			 */
			virtual const KMimeType::List& knownMimeTypes() const = 0;

			/* docu if the IPC method in Phonon::Factory is uncommented:
			 *
			 * Called when the application needs to free the soundcard device.
			 * When this method is called the backend needs to pause or even
			 * stop all media outputs to a soundcard device and then free all
			 * hardware devices.
			 *
			 * This method will be called from other applications for example
			 * when the user wants to use that other program that cannot access
			 * the device (e.g. he needs to use jackd and ALSA dmix won't work).
			 *
			 * In order for the application to not become useless until the next
			 * restart, you should wait a few seconds and then allow
			 * MediaObject::play calls to open the soundcard devices again.
			 */
			/**
			 * Called when the frontend thinks the soundcard device is not
			 * needed anymore. This happens if there are no audio signals
			 * produced.
			 *
			 * The backend should try to free the soundcard device when it's not
			 * needed anyway, but if you don't have any logic available to free
			 * the device you can just as well use this method to know when to
			 * free the device.
			 *
			 * As soon as you need the device again you are free to reopen it
			 * again. If opening fails (because some other application blocks
			 * the device) the backend needs to retry periodically to open the
			 * device.
			 *
			 * \todo provide a method to notify the application/end user that
			 * the device is blocked
			 */
			virtual void freeSoundcardDevices() = 0;

			/**
			 * Returns the number of available audio output devices. An associated
			 * name and description can be found using audioOutputDeviceName
			 * and audioOutputDeviceDescription.
			 *
			 * \return The number of available output devices.
			 * \see audioOutputDeviceName
			 * \see audioOutputDeviceDescription
			 *
			 * \test The return value has to be \f$\ge0\f$
			 */
			virtual int audioOutputDeviceCount() const = 0;

			/**
			 * Returns the name of the output device.
			 *
			 * \param index \f$0<\mathrm{index}\leq\mathrm{audioOutputDeviceCount}\f$
			 *
			 * \return The name of this output device.
			 *
			 * \test The return value has to be non-empty.
			 */
			virtual QString audioOutputDeviceName( int index ) const = 0;

			/**
			 * Returns a description for the output device.
			 *
			 * \param index \f$0<index\leq\f$ audioOutputDeviceCount
			 *
			 * \return The description for this output device.
			 */
			virtual QString audioOutputDeviceDescription( int index ) const = 0;

			/**
			 * Returns the number of available capture sources. An associated
			 * name and description can be found using audioCaptureDeviceName
			 * and audioCaptureDeviceDescription.
			 *
			 * \return The number of available capture sources.
			 * \see audioCaptureDeviceName
			 * \see audioCaptureDeviceDescription
			 *
			 * \test The return value has to be \f$\ge0\f$
			 */
			virtual int audioCaptureDeviceCount() const = 0;

			/**
			 * Returns the name of the capture source.
			 *
			 * \param index \f$0<\mathrm{index}\leq\mathrm{audioCaptureDeviceCount}\f$
			 *
			 * \return The name of this capture source.
			 *
			 * \test The return value has to be non-empty.
			 */
			virtual QString audioCaptureDeviceName( int index ) const = 0;

			/**
			 * Returns a description for the capture source.
			 *
			 * \param index \f$0<index\leq\f$ audioCaptureDeviceCount
			 *
			 * \return The description for this capture source.
			 */
			virtual QString audioCaptureDeviceDescription( int index ) const = 0;

			/**
			 * Returns the index of the video capture source that is associated
			 * with the given audio capture source. For example if the capture
			 * source is a TV card, the audio and video sources should be marked
			 * as belonging together.
			 */
			virtual int audioCaptureDeviceVideoIndex( int index ) const = 0;

			/**
			 * Returns the number of available capture sources. An associated
			 * name and description can be found using videoCaptureDeviceName
			 * and videoCaptureDeviceDescription.
			 *
			 * \return The number of available capture sources.
			 * \see videoCaptureDeviceName
			 * \see videoCaptureDeviceDescription
			 *
			 * \test The return value has to be \f$\ge0\f$
			 */
			virtual int videoCaptureDeviceCount() const = 0;

			/**
			 * Returns the name of the capture source.
			 *
			 * \param index \f$0<\mathrm{index}\leq\mathrm{videoCaptureDeviceCount}\f$
			 *
			 * \return The name of this capture source.
			 *
			 * \test The return value has to be non-empty.
			 */
			virtual QString videoCaptureDeviceName( int index ) const = 0;

			/**
			 * Returns a description for the capture source.
			 *
			 * \param index \f$0<\mathrm{index}\leq\mathrm{videoCaptureDeviceCount}\f$
			 *
			 * \return The description for this capture source.
			 */
			virtual QString videoCaptureDeviceDescription( int index ) const = 0;

			/**
			 * Returns the index of the audio capture source that is associated
			 * with the given video capture source. For example if the capture
			 * source is a TV card, the audio and video sources should be marked
			 * as belonging together.
			 */
			virtual int videoCaptureDeviceAudioIndex( int index ) const = 0;

			// effects
			virtual const QStringList& availableAudioEffects() const = 0;
			virtual const QStringList& availableVideoEffects() const = 0;

			/**
			 * Returns the name of the DSO implementing Ui::Ifaces.
			 */
			virtual const char* uiLibrary() const = 0;

			/**
			 * Returns the symbol used for retrieving the Ui::Ifaces::Backend
			 * instance.
			 *
			 * This is usefull if you have both Phonon::Ifaces and
			 * Phonon::Ui::Ifaces in the same DSO. Then you can add another
			 * function returning the Ui::Ifaces::Backend instance.
			 */
			virtual const char* uiSymbol() const { return 0; }

		private:
			class Private;
			Private * d;
	};
}} // namespace Phonon::Ifaces

// vim: sw=4 ts=4 noet tw=80
#endif // Phonon_IFACES_BACKEND_H
