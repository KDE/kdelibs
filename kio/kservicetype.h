/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kservicetype_h__
#define __kservicetype_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qmap.h>
#include <qshared.h>
#include <qdatastream.h>
#include <qproperty.h>

#include <ksharedptr.h>
#include <ksimpleconfig.h>

#include "ktypecode.h"

/**
 * A service type is the generic notion for a mimetype.
 * It is associated to services according to the user profile (kuserprofile.h)
 * and there are factories and registry entry classes for it (kregfactories.h)
 */
class KServiceType : public QShared
{
  K_TYPECODE( TC_KServiceType );
  
public:
  typedef KSharedPtr<KServiceType> Ptr;
  typedef const KSharedPtr<QProperty> PropertyPtr;

  KServiceType( const QString& _name, const QString& _icon, 
		const QString& _comment );
  KServiceType( KSimpleConfig& _cfg );
  KServiceType( QDataStream& _str ) { initStatic(); load( _str ); }
  KServiceType() { initStatic(); m_bValid = false; }
  
  virtual ~KServiceType();
  
  /** 
   * @return the icon associated with this service type. Some
   *         derived classes offer special functions which take for
   *         example an URL and returns a special icon for this
   *         URL. An example is @ref KMimeType, @ref KFoldeType and
   *         others.  
   */
  virtual QString icon() const { return m_strIcon; }
  /**
   * @return the descriptive comment associated, if any.
   */
  virtual QString comment() const { return m_strComment; }
  
  /**
   * @return the name of this service type.
   */
  virtual QString name() const { return m_strName; }

  virtual PropertyPtr property( const QString& _name ) const;
  virtual QStringList propertyNames() const;

  bool isValid() const { return m_bValid; }

  virtual QProperty::Type propertyDef( const QString& _name ) const;
  virtual QStringList propertyDefNames() const;
  virtual const QMap<QString,QProperty::Type>& propertyDefs() const { return m_mapPropDefs; }

  virtual void load( QDataStream& );
  virtual void save( QDataStream& ) const;
  
  /**  
   * @return a pointer to the servicetype '_name' or 0L if the
   *         service type is unknown.  
   */
  static KServiceType* serviceType( const QString& _name );

  /** 
   * @return a list of all the supported servicetypes. Useful for 
   *         showing the list of available mimetypes in a listbox,
   *         for example.
   */
  static const QList<KServiceType>& serviceTypes() { return *s_lstServiceTypes; }

protected:
  /**
   * Check for static variables initialised. Called by constructor.
   */
  static void initStatic();

  QString m_strName;
  QString m_strIcon;
  QString m_strComment;
  QMap<QString,QProperty> m_mapProps;
  QMap<QString,QProperty::Type> m_mapPropDefs;

  bool m_bValid;
  
  static QList<KServiceType>* s_lstServiceTypes;
};

QDataStream& operator>>( QDataStream& _str, KServiceType& s );
QDataStream& operator<<( QDataStream& _str, const KServiceType& s );

#endif
