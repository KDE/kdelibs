/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "rdfrepositorypublisher.h"

Nepomuk::Services::RDFRepositoryPublisher::RDFRepositoryPublisher( const QString& name, const QString& url )
    : Backbone::ServicePublisher( name, url, "http://nepomuk.semanticdesktop.org/services/RDFRepository" )
{
}


Nepomuk::Services::RDFRepositoryPublisher::~RDFRepositoryPublisher()
{
}


int Nepomuk::Services::RDFRepositoryPublisher::supportsQueryLanguage( const QString& lang )
{
    return( supportedQueryLanguages().contains( lang ) ? 1 : 0 );
}

#include "rdfrepositorypublisher.moc"
