	/*

	Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

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

#include <kapp.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/kmimetype.h>
#include <kio/jobclasses.h>
#include <qtimer.h>
#include <qdatastream.h>
#include "artsversion.h"
#include "kioinputstream_impl.moc"

using namespace Arts;

const unsigned int KIOInputStream_impl::PACKET_COUNT = 2;
const unsigned int KIOInputStream_impl::PACKET_BUFFER = 10;
const unsigned int KIOInputStream_impl::PACKET_SIZE = 1024;

KIOInputStream_impl::KIOInputStream_impl()
{
	m_job = 0;
	m_data = 0;
	m_finished = false;
}

KIOInputStream_impl::~KIOInputStream_impl()
{
	if(m_job != 0)
	    m_job->kill();
}

void KIOInputStream_impl::streamStart()
{
	outdata.setPull(PACKET_COUNT, PACKET_SIZE);

	if(m_job != 0)
		m_job->kill();
	m_job = KIO::get(m_url, false, false);
	m_job->addMetaData("accept", "audio/x-mp3");
	m_job->addMetaData("UserAgent", QString::fromLatin1("aRts/") + QString::fromLatin1(ARTS_VERSION));
	
	QObject::connect(m_job, SIGNAL(data(KIO::Job *, const QByteArray &)),
			 this, SLOT(slotData(KIO::Job *, const QByteArray &)));		     
	QObject::connect(m_job, SIGNAL(result(KIO::Job *)),
			 this, SLOT(slotResult(KIO::Job *)));		     
}

void KIOInputStream_impl::streamEnd()
{
	if(m_job == 0)
		return;
	QObject::disconnect(m_job, SIGNAL(data(KIO::Job *, const QByteArray &)),
	                    this, SLOT(slotData(KIO::Job *, const QByteArray &)));
	QObject::disconnect(m_job, SIGNAL(result(KIO::Job *)),
			    this, SLOT(slotResult(KIO::Job *)));		     
	
	m_job->kill();
	m_job = 0;

	outdata.endPull();

	while(!m_sendqueue.empty())
	{
	    DataPacket<mcopbyte> *packet = m_sendqueue.front();
	    packet->size = 0;
	    packet->send();
	    m_sendqueue.pop();
	}
}

bool KIOInputStream_impl::openURL(const std::string& url)
{
	m_url = KURL(url.c_str());
	return true;
}

void KIOInputStream_impl::slotData(KIO::Job *, const QByteArray &data)
{
	if(m_finished)
	    m_finished = false;

	QDataStream dataStream(m_data, IO_WriteOnly | IO_Append);
	dataStream << data;
	
	if(!m_sendqueue.empty())
	    processQueue();
}

void KIOInputStream_impl::slotResult(KIO::Job *job)
{
	m_finished = true;
	if(job->error())
	    job->showErrorDialog();
}

bool KIOInputStream_impl::eof()
{
	return (m_finished && m_data.size() == 0 && m_sendqueue.empty());
}

bool KIOInputStream_impl::seekOk()
{
	return false;
}

long KIOInputStream_impl::size()
{
	return m_data.size();
}

long KIOInputStream_impl::seek(long)
{
	return -1;
}

void KIOInputStream_impl::processQueue()
{
	if(m_data.size() > ((m_sendqueue.size() + PACKET_BUFFER) * PACKET_SIZE) && !m_job->isSuspended())
	    m_job->suspend();
	else if(m_data.size() < ((m_sendqueue.size() + PACKET_BUFFER) * PACKET_SIZE) && m_job->isSuspended())
	    m_job->resume();

	for(unsigned int i = 0; i < m_sendqueue.size(); i++)
	{
	    DataPacket<mcopbyte> *packet = m_sendqueue.front();
		
	    packet->size = std::min(PACKET_SIZE, m_data.size());
	    if(packet->size == 0)
		return;	    
	    m_sendqueue.pop();
	    memcpy(packet->contents, m_data.data(), packet->size);
	    memmove(m_data.data(), m_data.data() + packet->size, m_data.size() - packet->size);
	    m_data.resize(m_data.size() - packet->size);
	    packet->send();
	}
}

void KIOInputStream_impl::request_outdata(DataPacket<mcopbyte> *packet)
{
	m_sendqueue.push(packet);
	processQueue();
}

REGISTER_IMPLEMENTATION(KIOInputStream_impl);
