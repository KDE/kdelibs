/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#ifndef MEDIAOBJECT_P_H
#define MEDIAOBJECT_P_H

#include "mediaobject.h"
#include "ifaces/mediaobject.h"
#include "ifaces/bytestream.h"
#include "abstractmediaproducer_p.h"
#include <kurl.h>
#include <kio/jobclasses.h>
#include <kio/job.h>

namespace Phonon
{
namespace Ifaces
{
	class ByteStream;
}
class MediaObjectPrivate : public AbstractMediaProducerPrivate
{
	K_DECLARE_PUBLIC( MediaObject )
	protected:
		virtual bool aboutToDeleteIface();
		virtual void createIface();
		virtual void setIface( void* p )
		{
			// setIface( 0 ) is called after the base iface_ptr was deleted to
			// reset all dangling pointers. In the case that we are using a
			// ByteStream transparently the bytestream was deleted and must be
			// reset to 0.
			if( p == 0 )
			{
				bytestream = 0;
				if( kiojob )
				{
					kiojob->kill();
					kiojob = 0;
				}
			}
			iface_ptr = reinterpret_cast<Ifaces::MediaObject*>( p );
			AbstractMediaProducerPrivate::setIface( static_cast<Ifaces::AbstractMediaProducer*>( iface_ptr ) );
		}
	private:
		Ifaces::MediaObject* iface_ptr;
		inline Ifaces::MediaObject* iface() { return iface_ptr; }
		inline const Ifaces::MediaObject* iface() const { return iface_ptr; }
	protected:
		MediaObjectPrivate()
			: aboutToFinishTime( 0 )
			, bytestream( 0 )
			, kiojob( 0 )
		{
		}

		void setupKioStreaming();
		void _k_bytestreamNeedData();
		void _k_bytestreamEnoughData();
		void _k_bytestreamData( KIO::Job*, const QByteArray& );
		void _k_bytestreamResult( KIO::Job* );
		void _k_bytestreamTotalSize( KIO::Job*, KIO::filesize_t );

		KUrl url;
		long aboutToFinishTime;
		Ifaces::ByteStream* bytestream;
		KIO::TransferJob* kiojob;
};
}

#endif // MEDIAOBJECT_P_H
// vim: sw=4 ts=4 tw=80
