//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kbookmarkmanager.h>
#include <kdebug.h>

#include "kbookmarkdombuilder.h"

KBookmarkDomBuilder::KBookmarkDomBuilder(
   const KBookmarkGroup &bkGroup, KBookmarkManager *manager
) {
   m_manager = manager;
   m_stack.push(bkGroup);
}

KBookmarkDomBuilder::~KBookmarkDomBuilder() {
   m_list.clear();
   m_stack.clear();
}

void KBookmarkDomBuilder::connectImporter(const QObject *importer) {
   connect(importer, SIGNAL( newBookmark(const QString &, const QString &, const QString &) ),
                     SLOT( newBookmark(const QString &, const QString &, const QString &) ));
   connect(importer, SIGNAL( newFolder(const QString &, bool, const QString &) ),
                     SLOT( newFolder(const QString &, bool, const QString &) ));
   connect(importer, SIGNAL( newSeparator() ),
                     SLOT( newSeparator() ) );
   connect(importer, SIGNAL( endFolder() ),
                     SLOT( endFolder() ) );
}

void KBookmarkDomBuilder::newBookmark(
   const QString &text, const QString &url, const QString &additionalInfo
) {
   KBookmark bk = m_stack.top().addBookmark(
                                    m_manager, text,
                                    KUrl( url ), // utf8
                                    QString(), false);
   // store additional info
   bk.internalElement().setAttribute("netscapeinfo", additionalInfo);
}

void KBookmarkDomBuilder::newFolder(
   const QString & text, bool open, const QString & additionalInfo
) {
   // we use a qvaluelist so that we keep pointers to valid objects in the stack
   KBookmarkGroup gp = m_stack.top().createNewFolder(m_manager, text, false);
   m_list.append(gp);
   m_stack.push(m_list.last());
   // store additional info
   QDomElement element = m_list.last().internalElement();
   element.setAttribute("netscapeinfo", additionalInfo);
   element.setAttribute("folded", (open?"no":"yes"));
}

void KBookmarkDomBuilder::newSeparator() {
   m_stack.top().createNewSeparator();
}

void KBookmarkDomBuilder::endFolder() {
   m_stack.pop();
}

#include "kbookmarkdombuilder.moc"
