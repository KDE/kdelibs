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

#include "resourceclass.h"
#include "resourcetemplate.h"

// dummy resource for low level resource inheritance
ResourceClass* ResourceClass::s_defaultResource = new ResourceClass( "http://www.w3.org/2000/01/rdf-schema#Resource" );

static const QString s_typeComment =
"   // We always store all Resource types as plain Resource objects.\n"
"   // It does not introduce any overhead (due to the implicit sharing of\n"
"   // the data and has the advantage that we can mix setProperty calls\n"
"   // with the special Resource subclass methods.\n"
"   // More importantly Resource loads the data as Resource objects anyway.\n";


static QString writeComment( const QString& comment, int indent )
{
  static const int maxLine = 50;

  QString s;

  if( !comment.isEmpty() ) {
    s += QString().fill( ' ', indent );
    s += "/**\n"
      + QString().fill( ' ', indent+1 )
      + "* ";

    QStringList words = comment.split( QRegExp("\\s"), QString::SkipEmptyParts );
    int cnt = 0;
    for( int i = 0; i < words.count(); ++i ) {
      if( cnt >= maxLine ) {
	s += '\n'
	  + QString().fill( ' ', indent+1 )
	  + "* ";
	cnt = 0;
      }

      s += words[i] + ' ';
      cnt += words[i].length();
    }

    if( cnt > 0 )
      s += '\n';
    s += QString().fill( ' ', indent+1 )
      + "*/";
  }

  return s;
}



Property::Property()
  : list(true)
{
}


Property::Property( const QString& uri_,
		    const QString& type_ )
  : uri(uri_),
    type(type_),
    list(true)
{
}


QString Property::name() const
{
  //
  // many predicates are named "hasSomething"
  // we remove the "has" becasue setHasSomething sounds weird
  //
  QString n = uri.section( QRegExp( "[#:]" ), -1 );
  if( n.toLower().startsWith( "has" ) )
    return n.mid( 3 );
  else
    return n;
}


QString Property::typeString( bool simple, bool withNamespace ) const
{
  QString t;
  if( type.contains( "xs:" ) ) {
    // XML Schema types
    // FIXME: move this map somewhere else
    QHash<QString, QString> xmlSchemaTypes;
    xmlSchemaTypes.insert( "integer", "qint64" );
    xmlSchemaTypes.insert( "nonNegativeInteger", "quint64" );
    xmlSchemaTypes.insert( "nonPositiveInteger", "qint64" );
    xmlSchemaTypes.insert( "negativeInteger", "qint64" );
    xmlSchemaTypes.insert( "positiveInteger", "quint64" );
    xmlSchemaTypes.insert( "long", "qint64" );
    xmlSchemaTypes.insert( "unsignedLong", "quint64" );
    xmlSchemaTypes.insert( "int", "qint32" );
    xmlSchemaTypes.insert( "unsignedInt", "quint32" );
    xmlSchemaTypes.insert( "short", "qint16" );
    xmlSchemaTypes.insert( "unsignedShort", "quint16" );
    xmlSchemaTypes.insert( "byte", "char" );
    xmlSchemaTypes.insert( "unsignedByte", "unsigned char" );
    xmlSchemaTypes.insert( "float", "float" );
    xmlSchemaTypes.insert( "double", "double" );
    xmlSchemaTypes.insert( "boolean", "bool" );
    xmlSchemaTypes.insert( "dateTime", "QDateTime" );
    t = xmlSchemaTypes[type.mid(type.indexOf( "xs:" ) + 3 )];
  }
  else if( type.endsWith( "#Literal" ) ) {
    t = "QString";
  }
  else {
    t = type.section( QRegExp( "[#:]" ), -1 );
    if( withNamespace )
      t.prepend( "Nepomuk::KMetaData::" );
  }

  if( !simple && list ) {
    if( t == "QString" )
      return "QStringList";
    else
      return "QList<" + t + ">";
  }
  else
    return t;
}


bool Property::hasSimpleType() const
{
  return ( type.contains( "xs:" ) || type.endsWith( "#Literal" ) );
}


QString Property::setterDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
  return QString( "void %1set%2%3%4( const %5& value )" )
    .arg( withNamespace ? QString("Nepomuk::KMetaData::%1::").arg(rc->name()) : QString() )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( list ? QString("s") : QString() )
    .arg( typeString( false, withNamespace ) );
}


QString Property::getterDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
  return QString( "%1 %2get%3%4%5() const" )
    .arg( typeString( false, withNamespace ) )
    .arg( withNamespace ? QString("Nepomuk::KMetaData::%1::").arg(rc->name()) : QString() )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( list ? QString("s") : QString() );
}


QString Property::adderDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
  return QString( "void %1add%2%3( const %4& value )" )
    .arg( withNamespace ? QString("Nepomuk::KMetaData::%1::").arg(rc->name()) : QString() )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( typeString( true, withNamespace ) );
}


QString Property::reversePropertyGetterDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
  return QString( "%1 %2%3Of() const" )
    .arg( ( list ? QString("QList<") : QString() ) + domain->name( withNamespace ) + ( list ? QString(">") : QString() ) )
    .arg( withNamespace ? QString("Nepomuk::KMetaData::%1::").arg(rc->name()) : QString() )
    .arg( name() );
}


QString Property::setterDefinition( const ResourceClass* rc ) const
{
  QString s = setterDeclaration( rc, true ) + "\n";

  if( hasSimpleType() || typeString( true ) == "Resource" || !list ) {
    s += QString("{\n"
		 "   setProperty( \"%1\", Variant( value ) );\n"
		 "}\n" )
      .arg( uri );
  }
  else if( list ) {
    s += QString("{\n"
		 "%1"
		 "   QList<Resource> l;\n"
		 "   for( %2::const_iterator it = value.constBegin();\n"
		 "        it != value.constEnd(); ++it )\n"
		 "      l.append( Resource( (*it) ) );\n"
		 "   setProperty( \"%3\", Variant( l ) );\n"
		 "}\n" )
      .arg( s_typeComment )
      .arg( typeString() )
      .arg( uri );
  }
  else {
    s += QString("{\n"
		 "%1"
		 "   setProperty( \"%2\", Variant( Resource( value ) ) );\n"
		 "}\n" )
      .arg( s_typeComment )
      .arg( uri );
  }

  return s;
}


QString Property::getterDefinition( const ResourceClass* rc ) const
{
  QString s = getterDeclaration( rc, true ) + "\n";

  if( hasSimpleType() ) {
    // string lists have to be handled separately
    if( typeString( false ) == "QStringList" )
      s += QString( "{\n"
		    "   return getProperty( \"%1\" ).toStringList();\n"
		    "}\n" )
	.arg( uri );
    else if( list )
      s += QString( "{\n"
		    "   return getProperty( \"%2\" ).listValue<%1>();\n"
		    "}\n" )
	.arg( typeString( true ) )
	.arg( uri );
    else
      s += QString( "{\n"
		    "   return getProperty( \"%2\" ).value<%1>();\n"
		    "}\n" )
	.arg( typeString( true ) )
	.arg( uri );
  }
  else if( list ) {
    s += QString("{\n"
		 "%1"
		 "   return convertResourceList<%3>( getProperty( \"%2\" ).toResourceList() );\n"
		 "}\n" )
      .arg( s_typeComment )
      .arg( uri )
      .arg( typeString( true ) );
  }
  else {
    s += QString("{\n"
		 "%1"
		 "   return %2( getProperty( \"%3\" ).toResource().uri() );\n"
		 "}\n" )
      .arg( s_typeComment )
      .arg( typeString( true ) )
      .arg( uri );
  }

  return s;
}


QString Property::adderDefinition( const ResourceClass* rc ) const
{
  QString s = adderDeclaration( rc, true ) + "\n";

  if( hasSimpleType() ) {
    s += QString( "{\n"
		  "   Variant v = getProperty( \"%1\" );\n"
		  "   v.append( value );\n"
		  "   setProperty( \"%1\", v );\n"
		  "}\n" )
      .arg( uri );
  }
  else {
    s += QString( "{\n"
		  "%1"
		  "   Variant v = getProperty( \"%2\" );\n"
		  "   v.append( Resource( value ) );\n"
		  "   setProperty( \"%2\", v );\n"
		  "}\n" )
      .arg( s_typeComment )
      .arg( uri );
  }

  return s;
}


QString Property::reversePropertyGetterDefinition( const ResourceClass* rc ) const
{
  QString s = reversePropertyGetterDeclaration( rc, true ) + "\n";

  if( list )
    s += QString( "{\n"
		  "   return convertResourceList<%2>( ResourceManager::instance()->allResourcesWithProperty( \"%1\", *this ) );\n"
		  "}\n" )
      .arg( uri )
      .arg( domain->name() );
  else
    s += QString( "{\n"
		  "   QList<Resource> resources = ResourceManager::instance()->allResourcesWithProperty( \"%1\", *this );\n"
		  "   if( resources.isEmpty() )\n"
		  "      return %2();\n"
		  "   else\n"
		  "      return %2( resources.getFirst().uri() );\n"
		  "}\n" )
      .arg( uri )
      .arg( domain->name() );

  return s;
}



ResourceClass::ResourceClass()
  : parent( s_defaultResource )
{
}


ResourceClass::ResourceClass( const QString& uri_ )
  : parent( s_defaultResource ),
    uri( uri_ )
{
}


ResourceClass::~ResourceClass()
{
}


QString ResourceClass::name( bool withNamespace ) const
{
  QString s = uri.section( QRegExp( "[#:]" ), -1 );
  if( withNamespace )
    s.prepend( "Nepomuk::KMetaData::" );
  return s;
}


QString ResourceClass::headerName() const
{
  return name().toLower() + ".h";
}


QString ResourceClass::sourceName() const
{
  return name().toLower() + ".cpp";
}


QString ResourceClass::allResourcesDeclaration( bool withNamespace ) const
{
  return QString( "QList<%1%2> %3all%2s()" )
    .arg( withNamespace ? QString("Nepomuk::KMetaData::") : QString() )
    .arg( name() )
    .arg( withNamespace ? QString("Nepomuk::KMetaData::%1::").arg( name() ) : QString() );
}


QString ResourceClass::allResourcesDefinition() const
{
  return QString( "%1\n"
		  "{\n"
		  "   return Nepomuk::KMetaData::convertResourceList<%3>( ResourceManager::instance()->allResourcesOfType( \"%2\" ) );\n"
		  "}\n" )
    .arg( allResourcesDeclaration( true ) )
    .arg( uri )
    .arg( name() );
}


bool ResourceClass::writeHeader( QTextStream& stream ) const
{
  QString s = headerTemplate;
  s.replace( "RESOURCECOMMENT", writeComment( comment, 0 ) );
  s.replace( "RESOURCENAMEUPPER", name().toUpper() );
  s.replace( "RESOURCENAME", name() );
  s.replace( "PARENTRESOURCELOWER", parent->name().toLower() );
  s.replace( "PARENTRESOURCE", parent->name() );

  QString methods;
  QTextStream ms( &methods );
  QSet<QString> includes;

  QListIterator<const Property*> it( properties );
  while( it.hasNext() ) {
    const Property* p = it.next();

    if( p->type.isEmpty() ) {
      qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
      continue;
    }

    ms << writeComment( QString("Get property '%1'. ").arg(p->name()) + p->comment, 3 ) << endl;
    ms << "   " << p->getterDeclaration( this ) << ";" << endl;
    ms << endl;

    ms << writeComment( QString("Set property '%1'. ").arg(p->name()) + p->comment, 3 ) << endl;
    ms << "   " << p->setterDeclaration( this ) << ";" << endl;
    ms << endl;

    if( p->list ) {
      ms << writeComment( QString("Add a value to property '%1'. ").arg(p->name()) + p->comment, 3 ) << endl;
      ms << "   " << p->adderDeclaration( this ) << ";" << endl;
      ms << endl;
    }

    if( !p->hasSimpleType() )
      includes.insert( p->typeString( true ) );
  }


  it = reverseProperties;
  while( it.hasNext() ) {
    const Property* p = it.next();

    if( p->type.isEmpty() ) {
      qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
      continue;
    }

    ms << writeComment( QString("Get all resources that have this resource set as property '%1'. ")
			.arg(p->name()) + p->comment + QString(" \\sa ResourceManager::allResourcesWithProperty"), 3 ) << endl;
    ms << "   " << p->reversePropertyGetterDeclaration( this ) << ";" << endl;    
    ms << endl;

    includes.insert( p->domain->name() );
  }


  ms << writeComment( QString("Retrieve a list of all available %1 resources. "
			      "This list consists of all resource of type %1 that are stored "
			      "in the local Nepomuk meta data storage and any changes made locally. "
			      "Be aware that in some cases this list can get very big. Then it might "
			      "be better to use libKNep directly.").arg( name() ), 3 ) << endl;
  ms << "   static " << allResourcesDeclaration( false ) << ";" << endl;

  QString includeString;
  QSetIterator<QString> includeIt( includes );
  while( includeIt.hasNext() ) {
    includeString += "class " + includeIt.next() + ";\n";
  }

  s.replace( "OTHERCLASSES", includeString );
  s.replace( "METHODS", methods );

  stream << s;

  return true;
}


bool ResourceClass::writeSource( QTextStream& stream ) const
{
  QString s = sourceTemplate;
  s.replace( "RESOURCENAMELOWER", name().toLower() );
  s.replace( "RESOURCENAME", name() );
  s.replace( "RESOURCETYPEURI", uri );
  s.replace( "PARENTRESOURCE", parent->name() );

  QString methods;
  QTextStream ms( &methods );

  QListIterator<const Property*> it( properties );
  while( it.hasNext() ) {
    const Property* p = it.next();

    if( p->type.isEmpty() ) {
      qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
      continue;
    }

    ms << p->getterDefinition( this ) << endl
       << p->setterDefinition( this ) << endl;
    if( p->list )
      ms << p->adderDefinition( this ) << endl;
  }

  it = reverseProperties;
  while( it.hasNext() ) {
    const Property* p = it.next();

    if( p->type.isEmpty() ) {
      qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
      continue;
    }

    ms << p->reversePropertyGetterDefinition( this ) << endl;
  }

  ms << allResourcesDefinition() << endl;

  s.replace( "METHODS", methods );

  stream << s;

  return true;
}


bool ResourceClass::write( const QString& folder ) const
{
  QFile f( folder + headerName() );
  if( !f.open( QIODevice::WriteOnly ) )
    return false;

  QTextStream s( &f );
  if( !writeHeader( s ) )
    return false;

  f.close();

  f.setFileName( folder + sourceName() );
  if( !f.open( QIODevice::WriteOnly ) )
    return false;

  if( !writeSource( s ) )
    return false;

  return true;
}
