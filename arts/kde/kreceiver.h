    /*

    Copyright (C) 2001 Matthias Kretz
                       kretz@kde.org

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifndef _KRECEIVER__H
#define _KRECEIVER__H

#include "soundserver.h"
#include "stdsynthmodule.h"

#include <qstring.h>

#include <queue>


class KReceiver : public Arts::ByteSoundReceiver_skel,
                  public Arts::StdSynthModule
{
	private:
		int _samplingRate, _bits, _channels;
		std::string _title;

		Arts::ByteSoundReceiver m_bswrapper;
		Arts::SimpleSoundServer m_server;
		bool m_attached, m_blocking;
		int m_pos;
		queue<Arts::DataPacket<Arts::mcopbyte>* > inqueue;

	public:
		KReceiver( Arts::SimpleSoundServer, int rate, int bits, int channels, QString title = "record_stream" );
		~KReceiver();

		/**
		 * If you set blockingIO to true the read method will wait
		 * until it has enough data to return exactly what you asked for. If
		 * blockingIO is false you can count on having control back to your
		 * program soon enough
		 */
		void setBlockingIO( bool );
		bool blockingIO() const;

		/**
		 * set the name of the Receiver (the name that appears in the AudioManager of ArtsControl)
		 */
		void setName( const QString & );
		//QString name() const;

		/**
		 * You can read @p size number of bytes. If @ref blockingIO is set the method
		 * will return as soon as it has all your data and could probably block your
		 * program. The method returns the number of bytes that are actually in the
		 * buffer.
		 */
		int read( char *, int size );

		/**
		 * detaches the stream from the soundserver
		 */
		void pause();

		/**
		 * detaches the stream from the soundserver and flushes the inputbuffer
		 */
		void end();

		/**
		 * Attach to the soundserver and start getting data to read. This method
		 * must be called as soon as you want to receive data. In any case you have
		 * to call attach() before @ref read()
		 */
		void attach();

		long samplingRate() { return _samplingRate; }
		long channels()     { return _channels; }
		long bits()         { return _bits; }
		std::string title() { return _title; }

	protected:
		void process_indata(Arts::DataPacket<Arts::mcopbyte> *);
};

#endif //_KRECEIVER__H
