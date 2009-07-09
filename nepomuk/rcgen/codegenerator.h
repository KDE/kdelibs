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

#ifndef _NEPOMUK_CODE_GENERATOR_H_
#define _NEPOMUK_CODE_GENERATOR_H_

#include <QtCore/QString>
#include <QtCore/QList>

class AbstractCode;
class ResourceClass;
class QString;
class QTextStream;

class CodeGenerator
{
public:
    enum Mode {
        SafeMode,
        FastMode
    };

    CodeGenerator( Mode mode, const QList<ResourceClass*>& classes );
    ~CodeGenerator();

    void setVisibility( const QString& v ) { m_visibility = v; }

    QList<ResourceClass*> classes() const { return m_classes; }

    QStringList listHeader();
    QStringList listSources();
    bool writeSources( const QString& dir );

private:
    bool write( const ResourceClass* resourceClass, const QString& folder ) const;
    bool writeHeader( const ResourceClass* resourceClass, QTextStream& ) const;
    bool writeSource( const ResourceClass* resourceClass, QTextStream& ) const;
    bool writeDummyClasses( const QString &folder ) const;

    QString visibilityHeader() const;
    QString visibilityExportMacro() const;

    ResourceClass* findParentClass( ResourceClass* resourceClass ) const;

    const Mode m_mode;
    const AbstractCode* m_code;
    QString m_nameSpace;

    QList<ResourceClass*> m_classes;
    QString m_visibility;
};

#endif
