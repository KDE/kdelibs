    /*

    Copyright (C) 2002 Matthias Kretz
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

#ifndef _KAUDIORECORDSTREAM_P__H
#define _KAUDIORECORDSTREAM_P__H

#include <soundserver.h>
#include <stdsynthmodule.h>

#include <qobject.h>

class KByteSoundReceiver : public QObject,
                           public Arts::ByteSoundReceiver_skel,
                           public Arts::StdSynthModule
{
	Q_OBJECT

	public:
		KByteSoundReceiver( int rate, int bits, int channels, const char * title );
		~KByteSoundReceiver();

		long samplingRate() { return _samplingRate; }
		long channels()     { return _channels; }
		long bits()         { return _bits; }
		std::string title() { return _title; }

	signals:
		/**
		 * new data arrived, the data pointer will be deleted
		 * after this signal was emitted. So if you want to keep it
		 * you have to copy it.
		 */
		void data( const char * data, unsigned int size );

	protected:
		void process_indata(Arts::DataPacket<Arts::mcopbyte> *);

	private:
		int _samplingRate, _bits, _channels;
		std::string _title;
};

#endif // _KAUDIORECORDSTREAM_P__H
