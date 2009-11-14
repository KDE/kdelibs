/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

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

#include "category.h"

using namespace KNS3;

struct KNS3::CategoryPrivate {
    QString mName;
    QString mDescription;
    KUrl mIcon;
    QString mId;
};

Category::Category()
        : d(new CategoryPrivate)
{
}

Category::~Category()
{
    delete d;
}

void Category::setId(const QString& id)
{
    d->mId = id;
}

QString Category::id() const
{
    return d->mId;
}

void Category::setName(const QString& name)
{
    d->mName = name;
}

QString Category::name() const
{
    return d->mName;
}

void Category::setDescription(const QString &description)
{
    d->mDescription = description;
}

QString Category::description() const
{
    return d->mDescription;
}

void Category::setIcon(const KUrl& icon)
{
    d->mIcon = icon;
}

KUrl Category::icon() const
{
    return d->mIcon;
}

