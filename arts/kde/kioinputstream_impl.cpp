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

/*
 * How does it work?
 * -----------------
 *
 * First the buffer has to be filled. When it reaches a defined size the outdata
 * stream has to start pulling packets. If the buffer reaches a size of zero the
 * stream has to stop. If the buffer gets to big the job has to be suspended
 * until the buffer is small enough again.
 */

#include <kapplication.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/kmimetype.h>
#include <kio/jobclasses.h>
#include <qtimer.h>
#include <qdatastream.h>
#include "artsversion.h"
#include "kioinputstream_impl.moc"

using namespace Arts;

const unsigned int KIOInputStream_impl::PACKET_COUNT = 5;

KIOInputStream_impl::KIOInputStream_impl() : m_packetSize(1024)
{
	m_job = 0;
	m_data = 0;
	m_finished = false;
	m_firstBuffer = false;
	m_packetBuffer = 16;
	m_streamStarted = false;
	m_streamSuspended = false;
}

KIOInputStream_impl::~KIOInputStream_impl()
{
	if(m_job != 0)
	    m_job->kill();
}

void KIOInputStream_impl::streamStart()
{
	// prevent kill/reconnect
	if (m_streamStarted) {
		kdDebug() << "not restarting stream!\n";
		if (m_job->isSuspended())
			m_job->resume();
		return;
	}

	kdDebug() << "(re)starting stream\n";

	if(m_job != 0)
		m_job->kill();
	m_job = KIO::get(m_url, false, false);
	m_job->addMetaData("accept", "audio/x-mp3, video/mpeg, application/x-ogg");
	m_job->addMetaData("UserAgent", QString::fromLatin1("aRts/") + QString::fromLatin1(ARTS_VERSION));

	QObject::connect(m_job, SIGNAL(data(KIO::Job *, const QByteArray &)),
			 this, SLOT(slotData(KIO::Job *, const QByteArray &)));		     
	QObject::connect(m_job, SIGNAL(result(KIO::Job *)),
			 this, SLOT(slotResult(KIO::Job *)));		     
	QObject::connect(m_job, SIGNAL(mimetype(KIO::Job *, const QString &)),
			 this, SLOT(slotScanMimeType(KIO::Job *, const QString &)));

	m_streamStarted = true;
}

void KIOInputStream_impl::streamEnd()
{
	kdDebug() << "streamEnd()\n";

	if(m_job != 0)
	{
		QObject::disconnect(m_job, SIGNAL(data(KIO::Job *, const QByteArray &)),
	    				this, SLOT(slotData(KIO::Job *, const QByteArray &)));
		QObject::disconnect(m_job, SIGNAL(result(KIO::Job *)),
						this, SLOT(slotResult(KIO::Job *)));		     
		QObject::disconnect(m_job, SIGNAL(mimetype(KIO::Job *, const QString &)),
				 this, SLOT(slotScanMimeType(KIO::Job *, const QString &)));

		m_job->kill();
		m_job = 0;
	}	

	outdata.endPull();
	m_streamStarted = false;
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
	dataStream.writeRawBytes(data.data(), data.size());
	kdDebug() << "STREAMING: buffersize = " << m_data.size() << " bytes" << endl;
	
	processQueue();
}

void KIOInputStream_impl::slotResult(KIO::Job *job)
{
	// jobs delete themselves after emitting their result
	m_finished = true;
	m_streamStarted = false;
	m_job = 0;

	if(job->error()) {
		// break out of the event loop in case of
		// connection error
	    	emit mimeTypeFound("application/x-zerosize");
		job->showErrorDialog();
	}
}

void KIOInputStream_impl::slotScanMimeType(KIO::Job *, const QString &mimetype)
{
	kdDebug() << "got mimetype: " << mimetype << endl;
	emit mimeTypeFound(mimetype);
}

bool KIOInputStream_impl::eof()
{
	return (m_finished && m_data.size() == 0);
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
	if(m_job != 0)
	{
		if(m_data.size() > (m_packetBuffer * m_packetSize * 2) && !m_job->isSuspended())
		{
			kdDebug() << "STREAMING: suspend job" << endl;
			m_job->suspend();
		}
		else if(m_data.size() < (m_packetBuffer * m_packetSize) && m_job->isSuspended())
		{
			kdDebug() << "STREAMING: resume job" << endl;
			m_job->resume();
		}
	}

	if (!m_firstBuffer) {
		if(m_data.size() < (m_packetBuffer * m_packetSize * 2) ) {
			kdDebug() << "STREAMING: Buffering in progress... (Needed bytes before it starts to play: " << ((m_packetBuffer * m_packetSize) - m_data.size()) << ")" << endl;
			return;
		} else {
			m_firstBuffer = true;
			outdata.setPull(PACKET_COUNT, m_packetSize);
		} 
	}
}

void KIOInputStream_impl::request_outdata(DataPacket<mcopbyte> *packet)
{
	processQueue();
	packet->size = std::min(m_packetSize, m_data.size());
	//kdDebug() << "STREAMING: Filling one DataPacket with " << packet->size << " bytes of the stream!" << endl;

	if (!m_finished) {
		if( (unsigned)packet->size < m_packetSize || ! m_firstBuffer) {
			m_firstBuffer = false;
			packet->size = 0;
			outdata.endPull();
		}
	}
	
	if (packet->size > 0)
	{
		memcpy(packet->contents, m_data.data(), packet->size);
		memmove(m_data.data(), m_data.data() + packet->size, m_data.size() - packet->size);
		m_data.resize(m_data.size() - packet->size);
	}
	packet->send();
}

REGISTER_IMPLEMENTATION(KIOInputStream_impl);
