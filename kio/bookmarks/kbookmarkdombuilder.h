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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kbookmarkdombuilder_h
#define __kbookmarkdombuilder_h

#include <kbookmark.h>
#include <qptrstack.h>
#include <qobject.h>

class KBookmarkDomBuilder : public QObject {
   Q_OBJECT
public:
   KBookmarkDomBuilder(const KBookmarkGroup &group, KBookmarkManager *);
   virtual ~KBookmarkDomBuilder();
   void connectImporter(const QObject *);
protected slots:
   void newBookmark(const QString &text, const QCString &url, const QString &additionnalInfo);
   void newFolder(const QString &text, bool open, const QString &additionnalInfo);
   void newSeparator();
   void endFolder();
private:
   QPtrStack<KBookmarkGroup> m_stack;
   QValueList<KBookmarkGroup> m_list;
   KBookmarkManager *m_manager;
};

#endif
