/*
    This file is part of KNewStuff2.
    Copyright (c) 2006 Josef Spillner <spillner@kde.org>

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

#include "category.h"

using namespace KNS;

class CategoryPrivate
{
  public:
  CategoryPrivate(){}
};

Category::Category()
{
  d = NULL;
}

Category::~Category()
{
}

void Category::setId(const QString& id)
{
  mId = id;
}

QString Category::id() const
{
  return mId;
}

void Category::setName(const KTranslatable& name)
{
  mName = name;
}

KTranslatable Category::name() const
{
  return mName;
}

void Category::setDescription(const KTranslatable &description)
{
  mDescription = description;
}

KTranslatable Category::description() const
{
  return mDescription;
}

void Category::setIcon(const KURL& icon)
{
  mIcon = icon;
}

KURL Category::icon() const
{
  return mIcon;
}

