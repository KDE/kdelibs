/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2001 Waldo Bastian <bastian@kde.org>

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
#ifndef KPROTOCOLINFOPRIVATE_H
#define KPROTOCOLINFOPRIVATE_H

#include "kprotocolinfo.h"

#include <ksycocaentry_p.h>

class KProtocolInfoPrivate : public KSycocaEntryPrivate
{
public:
    K_SYCOCATYPE( KST_KProtocolInfo, KSycocaEntryPrivate )

    KProtocolInfoPrivate(const QString &path, KProtocolInfo *q_)
        : KSycocaEntryPrivate(path), q(q_)
    {
    }
    KProtocolInfoPrivate(QDataStream& _str, int offset, KProtocolInfo *q_)
        : KSycocaEntryPrivate(_str, offset), q(q_)
    {
    }

    virtual void save(QDataStream &s);

    virtual QString name() const
    {
        return q->m_name;
    }


  KProtocolInfo *q;
  QString docPath;
  QString protClass;
  QStringList archiveMimetype;
  KProtocolInfo::ExtraFieldList extraFields;
  bool showPreviews : 1;
  bool canRenameFromFile : 1;
  bool canRenameToFile : 1;
  bool canDeleteRecursive : 1;
  bool fileNameUsedForCopying : 1; // true if using UDS_NAME, false if using KUrl::fileName() [default]
  //KUrl::URIMode uriMode;
  QStringList capabilities;
  QString proxyProtocol;
  int maxSlavesPerHost;
};


#endif
