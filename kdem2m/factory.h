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

#ifndef Kdem2m_FACTORY_H
#define Kdem2m_FACTORY_H

#include <QObject>

#include <dcopobject.h>
#include <kstaticdeleter.h>

class KURL;

/**
 * \brief The KDE Multimedia classes
 *
 * In this Namespace you find the classes to access Multimedia functions for
 * audio and video playback. Those classes are not dependent
 * on any specific framework (like they were in pre KDE4 times) but rather use
 * exchangeable backends to do the "dirty" work.
 *
 * If you want to write a new backend you also find the necessary classes in
 * Kdem2m::Ifaces.
 *
 * \since 4.0
 */
namespace Kdem2m
{
	namespace Ifaces
	{
		class MediaObject;
		class SoundcardCapture;
		class ByteStream;

		class AudioEffect;
		class AudioPath;
		class AudioOutputBase;
		class AudioOutput;
		class AudioDataOutput;
		class AudioFftOutput;

		class VideoEffect;
		class VideoPath;
		class VideoOutputBase;

		class Backend;
	}

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
 * \since 4.0
 * \internal
 */
class Factory : public QObject, public DCOPObject
{
	friend void ::KStaticDeleter<Factory>::destructObject();

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
		Ifaces::SoundcardCapture* createSoundcardCapture( QObject* parent = 0 );
		Ifaces::ByteStream* createByteStream( QObject* parent = 0 );
		
		Ifaces::AudioPath* createAudioPath( QObject* parent = 0 );
		Ifaces::AudioEffect* createAudioEffect( QObject* parent = 0 );
		Ifaces::AudioOutput* createAudioOutput( QObject* parent = 0 );
		Ifaces::AudioDataOutput* createAudioDataOutput( QObject* parent = 0 );
		Ifaces::AudioFftOutput* createAudioFftOutput( QObject* parent = 0 );

		Ifaces::VideoPath* createVideoPath( QObject* parent = 0 );
		Ifaces::VideoEffect* createVideoEffect( QObject* parent = 0 );
		Ifaces::VideoOutputBase* createVideoOutputBase( QObject* parent = 0 );

		const Ifaces::Backend* Factory::backend() const;
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

	signals:
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

	private slots:
		void objectDestroyed( QObject * );

	private:
		static Factory * m_self;
		class Private;
		Private * d;

	K_DCOP
	k_dcop:
		/**
		 * \internal
		 * This is called via DCOP when the user changes the Kdem2m Backend.
		 */
		void kdem2mBackendChanged();
};
} // namespace Kdem2m

#endif // BACKENDFACTORY_H
// vim: sw=4 ts=4 tw=80 noet
