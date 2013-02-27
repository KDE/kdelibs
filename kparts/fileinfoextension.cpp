/* This file is part of the KDE project
   Copyright (C) 2010 Dawit Alemayehu <adawit@kde.org>

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

#include "fileinfoextension.h"

#include "part.h"

KParts::FileInfoExtension::FileInfoExtension(KParts::ReadOnlyPart* parent)
                          : QObject(parent),
                            d(0)

{
}

KParts::FileInfoExtension::~FileInfoExtension()
{
}

KParts::FileInfoExtension* KParts::FileInfoExtension::childObject(QObject* obj)
{
    return obj->findChild<KParts::FileInfoExtension*>(QString(), Qt::FindDirectChildrenOnly);
}

KParts::FileInfoExtension::QueryModes KParts::FileInfoExtension::supportedQueryModes() const
{
    return None;
}

bool KParts::FileInfoExtension::hasSelection() const
{
    return false;
}
