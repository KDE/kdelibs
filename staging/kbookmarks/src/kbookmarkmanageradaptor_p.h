//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KBOOKMARKMANAGERADAPTOR_H
#define KBOOKMARKMANAGERADAPTOR_H

#include <QtDBus/QtDBus>
class KBookmarkManager;

class KBookmarkManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KIO.KBookmarkManager")
public:
    KBookmarkManagerAdaptor(KBookmarkManager *parent);

public Q_SLOTS:
    // Not used by KDE, but useful for scripts, after changing the bookmarks.xml file.
    void notifyCompleteChange();

Q_SIGNALS:
    void bookmarkCompleteChange( QString caller );

    void bookmarksChanged( QString groupAddress );

    void bookmarkConfigChanged();
};

#endif
