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
#include "kioinputstream_impl.moc"

using namespace Arts;

const unsigned int KIOInputStream_impl::PACKET_COUNT = 8;
const unsigned int KIOInputStream_impl::PACKET_SIZE = 8192;

KIOInputStream_impl::KIOInputStream_impl()
{
	m_job = 0;
	m_data = 0;

	m_position = 0;
	m_size = 0;
	
	m_finished = false;
}

KIOInputStream_impl::~KIOInputStream_impl()
{
}

void KIOInputStream_impl::streamStart()
{
	outdata.setPull(PACKET_COUNT, PACKET_SIZE);

	if(m_job != 0)
		m_job->kill();
	m_job = KIO::get(m_url, false, false);
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
	
	m_size += data.size();

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
	return (m_finished || m_position >= m_size) && (m_sendqueue.size() == PACKET_COUNT);
}

bool KIOInputStream_impl::seekOk()
{
	return false;
}

long KIOInputStream_impl::size()
{
	return m_size;
}

long KIOInputStream_impl::seek(long)
{
	return -1;
}

void KIOInputStream_impl::processQueue()
{
	// Last part of flow control
	// CRASHES! :(
	/*
	if(m_sendqueue.size() > 10)
	    m_job->suspend();
	else if(m_sendqueue.size() < 10)
	{
	    if(m_job->isSuspended())
		m_job->resume();
	}
	*/
	for(unsigned int i = 0; i < m_sendqueue.size(); i++)
	{
	    if(m_position < m_size)
	    {
		DataPacket<mcopbyte> *packet = m_sendqueue.front();
		m_sendqueue.pop();
		
		packet->size = std::min(PACKET_SIZE, m_size - m_position);
		memcpy(packet->contents, m_data.data() + m_position, packet->size);
		m_position += packet->size;
		packet->send();
	    }
	}
}

void KIOInputStream_impl::request_outdata(DataPacket<mcopbyte> *packet)
{
	m_sendqueue.push(packet);
	processQueue();
}

REGISTER_IMPLEMENTATION(KIOInputStream_impl);
