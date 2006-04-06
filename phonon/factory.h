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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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
 * \internal
 * \brief Factory to access the preferred Backend.
 *
 * This class is used internally to get the backend's implementation of the
 * Ifaces interfaces. It keeps track of the objects that were created. When a
 * request for a backend changes comes it asks all frontend objects to delete
 * their backend objects and then checks whether they were all deleted. Only
 * then the old backend is unloaded and the new backend is loaded.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONCORE_EXPORT Factory : public QObject, public DCOPObject
{
	friend void ::KStaticDeleter<Factory>::destructObject();
	friend class Phonon::BasePrivate;

	Q_OBJECT
	public:
		/**
		 * Returns a pointer to the factory.
		 * Use this function to get an instance of Factory.
		 *
		 * \return a pointer to the factory. If no factory exists until now then
		 * one is created
		 */
		static Factory* self();

		/**
		 * Create a new Ifaces::MediaObject.
		 *
		 * \return a pointer to the Ifaces::MediaObject the backend provides
		 */
		Ifaces::MediaObject* createMediaObject( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AvCapture.
		 *
		 * \return a pointer to the Ifaces::AvCapture the backend provides
		 */
		Ifaces::AvCapture* createAvCapture( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::ByteStream.
		 *
		 * \return a pointer to the Ifaces::ByteStream the backend provides
		 */
		Ifaces::ByteStream* createByteStream( QObject* parent = 0 );
		
		/**
		 * Create a new Ifaces::AudioPath.
		 *
		 * \return a pointer to the Ifaces::AudioPath the backend provides
		 */
		Ifaces::AudioPath* createAudioPath( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AudioEffect.
		 *
		 * \return a pointer to the Ifaces::AudioEffect the backend provides
		 */
		Ifaces::AudioEffect* createAudioEffect( int effectId, QObject* parent = 0 );
		/**
		 * Create a new Ifaces::VolumeFaderEffect.
		 *
		 * \return a pointer to the Ifaces::VolumeFaderEffect the backend provides
		 */
		Ifaces::VolumeFaderEffect* createVolumeFaderEffect( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AudioOutput.
		 *
		 * \return a pointer to the Ifaces::AudioOutput the backend provides
		 */
		Ifaces::AudioOutput* createAudioOutput( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AudioDataOutput.
		 *
		 * \return a pointer to the Ifaces::AudioDataOutput the backend provides
		 */
		Ifaces::AudioDataOutput* createAudioDataOutput( QObject* parent = 0 );

		/**
		 * Create a new Ifaces::VideoPath.
		 *
		 * \return a pointer to the Ifaces::VideoPath the backend provides
		 */
		Ifaces::VideoPath* createVideoPath( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::VideoEffect.
		 *
		 * \return a pointer to the Ifaces::VideoEffect the backend provides
		 */
		Ifaces::VideoEffect* createVideoEffect( int effectId, QObject* parent = 0 );

		/**
		 * \return a pointer to the backend interface.
		 */
		const Ifaces::Backend* backend() const;

		/**
		 * \copydoc Phonon::Ifaces::Backend::uiLibrary()
		 */
		const char* uiLibrary() const;
		/**
		 * \copydoc Phonon::Ifaces::Backend::uiSymbol()
		 */
		const char* uiSymbol() const;
#if 0
		/**
		 * Checks whether a certain mimetype is playable.
		 */
		bool isMimeTypePlayable( const QString & mimetype ) const;
#endif
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

		/**
		 * Emitted after the backend has successfully been changed.
		 */
		void backendChanged();

	protected:
		/**
		 * \internal
		 * Singleton constructor
		 */
		Factory();
		~Factory();

	protected:
		/**
		 * \internal
		 * Gets the QObject interface and calls registerQObject
		 */
		template<class T> T* registerObject( T* o );
	public:
		/**
		 * \internal
		 * registers the backend object
		 */
		void registerQObject( QObject* o );

	private Q_SLOTS:
		/**
		 * \internal
		 * unregisters the backend object
		 */
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
