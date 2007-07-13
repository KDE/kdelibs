/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
 *               2001 KDE Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "kstaticdeleter.h"
#include "kglobal.h"

#include <QtCore/QList>
#include <QtCore/QCoreApplication>

typedef QList<KStaticDeleterBase *> KStaticDeleterList;

struct KStaticDeleterPrivate
{
    KStaticDeleterPrivate()
    {
        qAddPostRoutine(deleteStaticDeleters);
    }
    ~KStaticDeleterPrivate()
    {
        qRemovePostRoutine(deleteStaticDeleters);
        deleteStaticDeleters();
    }
    static void deleteStaticDeleters();

    KStaticDeleterList staticDeleters;
};

K_GLOBAL_STATIC(KStaticDeleterPrivate, staticDeleterPrivate)

void KStaticDeleterPrivate::deleteStaticDeleters()
{
    if (staticDeleterPrivate.isDestroyed()) {
        return;
    }
    KStaticDeleterPrivate *d = staticDeleterPrivate;
    while (!d->staticDeleters.isEmpty()) {
        d->staticDeleters.takeLast()->destructObject();
    }
}

void KStaticDeleterHelpers::registerStaticDeleter(KStaticDeleterBase *obj)
{
    KStaticDeleterPrivate *d = staticDeleterPrivate;
    if (d->staticDeleters.indexOf(obj) == -1) {
        d->staticDeleters.append(obj);
    }
}

void KStaticDeleterHelpers::unregisterStaticDeleter(KStaticDeleterBase *obj)
{
    if (staticDeleterPrivate.isDestroyed()) {
        return;
    }
    staticDeleterPrivate->staticDeleters.removeAll(obj);
}

void KStaticDeleterHelpers::deleteStaticDeleters()
{
    staticDeleterPrivate->deleteStaticDeleters();
}

// this helps gcc to emit the vtbl for KStaticDeleterBase
// only once, here in this file, not every time it's
// used, says Seli and thiago.
KStaticDeleterBase::~KStaticDeleterBase()
{
}

void KStaticDeleterBase::destructObject()
{
}

