/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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

#ifndef KNEWSTUFF2_DXS_ENGINE_H
#define KNEWSTUFF2_DXS_ENGINE_H

#include <knewstuff2/core/coreengine.h>

namespace KNS {

class Dxs;

class KNEWSTUFF_EXPORT DxsEngine : public CoreEngine
{
    Q_OBJECT
  public:
    DxsEngine();
    ~DxsEngine();

    enum Policy
    {
      DxsNever,
      DxsIfPossible,
      DxsAlways
    };

    void setDxsPolicy(Policy policy);

    void loadEntries(Provider *provider);
    //void downloadPreview(Entry *entry);
    //void downloadPayload(Entry *entry);
    // FIXME: the upload/download stuff is only necessary when we use
    // integrated base64-encoded files; maybe delay to later version?

    //bool uploadEntry(Provider *provider, Entry *entry);

  //signals:
    // FIXME: we need a lot more signals here for DXS!

  //private slots:
    // FIXME: idem for slots
    void slotEntriesLoaded(KNS::Entry::List list);
    void slotEntriesFailed();

  private:
    Dxs *m_dxs;
    Policy m_dxspolicy;
};

}

#endif
