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

#include <QtCore>
#include "resourceclass.h"
#include "ontologyparser.h"


static int usage()
{
  QTextStream( stderr, QIODevice::WriteOnly )
    << "Usage:" << endl
    << "   " << QCoreApplication::instance()->arguments()[0] 
    << " [--writeall <sourcefolder>] [--listheader] [--listsources] <ontologyfile>" << endl;
  return 1;
}


int main( int argc, char** argv )
{
  // we probably need a QCoreApplication instance for some
  // stuff. If not, who cares, we don't do anything time relevant here
  QCoreApplication app( argc, argv );

  bool writeAll = false, listHeader = false, listSource = false;
  QString ontoFile, targetDir;

  QStringList args = app.arguments();
  if( args[1] == "--listheader" ) {
    listHeader = true;
    if( args.count() != 3 )
      return usage();
    ontoFile = args.at(2);
  }
  else if( args[1] == "--listsources" ) {
    listSource = true;
    if( args.count() != 3 )
      return usage();
    ontoFile = args.at(2);
  }
  else if( args[1] == "--writeall" ) {
    if( args.count() != 4 )
      return usage();
    targetDir = args.at(3);
    ontoFile = args.at(3);
  }
  else
    return usage();

  if( !QFile::exists( ontoFile ) ) {
    qDebug() << "Ontology file " << ontoFile << " does not exist." << endl;
    return usage();
  }

  if( writeAll ) {
    if( !QFile::exists( targetDir ) ) {
      qDebug() << "Folder " << targetDir << " does not exist." << endl;
      return usage();
    }
  }

  OntologyParser prsr;
  if( !prsr.parse( ontoFile ) ) {
    qDebug() << "Parsing ontology file " << ontoFile << " failed." << endl;
    return usage();
  }

  if( writeAll ) {
    if( !prsr.writeSources( targetDir ) ) {
      qDebug() << "Writing sources to " << argv[2] << " failed." << endl;
      return usage();
    }
  }
  else if( listSource ) {
    QStringList l = prsr.listSources();
    QTextStream s( stdout, QIODevice::WriteOnly );
    QStringListIterator it( l );
    while( it.hasNext() )
      s << it.next() << endl;
  }
  else if( listHeader ) {
    QStringList l = prsr.listHeader();
    QTextStream s( stdout, QIODevice::WriteOnly );
    QStringListIterator it( l );
    while( it.hasNext() )
      s << it.next() << endl;
  }

  return 0;
}
