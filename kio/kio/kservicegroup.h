/* This file is part of the KDE project
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kservicegroup_h__
#define __kservicegroup_h__

#include <qlist.h>
#include <qstring.h>
#include <qshared.h>
#include <qdatastream.h>
#include <qvariant.h>

#include <kdesktopfile.h>

#include "ksycocaentry.h"
#include "ksycocatype.h"
#include "kservice.h"

class KBuildServiceGroupFactory;

/**
 * This class is typically used like this:
 *
 * // Lookup screensaver group
 * KServiceGroup::Ptr group = KServiceGroup::baseGroup("screensavers");
 * if (!group || !group->isValid()) return;
 *
 * KServiceGroup::List list = group->entries();
 *
 * // Iterate over all entries in the group
 * for( KServiceGroup::List::ConstIterator it = list.begin();
 *      it != list.end(); it++)
 * {
 *    KSycocaEntry *p = (*it);
 *    if (p->isType(KST_KService))
 *    {
 *       KService *s = static_cast<KService *>(p);
 *       printf("Name = %s\n", s->name().latin1());
 *    }
 *    else if (p->isType(KST_KServiceGroup))
 *    {
 *       KServiceGroup *g = static_cast<KServiceGroup *>(p);
 *       // Sub group ...
 *    }
 * }
 */

class KServiceGroup : public KSycocaEntry
{
  friend class KBuildServiceGroupFactory;
  K_SYCOCATYPE( KST_KServiceGroup, KSycocaEntry )

public:
  typedef KSharedPtr<KServiceGroup> Ptr;
  typedef KSharedPtr<KSycocaEntry> SPtr;
  typedef QValueList<SPtr> List;
public:
  /**
   * Construct a dummy servicegroup indexed with @p name
   */
  KServiceGroup( const QString & name );
  
  /**
   * Construct a service and take all informations from a config file
   * @param _fullpath full path to the config file
   * @param _relpath relative path to the config file
   */
  KServiceGroup( const QString & _fullpath, const QString & _relpath );

  /**
   * @internal construct a service from a stream.
   * The stream must already be positionned at the correct offset
   */
  KServiceGroup( QDataStream& _str, int offset, bool deep );

  virtual ~KServiceGroup();

  /**
   * @return true
   */
  bool isValid() const { return true; }

  /**
   * Name used for indexing.
   */
  virtual QString name() const { return entryPath(); }
  virtual QString relPath() const { return entryPath(); }

  /**
   * @return the caption of this group
   */
  QString caption() const { return m_strCaption; }

  /**
   * @return the icon associated with the group.
   */
  QString icon() const { return m_strIcon; }

  /**
   * @return the descriptive comment for the group, if there is one.
   */
  QString comment() const { return m_strComment; }

  /**
   * @return the total number of displayable services in this group and
   * any of its subgroups.
   */
  int childCount();

  /**
   * @return true if the NoDisplay flag was set, i.e. if this
   * group should be hidden from menus, while still being in ksycoca.
   */
  bool noDisplay() const;

  /**
   * @internal
   * Load the service from a stream.
   */
  virtual void load( QDataStream& );
  /**
   * @internal
   * Save the service to a stream.
   */
  virtual void save( QDataStream& );

  /**
   * List of all Services and ServiceGroups within this
   * ServiceGroup
   * @param sorted indicates whether to sort items
   * @param whether to include items marked "NoDisplay"
   */
  virtual List entries(bool sorted, bool excludeNoDisplay);
  /**
   * As above with excludeNoDisplay true.
   */
  virtual List entries(bool sorted = false);

  /**
   * @return non-empty string if the group is a special base group.
   * By default, "Settings/" is the kcontrol base group ("settings")
   * and "System/Screensavers/" is the screensavers base group ("screensavers").
   * This allows moving the groups without breaking those apps.
   *
   * The base group is defined by the X-KDE-BaseGroup key
   * in the .directory file.
   */
  QString baseGroupName() const { return m_strBaseGroupName; }

  /**
   * @return the group for the given baseGroupName
   * Can return 0L if the directory (or the .directory file) was deleted.
   */
  static Ptr baseGroup( const QString &baseGroupName );

  static Ptr root();
  static Ptr group(const QString &relPath);

  /**
   * @return the group of services that have X-KDE-ParentApp equal
   * to @p parent
   */
  static Ptr childGroup(const QString &parent);

protected:
  /**
   * @internal
   * Add a service to this group
   */
  void addEntry( KSycocaEntry *entry);

  QString m_strCaption;
  QString m_strIcon;
  QString m_strComment;

  List m_serviceList;
  bool m_bDeep;
  QString m_strBaseGroupName;
  int m_childCount;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class Private;
  Private* d;
};
#endif
