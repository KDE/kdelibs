/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkdombuilder.h"

#include <kbookmarkmanager.h>
#include <kdebug.h>

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
   if (!m_stack.isEmpty()) {
      KBookmark bk = m_stack.top().addBookmark(
                                    text,
                                    KUrl( url ), // utf8
                                    QString());
      // store additional info
      bk.internalElement().setAttribute("netscapeinfo", additionalInfo);
   }
   else
      kWarning() << "m_stack is empty. This should not happen when importing a valid bookmarks file!";
}

void KBookmarkDomBuilder::newFolder(
   const QString & text, bool open, const QString & additionalInfo
) {
   if (!m_stack.isEmpty()) {
      // we use a qvaluelist so that we keep pointers to valid objects in the stack
      KBookmarkGroup gp = m_stack.top().createNewFolder(text);
      m_list.append(gp);
      m_stack.push(m_list.last());
      // store additional info
      QDomElement element = m_list.last().internalElement();
      element.setAttribute("netscapeinfo", additionalInfo);
      element.setAttribute("folded", (open?"no":"yes"));
   }
   else
      kWarning() << "m_stack is empty. This should not happen when importing a valid bookmarks file!";
}

void KBookmarkDomBuilder::newSeparator() {
   if (!m_stack.isEmpty())
      m_stack.top().createNewSeparator();
   else
      kWarning() << "m_stack is empty. This should not happen when importing a valid bookmarks file!";
}

void KBookmarkDomBuilder::endFolder() {
   if (!m_stack.isEmpty())
      m_stack.pop();
   else
      kWarning() << "m_stack is empty. This should not happen when importing a valid bookmarks file!";
}

#include "kbookmarkdombuilder.moc"
