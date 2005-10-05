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

#include "kaudioplaystream.h"
#include "kaudioplaystream_p.h"

#include <kartsserver.h>
#include <kaudiomanagerplay.h>

#include <artsflow.h>
#include <soundserver.h>

#include <kglobal.h>
#include <kdebug.h>

#include <qstring.h>
//#include <qptrqueue.h>
//#include <qcstring.h> //QByteArray

#include <string.h> // for strncpy

//#include <assert.h>

KAudioPlayStreamPrivate::KAudioPlayStreamPrivate( KArtsServer* server, const QString title, QObject* p, const char* n )
 : QObject( p,n )
 , _server( server )
 , _play( new KAudioManagerPlay( _server, title ) )
 , _effectrack( Arts::StereoEffectStack::null() )
 , _polling( true ), _attached( false ), _effects( true )
{
kdDebug( 400 ) << k_funcinfo << endl;
	initaRts();
}

KAudioPlayStreamPrivate::~KAudioPlayStreamPrivate()
{
	kdDebug( 400 ) << k_funcinfo << endl;
	_play->stop();
	if ( _effects ) _effectrack.stop();
	_bs2a.stop();
}

void KAudioPlayStreamPrivate::initaRts() {
	kdDebug( 400 ) << k_funcinfo << endl;

	_effectrack = Arts::DynamicCast( _server->server().createObject( "Arts::StereoEffectStack" ) );
	if ( _effectrack.isNull() )
	{
		kdWarning( 400 ) << "Couldn't create EffectStack!" << endl;
		_effects = false;
	}

	_bs2a = Arts::DynamicCast( _server->server().createObject( "Arts::ByteStreamToAudio" ) );
	if ( _bs2a.isNull() )
		kdFatal( 400 ) << "Couldn't create ByteStreamToAudio" << endl;

	if ( _effects )
	{
		Arts::connect( _effectrack, _play->amanPlay() );
		Arts::connect( _bs2a, _effectrack );
	} else {
		Arts::connect( _bs2a, _play->amanPlay() );
	}

	_play->start();
	if ( _effects ) _effectrack.start();
}

KAudioPlayStream::KAudioPlayStream( KArtsServer* server, const QString title, QObject* p, const char* n )
 : QObject( p,n )
 , d( new KAudioPlayStreamPrivate( server, title, this ) )
{
	kdDebug( 400 ) << k_funcinfo << endl;
}
KAudioPlayStream::~KAudioPlayStream()
{
	kdDebug( 400 ) << k_funcinfo << endl;
}

void KAudioPlayStream::setPolling( bool n ) { d->_polling = n; }
bool KAudioPlayStream::polling() const { return d->_polling; }

bool KAudioPlayStream::running() const { return d->_attached; }

Arts::StereoEffectStack KAudioPlayStream::effectStack() const {
	return d->_effectrack;
}

void KAudioPlayStream::start( int samplingRate, int bits, int channels )
{
	kdDebug( 400 ) << k_funcinfo << "samplingRate: " << samplingRate << " bits: " << bits << " channels: " << channels << endl;
	if ( !d->_attached )
	{
		d->_bs2a.samplingRate( samplingRate );
		d->_bs2a.channels( channels );
		d->_bs2a.bits( bits );

		d->_sender = new KByteSoundProducer( this, d->_server->server().minStreamBufferTime(), samplingRate, bits, channels, "PS" );
		d->_artssender = Arts::ByteSoundProducerV2::_from_base( d->_sender );
		Arts::connect( d->_artssender, "outdata", d->_bs2a, "indata" );

		d->_bs2a.start();
		d->_artssender.start();

//		// Needed?
		Arts::Dispatcher::the()->ioManager()->processOneEvent( false );

		d->_attached = true;
		emit running( d->_attached );
	}
}
void KAudioPlayStream::stop()
{
	kdDebug( 400 ) << k_funcinfo << endl;
	if ( d->_attached )
	{
		d->_attached = false;

		d->_bs2a.stop();
		d->_artssender.stop();

		// Shortly stop the play so we dont get clicks and artefacts
		d->_play->stop();
		d->_play->start();

		Arts::disconnect( d->_artssender, d->_bs2a );
		d->_artssender = Arts::ByteSoundProducerV2::null();
		d->_sender = 0;

		emit running( d->_attached );
	}
}

void KAudioPlayStream::write( QByteArray& )
{
}

void KAudioPlayStream::fillData( Arts::DataPacket<Arts::mcopbyte> *packet )
{
	//kdDebug( 400 ) << k_funcinfo << "packet->size=" << packet->size << endl;
	if ( d->_polling )
	{
		QByteArray bytearray( packet->size );
		bytearray.setRawData( ( char* )packet->contents, packet->size );
		bytearray.fill( 0 );
		emit requestData( bytearray );
		bytearray.resetRawData( ( char* )packet->contents, packet->size );

		//for ( int i=0; i<10; i++ )
		//	kdDebug() << packet->contents[ i ] << " : " << bytearray.data()[ i ] << endl;
	} else {
		/// TODO: Implement a queue and fetching from it...
	}
}

// * * * KByteSoundProducer * * *

KByteSoundProducer::KByteSoundProducer( KAudioPlayStream* impl, float minBufferTime, int rate, int bits, int channels, const char * title )
 : _samplingRate( rate )
 , _channels( channels )
 , _bits( bits )
 , _packets( 7 )
 , _title( title )
 , _impl( impl )
{
	// Calculate packet count (packetsize is fixed to packetCapacity = 4096
	float streamBufferTime;
	do {
		_packets++;
		streamBufferTime = ( float )( _packets * packetCapacity * 1000 )
			/ ( float )( _samplingRate * _channels * 2 );
	} while ( streamBufferTime < minBufferTime );
	//kdDebug( 400 ) << k_funcinfo << "_packets:" << _packets << " packetCapacity:" << packetCapacity << endl;
}

KByteSoundProducer::~KByteSoundProducer()
{
}

void KByteSoundProducer::streamStart() { outdata.setPull( _packets, packetCapacity ); }
void KByteSoundProducer::streamEnd() { outdata.endPull(); }

void KByteSoundProducer::request_outdata( Arts::DataPacket<Arts::mcopbyte> *packet )
{
	if ( _impl->running() ) {
		_impl->fillData( packet );
		packet->send();
	}
}

// vim: sw=4 ts=4 tw=80

#include "kaudioplaystream.moc"
#include "kaudioplaystream_p.moc"
