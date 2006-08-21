//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

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

#include "kbookmarknotifier.h"
#include "QtDBus/QtDBus"

static void emitSignal(const QString &signalName, const QVariantList &args)
{
    QDBusMessage message =
        QDBusMessage::createSignal("/", "org.kde.KIO.KBookmarkNotifier", signalName);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

void KBookmarkNotifier::addedBookmark(const QString &filename, const QString &url,
                                      const QString &text, const QString &address,
                                      const QString &icon)
{
    emitSignal("addedBookmark", QVariantList() << filename << url << text << address << icon);
}

void KBookmarkNotifier::createdNewFolder(const QString &filename, const QString &text,
                                         const QString &address)
{
    emitSignal("createdNewFolder", QVariantList() << filename << text << address);
}

void KBookmarkNotifier::updatedAccessMetadata(const QString &filename, const QString &url)
{
    emitSignal("updatedAccessMetadata", QVariantList() << filename << url);
}
