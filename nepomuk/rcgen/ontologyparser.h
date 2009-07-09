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

#ifndef _ONTOLOGY_PARSER_H_
#define _ONTOLOGY_PARSER_H_

#include <QtCore/QStringList>
#include <QtCore/QList>

class ResourceClass;

class OntologyParser
{
 public:
    OntologyParser();
    ~OntologyParser();

    bool parse( const QString& filename, const QString& serialization = QString() );
    QList<ResourceClass*> parsedClasses() const;

 private:
    class Private;
    Private* d;
};

#endif
