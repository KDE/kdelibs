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

#include <qobject.h>
#include <qcstring.h>
#include <kio/jobclasses.h>
#include <kurl.h>
#include "artskde.h"
#include "stdsynthmodule.h" 

namespace Arts {

class KIOInputStream_impl : public QObject, virtual public KIOInputStream_skel, 
					    virtual public InputStream_skel,
					    virtual public StdSynthModule
{
Q_OBJECT
public:
	KIOInputStream_impl();
	~KIOInputStream_impl();
	
	void streamStart();	
	void streamEnd();

	bool eof();
	bool seekOk();
	long size();
	long seek(long);

	bool openURL(const std::string& url);

	void processQueue();
	void request_outdata(DataPacket<mcopbyte> *packet);

	long bufferPackets() { return m_packetBuffer; }
	void bufferPackets(long i) { m_packetBuffer = i; }

private slots:
	void slotData(KIO::Job *, const QByteArray &);
	void slotResult(KIO::Job *);

private:
	KURL m_url;
	KIO::TransferJob *m_job;
	std::queue<DataPacket<mcopbyte> *> m_sendqueue;
	QByteArray m_data;
	bool m_finished;
	bool m_firstBuffer;

	unsigned int m_packetBuffer;

	static const unsigned int PACKET_COUNT;
	static const unsigned int PACKET_SIZE;
};

};
