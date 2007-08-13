//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef __kbookmarkimporter_ns_h
#define __kbookmarkimporter_ns_h

#include <QtCore/QStringList>
#include <kconfig.h>
#include <kdemacros.h>

#include "kbookmarkimporter.h"
#include "kbookmarkexporter.h"

/**
 * A class for importing NS bookmarks
 * utf8 defaults to off
 */
class KIO_EXPORT KNSBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KNSBookmarkImporterImpl() : m_utf8(false) { }
    void setUtf8(bool utf8) { m_utf8 = utf8; }
    virtual void parse();
    virtual QString findDefaultLocation(bool forSaving = false) const;
private:
    bool m_utf8;
    class KNSBookmarkImporterImplPrivate *d;
};

/**
 * A class for importing Mozilla bookmarks
 * utf8 defaults to on
 */
class KIO_EXPORT KMozillaBookmarkImporterImpl : public KNSBookmarkImporterImpl
{
public:
    KMozillaBookmarkImporterImpl() { setUtf8(true); }
private:
    class KMozillaBookmarkImporterImplPrivate *d;
};

class KIO_EXPORT KNSBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KNSBookmarkExporterImpl(KBookmarkManager* mgr, const QString & fileName)
      : KBookmarkExporterBase(mgr, fileName)
    { ; }
    virtual ~KNSBookmarkExporterImpl() {}
    virtual void write(const KBookmarkGroup &parent);
    void setUtf8(bool);
protected:
    QString folderAsString(const KBookmarkGroup &parent) const;
private:
    bool m_utf8;
    class KNSBookmarkExporterImplPrivate *d;
};

#endif
