/* This file is part of the KDE libraries
    Copyright (C) 2000 David Smith <dsmith@algonet.se>

    This class was inspired by a previous KURLCompletion by
    Henner Zeller <zeller@think.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdir.h>

#include <stdlib.h>
#include <assert.h>
#include <kdebug.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kcompletion.h>
#include <kurl.h>
#include <kprotocolinfo.h>
#include <kio/jobclasses.h>
#include <kio/job.h>

#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

#include "kurlcompletion.h"

static void listEnvVar( QStringList * );
static void listUsers( QStringList * );
static bool qstrBeginEq( const QString &s1, const QString &s2 );

KURLCompletion::KURLCompletion() : KCompletion()
{
        init();
}


KURLCompletion::KURLCompletion( Mode mode ) : KCompletion()
{
        init();
        m_mode = mode;
}


KURLCompletion::~KURLCompletion()
{
        delete m_current_url;
}


void KURLCompletion::init()
{
        m_dir = QDir::homeDirPath();

        m_word_break_char = QChar(' ');
        m_quote_char1 = QChar('\"');
        m_quote_char2 = QChar('\'');
        m_escape_char = QChar('\\');

        m_running = false;
        m_mode = FileCompletion;

        m_replace_home = true;
        m_replace_env = true;
        m_current_url = 0L;
        m_list_job = 0L;
}

/*
 * makeCompletion()
 *
 * Entry point for file name completion
 */
QString KURLCompletion::makeCompletion(const QString &text)
{
        // Note: we don't kill a running list job here, since
        // we then might restart the same job again


        // Remove quotes from the text
        //
        QString text_copy = unquote(text);

        // This is a bad hack to make it work with file: URLs
        // Please, let me find time to fix this up later...
        //
        QString file_hack;

        if ( text_copy.left(5) == QString("file:") ) {
                file_hack = QString("file:");
                text_copy = text_copy.mid(5);
        }

        // Split "/dir/file" => "/dir/" and "file" (we only complete "file")
        //
        int lastSlash = text_copy.findRev('/');

        QString pathPart = text_copy.left(lastSlash + 1);
        QString filePart = text_copy.mid(lastSlash + 1);

        if ( pathPart.isNull() ) pathPart = QString("");
        if ( filePart.isNull() ) filePart = QString("");

        //kdDebug()"path '" << pathPart << "' file: " << filePart << endl;

        // Check if we have the wanted list job running already
        //
        if ( m_running
                && pathPart == m_last_path_listed
                && qstrBeginEq( m_last_file_listed, filePart )
                && m_mode == m_last_mode )
        {
                //kdDebug() << "KURLCompletion: The wanted list job is already running" << endl;

                // The right list job is already running => just change the
                // text that will go to KCompletion
                //
                m_compl_text = pathPart + filePart;

                return QString::null;
        }
        else {
                // The wanted list job is not running => kill any that is
                //
                if ( m_list_job ) m_list_job->kill();

                m_list_job = 0L;
                m_running = false;
                m_dirs.clear();
        }

        m_compl_text = QString::null;

        // User name completion
        //
        if ( m_replace_home
                && pathPart.isEmpty()
                && filePart[0] == QChar('~') )
        {

                //kdDebug() << "User name completion: " << filePart << endl;

                m_prepend = "";
                m_compl_text = file_hack + filePart;

                // Get new values if needed
                //
                if ( m_last_compl_type != 1 ) {

                        clear();

                        QStringList l;

                        listUsers( &l );

                        for ( QStringList::Iterator it = l.begin(); it != l.end(); it++ )
                                addItem( file_hack + QString("~") + *it );

                        // A single tilde is also a match...
                        addItem( file_hack + QString("~") );
                }
                // Expand the directory if completion is done again
                // on the same text
                //
                else if ( filePart == m_last_file_listed ) {

                        QString filePartExpanded = filePart;

                        expandTilde( filePartExpanded );

                        text_copy = pathPart + filePartExpanded;

                        int lastSlash = text_copy.findRev('/');

                        pathPart = text_copy.left(lastSlash + 1);
                        filePart = text_copy.mid(lastSlash + 1);

                        if ( pathPart.isNull() ) pathPart = QString("");
                        if ( filePart.isNull() ) filePart = QString("");

                        m_compl_text = QString::null;
                }

                m_last_compl_type = 1;

                m_last_file_listed = filePart;
                m_last_path_listed = pathPart;
        }

        // Environment variable completion
        //
        else if ( m_replace_env && filePart[0] == QChar('$') ) {

                //kdDebug() << "KURLCompletion: env completion: " << filePart << endl;

                m_prepend = "";
                m_compl_text = file_hack + pathPart + filePart;

                // Get new values if needed
                //
                if ( m_last_compl_type != 2 || m_last_path_listed != pathPart ) {

                        clear();

                        QStringList l;

                        listEnvVar( &l );

                        for ( QStringList::Iterator it = l.begin(); it != l.end(); it++ )
                                addItem( file_hack + pathPart + QString("$") + (*it) );
                }
                // Expand the directory if completion is done again
                // on the same text
                //
                else if ( filePart == m_last_file_listed
                                && pathPart == m_last_path_listed)
                {
                        QString filePartExpanded = filePart;

                        expandEnv( filePartExpanded );

                        text_copy = pathPart + filePartExpanded;

                        int lastSlash = text_copy.findRev('/');

                        pathPart = text_copy.left(lastSlash + 1);
                        filePart = text_copy.mid(lastSlash + 1);

                        if ( pathPart.isNull() ) pathPart = QString("");
                        if ( filePart.isNull() ) filePart = QString("");

                        m_compl_text = QString::null;
                }

                m_last_compl_type = 2;
                m_last_path_listed = pathPart;
                m_last_file_listed = filePart;
        }

        // File completion if not user/env completion
        //
        if ( m_compl_text == QString::null ) {
/*
                kdDebug() << "0. pathPart = " << pathPart << endl;
                kdDebug() << "0. filePart = " << filePart << endl;
                kdDebug() << "0. last_path = " << m_last_path_listed << endl;
                kdDebug() << "0. last_file = " << m_last_file_listed << endl;
                kdDebug() << "0. last_mode = " << m_last_mode << endl;
                kdDebug() << "0. last_type = " << m_last_compl_type << endl;
*/
                m_compl_text = file_hack + pathPart + filePart;
                m_prepend = file_hack + pathPart;

                // Get new values if needed
                //
                if ( m_last_compl_type != 3
                        || m_last_path_listed != pathPart
                        || !qstrBeginEq( m_last_file_listed, filePart )
                        || m_last_mode != m_mode )
                {
                        clear();

                        // Save these before expansion
                        //
                        m_last_path_listed = pathPart;
                        m_last_file_listed = filePart;

                        // Replace tilde, environment variables, and current dir (m_dir)
                        //
                        // XXX use KURIFilter here ?
                        //
                        QString pathPartExpanded = pathPart;

                        if ( m_replace_home ) expandTilde(pathPartExpanded);
                        if ( m_replace_env ) expandEnv(pathPartExpanded);

                        if ( !m_dir.isNull()
                                && pathPartExpanded.left(2) == QString("./") )
                        {
                                pathPartExpanded.replace( 0, 1, m_dir );
                        }

                        // Remove escapes
                        //
                        pathPartExpanded = unescape( pathPartExpanded );
                        filePart = unescape( filePart );
/*
                        kdDebug() << "1. pathPartExpanded = " << pathPartExpanded << endl;
                        kdDebug() << "1. pathPart = " << pathPart << endl;
                        kdDebug() << "1. filePart = " << filePart << endl;
*/
                        // Exe completion
                        //
                        if ( m_mode == ExeCompletion ) {

                                m_list_exe = true;

                                // Completion of full path
                                //
                                if ( pathPartExpanded[0] == QChar('/') ) {
                                        m_dirs.append( pathPartExpanded );
                                }
                                // Exe completion in $PATH
                                //
                                else if ( !filePart.isEmpty() && pathPartExpanded.isEmpty() ) {
                                        QStringList dirs =
                                                QStringList::split(':',
                                                        QString::fromLocal8Bit(::getenv("PATH")));

                                        for ( QStringList::Iterator it = dirs.begin();
                                                  it != dirs.end(); it++ )
                                        {
                                                m_dirs.append( (*it) + '/' );
                                        }
                                }
                        }
                        // File completion
                        //
                        else {

                                //KURL::encode( pathPartExpanded );

                                KURL urlPath( pathPartExpanded );

                                m_list_exe = false;

                                // Completion of full path
                                //
                                if ( pathPartExpanded[0] == QChar('/') ) {
                                        m_dirs.append( pathPartExpanded );
                                }
                                // Completion in the current dir (m_dir)
                                //
                                else if ( !m_dir.isNull()
                                        && (urlPath.isLocalFile() || urlPath.isMalformed() ) )
                                {
                                        m_dirs.append( m_dir + '/' + pathPartExpanded );
                                }
                                // The actual URL completion...
                                //
                                else {
                                        m_dirs.append( pathPartExpanded );
                                }
                        }

                        m_last_compl_type = 3;
                        m_last_mode = m_mode;

                        m_running = true;
/*
                        kDebugInfo("start listing...");

                        for ( QStringList::Iterator it = m_dirs.begin();
                              it != m_dirs.end(); it++ )
                        {
                                kdDebug() << *it << endl;
                        }
*/
                        // Return the first match or QString::null (async. completion)
                        //
                        return listDirectories();

                } // end need new entries
        }

        // Let KCompletion to do the rest of the work...
        //
        //kdDebug() << "makeCompletion(" << m_compl_text << ") 1" << endl;
        return KCompletion::makeCompletion( m_compl_text );
}

/*
 * postProcessMatch, postProcessMatches
 *
 * Called by KCompletion before emitting match() and matches()
 *
 * Add quotes when needed and add the part of the text that
 * was not completed
 */
void KURLCompletion::postProcessMatch( QString *match )
{
        //kdDebug() "KURLCompletion::postProcessMatch() -- in: " << *match << endl;

        if ( *match == QString::null )
                return;

        if ( match->right(1) == QChar('/') )
                quoteText( match, false, true ); // don't quote the trailing '/'
        else
                quoteText( match, false, false ); // quote the whole text

        //kdDebug() << "KURLCompletion::postProcessMatch() -- ut: " << *match << endl;
}

void KURLCompletion::postProcessMatches( QStringList *matches )
{
        //kDebugInfo("KURLCompletion::postProcessMatches()");

        for ( QStringList::Iterator it = matches->begin();
                  it != matches->end(); it++ )
        {
                if ( (*it) != QString::null ) {

                        if ( (*it).right(1) == QChar('/') )
                                quoteText( &(*it), false, true ); // don't quote trailing '/'
                        else
                                quoteText( &(*it), false, false ); // quote the whole text
                }
        }
}

/*
 * unquote
 *
 * Remove quotes and return the result in a new string
 *
 */
QString KURLCompletion::unquote(const QString &text)
{
        bool in_quote = false;
        bool escaped = false;
        QChar p_last_quote_char;
        QString result;

        for (uint pos = 0; pos < text.length(); pos++) {

                if ( escaped ) {
                        escaped = false;
                        result.insert( result.length(), text[pos] );
                }
                else if ( in_quote && text[pos] == p_last_quote_char ) {
                        in_quote = false;
                }
                else if ( !in_quote && text[pos] == m_quote_char1 ) {
                        p_last_quote_char = m_quote_char1;
                        in_quote = true;
                }
                else if ( !in_quote && text[pos] == m_quote_char2 ) {
                        p_last_quote_char = m_quote_char2;
                        in_quote = true;
                }
                else if ( text[pos] == m_escape_char ) {
                        escaped = true;
                        result.insert( result.length(), text[pos] );
                }
                else {
                        result.insert( result.length(), text[pos] );
                }

        }

        return result;
}

/*
 * unescape
 *
 * Remove escapes and return the result in a new string
 *
 */
QString KURLCompletion::unescape(const QString &text)
{
        QString result;

        for (uint pos = 0; pos < text.length(); pos++)
                if ( text[pos] != m_escape_char )
                        result.insert( result.length(), text[pos] );

        return result;
}

/*
 * list()
 *
 * Add directory listing to 'matches'.
 * Only executables beginning with 'filter' are listed
 * If 'add_dir' is true 'dir' is prepended to the matches
 */
void KURLCompletion::list(const QString& dir, const QString& filter,
                QStringList &matches, bool only_exe)
{
        DIR *dp;
        struct dirent *ep;

        dp = opendir( QFile::encodeName(dir) );
        if ( dp == NULL ) {
            kdDebug() << "Failed to open dir: " << dir << endl;
            return;
        }

        //kdDebug() << "Listing dir: " << dir << "  filter = " << filter << endl;

        // Loop through all directory entries
        while ( ( ep = readdir( dp ) ) != 0L ) {
                if (strcmp (ep->d_name, ".") == 0 || strcmp (ep->d_name, "..") == 0)
                        continue;

                QString file = QFile::decodeName( ep->d_name );

                //kdDebug() << "list: " <<  file << endl;

                if ( filter.isEmpty() || file.left(filter.length()) == filter ) {
                        QString full_path = dir + file;

                        struct stat sbuff;

                        //kdDebug() << "list match: " << full_path << endl;

                        if ( stat( QFile::encodeName(full_path), &sbuff ) == 0 ) {
                                // Verify executable
                                //
                                if ( only_exe && 0 == (sbuff.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH) ) )
                                        continue;

                                // Add '/' to directories
                                //
                                if ( S_ISDIR ( sbuff.st_mode ) )
                                        file.append( '/' );

                        }
                        else {
                                kdError() << "Could not stat file " << full_path << endl;
                                continue;
                        }

                        matches.append( file );

                }
        }

        (void) closedir( dp );

}

/*
 * quoteText()
 *
 * Add quotations to 'text' if needed or if 'force' = true
 * Returns true if quotes were added
 *
 * skip_last => ignore the last charachter (we add a space or '/' to all filenames)
 */
bool KURLCompletion::quoteText(QString *text, bool force, bool skip_last)
{
        int pos;

        if ( !force ) {
                pos = text->find( m_word_break_char );
                if ( skip_last && (pos == (int)(text->length())-1) ) pos = -1;
        }

        if ( !force && pos == -1 ) {
                pos = text->find( m_quote_char1 );
                if ( skip_last && (pos == (int)(text->length())-1) ) pos = -1;
        }

        if ( !force && pos == -1 ) {
                pos = text->find( m_quote_char2 );
                if ( skip_last && (pos == (int)(text->length())-1) ) pos = -1;
        }

        if ( !force && pos == -1 ) {
                pos = text->find( m_escape_char );
                if ( skip_last && (pos == (int)(text->length())-1) ) pos = -1;
        }

        if ( force || (pos >= 0) ) {

                // Escape \ in the string
                text->replace( QRegExp( m_escape_char ),
                                          QString( m_escape_char ) + m_escape_char );

                // Escape " in the string
                text->replace( QRegExp( m_quote_char1 ),
                                          QString( m_escape_char ) + m_quote_char1 );

                // " at the beginning
                text->insert( 0, m_quote_char1 );

                // " at the end
                if ( skip_last )
                        text->insert( text->length()-1, m_quote_char1 );
                else
                        text->insert( text->length(), m_quote_char1 );

                return true;
        }

        return false;
}

/*
 * expandEnv
 *
 * Expand environment variables in text. Escaped '$' are ignored.
 * Return true if expansion was made.
 */
bool KURLCompletion::expandEnv( QString &text )
{
        // Find all environment variables beginning with '$'
        //
        int pos = 0;

        bool expanded = false;

        while ( (pos = text.find('$', pos)) != -1 ) {

                // Skip escaped '$'
                //
                if ( text[pos-1] == m_escape_char ) {
                        pos++;
                }
                // Variable found => expand
                //
                else {
                        // Find the end of the variable = next '/' or ' '
                        //
                        int pos2 = text.find( m_word_break_char, pos+1 );
                        int pos_tmp = text.find( '/', pos+1 );

                        if ( pos2 == -1 || (pos_tmp != -1 && pos_tmp < pos2) )
                                pos2 = pos_tmp;

                        if ( pos2 == -1 )
                                pos2 = text.length();

                        // Replace if the variable is terminated by '/' or ' '
                        // and defined
                        //
                        if ( pos2 >= 0 ) {
                                int     len   = pos2 - pos;
                                QString key   = text.mid( pos+1, len-1);
                                QString value =
                                        QString::fromLocal8Bit( ::getenv(key.local8Bit()) );

                                if ( !value.isEmpty() ) {
                                        expanded = true;
                                        text.replace( pos, len, value );
                                        pos = pos + value.length();
                                }
                                else {
                                        pos = pos2;
                                }
                        }
                }
        }

        //kdDebug() << "Environment expanded: " << text << endl;

        return expanded;
}

/*
 * expandTilde
 *
 * Replace "~user" with the users home directory
 * Return true if expansion was made.
 */
bool KURLCompletion::expandTilde(QString &text)
{
        if ( text[0] != QChar('~') )
                return false;

        bool expanded = false;

        // Find the end of the user name = next '/' or ' '
        //
        int pos2 = text.find( m_word_break_char, 1 );
        int pos_tmp = text.find( '/', 1 );

        if ( pos2 == -1 || (pos_tmp != -1 && pos_tmp < pos2) )
                pos2 = pos_tmp;

        if ( pos2 == -1 )
                pos2 = text.length();

        // Replace ~user if the user name is terminated by '/' or ' '
        //
        if ( pos2 >= 0 ) {

                QString user = text.mid( 1, pos2-1 );
                QString dir;

                // A single ~ is replaced with $HOME
                //
                if ( user.isEmpty() ) {
                        dir = QDir::homeDirPath();
                }
                // ~user is replaced with the dir from passwd
                //
                else {
                        struct passwd *pw = ::getpwnam( user.local8Bit() );

                        if ( pw )
                                dir = QString::fromLocal8Bit( pw->pw_dir );

                        ::endpwent();
                }

                if ( !dir.isEmpty() ) {
                        expanded = true;
                        text.replace(0, pos2, dir);
                }
        }

        return expanded;
}

/*
 * listDirectories
 *
 * List directories in m_dirs.
 * Remote urls are always listed with KIO.  Local diretories are listed
 * with KIO if KURLCOMPLETION_LOCAL_KIO is set.
 *
 * Returns: the match from makeCompletion() if local listing is used and
 *          QString::null if KIO is used
 */
QString KURLCompletion::listDirectories()
{
        bool local = false;

        if ( m_dirs.count() > 0 ) {
                KURL url( m_dirs.first() );
                local = url.isLocalFile() || url.isMalformed();
        }

        assert( m_list_job == 0L );

        // List local files locally if not KURLCOMPLETION_LOCAL_KIO is set
        // This is a temporary solution for performance tests
        //
        if ( local && !::getenv("KURLCOMPLETION_LOCAL_KIO") ) {

                m_running = false;

                //kDebugInfo("listDirectories() -- local listing");

                QStringList l;

                QStringList::Iterator it;

                for ( it = m_dirs.begin(); it != m_dirs.end(); it++ )
                        list( (*it), m_last_file_listed, l, m_list_exe );

                for ( it = l.begin(); it != l.end(); it++ )
                        addItem( m_prepend + (*it) );

                m_dirs.clear();

                // We won't come to slotIOFinished() for local listing
                // so call makeCompletion here instead
                //
                //kdDebug() << "makeCompletion(" << m_compl_text << ") 2" << endl;

                return KCompletion::makeCompletion( m_compl_text );

        }
        // Start a list job for the next dir in m_dirs
        //
        else {

                // Sort of ugly to modify m_last_file_listed here, but for now
                // we don't use it with KIO, and this might enable us to avoid
                // some unnecessary list jobs
                //
                m_last_file_listed = QString::null;

                if ( m_dirs.count() > 0 ) {

                        if ( m_current_url )
                                delete m_current_url;

                        m_current_url = new KURL( m_dirs.first() );
						
						// List the url only if it supports listing
						bool listURL = KProtocolInfo::supportsListing( m_current_url->protocol() );
						// Remote URLs are not listed in auto completion
						if ( !local &&
							completionMode() == KGlobalSettings::CompletionAuto )
							listURL = false;

                        if ( listURL ) {
                            //kdDebug() "listDirectories() -- dir = " << m_current_url->url() << endl;

                            m_list_job = KIO::listDir( (*m_current_url), false );

                            assert( m_list_job );

                            connect(m_list_job, SIGNAL(result(KIO::Job*)),
                                    SLOT(slotIOFinished(KIO::Job*)));

                            connect(m_list_job,
                                    SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
                                    SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)));

                            m_dirs.remove( m_dirs.begin() );
                        }
						else {
							m_dirs.remove( m_dirs.begin() );
							return listDirectories();
						}
							
                }
        }

        return QString::null;
}

/*
 * slotEntries
 *
 * addItem() the files listed by KIO
 */
#define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)

void KURLCompletion::slotEntries(KIO::Job*, const KIO::UDSEntryList& entries)
{
    KIO::UDSEntryListConstIterator it = entries.begin();
    KIO::UDSEntryListConstIterator end = entries.end();

        //kdDebug() << "slotEntries() -- prepend = " << m_prepend << endl;

        // Iterate over all files
        //
    for (; it != end; ++it) {

                QString name;
                bool is_exe = false, is_dir = false;

                KIO::UDSEntry e = *it;
                KIO::UDSEntry::ConstIterator it_2 = e.begin();

                for( ; it_2 != e.end(); it_2++ ) {
                        switch ( (*it_2).m_uds ) {
                                case KIO::UDS_NAME:
                                        name = (*it_2).m_str;
                                        break;
                                case KIO::UDS_ACCESS:
                                        is_exe = ((*it_2).m_long & S_IXUGO) != 0;
                                        break;
                                case KIO::UDS_FILE_TYPE:
                                        is_dir = ((*it_2).m_long & S_IFDIR) != 0;
                                        break;
                        }
                }

                if ( name != QString(".") && name != QString("..") ) {
                        if ( is_dir )
                                name.append( '/' );

                        if ( is_exe || !m_list_exe )
                                addItem( m_prepend + name );
                }
    }
}

/*
 * slotIOFinished
 *
 * Called when a KIO job is finished.
 * makeCompletion() if all directories have been listed
 */
void KURLCompletion::slotIOFinished( KIO::Job * job )
{
    m_list_job = 0L;

    if (job && job->error()
                && job->error() != KIO::ERR_DOES_NOT_EXIST
                && job->error() != KIO::ERR_CANNOT_ENTER_DIRECTORY)
        {
                job->showErrorDialog();
        }

        //kDebugInfo("slotIOFinished() -- %d dirs to go", m_dirs.count());

        // Continue listing the next directory in the list
        //
        if ( m_dirs.count() > 0 )
                listDirectories();
        // No more directories? Then were ready to find completions
        //
        else {
                m_running = false;

                //kdDebug() << "makeCompletion(" << m_compl_text << ") 3" << endl;

                KCompletion::makeCompletion( m_compl_text );
        }
}

/*
 * listEnvVar
 *
 * Return a list with all environment variables
 */
extern char **environ; // Array of environment variables

static void listEnvVar( QStringList *l )
{
        char **env = environ;

        while ( *env ) {
                QString s = QString::fromLocal8Bit( *env );

                int pos = s.find('=');

                if ( pos == -1 ) pos = s.length();

                if ( pos > 0 )
                        l->append( s.left(pos) );

                env++;
        }
}

/*
 * listUsers
 *
 * Return a list with all users
 */
static void listUsers( QStringList *l )
{
        struct passwd *pw;

        while ( (pw = ::getpwent()) )
                l->append( QString::fromLocal8Bit( pw->pw_name ) );

        ::endpwent();
}

/*
 * qstrBeginEq
 *
 * Return true if the first part of s2 equals s1
 */
static bool qstrBeginEq( const QString &s1, const QString &s2 )
{
        if ( s1.isEmpty() )
                return true;
        else
                return ( s2.left( s1.length() ) == s1 );
}

#include "kurlcompletion.moc"

