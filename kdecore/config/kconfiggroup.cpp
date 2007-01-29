// -*- c-basic-offset: 2 -*-
/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include "kconfiggroup.h"

KConfigGroup::KConfigGroup(KConfigBase *master, const QString &_group)
{
    mGroup = _group.toUtf8();
    init(master);
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QByteArray &_group)
{
    mGroup = _group;
    init(master);
}

KConfigGroup::KConfigGroup(KConfigBase *master, const char * _group)
{
    mGroup = _group;
    init(master);
}

KConfigGroup::KConfigGroup(KSharedConfig::Ptr master, const QString &_group)
{
    mGroup = _group.toUtf8();
    init(master.data());
    mMasterShared = master;
}

KConfigGroup::KConfigGroup(KSharedConfig::Ptr master, const QByteArray &_group)
{
    mGroup = _group;
    init(master.data());
    mMasterShared = master;
}

KConfigGroup::KConfigGroup(KSharedConfig::Ptr master, const char * _group)
{
    mGroup = _group;
    init(master.data());
    mMasterShared = master;
}

void KConfigGroup::init(KConfigBase *master)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

KConfigGroup::~KConfigGroup()
{
}

void KConfigGroup::deleteGroup(WriteConfigFlags pFlags)
{
  mMaster->deleteGroup(KConfigBase::group(), pFlags);
}

bool KConfigGroup::groupIsImmutable() const
{
    return mMaster->groupIsImmutable(KConfigBase::group());
}

void KConfigGroup::setDirty(bool _bDirty)
{
  mMaster->setDirty(_bDirty);
}

void KConfigGroup::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  mMaster->putData(_key, _data, _checkGroup);
}

KEntry KConfigGroup::lookupData(const KEntryKey &_key) const
{
  return mMaster->lookupData(_key);
}

void KConfigGroup::sync()
{
  mMaster->sync();
}

QStringList KConfigGroup::groupList() const
{
  return QStringList();
}

KEntryMap KConfigGroup::internalEntryMap( const QString&) const
{
  return KEntryMap();
}

KEntryMap KConfigGroup::internalEntryMap() const
{
  return KEntryMap();
}

void KConfigGroup::virtual_hook( int id, void* data )
{ KConfigBase::virtual_hook( id, data ); }
