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
#include "kioinputstream_impl.moc"

using namespace Arts;

KIOInputStream_impl::KIOInputStream_impl()
{
    m_finished = false;
}

KIOInputStream_impl::~KIOInputStream_impl()
{
}

void KIOInputStream_impl::streamStart()
{
    m_job = KIO::get(m_url, false, true);
    QObject::connect(m_job, SIGNAL(data(KIO::Job *, const QByteArray &)),
    	    this, SLOT(slotData(KIO::Job *, const QByteArray &)));		     
}

void KIOInputStream_impl::streamEnd()
{
    QObject::disconnect(m_job, SIGNAL(data(KIO::Job *, const QByteArray &)),
	       this, SLOT(slotData(KIO::Job *, const QByteArray &)));
    
    m_job->kill(); 
}

bool KIOInputStream_impl::openURL(const std::string& url)
{
    m_url = KURL(url.c_str());
    return true;
}

void KIOInputStream_impl::slotData(KIO::Job *, const QByteArray &data)
{
    DataPacket<mcopbyte> *packet = outdata.allocPacket(data.size());
    memcpy(packet->contents, data.data(), packet->size);
    m_sendqueue.push(packet);

    processQueue();
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
    // Repsecet PACKET_SIZE!
    for(unsigned int i = 0; i < m_sendqueue.size(); i++)
    {
	DataPacket<mcopbyte> *packet = m_sendqueue.front();
	m_sendqueue.pop();
	packet->send();
    }
}

REGISTER_IMPLEMENTATION(KIOInputStream_impl);
