/* This file is part of the KDE libraries
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation, version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <iostream>

#include <qptrlist.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kfilemetainfo.h>
#include <klocale.h>
#include <kpropertiesdialog.h>

#include "fileprops.h"

#define KFILEVERSION "0.1"
#define INDENT "\t"

using namespace std;

FileProps::FileProps( const char *argument, const QString& path )
    : m_argument( argument ),
      m_dirty( false )
{
    m_info = new KFileMetaInfo( path );
}

FileProps::~FileProps()
{
    sync();
    delete m_info;
}

bool FileProps::sync()
{
    if ( !m_dirty )
        return true;

    return m_info->applyChanges();
}

bool FileProps::isValid() const
{
    return m_info->isValid();
}

QStringList FileProps::supportedGroups() const
{
    return m_info->supportedGroups();
}

QStringList FileProps::availableGroups() const
{
    return m_info->groups();
}

QStringList FileProps::supportedKeys( const QString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return g.supportedKeys();
}

QStringList FileProps::availableKeys( const QString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return g.keys();
}

QStringList FileProps::preferredKeys( const QString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return g.preferredKeys();
}

QString FileProps::getValue( const QString& group,
                             const QString& key ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return g[key].string();
}

bool FileProps::setValue( const QString& group,
                          const QString& key, const QString &value )
{
    KFileMetaInfoGroup g = m_info->group( group );
    bool ok = g[key].setValue( value );
    m_dirty |= ok;
    return ok;
}

QStringList FileProps::allValues( const QString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return createKeyValueList( g, g.keys() );
}

QStringList FileProps::preferredValues( const QString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return createKeyValueList( g, g.preferredKeys() );
}


// helper
QStringList FileProps::createKeyValueList( const KFileMetaInfoGroup& g,
                                            const QStringList& keys )
{
    QStringList result;
    QStringList::ConstIterator it = keys.begin();

    for ( ; it != keys.end(); ++it ) {
        KFileMetaInfoItem item = g.item( *it );
        QString tmp = item.translatedKey() + ":\t\t" + item.string();
        result.append( tmp );
    }

    return result;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////



// kfile --mimetype --listsupported --listavailable --listpreferred --listwritable --getValue "key" --setValue "key=value" --allValues --preferredValues --dialog --quiet file [file...]
// "key" may be a list of keys, separated by commas
static KCmdLineOptions options[] =
{
    { "m", 0, 0 }, // short option for --mimetype
    { "nomimetype", I18N_NOOP("Print the mimetype of the given file(s)"), 0 },

    { "ls", 0, 0 }, // short option for --listsupported
    { "listsupported <mimetype>",
      I18N_NOOP("List all supported metadata keys of the given file(s). "
                "If mimetype is not specified, the mimetype of the given "
                "files is used." ), "file" },

    { "lp", 0, 0 }, // short option for --listpreferred
    { "listpreferred <mimetype>",
      I18N_NOOP("List all preferred metadata keys of the given file(s). "
                "If mimetype is not specified, the mimetype of the given "
                "files is used." ), "file" },

    { "la", 0, 0 }, // short option for --listavailable
    { "listavailable",
      I18N_NOOP("List all metadata keys which have a value in the given "
                "file(s)."), 0 },

    { "sm", 0, 0 }, // short option for --supportedMimetypes
    { "supportedMimetypes",
      I18N_NOOP("Prints all mimetypes for which metadata support is "
                "available."), 0 },

    { "q", 0, 0 }, // short option for --quiet
    { "quiet",
      I18N_NOOP("Don't print a warning when more than one file was given "
                "and they don't have all the same mimetype."), 0 },

    { "av", 0, 0 }, // short option for --allValues
    { "allValues",
      I18N_NOOP("Prints all metadata values, available in the given "
                "file(s)."), 0 },

    { "pv", 0, 0 }, // short option for --preferredValues
    { "preferredValues",
      I18N_NOOP("Prints the preferred metadata values, available in the "
                "given file(s)."), 0 },

    { "dialog",
      I18N_NOOP("Opens a KDE properties dialog to allow viewing and "
                "modifying of metadata of the given file(s)"), 0 },

    { "getValue <key>",
      I18N_NOOP("Prints the value for 'key' of the given file(s). 'key' "
                "may also be a comma-separated list of keys"), 0 },

    { "setValue <key=value>",
      I18N_NOOP("Attempts to set the value 'value' for the metadata key "
                "'key' for the given file(s)"), 0 },

    { "+[files]",
      I18N_NOOP("The file (or a number of files) to operate on."), 0 },
    { 0, 0, 0 }
};


//
// helper functions
//

static void printSupportedMimeTypes()
{
    QStringList allMimeTypes = KFileMetaInfoProvider::self()->supportedMimeTypes();
    if ( allMimeTypes.isEmpty() )
    {
        cout <<
            i18n("No support for metadata extraction found.").local8Bit()
             << endl;
        return;
    }

    cout << i18n("Supported MimeTypes:").local8Bit() << endl;

    QStringList::ConstIterator it = allMimeTypes.begin();
    for ( ; it != allMimeTypes.end(); it++ )
        cout << (*it).local8Bit() << endl;
}

// caller needs to delete the returned list!
static KFileItemList * fileItemList( const KCmdLineArgs *args )
{
    KFileItemList * items = new KFileItemList();
    items->setAutoDelete( true );
    for ( int i = 0; i < args->count(); i++ )
        items->append( new KFileItem( KFileItem::Unknown,
                                     KFileItem::Unknown,
                                     args->url( i ) ));
    return items;
}

static void showPropertiesDialog( const KCmdLineArgs *args )
{
    KFileItemList *items = fileItemList( args );
    new KPropertiesDialog( *items, 0L, "props dialog", true );
    delete items;
}

static void printMimeTypes( const KCmdLineArgs *args )
{
    for ( int i = 0; i < args->count(); i++ )
    {
        KURL url = args->url( i );
        KMimeType::Ptr mt = KMimeType::findByURL( url );
        cout << args->arg(i) << ": " << mt->comment().local8Bit() << " ("
             << mt->name().local8Bit() << ")" << endl;
    }
}

static void printList( const QStringList& list )
{
    QStringList::ConstIterator it = list.begin();
    for ( ; it != list.end(); ++it )
        cout << INDENT << (*it).local8Bit() << endl;
    cout << endl;
}

static void processMetaDataOptions( const QPtrList<FileProps> propList,
                                    KCmdLineArgs *args )
{
// kfile --mimetype --supportedMimetypes --listsupported --listavailable --listpreferred --listwritable --getValue "key" --setValue "key=value" --allValues --preferredValues --dialog --quiet file [file...]
// "key" may be a list of keys, separated by commas

    FileProps *props;
    QPtrListIterator<FileProps> it( propList );
    for ( ; (props = it.current()); ++it )
    {
        cout << props->argument() << ":" << endl;

        if ( args->isSet( "listsupported" ) )
        {
            cout << i18n("Supported Keys").local8Bit() << endl;
            printList( props->supportedKeys() );
        }
        if ( args->isSet( "listpreferred" ) )
        {
            cout << i18n("Preferred Keys").local8Bit() << endl;
            printList( props->preferredKeys() );
        }
        if ( args->isSet( "listavailable" ) )
        {
            cout << "TODO :)" << endl;
        }
//         if ( args->isSet( "listwritable" ) )
//         {
//             cout << "TODO :)" << endl;
//         }
        if ( args->isSet( "getValue" ) )
        {
            //         QCString key = args->option("getValue");
        }
        if ( args->isSet( "allValues" ) )
        {
            QStringList groups = props->availableGroups();
            QStringList::ConstIterator group = groups.begin();
            for ( ; group != groups.end(); ++group )
            {
                cout << (*group).local8Bit() << endl;
                printList( props->allValues( *group ) );
            }
        }
        if ( args->isSet( "preferredValues" ) && !args->isSet("allValues") )
        {
            QStringList groups = props->availableGroups();
            QStringList::ConstIterator group = groups.begin();
            for ( ; group != groups.end(); ++group )
            {
                cout << (*group).local8Bit() << endl;
                printList( props->preferredValues( *group ) );
            }

        }
    }


}

int main( int argc, char **argv )
{
    KAboutData about(
	  "kfile", I18N_NOOP( "kfile" ), KFILEVERSION,
	  I18N_NOOP("A commandline tool to read and modify metadata of files." ),
	  KAboutData::License_LGPL, "(c) 2002, Carsten Pfeiffer",
	  0 /*text*/, "http://devel-home.kde.org/~pfeiffer/",
	  "pfeiffer@kde.org" );

    about.addAuthor( "Carsten Pfeiffer", 0, "pfeiffer@kde.org",
		     "http://devel-home.kde.org/~pfeiffer/" );

    KGlobal::locale()->setMainCatalogue("kdelibs");

    KCmdLineArgs::init( argc, argv, &about );

    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool useGUI = args->isSet( "dialog" );

    KApplication app( useGUI, useGUI );

    QPtrList<FileProps> m_props;
    m_props.setAutoDelete( true );

    bool quiet = args->isSet( "quiet" );

    if ( args->isSet( "supportedMimetypes" ) )
        printSupportedMimeTypes();

    int files = args->count();
    if ( files == 0 )
        KCmdLineArgs::usage( i18n("No files specified") ); // exit()s

    QString mimeType;

    for ( int i = 0; i < files; i++ )
    {
        if ( args->isSet( "dialog" ) )
        {
            showPropertiesDialog( args );
            return true;
        }

        if ( args->isSet( "mimetype" ) )
            printMimeTypes( args );

        FileProps *props = new FileProps( args->arg(i),
                                          args->url(i).path() );
        if ( props->isValid() )
            m_props.append( props );
        else if ( !quiet )
            cerr << args->arg(i) << ": " <<
                i18n("Cannot determine metadata").local8Bit() << endl;
    }


    processMetaDataOptions( m_props, args );

    return 0;
}
