/*  This file is part of the KDE project
    Copyright (C) 2003 Arnold Krille <arnold@arnoldarts.de>

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

#ifndef KAUDIOPLAYSTREAM_H
#define KAUDIOPLAYSTREAM_H

#include <qobject.h>

#include <qcstring.h>
#include <stdsynthmodule.h>

#include <kdelibs_export.h>

class KArtsServer;
namespace Arts { class StereoEffectStack; }

class KAudioPlayStreamPrivate;

/**
 * @brief A wrapper around ByteSoundProducer/ByteStreamToAudio/Synth_AMAN_PLAY.
 *
 * @author Arnold Krille <arnold@arnoldarts.de>
 * @since 3.2
*/

class KDE_ARTS_EXPORT KAudioPlayStream : public QObject {
   Q_OBJECT
public:
	/**
	 * Creates a KAudioPlayStream on server with a title. You should pass the KArtsServer also
	 * as parent to be sure this object is deleted before the server is.
	 *
	 * @param server The server where it should play to.
	 * @param title The title that is shown in the AudioManager.
	 * @param parent You will propably want to pass the server as parent to so this stream gets deleted before the server disappears.
	 * @param name The name of the stream.
	*/
	KAudioPlayStream( KArtsServer* server, const QString title, QObject* parent=0, const char* name=0 );
	/**
	 * Destructs the KAudioPlayStream.
	*/
	~KAudioPlayStream();

	/**
	 * Controls wether this Stream should poll the data from you via the signal requestData()
	 * or you use write() to fill the inputbuffer.
	 *
	 * Default is true
	*/
	void setPolling( bool );
	/**
	 * Returns the polling state.
	 * @see setPolling
	*/
	bool polling() const;

	/**
	 * @return wether this stream is running ("on air") or not.
	*/
	bool running() const;

	/**
	 * @return The Arts::StereoEffectStack right before the Synth_AMAN_PLAY.
	*/
	Arts::StereoEffectStack effectStack() const;
public slots:
	/**
	 * Start the stream.
	 * @param samplingRate how many samples per second ( typically 11000/22050/44100/48000 )
	 * @param bits how many bits per sample ( 8 / 16 )
	 * @param channels how many channels ( 1 or 2 )
	*/
	void start( int samplingRate, int bits, int channels );
	/**
	 * Stops the stream.
	*/
	void stop();

	/**
	 * Write data into the inputbuffer.
	 * If you ignore the signal noData() it will play 0 ( silence ).
	*/
	void write( QByteArray& data );
signals:
	/**
	 * This signal is emitted when audio should be played.
	 * You have to fill the array with data.
	*/
	void requestData( QByteArray& );

	/**
	 * Is emitted when the state changes.
	*/
	void running( bool );

	/**
	 * Is emitted if the inputbuffer runs dry and polling os off.
	*/
	void noData();
public:
	/**
	 * @internal
	*/
	void fillData( Arts::DataPacket<Arts::mcopbyte> *packet );
private:
	KAudioPlayStreamPrivate* d;
};

#endif // KAUDIOPLAYSTREAM_H

// vim: sw=4 ts=4 tw=80
