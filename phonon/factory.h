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

#ifndef Phonon_FACTORY_H
#define Phonon_FACTORY_H

#include <QObject>

#include <dcopobject.h>
#include <kstaticdeleter.h>
#include <kdelibs_export.h>

namespace Phonon
{
	namespace Ifaces
	{
		class MediaObject;
		class AvCapture;
		class ByteStream;

		class AudioEffect;
		class VolumeFaderEffect;
		class AudioPath;
		class AudioOutput;
		class AudioDataOutput;

		class VideoEffect;
		class VideoPath;

		class Backend;
	}
	class BasePrivate;

/**
 * \brief Factory to access the preferred Backend.
 *
 * This class is used internally to map the backends functionality to the API.
 * This class is your entry point to KDE Multimedia usage. It provides the
 * necessary objects for playing audio and video.
 *
 * For simple access to just playing an audio file see SimplePlayer.
 *
 * \remarks
 * Extensions to the existing functionality can either be added by using the
 * reserved virtual functions in Backend or by adding a new interface e.g.
 * BackendV2 and creating a BackendV2 instance when the Backend instance is
 * created.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \internal
 */
class PHONON_EXPORT Factory : public QObject, public DCOPObject
{
	friend void ::KStaticDeleter<Factory>::destructObject();
	friend class Phonon::BasePrivate;

	Q_OBJECT
	public:
		/**
		 * Returns a pointer to the factory.
		 * Use this function to get an instance of Factory.
		 *
		 * @return a pointer to the factory. If no factory exists until now then
		 * one is created
		 */
		static Factory* self();

		/**
		 * Create a new Ifaces::MediaObject.
		 *
		 * @return a pointer to the Ifaces::MediaObject the backend provides
		 */
		Ifaces::MediaObject* createMediaObject( QObject* parent = 0 );
		Ifaces::AvCapture* createAvCapture( QObject* parent = 0 );
		Ifaces::ByteStream* createByteStream( QObject* parent = 0 );
		
		Ifaces::AudioPath* createAudioPath( QObject* parent = 0 );
		Ifaces::AudioEffect* createAudioEffect( QObject* parent = 0 );
		Ifaces::VolumeFaderEffect* createVolumeFaderEffect( QObject* parent = 0 );
		Ifaces::AudioOutput* createAudioOutput( QObject* parent = 0 );
		Ifaces::AudioDataOutput* createAudioDataOutput( QObject* parent = 0 );

		Ifaces::VideoPath* createVideoPath( QObject* parent = 0 );
		Ifaces::VideoEffect* createVideoEffect( QObject* parent = 0 );

		const Ifaces::Backend* backend() const;
		const char* uiLibrary() const;
		const char* uiSymbol() const;
#if 0
		/**
		 * Checks whether a certain mimetype is playable.
		 */
		bool isMimeTypePlayable( const QString & mimetype ) const;

		/**
		 * Get the name of the Backend. It's the name from the .desktop file.
		 */
		QString backendName() const;

		/**
		 * Get the comment of the Backend. It's the comment from the .desktop file.
		 */
		QString backendComment() const;

		/**
		 * Get the version of the Backend. It's the version from the .desktop file.
		 *
		 * The version is especially interesting if there are several versions
		 * available for binary incompatible versions of the backend's media
		 * framework.
		 */
		QString backendVersion() const;

		/**
		 * Get the icon (name) of the Backend. It's the icon from the .desktop file.
		 */
		QString backendIcon() const;

		/**
		 * Get the website of the Backend. It's the website from the .desktop file.
		 */
		QString backendWebsite() const;
#endif

	Q_SIGNALS:
		/**
		 * This signal is emitted when the user changes the backend. At that
		 * point all references to Ifaces objects need to be freed.
		 */
		void deleteYourObjects();

		/**
		 * After you got a deleteYourObjects() signal the backend is changed
		 * internally. Then you will receive this signal, and only then should
		 * you reconstruct all your objects again. This time they will
		 * internally use the new backend.
		 */
		void recreateObjects();

		void backendChanged();

	protected:
		Factory();
		~Factory();

	protected:
		template<class T> T* Factory::registerObject( T* o );
	public:
		/**
		 * \internal
		 */
		void registerQObject( QObject* o );

	private Q_SLOTS:
		void objectDestroyed( QObject * );

	private:
		void registerFrontendObject( BasePrivate* );
		void deregisterFrontendObject( BasePrivate* );
		static Factory * m_self;
		class Private;
		Private * d;

	K_DCOP
	k_dcop:
		/**
		 * \internal
		 * This is called via DCOP when the user changes the Phonon Backend.
		 */
		void phononBackendChanged();

//X 		It is probably better if we can get away with internal handling of
//X 		freeing the soundcard device when it's not needed anymore and
//X 		providing an IPC method to stop all MediaProducers -> free all
//X 		devices
//X 		/**
//X 		 * \internal
//X 		 * This is called when the application needs to free the soundcard
//X 		 * device(s).
//X 		 */
//X 		void freeSoundcardDevices();
};
} // namespace Phonon

#endif // BACKENDFACTORY_H
// vim: sw=4 ts=4 tw=80 noet
