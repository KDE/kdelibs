	/*

	Copyright (C) 2002 Nikolas Zimmermann <wildfox@kde.org>

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

#include "config.h"
#include "artskde.h"
#include "connect.h"
#include "flowsystem.h"
#include "audiosubsys.h"
#include "dynamicrequest.h"
#include "kdatarequest_impl.h"
#include "kioinputstream_impl.h"
#include "kaudioconverter.moc"

#include <iostream>

#include <qfile.h>
#include <qtimer.h>

#include <kurl.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kapplication.h>

using namespace std;

KAudioConverter::KAudioConverter()
{
	m_incoming = 0;
	m_started = false;
}

KAudioConverter::~KAudioConverter()
{
	delete m_incoming;
}

bool KAudioConverter::setup(int samplingRate)
{
	string backupAudioIO = Arts::AudioSubSystem::the()->audioIO();
	int backupSamplingRate = Arts::AudioSubSystem::the()->samplingRate();
	
	Arts::AudioSubSystem::the()->audioIO("null");
	Arts::AudioSubSystem::the()->samplingRate(samplingRate);
	
	if(!Arts::AudioSubSystem::the()->open())
	{
		Arts::AudioSubSystem::the()->audioIO(backupAudioIO);
		Arts::AudioSubSystem::the()->samplingRate(backupSamplingRate);
		
		return false;
	}

	return true;
}

void KAudioConverter::slotMimeType(const QString &mimeType)
{
	m_mimeType = mimeType;
	kapp->exit_loop();
}

void KAudioConverter::requestPlayObject(const KURL &url)
{	
	string queryInterface = "Arts::PlayObject";
	
	Arts::KIOInputStream inputStream;
	
	if(!url.isLocalFile())
	{
		Arts::KIOInputStream_impl *inputStreamImpl = new Arts::KIOInputStream_impl();
		inputStream = Arts::KIOInputStream::_from_base(inputStreamImpl);

		QObject::connect(inputStreamImpl, SIGNAL(mimeTypeFound(const QString &)), SLOT(slotMimeType(const QString &)));

		inputStream.openURL(url.url().latin1());
		inputStream.streamStart();

		// ugly hacks.. :/
		kapp->enter_loop();

		queryInterface = "Arts::StreamPlayObject";		
	}
	else
	{
		KMimeType::Ptr mimetype = KMimeType::findByURL(url);
		m_mimeType = mimetype->name();
	}
	
	Arts::TraderQuery query;
	query.supports("Interface", queryInterface);
	query.supports("MimeType", string(m_mimeType.latin1()));

	string objectType;

	vector<Arts::TraderOffer> *offers = query.query();
	if(!offers->empty())
		objectType = offers->front().interfaceName(); // first offer

	delete offers;

	if(objectType == "")
	{
		m_incoming = 0;
		return;
	}

	if(!url.isLocalFile())
	{
		Arts::StreamPlayObject result = Arts::SubClass(objectType);
		result.streamMedia(inputStream);
		result._node()->start();

		m_incoming = new KPlayObject(result, true);
	}
	else
	{
		Arts::PlayObject result = Arts::SubClass(objectType);

		if(result.loadMedia(string(QFile::encodeName(url.path()))))
		{
			result._node()->start();
			m_incoming = new KPlayObject(result, false);
		}
		else
			m_incoming = 0;
	}
}

void KAudioConverter::start()
{
	if(m_started || !m_incoming)
		return;

	m_started = true;
	
	emit rawStreamStart();

	m_incoming->play();

	Arts::KDataRequest_impl *requestImpl = new Arts::KDataRequest_impl();
	m_request = Arts::KDataRequest::_from_base(requestImpl);

	Arts::connect(m_incoming->object(), "left", m_request, "left");
	Arts::connect(m_incoming->object(), "right", m_request, "right");

	QObject::connect(requestImpl, SIGNAL(newBlockSize(long)), SIGNAL(newBlockSize(long)));
	QObject::connect(requestImpl, SIGNAL(newBlockPointer(long)), SIGNAL(newBlockPointer(long)));
	QObject::connect(requestImpl, SIGNAL(newData()), SIGNAL(newData()));

	// Special mpeglib case
	// TODO: needed at all??
	usleep(100000);
	if(m_incoming->object()._base()->_isCompatibleWith("DecoderBaseObject"))
		if(!Arts::DynamicRequest(m_incoming->object()).method("_set_blocking").param(true).invoke())
			cerr << "mpeglib, and blocking attribute can't be changed?" << endl;
	
	m_request.start();
		
	// TODO: Maybe do this async, using QTimer::singleShot
	// But jowenn i think jowenn is right -> this would
	// lead to problems in slotNewData() when accessing the data
	// (could already be overwritten...)
	while(m_incoming->state() != Arts::posIdle)
		m_request.goOn();
		
	stop();
}

void KAudioConverter::stop()
{
	if(!m_started || !m_incoming)
		return;

	m_incoming->halt();
	m_request.streamEnd();

	m_started = false;

	emit rawStreamFinished();
}
