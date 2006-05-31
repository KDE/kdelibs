//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2001, 2003 Alexander Kellett <lypanov@kde.org>

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

#ifndef __kbookmarknotifier_h__
#define __kbookmarknotifier_h__

#include "kdelibs_export.h"

class QString;

/**
 * DCOP interface for a bookmark notifier (an object which emits signals
 * upon changes to the bookmarks).
 *
 * D-BUS Port: this is a temporary class. It should be cleaned up with proper
 * signal semantics: DCOP made signals public, which is wrong.
 */
class KIO_EXPORT KBookmarkNotifier
{
    KBookmarkNotifier();

public:
    static void addedBookmark(const QString &filename, const QString &url, const QString &text,
                              const QString &address, const QString &icon);
    static void createdNewFolder(const QString &filename, const QString &text,
                                 const QString &address);
    static void updatedAccessMetadata(const QString &filename, const QString &url);
};

#endif

