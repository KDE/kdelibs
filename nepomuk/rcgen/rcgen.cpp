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

#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>

#include "resourceclass.h"
#include "ontologyparser.h"


bool quiet = true;

namespace {
QStringList extractSpaceSeparatedLists( const QStringList& args )
{
    QStringList results;
    foreach( const QString& a, args ) {
        results << a.split( ' ' );
    }
    return results;
}
}

int main( int argc, char** argv )
{
    KAboutData aboutData( "nepomuk-rcgen",
                          "nepomuk-rcgen",
                          ki18n("Nepomuk Resource Class Generator"),
                          "0.3",
                          ki18n("Nepomuk Resource Class Generator"),
                          KAboutData::License_GPL,
                          ki18n("(c) 2006-2009, Sebastian Trüg"),
                          KLocalizedString(),
                          "http://nepomuk.kde.org" );
    aboutData.addAuthor(ki18n("Sebastian Trüg"), ki18n("Maintainer"), "trueg@kde.org");
    aboutData.addAuthor(ki18n("Tobias Koenig"), ki18n("Major cleanup - Personal hero of maintainer"), "tokoe@kde.org");
    aboutData.setProgramIconName( "nepomuk" );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("verbose", ki18n("Verbose output debugging mode."));
    options.add("fast", ki18n("Generate simple and fast wrapper classes not based on Nepomuk::Resource which do not provide any data integrity checking"));
    options.add("writeall", ki18n("Actually generate the code."));
    options.add("listincludes", ki18n("List all includes (deprecated)."));
    options.add("listheaders", ki18n("List all header files that will be generated via the --writeall command."));
    options.add("listsources", ki18n("List all source files that will be generated via the --writeall command."));
    options.add("ontologies <files>", ki18n("The ontology files containing the ontologies to be generated, a space separated list (deprecated: use arguments instead."));
    options.add("prefix <prefix>", ki18n("Include path prefix (deprecated)"));
    options.add("target <target-folder>", ki18n("Specify the target folder to store generated files into."));
    options.add("templates <templates>", ki18n("Templates to be used (deprecated)."));
    options.add("class <classname>", ki18n("Optionally specify the classes to be generated. Use option multiple times (defaults to all classes)"));
    options.add("serialization <rdf-serialization>", ki18n("Serialization used in the ontology files. Will default to primitive file extension detection."));
    options.add("+[ontologies]", ki18n("The ontology files containing the ontologies to be generated."));

    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app( false /* no gui */ );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    bool writeAll = args->isSet("writeall");
    bool listHeader = args->isSet("listheaders");
    bool listSource = args->isSet("listsources");
    bool listIncludes = args->isSet("listincludes");
    bool fastMode = args->isSet("fast");
    quiet = !args->isSet("verbose");

    QStringList ontoFiles = extractSpaceSeparatedLists( args->getOptionList("ontologies") ); // backwards comp
    for(int i = 0; i < args->count(); ++i )
        ontoFiles << args->arg(i);
    QString targetDir = args->getOption("target");
    QString prefix = args->getOption("prefix");
    QStringList templates = args->getOptionList("templates");
    QStringList classes = args->getOptionList( "class" );

    foreach( const QString& ontoFile, ontoFiles ) {
        if( !QFile::exists( ontoFile ) ) {
            qDebug() << "Ontology file " << ontoFile << " does not exist." << endl;
            return -1;
        }
    }

    if( writeAll ) {
        if( !QFile::exists( targetDir ) ) {
            qDebug() << "Folder " << targetDir << " does not exist." << endl;
            return -1;
        }
    }

    OntologyParser prsr;
    if( !classes.isEmpty() )
        prsr.setClassesToGenerate( classes );

    foreach( const QString& ontoFile, ontoFiles ) {
        if( !prsr.parse( ontoFile, args->getOption("serialization") ) ) {
            qDebug() << "Parsing ontology file " << ontoFile << " failed." << endl;
            return -1;
        }
    }

    if( writeAll ) {
        if( !prsr.assignTemplates( templates ) ) {
            return -1;
        }

        if( !prsr.writeSources( targetDir, fastMode ) ) {
            qDebug() << "Writing sources to " << targetDir << " failed." << endl;
            return -1;
        }
    }
    else if( listSource ) {
        QStringList l = prsr.listSources();
        QTextStream s( stdout, QIODevice::WriteOnly );
        QStringListIterator it( l );
        while( it.hasNext() )
            s << prefix << it.next() << ";";

        if( fastMode )
            s << prefix << "resource.cpp;";
    }
    else if( listHeader ) {
        QStringList l = prsr.listHeader();
        QTextStream s( stdout, QIODevice::WriteOnly );
        QStringListIterator it( l );
        while( it.hasNext() )
            s << prefix << it.next() << ";";

        if( fastMode )
            s << prefix << "resource.h;";
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
