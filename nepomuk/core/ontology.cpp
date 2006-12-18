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

#include "ontology.h"
#include "variant.h"

const QString Nepomuk::KMetaData::Ontology::RDF_NAMESPACE = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const QString Nepomuk::KMetaData::Ontology::RDFS_NAMESPACE = "http://www.w3.org/2000/01/rdf-schema#";

// FIXME: insert final NRL namespace
const QString Nepomuk::KMetaData::Ontology::NRL_NAMESPACE = "http://semanticdesktop.org/ontology/nrl-yyyymmdd#";



class Nepomuk::KMetaData::Ontology::Private
{
public:
  QStringList types;
  QHash<QString, QStringList> properties;
};


#include <kmetadata/ontology_ctor.cpp>


QString Nepomuk::KMetaData::Ontology::defaultNamespace() const
{
  // FIXME: replace with the real thing
  return NRL_NAMESPACE;
}


QString Nepomuk::KMetaData::Ontology::defaultType() const
{
  static QString s = RDFS_NAMESPACE + QString("Resource");
  return s;
}


QStringList Nepomuk::KMetaData::Ontology::types() const
{
  return d->types;
}


QStringList Nepomuk::KMetaData::Ontology::properties( const QString& type ) const
{
  if( type.isEmpty() ) {
    QStringList l;
    for( QHash<QString, QStringList>::const_iterator it = d->properties.constBegin();
	 it != d->properties.constEnd(); ++it )
      l += it.value();
    return l;
  }
  else
    return d->properties[type];
}


QString Nepomuk::KMetaData::Ontology::typeName( const QString& type ) const
{
  // FIXME: use translated names generated from the ontology
  return type.section( QRegExp( "[#:]" ), -1 );
}


QString Nepomuk::KMetaData::Ontology::propertyName( const QString& property ) const
{
  // FIXME: use translated names generated from the ontology
  QString n = property.section( QRegExp( "[#:]" ), -1 );
  if( n.toLower().startsWith( "has" ) )
    return n.mid( 3 );
  else
    return n;
}
