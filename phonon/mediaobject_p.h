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
	protected:
		MediaObjectPrivate()
			: aboutToFinishTime( 0 )
			, kiojob( 0 )
		{
		}

		void setupKioStreaming();
		void _k_bytestreamNeedData();
		void _k_bytestreamEnoughData();
		void _k_bytestreamData( KIO::Job*, const QByteArray& );
		void _k_bytestreamResult( KJob* );
		void _k_bytestreamTotalSize( KJob*, qulonglong );
		void _k_cleanupByteStream();

		KUrl url;
		qint32 aboutToFinishTime;
		KIO::TransferJob* kiojob;
};
}

#endif // MEDIAOBJECT_P_H
// vim: sw=4 ts=4 tw=80
