    /*

    Copyright (C) 2001, 2002 Matthias Kretz
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

#include "kaudiorecordstream.h"
#include "kaudiorecordstream_p.h"
#include "kartsserver.h"

#include <kglobal.h>
#include <kdebug.h>

#include <qstring.h>
#include <qptrqueue.h>
#include <qcstring.h> //QByteArray

#include <assert.h>

struct KAudioRecordStream::Data
{
	Arts::ByteSoundReceiver receiver;
	KByteSoundReceiver * receiver_base;
	KArtsServer * kserver;
	bool attached;
	bool blocking;
	bool polling;
	unsigned int pos;
	QPtrQueue<QByteArray> inqueue;
};

KAudioRecordStream::KAudioRecordStream( KArtsServer * kserver, int samplingrate, int bits, int channels, QString title, QObject * parent, const char * name )
	: QObject( parent, name )
	, d( new Data )
{
	d->receiver_base = new KByteSoundReceiver( samplingrate, bits, channels, title.local8Bit() );
	d->receiver = Arts::ByteSoundReceiver::_from_base( d->receiver_base );
	d->kserver = kserver;
	d->attached = false;
	d->blocking = true;
	d->polling = false;
	d->pos = 0;
	d->inqueue.setAutoDelete( true );

	connect( d->kserver, SIGNAL( restartedServer() ), SLOT( slotRestartedServer() ) );
	connect( d->receiver_base, SIGNAL( data( const char *, unsigned int ) ),
			SLOT( slotData( const char *, unsigned int ) ) );
}

KAudioRecordStream::~KAudioRecordStream()
{
	d->receiver = Arts::ByteSoundReceiver::null();
	// don't delete receiver_base because aRts takes care of that (in the line
	// above)
	delete d;
}

int KAudioRecordStream::read( char * buffer, int size )
{
	kdDebug( 400 ) << k_funcinfo << endl;
	unsigned int remaining = size;
	while( remaining )
	{
		if( d->blocking )
			while( d->inqueue.isEmpty() )
				Arts::Dispatcher::the()->ioManager()->processOneEvent( true );
		else
		{
			if( d->inqueue.isEmpty() )
				Arts::Dispatcher::the()->ioManager()->processOneEvent( false );
			if( d->inqueue.isEmpty() )
				return size - remaining;
		}
		QByteArray * data = d->inqueue.head();
		unsigned int tocopy = kMin( remaining, data->size() - d->pos );
		memcpy( buffer, data->data() + d->pos, tocopy );
		d->pos += tocopy;
		buffer += tocopy;
		remaining -= tocopy;
		if( d->pos == data->size() )
		{
			d->inqueue.remove();
			d->pos = 0;
		}
	}
	return size;
}

void KAudioRecordStream::setBlockingIO( bool blocking )
{
	d->blocking = blocking;
}

bool KAudioRecordStream::blockingIO() const
{
	return d->blocking;
}

void KAudioRecordStream::usePolling( bool polling )
{
	d->polling = polling;
	if( ! polling )
		flush();
}

bool KAudioRecordStream::polling() const
{
	return d->polling;
}

void KAudioRecordStream::stop()
{
	kdDebug( 400 ) << k_funcinfo << endl;
	if( d->attached )
	{
		d->kserver->server().detachRecorder( d->receiver );
		d->receiver.stop();
		d->attached = false;
	}
}

void KAudioRecordStream::start()
{
	kdDebug( 400 ) << k_funcinfo << endl;
	if( ! d->attached )
	{
		assert( d->kserver );
		d->kserver->server().attachRecorder( d->receiver );
		d->receiver.start();
		//### needed?
		//Arts::Dispatcher::the()->ioManager()->processOneEvent( false );
		d->attached = true;
	}
}

void KAudioRecordStream::flush()
{
	kdDebug( 400 ) << k_funcinfo << endl;
	d->inqueue.clear();
}

void KAudioRecordStream::slotRestartedServer() { }

void KAudioRecordStream::slotData( const char * contents, unsigned int size )
{
	kdDebug( 400 ) << k_funcinfo << endl;
	QByteArray * bytearray = new QByteArray( size );
	// copy the contents to the bytearray
	// this has to be deleted later
	bytearray->duplicate( contents, size );
	if( d->polling )
	{
		kdDebug( 400 ) << "enqueue the data\n";
		d->inqueue.enqueue( bytearray );
	}
	else
	{
		kdDebug( 400 ) << "emit the data\n";
		emit data( *bytearray );
		kdDebug( 400 ) << "delete the data\n";
		delete bytearray;
	}
}

////////////////////////////////////////
// ---*--- KByteSoundReceiver ---*--- //
////////////////////////////////////////

KByteSoundReceiver::KByteSoundReceiver( int rate, int bits, int channels, const char * title )
	: _samplingRate( rate )
	, _bits( bits )
	, _channels( channels )
	, _title( title )
{
}

KByteSoundReceiver::~KByteSoundReceiver()
{
}

void KByteSoundReceiver::process_indata( Arts::DataPacket<Arts::mcopbyte> * inpacket )
{
	kdDebug( 400 ) << k_funcinfo << " size of the packet: " << inpacket->size << endl;
	emit data( (char *)inpacket->contents, inpacket->size );
	inpacket->processed();
}

#include "kaudiorecordstream.moc"
#include "kaudiorecordstream_p.moc"
