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
    << "   " << QCoreApplication::instance()->arguments()[0] << " --writeall [--templates <tmpl1> [<tmpl2> [<tmpl3> ...]]] <sourcefolder> <ontologyfile>" << endl
    << "   " << QCoreApplication::instance()->arguments()[0] << " --listincludes <ontologyfile>" << endl
    << "   " << QCoreApplication::instance()->arguments()[0] << " --listheaders [--prefix <listprefix>] <ontologyfile>" << endl
    << "   " << QCoreApplication::instance()->arguments()[0] << " --listsources [--prefix <listprefix>] <ontologyfile>" << endl;
  return 1;
}


int main( int argc, char** argv )
{
  // we probably need a QCoreApplication instance for some
  // stuff. If not, who cares, we don't do anything time relevant here
  QCoreApplication app( argc, argv );

  bool writeAll = false, listHeader = false, listSource = false, listIncludes = false;
  QString ontoFile, targetDir, prefix;
  QStringList templates;

  QStringList args = app.arguments();
  if( args.count() < 2 )
    return usage();
  
  if( args[1] == "--writeall" ) {
    writeAll = true;
    if( args.count() < 4 )
      return usage();
    int pos = 2;
    if( args[2] == "--templates" ) {
      ++pos;
      while( pos < args.count() - 2 )
	templates += args[pos++];
    }
    if( args.count() != pos+2 )
      return usage();
    targetDir = args.at(pos);
    ontoFile = args.at(pos+1);
  }
  else if( args[1] == "--listincludes" ) {
    listIncludes = true;
    if( args.count() != 3 )
      return usage();
    ontoFile = args.at(2);
  }
  else if( args[1] == "--listheaders" ) {
    listHeader = true;
    if( args.count() == 3 )
      ontoFile = args.at(2);
    else if( args.count() == 5 && args[2] == "--prefix" ) {
      ontoFile = args.at(4);
      prefix = args.at(3);
    }
    else
      return usage(); 
  }
  else if( args[1] == "--listsources" ) {
    listSource = true;
    if( args.count() == 3 )
      ontoFile = args.at(2);
    else if( args.count() == 5 && args[2] == "--prefix" ) {
      ontoFile = args.at(4);
      prefix = args.at(3);
    }
    else
      return usage(); 
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
    if( !prsr.assignTemplates( templates ) ) {
      return usage();
    }

    qDebug() << "Writing sources to " << targetDir << endl;
    if( !prsr.writeSources( targetDir ) ) {
      qDebug() << "Writing sources to " << targetDir << " failed." << endl;
      return usage();
    }
    else if( !prsr.writeOntology( targetDir ) ) {
      qDebug() << "Writing Ontology constructor to " << targetDir << " failed." << endl;
      return usage();
    }
  }
  else if( listSource ) {
    QStringList l = prsr.listSources();
    QTextStream s( stdout, QIODevice::WriteOnly );
    QStringListIterator it( l );
    while( it.hasNext() )
      s << prefix << it.next() << ";";
  }
  else if( listHeader ) {
    QStringList l = prsr.listHeader();
    QTextStream s( stdout, QIODevice::WriteOnly );
    QStringListIterator it( l );
    while( it.hasNext() )
      s << prefix << it.next() << ";";
  }
  else if( listIncludes ) {
    QStringList l = prsr.listHeader();
    QTextStream s( stdout, QIODevice::WriteOnly );
    QStringListIterator it( l );
    while( it.hasNext() )
      s << "#include <kmetadata/" << it.next() << ">" << endl;
  }

  return 0;
}
