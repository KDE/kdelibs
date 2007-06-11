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

#include "result.h"

#include <QtCore/QSharedData>

class Nepomuk::Middleware::Result::Private : public QSharedData
{
public:
    Private()
        : status(0) {
    }

    QVariant value;
    int status;

    QString errorName;
    QString errorMsg;
};

Nepomuk::Middleware::Result::Result()
{
    d = new Private;
}


Nepomuk::Middleware::Result::Result( const Nepomuk::Middleware::Result& r )
{
    d = r.d;
}


Nepomuk::Middleware::Result::~Result()
{
}


void Nepomuk::Middleware::Result::setValue( const QVariant& v )
{
    d->value = v;
}


void Nepomuk::Middleware::Result::setStatus( int s )
{
    d->status = s;
}


const QVariant& Nepomuk::Middleware::Result::value() const
{
    return d->value;
}


int Nepomuk::Middleware::Result::status() const
{
    return d->status;
}


QString Nepomuk::Middleware::Result::errorName() const
{
    return d->errorName;
}


QString Nepomuk::Middleware::Result::errorMessage() const
{
    return d->errorMsg;
}


Nepomuk::Middleware::Result& Nepomuk::Middleware::Result::operator=( const Nepomuk::Middleware::Result& r )
{
    d = r.d;
    return *this;
}


Nepomuk::Middleware::Result Nepomuk::Middleware::Result::createSimpleResult( const QVariant& value )
{
    Result r;
    r.d->value = value;
    return r;
}


Nepomuk::Middleware::Result Nepomuk::Middleware::Result::createErrorResult( int status, const QString& name, const QString& message )
{
    Result r;
    r.d->status = status;
    r.d->errorName = name;
    r.d->errorMsg = message;
    return r;
}
