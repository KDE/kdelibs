/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kdirsize.h"
#include <kdebug.h>
#include <qapplication.h>

KDirSize::KDirSize( const KURL & directory )
    : Job(false /*No GUI*/)
{
    m_bAsync = true;
    m_totalSize = 0L;
    KIO::ListJob * listJob = KIO::listRecursive( directory, false /* no GUI */ );
    connect( listJob, SIGNAL(entries( KIO::Job *,
                                      const KIO::UDSEntryList& )),
             SLOT( slotEntries( KIO::Job*,
                                const KIO::UDSEntryList& )));
    addSubjob( listJob );
}

void KDirSize::slotEntries( KIO::Job*, const KIO::UDSEntryList & list )
{
    KIO::UDSEntryListConstIterator it = list.begin();
    KIO::UDSEntryListConstIterator end = list.end();
    for (; it != end; ++it) {
        KIO::UDSEntry::ConstIterator it2 = (*it).begin();
        int size = 0L;
        //bool isDir = false;
        QString name;
        for( ; it2 != (*it).end(); it2++ ) {
          switch( (*it2).m_uds ) {
            case KIO::UDS_NAME:
              name = (*it2).m_str;
              break;
            //case KIO::UDS_FILE_TYPE:
            //  isDir = S_ISDIR((*it2).m_long);
            //  break;
            case KIO::UDS_SIZE:
              size = ((*it2).m_long);
              break;
            default:
              break;
          }
        }
        //if ( !isDir ) // they take space too
        if ( name != QString::fromLatin1("..") )
        {
            m_totalSize += size;
            //kdDebug() << name << ":" << size << endl;
        }
    }
}

//static
KDirSize * KDirSize::dirSizeJob( const KURL & directory )
{
    return new KDirSize( directory ); // useless - but consistent with other jobs
}

//static
unsigned long KDirSize::dirSize( const KURL & directory )
{
    KDirSize * dirSize = dirSizeJob( directory );
    dirSize->setSync();
    qApp->enter_loop();
    return dirSize->totalSize();
}


void KDirSize::slotResult( KIO::Job * job )
{
    kdDebug() << " KDirSize::slotResult( KIO::Job * job ) " << endl;
    if ( !m_bAsync )
        qApp->exit_loop();
    KIO::Job::slotResult( job );
}

#include "kdirsize.moc"
