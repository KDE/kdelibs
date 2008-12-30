/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

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

#include "author.h"

using namespace KNS;

struct KNS::AuthorPrivate {
    QString mName;
    QString mEmail;
    QString mJabber;
    QString mHomepage;
};

Author::Author()
        : d(new AuthorPrivate)
{
}

Author::Author(const Author& other)
        : d(new AuthorPrivate(*other.d))

{
}

Author& Author::operator=(const Author & other)
{
    *this->d = *other.d;
    return *this;
}

Author::~Author()
{
    delete d;
}

void Author::setName(const QString& _name)
{
    d->mName = _name;
}

QString Author::name() const
{
    return d->mName;
}

void Author::setEmail(const QString& _email)
{
    d->mEmail = _email;
}

QString Author::email() const
{
    return d->mEmail;
}

void Author::setJabber(const QString& _jabber)
{
    d->mJabber = _jabber;
}

QString Author::jabber() const
{
    return d->mJabber;
}

void Author::setHomepage(const QString& _homepage)
{
    d->mHomepage = _homepage;
}

QString Author::homepage() const
{
    return d->mHomepage;
}

