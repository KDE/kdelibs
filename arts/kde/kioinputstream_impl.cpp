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

#include <kio/job.h>
#include <kdebug.h>
#include <qtimer.h>
#include <kio/kmimetype.h>
#include <kapp.h>
#include "kioinputstream_impl.moc"
#include <stdlib.h>

using namespace Arts;

KIOInputStream_impl::KIOInputStream_impl()
{
	m_job = 0;
	m_finished = false;
	QTimer::singleShot(0, this, SLOT(slotTest()));
}

KIOInputStream_impl::~KIOInputStream_impl()
{
}

void KIOInputStream_impl::slotTest()
{
    kdDebug() << "SLOT WORKING!" << endl;
}

void KIOInputStream_impl::streamStart()
{
	if(m_job != 0)
		m_job->kill();
	m_job = KIO::get(m_url, false, true);
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
}

bool KIOInputStream_impl::openURL(const std::string& url)
{
	m_url = KURL(url.c_str());
	return true;
}

void KIOInputStream_impl::slotData(KIO::Job *, const QByteArray &data)
{
	kdDebug() << "DATA!" << endl;
	DataPacket<mcopbyte> *packet = outdata.allocPacket(data.size());
	kdDebug() << "1" << endl;
	memcpy(packet->contents, data.data(), packet->size);
	kdDebug() << "2" << endl;
	m_sendqueue.push(packet);
	kdDebug() << "3" << endl;
	processQueue();
}

void KIOInputStream_impl::slotResult(KIO::Job *)
{
	kdDebug() << "RESULT!" << endl;
	m_job = 0;
}

bool KIOInputStream_impl::eof()
{
	return m_finished;
}

bool KIOInputStream_impl::seekOk()
{
	return false;
}

long KIOInputStream_impl::size()
{
	return -1;
}

long KIOInputStream_impl::seek(long)
{
	return -1;
}

void KIOInputStream_impl::processQueue()
{
	// TODO:
	// Respect PACKET_SIZE!
	kdDebug() << "p1" << endl;
	for(unsigned int i = 0; i < m_sendqueue.size(); i++)
	{
		kdDebug() << "L1" << endl;
		DataPacket<mcopbyte> *packet = m_sendqueue.front();
		kdDebug() << "L2" << endl;
		packet->send();
		kdDebug() << "L3" << endl;
		m_sendqueue.pop();
		kdDebug() << "L4" << endl;
	}
	kdDebug() << "p2" << endl;	
}

REGISTER_IMPLEMENTATION(KIOInputStream_impl);
