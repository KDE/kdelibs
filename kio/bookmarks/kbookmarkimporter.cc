//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcharsets.h>
#include <qtextcodec.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>

#include "kbookmarkmanager.h"

#include "kbookmarkimporter_ns.h"
#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_ie.h"

#include "kbookmarkimporter.h"

void KXBELBookmarkImporterImpl::parse()
{
  //kdDebug() << "KXBELBookmarkImporterImpl::parse()" << endl;
  KBookmarkManager *manager = KBookmarkManager::managerForFile(m_fileName);
  KBookmarkGroup root = manager->root();
  traverse(root);
  // FIXME delete it!
  // delete manager;
}

void KXBELBookmarkImporterImpl::visit(const KBookmark &bk)
{
  //kdDebug() << "KXBELBookmarkImporterImpl::visit" << endl;
  if (bk.isSeparator())
    emit newSeparator();
  else
    emit newBookmark(bk.fullText(), bk.url().url(), "");
}

void KXBELBookmarkImporterImpl::visitEnter(const KBookmarkGroup &grp)
{
  //kdDebug() << "KXBELBookmarkImporterImpl::visitEnter" << endl;
  emit newFolder(grp.fullText(), false, "");
}

void KXBELBookmarkImporterImpl::visitLeave(const KBookmarkGroup &)
{
  //kdDebug() << "KXBELBookmarkImporterImpl::visitLeave" << endl;
  emit endFolder();
}

void KBookmarkImporterBase::setupSignalForwards(QObject *src, QObject *dst)
{
  connect(src, SIGNAL( newBookmark( const QString &, const QString &, const QString & ) ),
          dst, SIGNAL( newBookmark( const QString &, const QString &, const QString & ) ));
  connect(src, SIGNAL( newFolder( const QString &, bool, const QString & ) ),
          dst, SIGNAL( newFolder( const QString &, bool, const QString & ) ));
  connect(src, SIGNAL( newSeparator() ),
          dst, SIGNAL( newSeparator() ) );
  connect(src, SIGNAL( endFolder() ),
          dst, SIGNAL( endFolder() ) );
}

KBookmarkImporterBase* KBookmarkImporterBase::factory( const QString &type )
{
  if (type == "netscape")
    return new KNSBookmarkImporterImpl;
  else if (type == "mozilla")
    return new KMozillaBookmarkImporterImpl;
  else if (type == "xbel")
    return new KXBELBookmarkImporterImpl;
  else if (type == "ie")
    return new KIEBookmarkImporterImpl;
  else if (type == "opera")
    return new KOperaBookmarkImporterImpl;
  else
    return 0;
}

#include <kbookmarkimporter.moc>
