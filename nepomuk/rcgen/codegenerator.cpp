/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "codegenerator.h"

#include "abstractcode.h"
#include "fastcode.h"
#include "property.h"
#include "resourceclass.h"
#include "safecode.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtCore/QStringList>

extern bool quiet;

static QString headerTemplate( CodeGenerator::Mode mode )
{
    QFile gplFile( ":gpl.tpl" );
    gplFile.open( QIODevice::ReadOnly );

    QFile headerFile( QString::fromLatin1( ":header_%1.tpl" )
            .arg( mode == CodeGenerator::SafeMode ? QLatin1String( "safe" ) : QLatin1String( "fast" ) ) );
    headerFile.open( QIODevice::ReadOnly );

    QString result = QString::fromLatin1( gplFile.readAll() );
    result += QString::fromLatin1( headerFile.readAll() );

    return result;
}

static QString sourceTemplate( CodeGenerator::Mode mode )
{
    QFile gplFile( ":gpl.tpl" );
    gplFile.open( QIODevice::ReadOnly );

    QFile sourceFile( QString::fromLatin1( ":source_%1.tpl" )
            .arg( mode == CodeGenerator::SafeMode ? QLatin1String( "safe" ) : QLatin1String( "fast" ) ) );
    sourceFile.open( QIODevice::ReadOnly );

    QString result = QString::fromLatin1( gplFile.readAll() );
    result += QString::fromLatin1( sourceFile.readAll() );

    return result;
}

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

CodeGenerator::CodeGenerator( Mode mode )
    : m_mode( mode )
{
    if ( m_mode == SafeMode ) {
        m_code = new SafeCode;
        m_nameSpace = QLatin1String("Nepomuk");
    } else {
        m_code = new FastCode;
        m_nameSpace = QLatin1String("NepomukFast");
    }
}

CodeGenerator::~CodeGenerator()
{
    delete m_code;
}

bool CodeGenerator::write( const ResourceClass* resourceClass, const QString& folder ) const
{
    QFile f( folder + resourceClass->headerName() );
    if( !f.open( QIODevice::WriteOnly ) )
        return false;

    QTextStream s( &f );
    if( !writeHeader( resourceClass, s ) )
        return false;

    f.close();

    f.setFileName( folder + resourceClass->sourceName() );
    if( !f.open( QIODevice::WriteOnly ) )
        return false;

    if( !writeSource( resourceClass, s ) )
        return false;

    return true;
}

bool CodeGenerator::writeDummyClasses( const QString &folder ) const
{
    if ( m_mode == FastMode ) {
        QFile headerOutput( folder + "resource.h" );
        if ( !headerOutput.open( QIODevice::WriteOnly ) )
            return false;

        QFile sourceOutput( folder + "resource.cpp" );
        if ( !sourceOutput.open( QIODevice::WriteOnly ) )
            return false;

        QFile headerInput( ":dummyresource_header_fast.tpl" );
        headerInput.open( QIODevice::ReadOnly );

        QFile sourceInput( ":dummyresource_source_fast.tpl" );
        sourceInput.open( QIODevice::ReadOnly );

        headerOutput.write( headerInput.readAll() );
        sourceOutput.write( sourceInput.readAll() );
    }

    return true;
}

bool CodeGenerator::writeHeader( const ResourceClass *resourceClass, QTextStream& stream ) const
{
    QString s = headerTemplate( m_mode );
    if( QFile::exists( resourceClass->headerTemplateFilePath() ) ) {
        QFile f( resourceClass->headerTemplateFilePath() );
        if( !f.open( QIODevice::ReadOnly ) ) {
            qDebug() << "Failed to open " << resourceClass->headerTemplateFilePath();
            return false;
        }
        s = QTextStream( &f ).readAll();
    }
    s.replace( "NEPOMUK_RESOURCECOMMENT", writeComment( resourceClass->comment(), 0 ) );
    s.replace( "NEPOMUK_RESOURCENAMEUPPER", resourceClass->name().toUpper() );
    s.replace( "NEPOMUK_RESOURCENAME", resourceClass->name() );
    if ( m_mode == FastMode && resourceClass->parentResource()->name() == "Resource" )
        s.replace( "NEPOMUK_PARENTRESOURCE", "NepomukFast::Resource" );
    else
        s.replace( "NEPOMUK_PARENTRESOURCE", resourceClass->parentResource()->name() );

    // A resource that is not part of the currently generated stuff is supposed
    // to be installed in include/nepomuk
    if ( resourceClass->parentResource()->generateClass() ) {
        s.replace( "NEPOMUK_PARENT_INCLUDE", QString("\"%1.h\"").arg( resourceClass->parentResource()->name().toLower() ) );
    }
    else {
        if ( m_mode == SafeMode )
            s.replace( "NEPOMUK_PARENT_INCLUDE", QString("<nepomuk/%1.h>").arg( resourceClass->parentResource()->name().toLower() ) );
        else
            s.replace( "NEPOMUK_PARENT_INCLUDE", QString("\"resource.h\"") );
    }

    QString methods;
    QTextStream ms( &methods );
    QSet<QString> includes;

    QListIterator<const Property*> it( resourceClass->allProperties() );
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type().isEmpty() ) {
            if ( !quiet )
                qDebug() << "(CodeGenerator::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        if ( m_mode == SafeMode ) {
            ms << writeComment( QString("Get property '%1'. ").arg(p->name()) + p->comment(), 3*4 ) << endl;
            ms << "            " << m_code->propertyGetterDeclaration( p, resourceClass ) << ";" << endl;
            ms << endl;
        }

        ms << writeComment( QString("Set property '%1'. ").arg(p->name()) + p->comment(), 3*4 ) << endl;
        ms << "            " << m_code->propertySetterDeclaration( p, resourceClass ) << ";" << endl;
        ms << endl;

        if( p->isList() ) {
            ms << writeComment( QString("Add a value to property '%1'. ").arg(p->name()) + p->comment(), 3*4 ) << endl;
            ms << "            " << m_code->propertyAdderDeclaration( p, resourceClass ) << ";" << endl;
            ms << endl;
        }

        ms << writeComment( QString( "\\return The URI of the property '%1'." ).arg( p->name() ), 3*4 ) << endl;
        ms << "            " << "static QUrl " << p->name()[0].toLower() << p->name().mid(1) << "Uri();" << endl;
        ms << endl;

        if( !p->hasSimpleType() )
            includes.insert( p->typeString( true ) );
    }


    it = resourceClass->allReverseProperties();
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type().isEmpty() ) {
            if ( !quiet )
                qDebug() << "(CodeGenerator::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        if ( p->inverseProperty() ) {
            // we already define a reverse property. So leave the generated one out
            continue;
        }

        if ( m_mode == SafeMode ) {
            ms << writeComment( QString("Get all resources that have this resource set as property '%1'. ")
                            .arg(p->name()) + p->comment() + QString(" \\sa ResourceManager::allResourcesWithProperty"), 3*4 ) << endl;
            ms << "            " << m_code->propertyReversePropertyGetterDeclaration( p, resourceClass ) << ";" << endl;
            ms << endl;
        }

        if( !p->hasSimpleType() )
            includes.insert( p->domain()->name() );
    }


    //
    // Nepomuk does not support multiple inheritance
    // So we have to use a workaround instead
    //
    if( resourceClass->allParentResources().count() > 1 ) {
        foreach( ResourceClass* rc, resourceClass->allParentResources() ) {
            // ignore the one we derived from
            if( rc != resourceClass->parentResource() ) {
                const QString decl = m_code->resourcePseudoInheritanceDeclaration( resourceClass, rc, false );
                if ( decl.isEmpty() )
                    continue;
                ms << writeComment( QString("Nepomuk does not support multiple inheritance. Thus, to access "
                                            "properties from all parent classes helper methods like this are "
                                            "introduced. The object returned represents the exact same resource."), 3*4 ) << endl
                   << "            " << decl << ";" << endl << endl;

                includes.insert( rc->name() );
            }
        }
    }

    if ( m_mode == SafeMode ) {
        ms << writeComment( QString("Retrieve a list of all available %1 resources. "
                                    "This list consists of all resource of type %1 that are stored "
                                    "in the local Nepomuk meta data storage and any changes made locally. "
                                    "Be aware that in some cases this list can get very big. Then it might "
                                    "be better to use libKNep directly.").arg( resourceClass->name() ), 3*4 ) << endl;
        ms << "            static " << m_code->resourceAllResourcesDeclaration( resourceClass, false ) << ";" << endl;
    }

    QString includeString;
    QSetIterator<QString> includeIt( includes );
    while( includeIt.hasNext() ) {
        includeString += "        class " + includeIt.next() + ";\n";
    }

    s.replace( "NEPOMUK_OTHERCLASSES", includeString );
    s.replace( "NEPOMUK_METHODS", methods );

    stream << s;

    return true;
}

bool CodeGenerator::writeSource( const ResourceClass* resourceClass, QTextStream& stream ) const
{
    QString s = sourceTemplate( m_mode );
    if( QFile::exists( resourceClass->sourceTemplateFilePath() ) ) {
        QFile f( resourceClass->sourceTemplateFilePath() );
        if( !f.open( QIODevice::ReadOnly ) ) {
            qDebug() << "Failed to open " << resourceClass->sourceTemplateFilePath();
            return false;
        }
        s = QTextStream( &f ).readAll();
    }
    s.replace( "NEPOMUK_RESOURCENAMELOWER", resourceClass->name().toLower() );
    s.replace( "NEPOMUK_RESOURCENAME", resourceClass->name() );
    s.replace( "NEPOMUK_RESOURCETYPEURI", resourceClass->uri() );
    if ( m_mode == FastMode && resourceClass->parentResource()->name() == "Resource" )
        s.replace( "NEPOMUK_PARENTRESOURCE", "NepomukFast::Resource" );
    else
        s.replace( "NEPOMUK_PARENTRESOURCE", resourceClass->parentResource()->name() );

    QString methods;
    QStringList includes;
    QTextStream ms( &methods );

    QListIterator<const Property*> it( resourceClass->allProperties() );
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type().isEmpty() ) {
            if ( !quiet )
                qDebug() << "(CodeGenerator::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        if ( !p->hasSimpleType() ) {
            includes.append( QString( "#include \"%1.h\"" ).arg( p->typeString( true, false ).toLower() ) );
        }

        if ( m_mode == SafeMode )
            ms << m_code->propertyGetterDefinition( p, resourceClass ) << endl;

        ms << m_code->propertySetterDefinition( p, resourceClass ) << endl;
        if( p->isList() )
            ms << m_code->propertyAdderDefinition( p, resourceClass ) << endl;

        // write the static method that returns the property's Uri
        ms << "QUrl " << resourceClass->name( m_nameSpace ) << "::" << p->name()[0].toLower() << p->name().mid(1) << "Uri()" << endl
           << "{" << endl
           << "    return QUrl::fromEncoded(\"" << p->uri() << "\");" << endl
           << "}" << endl << endl;
    }

    it = resourceClass->allReverseProperties();
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type().isEmpty() ) {
            if ( !quiet )
                qDebug() << "(CodeGenerator::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        if ( p->inverseProperty() ) {
            // we already define a reverse property. So leave the generated one out
            continue;
        }

        if ( m_mode == SafeMode )
            ms << m_code->propertyReversePropertyGetterDefinition( p, resourceClass ) << endl;

        includes.append( QString( "#include \"%1\"" ).arg( p->domain()->headerName() ) );
    }

    //
    // Nepomuk does not support multiple inheritance
    // So we have to use a workaround instead
    //
    if( resourceClass->allParentResources().count() > 1 ) {
        foreach( ResourceClass* rc, resourceClass->allParentResources() ) {
            // ignore the one we derived from
            if( rc != resourceClass->parentResource() ) {
                ms << m_code->resourcePseudoInheritanceDefinition( resourceClass, rc ) << endl;
                includes.append( QString("#include \"%1.h\"").arg( rc->name().toLower() ) );
            }
        }
    }

    if ( m_mode == SafeMode )
        ms << m_code->resourceAllResourcesDefinition( resourceClass ) << endl;

    // HACK: remove duplicates and resource include
    includes = includes.toSet().toList();
    includes.removeAll( "#include \"resource.h\"" );

    s.replace( "NEPOMUK_METHODS", methods );
    s.replace( "NEPOMUK_INCLUDES", includes.join( "\n" ) );

    stream << s;

    return true;
}
