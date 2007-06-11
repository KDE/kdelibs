/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "rdfrepositorypublisher.h"

Nepomuk::Services::RDFRepositoryPublisher::RDFRepositoryPublisher( const QString& name, const QString& url )
    : Middleware::ServicePublisher( name, url, "http://nepomuk.semanticdesktop.org/services/RDFRepository" )
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
