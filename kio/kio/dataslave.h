// -*- c++ -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Leo Savernik <l.savernik@aon.at>
 *  Derived from slave.h
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __KIO_DATASLAVE_H__
#define __KIO_DATASLAVE_H__

#include <kio/global.h>
#include <kio/slave.h>

class QTimer;

namespace KIO {

    /**
     * This class provides a high performance implementation for the data
     * url scheme (rfc2397).
     *
     * @internal
     * Do not use this class in external applications. It is an implementation
     * detail of KIO and subject to change without notice.
     * @author Leo Savernik
     */
    class KIO_EXPORT DataSlave : public KIO::Slave {
	Q_OBJECT
    public:
	DataSlave();

        virtual ~DataSlave();

        virtual void setHost(const QString &host, int port,
			const QString &user, const QString &passwd);
	virtual void setConfig(const MetaData &config);

        virtual void suspend();
        virtual void resume();
        virtual bool suspended();
        virtual void send(int cmd, const QByteArray &arr = QByteArray());

	virtual void hold(const KURL &url);

	// pure virtual methods that are defined by the actual protocol
	virtual void get(const KURL &url) = 0;
	virtual void mimetype(const KURL &url) = 0;

    protected:
        /**
	 * Sets metadata
	 * @internal
	 */
	void setAllMetaData(const MetaData &);
	/**
	 * Sends metadata set with setAllMetaData
	 * @internal
	 */
	void sendMetaData();

	// queueing methods
	/** identifiers of functions to be queued */
	enum QueueType { QueueMimeType = 1, QueueTotalSize, QueueSendMetaData,
		QueueData, QueueFinished };
	/** structure for queueing. It is very primitive, it doesn't
	 * even try to conserve memory.
	 */
	struct QueueStruct {
	  QueueType type;
	  QString s;
	  KIO::filesize_t size;
	  QByteArray ba;

	  QueueStruct() {}
	  QueueStruct(QueueType type) : type(type) {}
	};
        typedef QValueList<QueueStruct> DispatchQueue;
	DispatchQueue dispatchQueue;

	void dispatch_mimeType(const QString &s) {
	  if (_suspended) {
	    QueueStruct q(QueueMimeType);
	    q.s = s;
	    dispatchQueue.push_back(q);
	  } else
	    mimeType(s);
	}
	void dispatch_totalSize(KIO::filesize_t size) {
	  if (_suspended) {
	    QueueStruct q(QueueTotalSize);
	    q.size = size;
	    dispatchQueue.push_back(q);
	  } else
	    totalSize(size);
	}
	void dispatch_sendMetaData() {
	  if (_suspended) {
	    QueueStruct q(QueueSendMetaData);
	    dispatchQueue.push_back(q);
	  } else
	    sendMetaData();
	}
	void dispatch_data(const QByteArray &ba) {
	  if (_suspended) {
	    QueueStruct q(QueueData);
	    q.ba = ba;
	    dispatchQueue.push_back(q);
	  } else
	    data(ba);
	}
	void dispatch_finished() {
	  // Allways queue, since it will delete us
          QueueStruct q(QueueFinished);
          dispatchQueue.push_back(q);
	}

    protected slots:
	/** dispatches next queued method. Does nothing if there are no
	 * queued methods.
	 */
	void dispatchNext();
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	MetaData meta_data;
	bool _suspended;
	QTimer *timer;
    };

}

#endif /*__KIO_DATASLAVE_H__*/
