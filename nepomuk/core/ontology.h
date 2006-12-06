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

#ifndef _NEPOMUK_ONTOLOGY_H_
#define _NEPOMUK_ONTOLOGY_H_

#include <QtCore>

#include <kmetadata/variant.h>
#include <kmetadata/kmetadata_export.h>

namespace Nepomuk {
  namespace KMetaData {
    /**
     * This class represents the basic NEPOMUK meta data ontology.
     * It provides information about types and stuff.
     */
    class KMETADATA_EXPORT Ontology
      {
      public:
	static QString defaultGraph();
	static QString typePredicate();
	static QString valueToRDFLiteral( const Variant& );
	static QStringList valuesToRDFLiterals( const Variant& );
	static Variant RDFLiteralToValue( const QString& );

	static QString rdfNamespace();
	static QString rdfsNamespace();
	static QString nrlNamespace();
      };
  }
}

#endif
