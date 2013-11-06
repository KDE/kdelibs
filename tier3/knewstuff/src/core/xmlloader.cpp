/*
    knewstuff3/xmlloader.cpp.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include "xmlloader_p.h"

#include <QtCore/QByteArray>

#include <kconfig.h>
#include <QDebug>
#include <kio/job.h>
#include <klocalizedstring.h>

namespace KNS3
{

    XmlLoader::XmlLoader(QObject* parent)
            : QObject(parent)
    {
    }

    void XmlLoader::load(const QUrl &url)
    {
        m_jobdata.clear();

        // qDebug() << "XmlLoader::load(): url: " << url;

        KIO::TransferJob *job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
        connect(job, SIGNAL(result(KJob*)),
                SLOT(slotJobResult(KJob*)));
        connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
                SLOT(slotJobData(KIO::Job*,QByteArray)));
	
        emit jobStarted(job);
    }

    void XmlLoader::slotJobData(KIO::Job *, const QByteArray &data)
    {
        // qDebug() << "XmlLoader::slotJobData()";

        m_jobdata.append(data);
    }

    void XmlLoader::slotJobResult(KJob *job)
    {
        if (job->error()) {
            emit signalFailed();
            return;
        }
/*
        // qDebug() << "--Xml Loader-START--";
        // qDebug() << QString::fromUtf8(m_jobdata);
        // qDebug() << "--Xml Loader-END--";
*/
        QDomDocument doc;
        if (!doc.setContent(m_jobdata)) {
            emit signalFailed();
            return;
        }
        emit signalLoaded(doc);
    }

    QDomElement addElement(QDomDocument& doc, QDomElement& parent,
                                         const QString& tag, const QString& value)
    {
        QDomElement n = doc.createElement(tag);
        n.appendChild(doc.createTextNode(value));
        parent.appendChild(n);

        return n;
    }
} // end KNS namespace

