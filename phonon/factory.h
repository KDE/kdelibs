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

#ifndef Phonon_FACTORY_H
#define Phonon_FACTORY_H

#include <QObject>

#include <kstaticdeleter.h>
#include <kdelibs_export.h>

namespace Phonon
{
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
class PHONONCORE_EXPORT Factory : public QObject
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
		QObject* createMediaObject( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::MediaQueue.
		 *
		 * \return a pointer to the Ifaces::MediaQueue the backend provides
		 */
		QObject* createMediaQueue( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AvCapture.
		 *
		 * \return a pointer to the Ifaces::AvCapture the backend provides
		 */
		QObject* createAvCapture( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::ByteStream.
		 *
		 * \return a pointer to the Ifaces::ByteStream the backend provides
		 */
		QObject* createByteStream( QObject* parent = 0 );

		/**
		 * Create a new Ifaces::AudioPath.
		 *
		 * \return a pointer to the Ifaces::AudioPath the backend provides
		 */
		QObject* createAudioPath( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AudioEffect.
		 *
		 * \return a pointer to the Ifaces::AudioEffect the backend provides
		 */
		QObject* createAudioEffect( int effectId, QObject* parent = 0 );
		/**
		 * Create a new Ifaces::VolumeFaderEffect.
		 *
		 * \return a pointer to the Ifaces::VolumeFaderEffect the backend provides
		 */
		QObject* createVolumeFaderEffect( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AudioOutput.
		 *
		 * \return a pointer to the Ifaces::AudioOutput the backend provides
		 */
		QObject* createAudioOutput( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::AudioDataOutput.
		 *
		 * \return a pointer to the Ifaces::AudioDataOutput the backend provides
		 */
		QObject* createAudioDataOutput( QObject* parent = 0 );

		/**
		 * Create a new Ifaces::Visualization.
		 *
		 * \return a pointer to the Ifaces::Visualization the backend provides
		 */
		QObject* createVisualization( QObject* parent = 0 );

		/**
		 * Create a new Ifaces::VideoPath.
		 *
		 * \return a pointer to the Ifaces::VideoPath the backend provides
		 */
		QObject* createVideoPath( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::VideoEffect.
		 *
		 * \return a pointer to the Ifaces::VideoEffect the backend provides
		 */
		QObject* createVideoEffect( int effectId, QObject* parent = 0 );
		QObject* createBrightnessControl( QObject* parent = 0 );
		/**
		 * Create a new Ifaces::VideoDataOutput.
		 *
		 * \return a pointer to the Ifaces::VideoDataOutput the backend provides
		 */
		QObject* createVideoDataOutput( QObject* parent = 0 );

		/**
		 * \return a pointer to the backend interface.
		 */
		QObject* backend( bool createWhenNull = true );

		/**
		 * \copydoc Phonon::Ifaces::Backend::uiLibrary()
		 */
		const char* uiLibrary();
		/**
		 * \copydoc Phonon::Ifaces::Backend::uiSymbol()
		 */
		const char* uiSymbol();

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

	public:
		/**
		 * \internal
		 * registers the backend object
		 */
		QObject* registerQObject( QObject* o );

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

	private Q_SLOTS:
		/**
		 * \internal
		 * This is called via DCOP when the user changes the Phonon Backend.
		 */
		void phononBackendChanged();

//X		It is probably better if we can get away with internal handling of
//X		freeing the soundcard device when it's not needed anymore and
//X		providing an IPC method to stop all MediaProducers -> free all
//X		devices
//X		/**
//X		 * \internal
//X		 * This is called when the application needs to free the soundcard
//X		 * device(s).
//X		 */
//X		void freeSoundcardDevices();
};
} // namespace Phonon

#endif // BACKENDFACTORY_H
// vim: sw=4 ts=4 tw=80 noet
