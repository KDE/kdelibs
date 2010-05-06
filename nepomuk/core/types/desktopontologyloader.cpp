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

#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>

#include <QtCore/QDir>

#include <soprano/statement.h>

struct OntoBuffer
{
    QString fileName;
    Soprano::RdfSerialization serialization;
};


class Nepomuk::DesktopOntologyLoader::Private
{
public:
    Private() {
    }

    void updateOntologyCache() {
//        ontoCache.clear();

        QStringList allOntologies = KGlobal::dirs()->findAllResources( "data", "nepomuk/ontologies/*.desktop" );
        foreach( const QString &ontoDesktopFilePath, allOntologies ) {
            KDesktopFile ontoDesktopFile( ontoDesktopFilePath );

            if ( ontoDesktopFile.hasLinkType() ) {
                QUrl uri = ontoDesktopFile.readUrl();

                if ( !ontoCache.contains( uri ) ) {
                    QString path = ontoDesktopFile.readPath();

                    // make it an absolute path
                    if ( path[0] != QDir::separator() ) {
                        path.prepend( ontoDesktopFilePath.section( QDir::separator(),
                                                                   0, -2,
                                                                   QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep ) );
                    }

                    OntoBuffer onto;
                    onto.fileName = path;
                    onto.serialization = Soprano::mimeTypeToSerialization( ontoDesktopFile.desktopGroup().readEntry( "MimeType", "application/rdf+xml" ) );

                    kDebug() << "(Nepomuk::DesktopOntologyLoader) found ontology " << uri;
                    ontoCache.insert( uri, onto );
                }
            }
        }
    }

    // contains the filename for each cached ontology
    QHash<QUrl, OntoBuffer> ontoCache;
};


Nepomuk::DesktopOntologyLoader::DesktopOntologyLoader()
    : OntologyLoader(),
      d( new Private() )
{
}


Nepomuk::DesktopOntologyLoader::~DesktopOntologyLoader()
{
    delete d;
}


QList<QUrl> Nepomuk::DesktopOntologyLoader::allOntologies()
{
    d->updateOntologyCache();
    return d->ontoCache.keys();
}


QList<Soprano::Statement> Nepomuk::DesktopOntologyLoader::loadOntology( const QUrl& url )
{
    d->updateOntologyCache();
    if ( d->ontoCache.contains( url ) ) {
        const OntoBuffer& onto = d->ontoCache[url];
        FileOntologyLoader loader( onto.fileName, onto.serialization );
        return loader.loadOntology( url );
    }
    else {
        kDebug() << "(Nepomuk::DesktopOntologyLoader) could not find ontology " << url;
        return QList<Soprano::Statement>();
    }
}
