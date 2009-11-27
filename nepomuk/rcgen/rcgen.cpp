/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2009 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QCoreApplication>
#include <QtCore/QRegExp>

#include "kaboutdata.h"
#include "kcmdlineargs.h"
#include "kcomponentdata.h"

#include "resourceclass.h"
#include "ontologyparser.h"
#include "codegenerator.h"


bool quiet = true;

namespace {
/**
 * To be backwards-compatible with older versions which used a space-separated list.
 */
QStringList extractOntologyFileList( const QStringList& args )
{
    QStringList results;
    foreach( const QString& a, args ) {
        results << a.split( QRegExp("[\\s]") );
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
    KComponentData component( aboutData );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("verbose", ki18n("Verbose output debugging mode."));
    options.add("fast", ki18n("Generate simple and fast wrapper classes not based on Nepomuk::Resource which do not provide any data integrity checking"));
    options.add("writeall", ki18n("Actually generate the code."));
    options.add("listincludes", ki18n("List all includes (deprecated)."));
    options.add("listheaders", ki18n("List all header files that will be generated via the --writeall command."));
    options.add("listsources", ki18n("List all source files that will be generated via the --writeall command."));
    options.add("ontologies <files>", ki18n("The ontology files containing the ontologies to be generated, a space separated list (deprecated: use arguments instead.)"));
    options.add("prefix <prefix>", ki18n("Include path prefix (deprecated)"));
    options.add("target <target-folder>", ki18n("Specify the target folder to store generated files into."));
    // (romain_kdab) : watch out for a regression with --templates :
    // KCmdLineOptions doesn't allow empty option arguments, so e.g. "--templates --foo" will treat --foo as the argument to --templates
    // Since the option is deprecated it's probably not worth changing the KCmdLineOptions behaviour only for this ?
    options.add("templates <templates>", ki18n("Templates to be used (deprecated)."));
    options.add("class <classname>", ki18n("Optionally specify the classes to be generated. Use option multiple times (defaults to all classes)"));
    options.add("serialization <rdf-serialization>", ki18n("Serialization used in the ontology files. Will default to primitive file extension detection."));
    options.add("visibility <visibility-name>", ki18n("Set the used visibility in case the classes are to be used in public API. <visibility-name> will be used to construct the export macro name and the export header. By default classes will not be exported."));
    options.add("+[ontologies]", ki18n("The ontology files containing the ontologies to be generated."));

    KCmdLineArgs::addCmdLineOptions( options );
    QCoreApplication app( argc,argv );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();


    // =====================================================
    // prepare configuration
    // =====================================================
    bool writeAll = args->isSet("writeall");
    bool listHeader = args->isSet("listheaders");
    bool listSource = args->isSet("listsources");
    bool listIncludes = args->isSet("listincludes");
    bool fastMode = args->isSet("fast");
    quiet = !args->isSet("verbose");
    QStringList ontoFiles = extractOntologyFileList( args->getOptionList("ontologies") ); // backwards comp
    for(int i = 0; i < args->count(); ++i )
        ontoFiles << args->arg(i);
    QString targetDir = args->getOption("target");
    QString prefix = args->getOption("prefix");
    QStringList templates = args->getOptionList("templates");
    QStringList classesToGenerate = args->getOptionList( "class" );
    QString visibility = args->getOption("visibility");

    // =====================================================
    // a few checks for valid parameters (not complete!)
    // =====================================================
    if( ontoFiles.isEmpty() ) {
        QTextStream s( stderr );
        s << "No ontology files specified." << endl;
        return -1;
    }

    foreach( const QString& ontoFile, ontoFiles ) {
        if( !QFile::exists( ontoFile ) ) {
            QTextStream s( stderr );
            s << "Ontology file " << ontoFile << " does not exist." << endl;
            return -1;
        }
    }

    if( fastMode && !visibility.isEmpty() ) {
        QTextStream s( stderr );
        s << "Cannot export fast classes. They are only meant to be used as private classes." << endl;
        return -1;
    }


    if( writeAll ) {
        if( !QFile::exists( targetDir ) ) {
            QTextStream s( stderr );
            s << "Folder " << targetDir << " does not exist." << endl;
            return -1;
        }
    }


    // =====================================================
    // parse the data and determine the classes to generate
    // =====================================================
    OntologyParser prsr;
    foreach( const QString& ontoFile, ontoFiles ) {
        if( !prsr.parse( ontoFile, args->getOption("serialization") ) ) {
            QTextStream s( stderr );
            s << "Parsing ontology file " << ontoFile << " failed." << endl;
            return -1;
        }
    }

    // if classes to be generated have been specified on the command line, reset all ResourceClass
    // instances in terms of generation
    if( !classesToGenerate.isEmpty() ) {
        foreach( ResourceClass* rc, prsr.parsedClasses() ) {
            rc->setGenerateClass( classesToGenerate.contains( rc->uri().toString() ) );
        }
    }


    // =====================================================
    // create the code generator which will take care of the rest
    // =====================================================
    CodeGenerator codeGen( fastMode ? CodeGenerator::FastMode : CodeGenerator::SafeMode, prsr.parsedClasses() );
    codeGen.setVisibility( visibility );

    if( writeAll ) {
        if( !codeGen.writeSources( targetDir ) ) {
            QTextStream s( stderr );
            s << "Writing sources to " << targetDir << " failed." << endl;
            return -1;
        }
    }
    else if( listSource ) {
        QStringList l = codeGen.listSources();
        QTextStream s( stdout, QIODevice::WriteOnly );
        QStringListIterator it( l );
        while( it.hasNext() )
            s << prefix << it.next() << ";";

        if( fastMode )
            s << prefix << "resource.cpp;";
    }
    else if( listHeader ) {
        QStringList l = codeGen.listHeader();
        QTextStream s( stdout, QIODevice::WriteOnly );
        QStringListIterator it( l );
        while( it.hasNext() )
            s << prefix << it.next() << ";";

        if( fastMode )
            s << prefix << "resource.h;";
    }
    else if( listIncludes ) {
        QStringList l = codeGen.listHeader();
        QTextStream s( stdout, QIODevice::WriteOnly );
        QStringListIterator it( l );
        while( it.hasNext() )
            s << "#include <nepomuk/" << it.next() << ">" << endl;
    }

    return 0;
}
