/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                       1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kservicetype_h__
#define __kservicetype_h__

#include "ksycocaentry.h"
#include "kservice.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qmap.h>
#include <qshared.h>
#include <qdatastream.h>
#include <qvariant.h>

#include <ksimpleconfig.h>

/**
 * A service type is the generic notion for a mimetype, a type of service
 * instead of a type of file.
 * For instance, KOfficeFilter is a service type.
 * It is associated to services according to the user profile (kuserprofile.h)
 */
class KServiceType : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KServiceType, KSycocaEntry )

public:
  typedef KSharedPtr<KServiceType> Ptr;
  typedef QValueList<Ptr> List;
public:

  /**
   * Constructor.  You may pass in arguments to create a servicetype with
   * specific properties
   */
  KServiceType( const QString & _fullpath, const QString& _name,
                const QString& _icon, const QString& _comment);
  /**
   * Construct a service type and take all informations from a config file.
   * @param _fullpath set to "" if calling from a inherited constructor.
   */
  KServiceType( const QString & _fullpath );

  /**
   * Construct a service type and take all informations from a deskop file.
   */
  KServiceType( KDesktopFile *config);

  /**
   * @internal construct a service from a stream.
   * The stream must already be positionned at the correct offset
   */
  KServiceType( QDataStream& _str, int offset );

  virtual ~KServiceType();

  /**
   * @return the icon associated with this service type. Some
   *         derived classes offer special functions which take for
   *         example an URL and returns a special icon for this
   *         URL. An example is @ref KMimeType, @ref KFolderType and
   *         others.
   */
  QString icon() const { return m_strIcon; }

  /**
   * @return the descriptive comment associated, if any.
   */
  QString comment() const { return m_strComment; }

  /**
   * @return the name of this service type.
   */
  QString name() const { return m_strName; }

  /**
   * @return the relative path to the desktop entry file responsible for
   *         this servicetype.
   * For instance inode/directory.desktop, or kpart.desktop
   */
  QString desktopEntryPath() const { return entryPath(); }

  /**
   * @return true if this service type inherits another one
   */
  bool isDerived() const { return m_bDerived; }

  /**
   * If this service type inherits from another service type,
   * return the name of the parent. Otherwise QString::null.
   */
  QString parentServiceType() const;

  virtual QVariant property( const QString& _name ) const;
  virtual QStringList propertyNames() const;

  bool isValid() const { return m_bValid; }

  virtual QVariant::Type propertyDef( const QString& _name ) const;
  virtual QStringList propertyDefNames() const;
  virtual const QMap<QString,QVariant::Type>& propertyDefs() const { return m_mapPropDefs; }

  /**
   * Save ourselves to the data stream.
   */
  virtual void save( QDataStream& );

  /**
   * Load ourselves from the data stream.
   */
  virtual void load( QDataStream& );

  /**
   * @return a pointer to the servicetype '_name' or 0L if the
   *         service type is unknown.
   * VERY IMPORTANT : don't store the result in a KServiceType * !
   */
  static Ptr serviceType( const QString& _name );

  /**
   * @return all services supporting the given servicetype name
   * This doesn't take care of the user profile.
   * In fact it is used by KServiceTypeProfile,
   * which is used by KTrader, and that's the one you should use.
   */
  static KService::List offers( const QString& _servicetype );

  /**
   * @return a list of all the supported servicetypes. Useful for
   *         showing the list of available servicetypes in a listbox,
   *         for example.
   * More memory consuming than the ones above, don't use unless
   * really necessary.
   */
  static List allServiceTypes();

protected:
  void init( KDesktopFile *config );

protected:
  QString m_strName;
  QString m_strIcon;
  QString m_strComment;
  QMap<QString,QVariant> m_mapProps;
  QMap<QString,QVariant::Type> m_mapPropDefs;

  bool m_bValid:1;
  bool m_bDerived:1;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServiceTypePrivate* d;
};

//QDataStream& operator>>( QDataStream& _str, KServiceType& s );
//QDataStream& operator<<( QDataStream& _str, KServiceType& s );

#endif
