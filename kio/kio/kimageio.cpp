// -*- c-basic-offset: 3 -*-
/**
* kimgio.h -- Implementation of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#include <kmimetype.h>
#include <klocale.h>
#include <kdebug.h>

#include "kimageio.h"

QString
KImageIO::pattern(Mode _mode)
{
    QStringList patterns;
    QString allPatterns;
    QString separator("|");
    
    KService::List services = KServiceType::offers("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services)
    {
        if ( (service->property("X-KDE-Read").toBool() && _mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && _mode == Writing ) ) {
        
	        QString mimeType = service->property("X-KDE-MimeType").toString();
	        if ( mimeType.isEmpty() ) continue;
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

    KService::List services = KServiceType::offers("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services) {
        if ( (service->property("X-KDE-Read").toBool() && _mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && _mode == Writing ) ) {
        
            mimeList.append( service->property("X-KDE-MimeType").toString() );
        }
    }

    return mimeList;
}

QStringList KImageIO::types( Mode _mode )
{
    QStringList imagetypes;
    KService::List services = KServiceType::offers("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services) {
        if ( (service->property("X-KDE-Read").toBool() && _mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && _mode == Writing ) ) {
             
             imagetypes += service->property("X-KDE-ImageFormat").toStringList();
             
        }
    }
    return imagetypes;
}

bool KImageIO::isSupported( const QString& _mimeType, Mode _mode )
{
    KService::List services = KServiceType::offers("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services) {
        if ( _mimeType == service->property("X-KDE-MimeType").toString() ) {

            if ( (service->property("X-KDE-Read").toBool() && _mode == Reading) ||
                 (service->property("X-KDE-Write").toBool() && _mode == Writing ) ) {
             
                return true;
            } else {
                return false;
            } 
        }
    }
    return false;
}
