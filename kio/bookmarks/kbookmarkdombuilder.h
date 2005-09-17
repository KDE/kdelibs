//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kbookmarkdombuilder_h
#define __kbookmarkdombuilder_h

#include <qstack.h>
#include <qobject.h>
#include <kbookmark.h>

/**
 * @since 3.2
 */
class KIO_EXPORT KBookmarkDomBuilder : public QObject {
   Q_OBJECT
public:
   KBookmarkDomBuilder(const KBookmarkGroup &group, KBookmarkManager *);
   virtual ~KBookmarkDomBuilder();
   void connectImporter(const QObject *);
protected slots:
   void newBookmark(const QString &text, const QString &url, const QString &additionalInfo);
   void newFolder(const QString &text, bool open, const QString &additionalInfo);
   void newSeparator();
   void endFolder();
private:
   QStack<KBookmarkGroup> m_stack;
   QList<KBookmarkGroup> m_list;
   KBookmarkManager *m_manager;
   class KBookmarkDomBuilderPrivate *p;
};

#endif
