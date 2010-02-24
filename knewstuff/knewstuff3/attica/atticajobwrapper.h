/*
    Copyright (c) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KNEWSTUFF3_ATTICA_JOBWRAPPER_H
#define KNEWSTUFF3_ATTICA_JOBWRAPPER_H

#include <QtNetwork/QNetworkReply>
#include "kjob.h"
#include <attica/atticabasejob.h>
         #include <kdebug.h>
namespace KNS3
{
    /**
     * @short KNewStuff Attica KJob Wrapper
     *
     * A small wrapper to provide progress from attica jobs as KJobs
     *
     * @author Frederik Gladhorn <gladhorn@kde.org>
     *
     * @internal
     */
    class AtticaJobWrapper: public KJob
    {
        Q_OBJECT
    public:
        AtticaJobWrapper(QNetworkReply* reply)
        {
            connect(reply, SIGNAL(finished()), SLOT(jobFinished()));
            connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(downloadProgress(qint64, qint64)));
        }
        
    private Q_SLOTS:
        void jobFinished() { emitResult(); deleteLater(); }
        
        void downloadProgress ( qint64 bytesReceived, qint64 bytesTotal )
        {
            setTotalAmount(Bytes, bytesTotal);
            setProcessedAmount(Bytes, bytesReceived);
            kDebug() << "DOWNLOADPROGRESS" << bytesTotal << " " << bytesReceived;
        }
        
    private:
        void start(){}
      
    };

}

#endif
