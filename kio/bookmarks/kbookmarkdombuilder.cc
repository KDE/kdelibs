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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kbookmarkmanager.h>

#include "kbookmarkdombuilder.h"

KBookmarkDomBuilder::KBookmarkDomBuilder(
   const KBookmarkGroup &bkGroup, KBookmarkManager *manager
) {
   m_manager = manager;
   m_stack.push(&bkGroup);
}

KBookmarkDomBuilder::~KBookmarkDomBuilder() {
   m_list.clear();
   m_stack.clear();
}

void KBookmarkDomBuilder::connectImporter(const QObject *importer) {
   connect(importer, SIGNAL( newBookmark(const QString &, const QCString &, const QString &) ),
                     SLOT( newBookmark(const QString &, const QCString &, const QString &) ));
   connect(importer, SIGNAL( newFolder(const QString &, bool, const QString &) ),
                     SLOT( newFolder(const QString &, bool, const QString &) ));
   connect(importer, SIGNAL( newSeparator() ),
                     SLOT( newSeparator() ) );
   connect(importer, SIGNAL( endFolder() ),
                     SLOT( endFolder() ) );
}

void KBookmarkDomBuilder::newBookmark(
   const QString &text, const QCString &url, const QString &additionnalInfo
) {
   KBookmark bk = m_stack.top()->addBookmark( 
                                    m_manager, text, 
                                    QString::fromUtf8(url),
                                    QString::null, false);
   // store additionnal info
   bk.internalElement().setAttribute("netscapeinfo", additionnalInfo);
}

void KBookmarkDomBuilder::newFolder( 
   const QString & text, bool open, const QString & additionnalInfo 
) {
   // we use a qvaluelist so that we keep pointers to valid objects in the stack
   KBookmarkGroup gp = m_stack.top()->createNewFolder(m_manager, text, false);
   m_list.append(gp);
   m_stack.push(&(m_list.last()));
   // store additionnal info
   QDomElement element = m_list.last().internalElement();
   element.setAttribute("netscapeinfo", additionnalInfo);
   element.setAttribute("folded", (open?"no":"yes"));
}

void KBookmarkDomBuilder::newSeparator() {
   m_stack.top()->createNewSeparator();
}

void KBookmarkDomBuilder::endFolder() {
   m_stack.pop();
}

#include "kbookmarkdombuilder.moc"
