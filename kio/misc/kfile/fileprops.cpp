/* This file is part of the KDE libraries
   Copyright (C) 2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

   library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation, version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <iostream>

#include <qfile.h>
#include <q3ptrlist.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kfilemetainfo.h>
#include <klocale.h>
#include <kpropertiesdialog.h>
#include <kdebug.h>

#include "fileprops.h"

#define KFILEVERSION "0.2"
#define INDENT "\t"

using namespace std;

static QString beatifyValue( const QString& value )
{
    if ( value.isNull() )
        return QString("(no value for key available)");
    else if ( value.isEmpty() )
        return QString("(empty)");

    return value;
}

FileProps::FileProps( const QString& path, const QStringList& suppliedGroups )
    : m_dirty( false )
{
    m_info = new KFileMetaInfo(path, QString(), KFileMetaInfo::Everything);
    m_userSuppliedGroups = !suppliedGroups.isEmpty();
    m_groupsToUse = m_userSuppliedGroups ? suppliedGroups : m_info->groups();
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
    QStringList allKeys = g.keys();
    QStringList ret;
    QStringList::ConstIterator it = allKeys.begin();
    for ( ; it != allKeys.end(); ++it )
    {
        if ( g.item( *it ).isValid() )
            ret.append( *it );
    }

    return ret;
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
    return FileProps::createKeyValue( g, key );
}

bool FileProps::setValue( const QString& group,
                          const QString& key, const QString &value )
{
    KFileMetaInfoGroup g = m_info->group( group );
    bool wasAdded = false;
    if ( !g.isValid() )
    {
        if ( m_info->addGroup( group ) )
        {
            wasAdded = true;
            g = m_info->group( group );
        }
        else
            return false;
    }

    bool ok = g[key].setValue( value );

    if ( !ok && wasAdded ) // remove the created group again
        (void) m_info->removeGroup( group );
        
    m_dirty |= ok;
    return ok;
}

QStringList FileProps::allValues( const QString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return FileProps::createKeyValueList( g, g.keys() );
}

QStringList FileProps::preferredValues( const QString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return FileProps::createKeyValueList( g, g.preferredKeys() );
}

// static helper:
// creates strings like
// "group:       translatedKey:               value"
QString FileProps::createKeyValue( const KFileMetaInfoGroup& g,
                                   const QString& key )
{
    static const int MAX_SPACE = 25;
    KFileMetaInfoItem item = g.item( key );

    QString result("%1");
    result = result.arg( (item.isValid() ? item.translatedKey() : key) + ":",
                         -MAX_SPACE );
    result.append( beatifyValue( item.string() ) );

    QString group("%1");
    group = group.arg( g.translatedName() + ":", -MAX_SPACE );
    result.prepend( group );

    return result;
}

// static
QStringList FileProps::createKeyValueList( const KFileMetaInfoGroup& g,
                                           const QStringList& keys )
{
    QStringList result;
    QStringList::ConstIterator it = keys.begin();

    for ( ; it != keys.end(); ++it )
        result.append( FileProps::createKeyValue( g, *it ) );

    return result;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////



// kfile --mimetype --listsupported --listavailable --listpreferred --listwritable --getValue "key" --setValue "key=value" --allValues --preferredValues --dialog --quiet file [file...]
// "key" may be a list of keys, separated by commas
static KCmdLineOptions options[] =
{
    { "m", 0, 0 }, // short option for --mimetype
    { "nomimetype", I18N_NOOP("Do not print the mimetype of the given file(s)"), 0 },

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
      I18N_NOOP("Do not print a warning when more than one file was given "
                "and they do not all have the same mimetype."), 0 },

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

    { "!groups <arguments>", I18N_NOOP("The group to get values from or set values to"),
      0 },

    { "+[files]",
      I18N_NOOP("The file (or a number of files) to operate on."), 0 },
    KCmdLineLastOption
};


//
// helper functions
//

static void printSupportedMimeTypes()
{
    QStringList allMimeTypes = KFileMetaInfoProvider::self()->supportedMimeTypes();
    if ( allMimeTypes.isEmpty() )
    {
        kdDebug() <<
            i18n("No support for metadata extraction found.").toLocal8Bit()
             << endl;
        return;
    }

    kdDebug() << i18n("Supported MimeTypes:").toLocal8Bit() << endl;

    QStringList::ConstIterator it = allMimeTypes.begin();
    for ( ; it != allMimeTypes.end(); it++ )
        kdDebug() << (*it).toLocal8Bit() << endl;
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
        kdDebug() << args->arg(i) << ": " << mt->comment().toLocal8Bit() << " ("
             << mt->name().toLocal8Bit() << ")" << endl;
    }
}

static void printList( const QStringList& list )
{
    QStringList::ConstIterator it = list.begin();
    for ( ; it != list.end(); ++it )
        kdDebug() << (*it).toLocal8Bit() << endl;
    kdDebug() << endl;
}

static void processMetaDataOptions( const Q3PtrList<FileProps> propList,
                                    KCmdLineArgs *args )
{
// kfile --mimetype --supportedMimetypes --listsupported --listavailable --listpreferred --listwritable --getValue "key" --setValue "key=value" --allValues --preferredValues --dialog --quiet file [file...]
// "key" may be a list of keys, separated by commas

    QString line("-- -------------------------------------------------------");
    FileProps *props;
    Q3PtrListIterator<FileProps> it( propList );
    for ( ; (props = it.current()); ++it )
    {
        QString file = props->fileName() + " ";
        QString fileString = line.replace( 3, file.length(), file );
        kdDebug() << QFile::encodeName( fileString ) << endl;
            
        if ( args->isSet( "listsupported" ) )
        {
            kdDebug() << "=Supported Keys=" << endl;
            printList( props->supportedKeys() );
        }
        if ( args->isSet( "listpreferred" ) )
        {
            kdDebug() << "=Preferred Keys=" << endl;
            printList( props->preferredKeys() );
        }
        if ( args->isSet( "listavailable" ) )
        {
            kdDebug() << "=Available Keys=" << endl;
            QStringList groups = props->availableGroups();
            QStringList::ConstIterator git = groups.begin();
            for ( ; git != groups.end(); ++git )
            {
                kdDebug() << "Group: " << (*git).toLocal8Bit() << endl;
                printList( props->availableKeys( *git ) );
            }
        }
//         if ( args->isSet( "listwritable" ) )
//         {
//             kdDebug() << "TODO :)" << endl;
//         }
        if ( args->isSet( "getValue" ) )
        {
            kdDebug() << "=Value=" << endl;
            QString key = QString::fromLocal8Bit( args->getOption("getValue"));
            QStringList::ConstIterator git = props->groupsToUse().begin();
            for ( ; git != props->groupsToUse().end(); ++git )
                kdDebug() << props->getValue( *git, key ).toLocal8Bit() << endl;
        }

        if ( args->isSet( "setValue" ) )
        {
            // separate key and value from the line "key=value"
            QString cmd = QString::fromLocal8Bit( args->getOption("setValue"));
            QString key = cmd.section( '=', 0, 0 );
            QString value = cmd.section( '=', 1 );

            // either use supplied groups or all supported groups
            // (not only the available!)
            QStringList groups = props->userSuppliedGroups() ?
                                 props->groupsToUse() :
                                 props->supportedGroups();

            QStringList::ConstIterator git = groups.begin();
            for ( ; git != groups.end(); ++git )
                props->setValue( *git, key, value );
        }

        if ( args->isSet( "allValues" ) )
        {
            kdDebug() << "=All Values=" << endl;
            QStringList groups = props->availableGroups();
            QStringList::ConstIterator group = groups.begin();
            for ( ; group != groups.end(); ++group )
                printList( props->allValues( *group ) );
        }
        if ( args->isSet( "preferredValues" ) && !args->isSet("allValues") )
        {
            kdDebug() << "=Preferred Values=" << endl;
            QStringList groups = props->availableGroups();
            QStringList::ConstIterator group = groups.begin();
            for ( ; group != groups.end(); ++group )
                printList( props->preferredValues( *group ) );
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

    KCmdLineArgs::init( argc, argv, &about );

    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool useGUI = args->isSet( "dialog" );

    KApplication app( useGUI );

    Q3PtrList<FileProps> m_props;
    m_props.setAutoDelete( true );

    bool quiet = args->isSet( "quiet" );

    if ( args->isSet( "supportedMimetypes" ) )
        printSupportedMimeTypes();

    int files = args->count();
    if ( files == 0 )
        KCmdLineArgs::usage( i18n("No files specified") ); // exit()s

    if ( args->isSet( "dialog" ) )
    {
        showPropertiesDialog( args );
        return true;
    }

    QStringList groupsToUse;
    QByteArrayList suppliedGroups = args->getOptionList( "groups" );
    foreach (QByteArray array, suppliedGroups)
        groupsToUse.append( QString::fromLocal8Bit( array ) );

    QString mimeType;

    for ( int i = 0; i < files; i++ )
    {
        if ( args->isSet( "mimetype" ) )
            printMimeTypes( args );

        FileProps *props = new FileProps( args->url(i).path(), groupsToUse );
        if ( props->isValid() )
            m_props.append( props );
        else
        {
            if ( !quiet )
            {
                kdWarning() << args->arg(i) << ": " <<
                i18n("Cannot determine metadata").toLocal8Bit() << endl;
            }
            delete props;
        }
    }


    processMetaDataOptions( m_props, args );

    m_props.clear(); // force destruction/sync of props
    kdDebug().flush();

    return 0;
}
