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

class KServiceGroup : public KSycocaEntry
{
  friend KBuildServiceGroupFactory;
  K_SYCOCATYPE( KST_KServiceGroup, KSycocaEntry )

public:
  typedef KSharedPtr<KServiceGroup> Ptr;
  typedef KSharedPtr<KSycocaEntry> SPtr;
  typedef QValueList<SPtr> List;
public:

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
   */
  virtual List entries();

  static Ptr root();
  static Ptr group(const QString &relPath);

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
};
#endif
