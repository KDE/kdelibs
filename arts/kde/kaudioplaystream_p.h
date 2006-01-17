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

#ifndef KAUDIOPLAYSTREAM_P_H
#define KAUDIOPLAYSTREAM_P_H

#include <soundserver.h>
#include <stdsynthmodule.h>

#include <qobject.h>

class KArtsServer;
class KAudioManagerPlay;
namespace Arts {
	class Synth_AMAN_PLAY;
	class StereoEffectStack;
	class ByteStreamToAudio;
}

class KAudioPlayStream;
class KByteSoundProducer;

class KAudioPlayStreamPrivate : public QObject {
   Q_OBJECT
public:
	KAudioPlayStreamPrivate( KArtsServer*, const QString title, QObject*, const char* =0 );
	~KAudioPlayStreamPrivate();

	KArtsServer* _server;
	KAudioManagerPlay* _play;
	Arts::StereoEffectStack _effectrack;
	Arts::ByteStreamToAudio _bs2a;
	KByteSoundProducer* _sender;
	Arts::ByteSoundProducerV2 _artssender;
	bool _polling, _attached, _effects;

public Q_SLOTS:
	void initaRts();
};

class KByteSoundProducer : virtual public Arts::ByteSoundProducerV2_skel
                         , virtual public Arts::StdSynthModule
{
public:
	KByteSoundProducer( KAudioPlayStream*, float minBufferTime, int rate, int bits, int channels, const char * title );
	~KByteSoundProducer();

	long samplingRate() { return _samplingRate; }
	long channels() { return _channels; }
	long bits() { return _bits; }
	std::string title() { return _title; }

	void streamStart();
	void streamEnd();

protected:
	void request_outdata( Arts::DataPacket<Arts::mcopbyte> *packet );

private:
	long _samplingRate, _channels, _bits, _packets;
	std::string _title;
	enum { packetCapacity = 4096 };
	KAudioPlayStream* _impl;
};

#endif // KAUDIOPLAYSTREAM_P_H

// vim: sw=4 ts=4
