/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007-2010 Sebastian Trueg <trueg@kde.org>

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

#include "entitymanager.h"
#include "class_p.h"
#include "property_p.h"
#include "ontology_p.h"

#include "resourcemanager.h"

#include <QtCore/QMutexLocker>

Q_GLOBAL_STATIC( Nepomuk::Types::EntityManager, entityManager )

Nepomuk::Types::EntityManager::EntityManager()
{
}


QExplicitlySharedDataPointer<Nepomuk::Types::ClassPrivate> Nepomuk::Types::EntityManager::getClass( const QUrl& uri )
{
    QMutexLocker lock( &m_mutex );

    QHash<QUrl, QExplicitlySharedDataPointer<ClassPrivate> >::const_iterator it = m_classMap.constFind( uri );
    if ( it != m_classMap.constEnd() ) {
        return it.value();
    }
    else {
        QExplicitlySharedDataPointer<ClassPrivate> scp( new ClassPrivate( uri ) );
        m_classMap.insert( uri, scp );
        return scp;
    }
}


QExplicitlySharedDataPointer<Nepomuk::Types::PropertyPrivate> Nepomuk::Types::EntityManager::getProperty( const QUrl& uri )
{
    QMutexLocker lock( &m_mutex );

    QHash<QUrl, QExplicitlySharedDataPointer<PropertyPrivate> >::const_iterator it = m_propertyMap.constFind( uri );
    if ( it != m_propertyMap.constEnd() ) {
        return it.value();
    }
    else {
        QExplicitlySharedDataPointer<PropertyPrivate> cp( new PropertyPrivate( uri ) );
        m_propertyMap.insert( uri, cp );
        return cp;
    }
}


QExplicitlySharedDataPointer<Nepomuk::Types::OntologyPrivate> Nepomuk::Types::EntityManager::getOntology( const QUrl& uri )
{
    QMutexLocker lock( &m_mutex );

    QHash<QUrl, QExplicitlySharedDataPointer<OntologyPrivate> >::const_iterator it = m_ontologyMap.constFind( uri );
    if ( it != m_ontologyMap.constEnd() ) {
        return it.value();
    }
    else {
        QExplicitlySharedDataPointer<OntologyPrivate> cp( new OntologyPrivate( uri ) );
        m_ontologyMap.insert( uri, cp );
        return cp;
    }
}


Nepomuk::Types::EntityManager* Nepomuk::Types::EntityManager::self()
{
    return entityManager();
}
