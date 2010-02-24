/*
    knewstuff3/xmlloader.h.
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
#ifndef KNEWSTUFF3_XML_LOADER_H
#define KNEWSTUFF3_XML_LOADER_H

#include <QtXml/qdom.h>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <kurl.h>

class KJob;

namespace KIO
{
    class Job;
}

namespace KNS3
{

QDomElement addElement(QDomDocument& doc, QDomElement& parent,
                                 const QString& tag, const QString& value);

/**
 * KNewStuff xml loader.
 * This class loads an xml document from a kurl and returns the
 * resulting domdocument once completed.
 * It should probably not be used directly by the application.
 *
 * @internal
 */
class XmlLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    XmlLoader(QObject* parent);

    /**
     * Starts asynchronously loading the xml document from the
     * specified URL.
     *
     * @param url location of the XML file
     */
    void load(const KUrl & url);

Q_SIGNALS:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void signalLoaded(const QDomDocument&);
    void signalFailed();
    
    void jobStarted(KJob*);

protected Q_SLOTS:
    void slotJobData(KIO::Job *, const QByteArray &);
    void slotJobResult(KJob *);

private:
    QByteArray m_jobdata;
};

}

#endif
