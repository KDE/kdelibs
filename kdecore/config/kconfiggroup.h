/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

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

#ifndef KCONFIGGROUP_H
#define KCONFIGGROUP_H

#include <kdelibs_export.h>
#include "kconfig.h"

/**
 * A KConfigBase derived class for one specific group in a KConfig object.
 */
class KDECORE_EXPORT KConfigGroup: public KConfigBase
{
public:
   /**
    * Construct a config group corresponding to @p group in @p master.
    * @p group is the group name encoded in UTF-8.
    */
   KConfigGroup(KConfigBase *master, const QByteArray &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    */
   KConfigGroup(KConfigBase *master, const QString &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    * @p group is the group name encoded in UTF-8.
    */
   KConfigGroup(KConfigBase *master, const char * group);
   KConfigGroup(KSharedConfig::Ptr master, const QByteArray &group);
   KConfigGroup(KSharedConfig::Ptr master, const QString &group);
   KConfigGroup(KSharedConfig::Ptr master, const char * group);

   ~KConfigGroup();

   /**
    * Delete all entries in the entire group
    * @param pFlags flags passed to KConfigBase::deleteGroup
    */
   void deleteGroup(WriteConfigFlags pFlags=Normal);

   /**
   * Checks whether it is possible to change this group.
   * @return whether changes may be made to this group in this configuration
   * file.
   */
  bool groupIsImmutable() const;

  /**
   * Returns a map (tree) of entries for all entries in this group.
   *
   * Only the actual entry string is returned, none of the
   * other internal data should be included.
   *
   * @return A map of entries in this group, indexed by key.
   */
  QMap<QString, QString> entryMap() const;

   // The following functions are reimplemented:
   virtual void setDirty(bool _bDirty);
   virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true);
   virtual KEntry lookupData(const KEntryKey &_key) const;
   virtual void sync();

private:
   // Hide the following members:
   void setGroup() { }
   void setDesktopGroup() { }
   void group() { }
   void hasGroup() { }
   void setReadOnly(bool) { }
   void isDirty() { }

   // The following members are not used.
   virtual QStringList groupList() const;
   virtual void rollback(bool) { }
   virtual void reparseConfiguration() { }
   virtual QMap<QString, QString> entryMap(const QString &) const
    { return QMap<QString,QString>(); }
   virtual KEntryMap internalEntryMap( const QString&) const;
   virtual KEntryMap internalEntryMap() const;
   virtual bool internalHasGroup(const QByteArray &) const
    { return false; }

   void getConfigState() { }
   void init(KConfigBase *master);

   KConfigBase *mMaster;
   KSharedConfig::Ptr mMasterShared;
protected:
   /** Virtual hook, used to add new "virtual" functions while maintaining
       binary compatibility. Unused in this class.
   */
   virtual void virtual_hook( int id, void* data );
private:
   Q_DISABLE_COPY(KConfigGroup)
   class Private;
   Private* d;
};

#endif // KCONFIGGROUP_H
