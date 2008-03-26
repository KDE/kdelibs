/***************************************************************************
 * object.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "object.h"

using namespace Kross;

class Object::Private
{
    public:
};

Object::Object()
    : QSharedData()
    , ErrorInterface()
    , d(new Private)
{
}

Object::Object(const Object &other)
    : QSharedData()
    , ErrorInterface()
    , d(new Private)
{
    Q_UNUSED(other);
}

Object::~Object()
{
    delete d;
}

QVariant Object::callMethod(const QString& name, const QVariantList& args)
{
    Q_UNUSED(name);
    Q_UNUSED(args);
    return QVariant();
}

QStringList Object::methodNames()
{
    return QStringList();
}

void Object::virtual_hook(int id, void* ptr)
{
    Q_UNUSED(id);
    Q_UNUSED(ptr);
}
