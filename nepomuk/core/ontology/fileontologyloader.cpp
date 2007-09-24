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

#include "fileontologyloader.h"

#include <soprano/soprano.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>


class Nepomuk::FileOntologyLoader::Private
{
public:
    Private()
        : serialization( Soprano::SerializationUnknown ) {
    }

    QString filename;
    Soprano::RdfSerialization serialization;
};


Nepomuk::FileOntologyLoader::FileOntologyLoader()
    : OntologyLoader(),
      d( new Private() )
{
}


Nepomuk::FileOntologyLoader::FileOntologyLoader( const QString& filename, Soprano::RdfSerialization serialization )
    : OntologyLoader(),
      d( new Private() )
{
    setFileName( filename );
    setSerialization( serialization );
}


Nepomuk::FileOntologyLoader::~FileOntologyLoader()
{
    delete d;
}


void Nepomuk::FileOntologyLoader::setFileName( const QString& filename )
{
    d->filename = filename;
}


void Nepomuk::FileOntologyLoader::setSerialization( Soprano::RdfSerialization s )
{
    d->serialization = s;
}


QString Nepomuk::FileOntologyLoader::fileName() const
{
    return d->filename;
}


QList<Soprano::Statement> Nepomuk::FileOntologyLoader::loadOntology( const QUrl& url )
{
    QString filename;

    if ( d->filename.isEmpty() ) {
        // FIXME: use KIO or is QT sufficient? Actually how about NetAccess and a temp file?
        qDebug() << "(FileOntologyLoader) remote support not implemented yet.";
        return QList<Soprano::Statement>();
    }
    else {
        filename = d->filename;
    }

    QList<Soprano::Statement> sl;

    QFile f( filename );
    if ( f.open( QIODevice::ReadOnly ) ) {
        // TODO: how can we check if the requested onto is really defined in this file?
        const Soprano::Parser* rdfParser = Soprano::PluginManager::instance()->discoverParserForSerialization( d->serialization );
        if ( rdfParser ) {
            sl = rdfParser->parseFile( d->filename, url, d->serialization ).allStatements();
        }
    }
    else {
        qDebug() << "(FileOntologyLoader) failed to open " << filename;
    }

    return sl;
}
