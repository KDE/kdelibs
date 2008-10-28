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
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include "resourceclass.h"
#include "ontologyparser.h"


bool quiet = true;

static int usage()
{
    QTextStream( stderr, QIODevice::WriteOnly )
        << "Usage:" << endl
        << "   " << QCoreApplication::instance()->arguments()[0] << " --writeall [--templates <tmpl1> [<tmpl2> [<tmpl3> ...]]] --target <sourcefolder> --ontologies <ontologyfile(s)>" << endl
        << "   " << QCoreApplication::instance()->arguments()[0] << " --listincludes --ontologies <ontologyfile(s)>" << endl
        << "   " << QCoreApplication::instance()->arguments()[0] << " --listheaders [--prefix <listprefix>] --ontologies <ontologyfile(s)>" << endl
        << "   " << QCoreApplication::instance()->arguments()[0] << " --listsources [--prefix <listprefix>] --ontologies <ontologyfile(s)>" << endl;
    return 1;
}


int main( int argc, char** argv )
{
    // we probably need a QCoreApplication instance for some
    // stuff. If not, who cares, we don't do anything time relevant here
    QCoreApplication app( argc, argv );

    bool writeAll = false, listHeader = false, listSource = false, listIncludes = false;
    QStringList ontoFiles;
    QString targetDir, prefix;
    QStringList templates;

    QStringList args = app.arguments();
    if( args.count() < 2 )
        return usage();

    QStringList::const_iterator argIt = args.constBegin();
    ++argIt; // skip the app name
    while (argIt != args.constEnd()) {
        const QString& arg = *argIt;

        // new parameter
        if ( arg.startsWith("--") ) {
            // gather parameter arg
            QStringList paramArgs;
            ++argIt;
            while ( argIt != args.constEnd() &&
                    !(*argIt).startsWith("--") ) {
                paramArgs += *argIt;
                ++argIt;
            }

            // now lets see what we have
            if ( arg == "--writeall" ) {
                writeAll = true;
            }
            else if ( arg == "--listincludes" ) {
                listIncludes = true;
            }
            else if ( arg == "--listheaders" ) {
                listHeader = true;
            }
            else if ( arg == "--listsources" ) {
                listSource = true;
            }
            else if ( arg == "--templates" ) {
                templates = paramArgs;
            }
            else if ( arg == "--ontologies" ) {
                if ( paramArgs.isEmpty() ) {
                    return usage();
                }
                ontoFiles = paramArgs;
            }
            else if ( arg == "--prefix" ) {
                if ( paramArgs.count() != 1 ) {
                    return usage();
                }
                prefix = paramArgs.first();
            }
            else if ( arg == "--target" ) {
                if ( paramArgs.count() != 1 ) {
                    return usage();
                }
                targetDir = paramArgs.first();
            }
            else if ( arg == "--verbose" ) {
                quiet = false;
            }
            else {
                return usage();
            }
        }
        else
            return usage();
    }

    foreach( const QString &ontoFile, ontoFiles ) {
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
    foreach( const QString &ontoFile, ontoFiles ) {
        if( !prsr.parse( ontoFile ) ) {
            qDebug() << "Parsing ontology file " << ontoFile << " failed." << endl;
            return -1;
        }
    }

    if( writeAll ) {
        if( !prsr.assignTemplates( templates ) ) {
            return -1;
        }

        if( !prsr.writeSources( targetDir ) ) {
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
