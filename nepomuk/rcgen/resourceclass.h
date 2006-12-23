/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _NEPOMUK_RESOURCE_CLASS_H_
#define _NEPOMUK_RESOURCE_CLASS_H_

#include <QtCore>

class ResourceClass;

class Property
{
 public:
  Property();
  Property( const QString& uri,
	    const QString& type );

  /**
   * The uri of the property
   */
  QString uri;

  /**
   * The scope of the property
   */
  QString type;

  QString comment;

  bool list;

  ResourceClass* domain;

  QString name() const;
  QString typeString( bool simple = false, bool withNamespace = false ) const;
  QString setterDeclaration( const ResourceClass* rc, bool withNamespace = false ) const;
  QString getterDeclaration( const ResourceClass* rc, bool withNamespace = false ) const;
  QString adderDeclaration( const ResourceClass* rc, bool withNamespace = false ) const;
  QString reversePropertyGetterDeclaration( const ResourceClass* rc, bool withNamespace = false ) const;

  QString setterDefinition( const ResourceClass* rc ) const;
  QString getterDefinition( const ResourceClass* rc ) const;
  QString adderDefinition( const ResourceClass* rc ) const;
  QString reversePropertyGetterDefinition( const ResourceClass* rc ) const;

  bool hasSimpleType() const;
};


class ResourceClass
{
 public:
  ResourceClass();
  ResourceClass( const QString& uri );
  ~ResourceClass();

  ResourceClass* parent;

  QString name( bool withNamespace = false ) const;
  QString uri;
  QString comment;

  QList<const Property*> properties;
  QList<const Property*> reverseProperties;

  QString headerName() const;
  QString sourceName() const;

  bool writeHeader( QTextStream& ) const;
  bool writeSource( QTextStream& ) const;

  QString allResourcesDeclaration( bool withNamespace = false ) const;
  QString allResourcesDefinition() const;

  bool write( const QString& folder ) const;

  static ResourceClass* s_defaultResource;
};

#endif
