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

#ifndef _NEPOMUK_SAFECODE_H_
#define _NEPOMUK_SAFECODE_H_

#include "abstractcode.h"

/**
 * @short Specific code implementation with full checks.
 *
 * This implementation of the AbstractCode interface returns generated
 * code that uses Nepomuk::Property to full check syntax and semantic
 * of the data before writing it to the Nepomuk storage.
 */
class SafeCode : public AbstractCode
{
    public:
        QString propertySetterDeclaration( const Property *property,
                                           const ResourceClass* resourceClass,
                                           const QString &nameSpace = QString() ) const;

        QString propertyGetterDeclaration( const Property *property,
                                           const ResourceClass* resourceClass,
                                           const QString &nameSpace = QString() ) const;

        QString propertyAdderDeclaration( const Property *property,
                                          const ResourceClass* resourceClass,
                                          const QString &nameSpace = QString() ) const;

        QString propertyReversePropertyGetterDeclaration( const Property *property,
                                                          const ResourceClass* resourceClass,
                                                          const QString &nameSpace = QString() ) const;

        QString resourceAllResourcesDeclaration( const ResourceClass* resourceClass,
                                                 const QString &nameSpace = QString() ) const;

        QString resourcePseudoInheritanceDeclaration( const ResourceClass* resourceBaseClass,
                                                      const ResourceClass* resourceClass,
                                                      const QString &nameSpace ) const;

        QString propertySetterDefinition( const Property *property, const ResourceClass* resourceClass ) const;
        QString propertyGetterDefinition( const Property *property, const ResourceClass* resourceClass ) const;
        QString propertyAdderDefinition( const Property *property, const ResourceClass* resourceClass ) const;
        QString propertyReversePropertyGetterDefinition( const Property *property,
                                                         const ResourceClass* resourceClass ) const;

        QString resourceAllResourcesDefinition( const ResourceClass* resourceClass ) const;
        QString resourcePseudoInheritanceDefinition( const ResourceClass* resourceBaseClass,
                                                     const ResourceClass* resourceClass ) const;
};

#endif
