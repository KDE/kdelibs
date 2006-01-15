// -*- c-basic-offset: 3 -*-
/**
* kimgio.h -- Implementation of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#include <QImageReader>
#include <QImageWriter>

#include <kmimetype.h>
#include <klocale.h>
#include <kdebug.h>

#include "kimageio.h"

QString
KImageIO::pattern(Mode _mode)
{
    QStringList patterns, allFormats;
    QString allPatterns;
    QString separator("|");
    
    QList<QByteArray> formats = ( _mode == KImageIO::Reading ) ? 
        QImageReader::supportedImageFormats() :
        QImageWriter::supportedImageFormats();
    KService::List services = KServiceType::offers("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services)
    {
        allFormats = service->property("X-KDE-ImageFormat").toStringList();
        if ( allFormats.isEmpty() ) continue;
        QString format = allFormats[0];
        if ( formats.contains( format.toLatin1() ) ) {
	    QString mimeType = service->property("X-KDE-MimeType").toString();
	    KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
	    QString pattern = mime->patterns().join(" ");
	    patterns.append( pattern + separator + mime->comment() );
	    if (!allPatterns.isEmpty() )
	        allPatterns += " ";
	    allPatterns += pattern;
	    
	}
    }

    allPatterns = allPatterns + separator + i18n("All Pictures");
    patterns.sort();
    patterns.prepend(allPatterns);

    QString pattern = patterns.join(QLatin1String("\n"));
    return pattern;
}

QStringList KImageIO::typeForMime(const QString& mimeType)
{
    KService::List services = KServiceType::offers("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services) {
        if ( mimeType == service->property("X-KDE-MimeType").toString() )
            return ( service->property("X-KDE-ImageFormat").toStringList() );
    }
    return QStringList();
}

QStringList KImageIO::mimeTypes( Mode _mode )
{
    QStringList mimeList, allFormats;
    QList<QByteArray> formats = ( _mode == KImageIO::Reading ) ? 
        QImageReader::supportedImageFormats() :
        QImageWriter::supportedImageFormats();

    KService::List services = KServiceType::offers("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services) {
        allFormats = service->property("X-KDE-ImageFormat").toStringList();
        if ( allFormats.isEmpty() ) continue;
        if ( formats.contains( allFormats[0].toLatin1() ) )
            mimeList.append( service->property("X-KDE-MimeType").toString() );
    }

    return mimeList;
}

bool KImageIO::isSupported( const QString& _mimeType, Mode _mode )
{
    QStringList types = typeForMime( _mimeType );
    if ( types.empty() ) return false;
    QString type = types[0];
    QList<QByteArray> formats = ( _mode == KImageIO::Reading ) ? 
        QImageReader::supportedImageFormats() :
        QImageWriter::supportedImageFormats();

    QByteArray format;
    return formats.contains( type.toLatin1() );
}
