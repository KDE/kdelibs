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

#include "dummyservice.h"

Nepomuk::Backbone::Services::DummyServicePublisher::DummyServicePublisher( const QString& name, const QString& url )
    : ServicePublisher( name, url, "http://nepomuk.semanticdesktop.org/services/DummyService" )
{
}


Nepomuk::Backbone::Services::DummyServicePublisher::~DummyServicePublisher()
{
}



Nepomuk::Backbone::DBus::DummyServicePublisherInterface::DummyServicePublisherInterface( ServicePublisher* s )
    : ServicePublisherInterface( s )
{
}


QString Nepomuk::Backbone::DBus::DummyServicePublisherInterface::test()
{
    return static_cast<Services::DummyServicePublisher*>(servicePublisher())->test();
}

#include "dummyservice.moc"
