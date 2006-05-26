/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                       1999 Waldo Bastian <bastian@kde.org>

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

#ifndef __kservicetype_h__
#define __kservicetype_h__

#include "ksycocaentry.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qvariant.h>

#include <ksimpleconfig.h>

class KDesktopFile;

/**
 * A service type is the generic notion for a mimetype, a type of service
 * instead of a type of file.
 * For instance, KOfficeFilter is a service type.
 * It is associated to services according to the user profile (kservicetypeprofile.h).
 * Service types are stored as desktop files in $KDEHOME/share/servicetypes.
 * @see KService
 */
class KIO_EXPORT KServiceType : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KServiceType, KSycocaEntry )

public:
  typedef KSharedPtr<KServiceType> Ptr;
  typedef QList<Ptr> List;
public:

  /**
   * Constructor.  You may pass in arguments to create a servicetype with
   * specific properties.
   * @param _fullpath the path of the service type's desktop file
   * @param _name the name of the service type
   * @param _comment a comment (can be empty)
   */
  KServiceType( const QString & _fullpath, const QString& _name,
                const QString& _comment);

  /**
   * Construct a service type and take all information from a config file.
   * @param _fullpath path of the desktop file, set to "" if calling from
   *                  a inherited constructor.
   */
  explicit KServiceType( const QString & _fullpath );

  /**
   * Construct a service type and take all information from a desktop file.
   * @param config the configuration file
   */
  explicit KServiceType( KDesktopFile *config);

  /**
   * @internal construct a service from a stream.
   * The stream must already be positionned at the correct offset
   */
  KServiceType( QDataStream& _str, int offset );

  virtual ~KServiceType();

  /**
   * Returns the descriptive comment associated, if any.
   * @return the comment, or QString()
   */
  QString comment() const { return m_strComment; }

  /**
   * Returns the name of this service type.
   * @return the name of the service type
   */
  QString name() const { return m_strName; }

  /**
   * Returns the relative path to the desktop entry file responsible for
   *         this servicetype.
   * For instance inode/directory.desktop, or kpart.desktop
   * @return the path of the desktop file
   */
  QString desktopEntryPath() const { return entryPath(); }

  /**
   * Checks whether this service type inherits another one.
   * @return true if this service type inherits another one
   * @see parentServiceType()
   */
  bool isDerived() const { return m_bDerived; }

  /**
   * If this service type inherits from another service type,
   * return the name of the parent.
   * @return the parent service type, or QString:: null if not set
   * @see isDerived()
   */
  QString parentServiceType() const;

  /**
   * Checks whether this service type is or inherits from @p servTypeName.
   * @return true if this servicetype is or inherits from @p servTypeName
   */
  bool inherits( const QString& servTypeName ) const;

  /**
   * Returns the requested property. Some often used properties
   * have convenience access functions like name(),
   * comment() etc.
   *
   * @param _name the name of the property
   * @return the property, or invalid if not found
   */
  virtual QVariant property( const QString& _name ) const;

  /**
   * Returns the list of all properties of this service type.
   * @return the list of properties
   */
  virtual QStringList propertyNames() const;

  /**
   * Checks whether the service type is valid.
   * @return true if the service is valid (e.g. name is not empty)
   */
  bool isValid() const { return m_bValid; }

  /**
   * Returns the type of the property with the given @p _name.
   *
   * @param _name the name of the property
   * @return the property type, or null if not found
   */
  virtual QVariant::Type propertyDef( const QString& _name ) const;

  virtual QStringList propertyDefNames() const;
  virtual const QMap<QString,QVariant::Type>& propertyDefs() const { return m_mapPropDefs; }

  /**
   * @internal
   * Save ourselves to the data stream.
   */
  virtual void save( QDataStream& );

  /**
   * @internal
   * Load ourselves from the data stream.
   */
  virtual void load( QDataStream& );

  /**
   * @internal
   * Pointer to parent service type
   */
  Ptr parentType();
  /**
   * @internal  only used by kbuildsycoca
   * Register offset into offers list
   */
  void setServiceOffersOffset( int offset );
  /**
   * @internal  only used by kbuildsycoca
   */
  int serviceOffersOffset() const;

  /**
   * Returns a pointer to the servicetype '_name' or 0L if the
   *         service type is unknown.
   * VERY IMPORTANT : don't store the result in a KServiceType * !
   * @param _name the name of the service type to search
   * @return the pointer to the service type, or 0
   */
  static Ptr serviceType( const QString& _name );

  /**
   * Returns a list of all the supported servicetypes. Useful for
   *         showing the list of available servicetypes in a listbox,
   *         for example.
   * More memory consuming than the ones above, don't use unless
   * really necessary.
   * @return the list of all services
   */
  static List allServiceTypes();

protected:
  void init( KDesktopFile *config );

protected: // used by KMimeType
  QMap<QString,QVariant> m_mapProps;

private:
  QString m_strName;
  QString m_strComment;
  int m_serviceOffersOffset;
  QMap<QString,QVariant::Type> m_mapPropDefs;
  bool m_bValid;
  bool m_bDerived;
  bool m_parentTypeLoaded;
  bool m_unused; // for future usage
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServiceTypePrivate;
  KServiceTypePrivate* d;
};

//QDataStream& operator>>( QDataStream& _str, KServiceType& s );
//QDataStream& operator<<( QDataStream& _str, KServiceType& s );

#endif
