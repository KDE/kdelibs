/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "desktopontologyloader.h"
#include "fileontologyloader.h"
#include "qurlhash.h"

#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kdebug.h>
#include <kglobal.h>

#include <QDir>

#include <soprano/statement.h>

struct OntoBuffer
{
    QString fileName;
    Soprano::RdfSerialization serialization;
};


class Konto::DesktopOntologyLoader::Private
{
public:
    Private() {
    }

    void updateOntologyCache() {
        ontoCache.clear();

        QStringList allOntologies = KGlobal::dirs()->findAllResources( "data", "knepomuk/ontologies/*.desktop" );
        foreach( QString ontoDesktopFilePath, allOntologies ) {
            KDesktopFile ontoDesktopFile( ontoDesktopFilePath );

            if ( ontoDesktopFile.hasLinkType() ) {
                QString uri = ontoDesktopFile.readUrl();
                QString path = ontoDesktopFile.readPath();

                // make it an absolute path
                if ( path[0] != QDir::separator() ) {
                    path.prepend( ontoDesktopFilePath.section( QDir::separator(),
                                                               0, -2,
                                                               QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep ) );
                }

                OntoBuffer onto;
                onto.fileName = path;
                onto.serialization = Soprano::mimeTypeToSerialization( ontoDesktopFile.readEntry( "MimeType", "application/rdf+xml" ) );;

                kDebug() << "(Konto::DesktopOntologyLoader) found ontology " << uri << endl;
                ontoCache.insert( QUrl( uri ), onto );
            }
        }
    }

    // contains the filename for each cached ontology
    QHash<QUrl, OntoBuffer> ontoCache;
};


Konto::DesktopOntologyLoader::DesktopOntologyLoader()
    : OntologyLoader(),
      d( new Private() )
{
}


Konto::DesktopOntologyLoader::~DesktopOntologyLoader()
{
    delete d;
}


QList<QUrl> Konto::DesktopOntologyLoader::allOntologies()
{
    d->updateOntologyCache();
    return d->ontoCache.keys();
}


QList<Soprano::Statement> Konto::DesktopOntologyLoader::loadOntology( const QUrl& url )
{
    d->updateOntologyCache();
    if ( d->ontoCache.contains( url ) ) {
        const OntoBuffer& onto = d->ontoCache[url];
        FileOntologyLoader loader( onto.fileName, onto.serialization );
        return loader.loadOntology( url );
    }
    else {
        kDebug() << "(Konto::DesktopOntologyLoader) could not find ontology " << url << endl;
        return QList<Soprano::Statement>();
    }
}
