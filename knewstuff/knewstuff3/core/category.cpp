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

using namespace KNS;

struct KNS::CategoryPrivate {
    KTranslatable mName;
    KTranslatable mDescription;
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

void Category::setName(const KTranslatable& name)
{
    d->mName = name;
}

KTranslatable Category::name() const
{
    return d->mName;
}

void Category::setDescription(const KTranslatable &description)
{
    d->mDescription = description;
}

KTranslatable Category::description() const
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

