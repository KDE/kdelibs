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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.	If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kdebug.h>
#include <kcompletion.h>
#include <kurl.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>

#include "kurlcompletion.h"

#if defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif

static bool expandTilde(QString &);
static bool expandEnv(QString &);

static QString unescape(const QString &text);

// Permission mask for files that are executable by
// user, group or other
#define MODE_EXE (S_IXUSR | S_IXGRP | S_IXOTH)

// Constants for types of completion
enum ComplType {CTNone=0, CTEnv, CTUser, CTMan, CTExe, CTFile, CTUrl, CTInfo};

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// MyURL - wrapper for KURL with some different functionality
//

class KURLCompletion::MyURL
{
public:
	MyURL(const QString &url, const QString &cwd);
	MyURL(const MyURL &url);
	~MyURL();

	KURL *kurl() const { return m_kurl; };

	QString protocol() const { return m_kurl->protocol(); };
	// The directory with a trailing '/'
	QString dir() const { return m_kurl->directory(false, false); };
	QString file() const { return m_kurl->fileName(false); };
	
	QString url() const { return m_url; };

	QString orgUrlWithoutFile() const { return m_orgUrlWithoutFile; };
	
	void filter( bool replace_user_dir, bool replace_env );

private:
	void init(const QString &url, const QString &cwd);

	KURL *m_kurl;
	QString m_url;
	QString m_orgUrlWithoutFile;
};

KURLCompletion::MyURL::MyURL(const QString &url, const QString &cwd)
{
	init(url, cwd);
}

KURLCompletion::MyURL::MyURL(const MyURL &url)
{
	m_kurl = new KURL( *(url.m_kurl) );
	m_url = url.m_url;
	m_orgUrlWithoutFile = url.m_orgUrlWithoutFile;
}

void KURLCompletion::MyURL::init(const QString &url, const QString &cwd)
{
	// Save the original text
	m_url = url;

	// Non-const copy
	QString url_copy = url;
	
	// Special shortcuts for "man:" and "info:"
	if ( url_copy[0] == '#' ) {
		if ( url_copy[1] == '#' )
			url_copy.replace( 0, 2, QString("info:") );
		else
			url_copy.replace( 0, 1, QString("man:") );
	}
		
	// Look for a protocol in 'url' 
	QRegExp protocol_regex = QRegExp( "^[^/\\s\\\\]*:" );
	
	// Assume "file:" or whatever is given by 'cwd' if there is 
	// no protocol.  (KURL does this only for absoute paths)
	if ( protocol_regex.search( url_copy ) == 0 ) {
		m_kurl = new KURL( url_copy );

		// KURL doesn't parse only a protocol (like "smb:")
		if ( m_kurl->protocol().isEmpty() ) {
			QString protocol = url_copy.left( protocol_regex.matchedLength() - 1 );
			m_kurl->setProtocol( protocol );
		}
	}
	else if ( protocol_regex.search( cwd ) == 0 
			&& url_copy[0] != '/'
			&& url_copy[0] != '~' )
	{
		// 'cwd' contains a protocol and url_copy is not absolute
		// or a users home directory
		QString protocol = cwd.left( protocol_regex.matchedLength() - 1 );
		m_kurl = new KURL( protocol + ":" + url_copy );
	}
	else {
		// Use 'file' as default protocol 
		m_kurl = new KURL( QString("file:") + url_copy );
	}

	// URL with file stripped
	m_orgUrlWithoutFile = m_url.left( m_url.length() - file().length() );
}	

KURLCompletion::MyURL::~MyURL()
{
	delete m_kurl;
}

void KURLCompletion::MyURL::filter( bool replace_user_dir, bool replace_env )
{
	if ( !dir().isEmpty() ) {
		QString d = dir();
		if ( replace_user_dir ) expandTilde( d );
		if ( replace_env ) expandEnv( d );
		m_kurl->setPath( d + file() );
	}
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// DirLister - list files with timeout
//

class KURLCompletion::DirLister
{
public:
	DirLister() : m_current(0), m_only_exe(false), m_only_dir(false), m_no_hidden(false), 
		      m_append_slash_to_dir(false), m_dp(0L), m_clk(0), m_timeout(50) { };
	~DirLister();

	bool listDirectories( const QStringList &dirs,
	                      const QString &filter,
	                      bool only_exe,
	                      bool only_dir,
	                      bool no_hidden,
	                      bool append_slash_to_dir);

	void setFilter( const QString& filter );

	bool isRunning();
	void stop();

	bool listBatch();
	
	QStringList *files() { return &m_files; };

	void setTimeout(int milliseconds) { m_timeout = milliseconds; };

private:
	QStringList  m_dir_list;
	unsigned int m_current;

	QString m_filter;
	bool    m_only_exe;
	bool    m_only_dir;
	bool    m_no_hidden;
	bool    m_append_slash_to_dir;

	DIR *m_dp;

	QStringList m_files;
	
	clock_t m_clk;
	clock_t m_timeout;

	void  startTimer();
	bool  timeout();
};

KURLCompletion::DirLister::~DirLister()
{
	stop();
}

// Start the internal time out counter. Used by listBatch()
void KURLCompletion::DirLister::startTimer()
{
	m_clk = ::clock();
}

#define CLOCKS_PER_MS (CLOCKS_PER_SEC/1000)

// Returns true m_timeout ms after startTimer() has been called
bool KURLCompletion::DirLister::timeout()
{
	return (m_clk > 0) &&
	         (::clock() - m_clk > m_timeout * CLOCKS_PER_MS);
}

// Change the file filter while DirLister is running
void KURLCompletion::DirLister::setFilter( const QString& filter )
{
	m_filter = filter;
}

// Returns true until alla directories have been listed
// after a call to listDirectoris
bool KURLCompletion::DirLister::isRunning()
{
	return m_dp != 0L || m_current < m_dir_list.count();
}

void KURLCompletion::DirLister::stop()
{
	if ( m_dp ) {
		::closedir( m_dp );
		m_dp = 0L;
	}
}

/*
 * listDirectories
 *
 * List the given directories, putting the result in files()
 * Gives control back after m_timeout ms, then listBatch() can be called to
 * go on for another timeout period until all directories are done
 *
 * Returns true if all directories are done within the first 50 ms
 */
bool KURLCompletion::DirLister::listDirectories(
		const QStringList& dir_list,
		const QString& filter,
		bool only_exe,
		bool only_dir,
		bool no_hidden,
		bool append_slash_to_dir)
{
	stop();

	m_dir_list = dir_list;
	m_filter = filter;
	m_only_exe = only_exe;
	m_only_dir = only_dir;
	m_no_hidden = no_hidden;
	m_append_slash_to_dir = append_slash_to_dir;

//	kdDebug() << "DirLister: stat_files = " << (m_only_exe || m_append_slash_to_dir) << endl;

	m_files.clear();
	m_current = 0;
	
	// Start listing
	return listBatch();
}

/*
 * listBatch
 *
 * Get entries from directories in m_dir_list
 * Return false if timed out, and true when all directories are done
 */
bool KURLCompletion::DirLister::listBatch()
{
	startTimer();

	while ( m_current < m_dir_list.count() ) {

		// Open the next directory
		if ( !m_dp ) {
			m_dp = ::opendir( QFile::encodeName( m_dir_list[ m_current ] ) );
			
			if ( m_dp == NULL ) {
				kdDebug() << "Failed to open dir: " << m_dir_list[ m_current ] << endl;
				return true;
			}
		}

		// A trick from KIO that helps performance by a little bit:
		// chdir to the directroy so we won't have to deal with full paths
		// with stat()
		char path_buffer[PATH_MAX];
		::getcwd(path_buffer, PATH_MAX - 1);
		::chdir( QFile::encodeName( m_dir_list[m_current] ) );
		
		struct dirent *ep;
		int cnt = 0;
		bool time_out = false;

		int filter_len = m_filter.length();

		// Loop through all directory entries
		while ( !time_out && ( ep = ::readdir( m_dp ) ) != 0L ) {
			
			// Time to rest...?
			if ( cnt++ % 10 == 0 && timeout() )
				time_out = true;  // finish this file, then break

			// Skip ".." and "."
			// Skip hidden files if m_no_hidden is true
			if ( ep->d_name[0] == '.' ) {
				if ( m_no_hidden )
					continue;
				if ( ep->d_name[1] == '\0' ||
					  ( ep->d_name[1] == '.' && ep->d_name[2] == '\0' ) )
					continue;
			}

			QString file = QFile::decodeName( ep->d_name );

			if ( filter_len == 0 || file.startsWith( m_filter ) ) {
				
				if ( m_only_exe || m_only_dir || m_append_slash_to_dir ) {
					struct stat sbuff;
			
					if ( ::stat( ep->d_name, &sbuff ) == 0 ) {
						// Verify executable
						//
						if ( m_only_exe && 0 == (sbuff.st_mode & MODE_EXE) )
							continue;
					
						// Verify directory
						//
						if ( m_only_dir && !S_ISDIR ( sbuff.st_mode ) )
							continue;

						// Add '/' to directories
						//
						if ( m_append_slash_to_dir && S_ISDIR ( sbuff.st_mode ) )
							file.append( '/' );
						
					}
					else {
						kdDebug() << "Could not stat file " << file << endl;
						continue;
					}
				}
				m_files.append( file );
			}
		}

		// chdir to the original directory
		::chdir( path_buffer );

		if ( time_out ) {
			return false; // not done
		}
		else {
			::closedir( m_dp );
			m_dp = NULL;
			m_current++;
		}
	}

	return true; // all directories listed
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// KURLCompletionPrivate
//
class KURLCompletionPrivate
{
public:
	KURLCompletionPrivate() : dir_lister(0L),
	                          url_auto_completion(true) {};
	~KURLCompletionPrivate();

	QValueList<KURL*> list_urls;

	KURLCompletion::DirLister *dir_lister;
	
	// urlCompletion() in Auto/Popup mode?
	bool url_auto_completion;
	
	// Append '/' to directories in Popup mode?
	// Doing that stat's all files and is slower
	bool popup_append_slash;

	// Keep track of currently listed files to avoid reading them again
	QString last_path_listed;
	QString last_file_listed;
	int last_compl_type;
	int last_no_hidden;

	QString cwd; // "current directory" = base dir for completion
	
	KURLCompletion::Mode mode; // ExeCompletion, FileCompletion, DirCompletion
	bool replace_env;
	bool replace_home;

	KIO::ListJob *list_job; // kio job to list directories

	QString prepend; // text to prepend to listed items
	QString compl_text; // text to pass on to KCompletion

	// Filters for files read with  kio
	bool list_urls_only_exe; // true = only list executables
	bool list_urls_no_hidden;
	QString list_urls_filter; // filter for listed files
};

KURLCompletionPrivate::~KURLCompletionPrivate()
{
	assert( dir_lister == 0L );
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// KURLCompletion
//

KURLCompletion::KURLCompletion() : KCompletion()
{
	init( FileCompletion );
}


KURLCompletion::KURLCompletion( Mode mode ) : KCompletion()
{
	init( mode );
}

KURLCompletion::~KURLCompletion()
{
	stop();
	delete d;
}


void KURLCompletion::init( Mode mode )
{
	d = new KURLCompletionPrivate;

	d->mode = mode;
	d->cwd = QDir::homeDirPath();

	d->replace_home = true;
	d->replace_env = true;
	d->last_no_hidden = false;
	d->last_compl_type = 0;

	d->list_job = 0L;

	// Read settings
	KConfig *c = KGlobal::config();
	KConfigGroupSaver cgs( c, "URLCompletion" );

	d->url_auto_completion = c->readBoolEntry("alwaysAutoComplete", true);
	d->popup_append_slash = c->readBoolEntry("popupAppendSlash", true);
}

void KURLCompletion::setDir(const QString &dir)
{
	d->cwd = dir;
}

QString KURLCompletion::dir() const 
{
	return d->cwd;
}

KURLCompletion::Mode KURLCompletion::mode() const
{
	return d->mode;
}

void KURLCompletion::setMode( Mode mode )
{
	d->mode = mode;
}

bool KURLCompletion::replaceEnv() const
{
	return d->replace_env;
}

void KURLCompletion::setReplaceEnv( bool replace )
{
	d->replace_env = replace;
}

bool KURLCompletion::replaceHome() const
{
	return d->replace_home;
};

void KURLCompletion::setReplaceHome( bool replace )
{
	d->replace_home = replace;
}

/*
 * makeCompletion()
 *
 * Entry point for file name completion
 */
QString KURLCompletion::makeCompletion(const QString &text)
{
	//kdDebug() << "KURLCompletion::makeCompletion: " << text << endl;

	MyURL url(text, d->cwd);

	d->compl_text = text;
	d->prepend = url.orgUrlWithoutFile();

	QString match;
	
	// Environment variables
	//
	if ( d->replace_env && envCompletion( url, &match ) )
		return match;
	
	// User directories
	//
	if ( d->replace_home && userCompletion( url, &match ) )
		return match;
	
	// Replace user directories and variables
	url.filter( d->replace_home, d->replace_env );

	//kdDebug() << "Filtered: proto=" << url.protocol()
	//          << ", dir=" << url.dir()
	//          << ", file=" << url.file()
	//          << ", kurl url=" << url.kurl()->url() << endl;

	if ( d->mode == ExeCompletion ) {
		// Executables
		//
		if ( exeCompletion( url, &match ) )
			return match;

		// KRun can run "man:" and "info:" etc. so why not treat them
		// as executables...
		
		if ( urlCompletion( url, &match ) )
			return match;
	}
	else {
		// Local files, directories
		//
		if ( fileCompletion( url, &match ) )
			return match;

		// All other...
		//
		if ( urlCompletion( url, &match ) )
			return match;
	}

	setListedURL( CTNone );
	stop();

	return QString::null;
}

/*
 * finished
 *
 * Go on and call KCompletion.
 * Called when all matches have been added
 */
QString KURLCompletion::finished()
{
	if ( d->last_compl_type == CTInfo )
		return KCompletion::makeCompletion( d->compl_text.lower() );
	else
		return KCompletion::makeCompletion( d->compl_text );
}

/*
 * isRunning
 *
 * Return true if either a KIO job or the DirLister
 * is running
 */
bool KURLCompletion::isRunning() const
{
	return (d->list_job != 0L ||
	        (d->dir_lister != 0L && d->dir_lister->isRunning() ));
}

/*
 * stop
 *
 * Stop and delete a running KIO job or the DirLister
 */
void KURLCompletion::stop()
{
	if ( d->list_job ) {
		d->list_job->kill();
		d->list_job = 0L;
	}

	if ( !d->list_urls.isEmpty() ) {
		QValueList<KURL*>::Iterator it = d->list_urls.begin();
		for ( ; it != d->list_urls.end(); it++ )
			delete (*it);
		d->list_urls.clear();
	}
	
	if ( d->dir_lister ) {
		delete d->dir_lister;
		d->dir_lister = 0L;
	}
}

/*
 * Keep track of the last listed directory
 */
void KURLCompletion::setListedURL( int complType,
                                   QString dir,
                                   QString filter,
                                   bool no_hidden )
{
	d->last_compl_type = complType;
	d->last_path_listed = dir;
	d->last_file_listed = filter;
	d->last_no_hidden = (int)no_hidden;
}

bool KURLCompletion::isListedURL( int complType,
                                  QString dir,
                                  QString filter,
                                  bool no_hidden )
{
	return  d->last_compl_type == complType
			&& ( d->last_path_listed == dir
					|| (dir.isEmpty() && d->last_path_listed.isEmpty()) )
			&& ( filter.startsWith(d->last_file_listed)
					|| (filter.isEmpty() && d->last_file_listed.isEmpty()) )
			&& d->last_no_hidden == (int)no_hidden;
}

/*
 * isAutoCompletion
 *
 * Returns true if completion mode is Auto or Popup
 */
bool KURLCompletion::isAutoCompletion()
{
	return completionMode() == KGlobalSettings::CompletionAuto
	       || completionMode() == KGlobalSettings::CompletionPopup
	       || completionMode() == KGlobalSettings::CompletionMan
	       || completionMode() == KGlobalSettings::CompletionPopupAuto;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
// User directories
//

bool KURLCompletion::userCompletion(const MyURL &url, QString *match)
{
	if ( url.protocol() != "file"
	      || !url.dir().isEmpty()
	      || url.file().at(0) != '~' )
		return false;

	if ( !isListedURL( CTUser ) ) {
		stop();
		clear();

		struct passwd *pw;

		QString tilde = QString("~");

		QStringList l;
		
		while ( (pw = ::getpwent()) ) {
			QString user = QString::fromLocal8Bit( pw->pw_name );
			
			l.append( tilde + user );
		}
	
		::endpwent();
		
		l.append( tilde ); // just ~ is a match to

		addMatches( &l );
	}

	setListedURL( CTUser );

	*match = finished();
	return true;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Environment variables
//

extern char **environ; // Array of environment variables

bool KURLCompletion::envCompletion(const MyURL &url, QString *match)
{
	if ( url.file().at(0) != '$' )
		return false;

	if ( !isListedURL( CTEnv ) ) {
		stop();
		clear();
		
		char **env = environ;

		QString dollar = QString("$");
		
		QStringList l;
		
		while ( *env ) {
			QString s = QString::fromLocal8Bit( *env );

			int pos = s.find('=');
			
			if ( pos == -1 )
				pos = s.length();

			if ( pos > 0 )
				l.append( dollar + s.left(pos) );

			env++;
		}
		
		addMatches( &l );
	}

	setListedURL( CTEnv );

	*match = finished();
	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Executables
//

bool KURLCompletion::exeCompletion(const MyURL &url, QString *match)
{
	if ( url.protocol() != "file" )
		return false;
		
	QString dir = url.dir();

	dir = unescape( dir ); // remove escapes

	// Find directories to search for completions, either
	//
	// 1. complete path given in url
	// 2. current directory (d->cwd)
	// 3. $PATH
	// 4. no directory at all

	QStringList dirList;

	if ( dir[0] == '/' ) {
		// complete path in url
		dirList.append( dir );
	}
	else if ( !dir.isEmpty() && !d->cwd.isEmpty() ) {
		// current directory
		dirList.append( d->cwd + '/' + dir );
	}
	else if ( !url.file().isEmpty() ) {
		// $PATH
		dirList = QStringList::split(':',
					QString::fromLocal8Bit(::getenv("PATH")));

		QStringList::Iterator it = dirList.begin();
		
		for ( ; it != dirList.end(); it++ )
			(*it).append('/');
	}

	// No hidden files unless the user types "."
	bool no_hidden_files = url.file().at(0) != '.';
		
	// List files if needed
	//
	if ( !isListedURL( CTExe, dir, url.file(), no_hidden_files ) )
	{	
		stop();
		clear();
		
		setListedURL( CTExe, dir, url.file(), no_hidden_files );

		*match = listDirectories( dirList, url.file(), true, false, no_hidden_files );
	}
	else if ( !isRunning() ) {
		*match = finished();
	}
	else {
		if ( d->dir_lister ) {
			setListedURL( CTExe, dir, url.file(), no_hidden_files );
			d->dir_lister->setFilter( url.file() );
		}
		*match = QString::null;
	}

	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Local files
//

bool KURLCompletion::fileCompletion(const MyURL &url, QString *match)
{
	if ( url.protocol() != "file" )
		return false;
		
	QString dir = url.dir();

	dir = unescape( dir ); // remove escapes

	// Find directories to search for completions, either
	//
	// 1. complete path given in url
	// 2. current directory (d->cwd)
	// 3. no directory at all

	QStringList dirList;

	if ( dir[0] == '/' ) {
		// complete path in url
		dirList.append( dir );
	}
	else if ( !d->cwd.isEmpty() ) {
		// current directory
		dirList.append( d->cwd + '/' + dir );
	}

	// No hidden files unless the user types "."
	bool no_hidden_files = ( url.file().at(0) != '.' );
		
	// List files if needed
	//
	if ( !isListedURL( CTFile, dir, "", no_hidden_files ) )
	{	
		stop();
		clear();
		
		setListedURL( CTFile, dir, "", no_hidden_files );
	
		// Append '/' to directories in Popup mode?
		bool append_slash = ( d->popup_append_slash
	    	&& (completionMode() == KGlobalSettings::CompletionPopup ||
		    completionMode() == KGlobalSettings::CompletionPopupAuto ) );

		bool only_dir = ( d->mode == DirCompletion );
		
		*match = listDirectories( dirList, "", false, only_dir, no_hidden_files,
		                          append_slash );
	}
	else if ( !isRunning() ) {
		*match = finished();
	}
	else {
/*		if ( d->dir_lister ) {
			setListedURL( CTFile, dir, url.file(), no_hidden_files );
			d->dir_lister->setFilter( url.file() );
		}
*/
		*match = QString::null;
	}

	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// URLs not handled elsewhere...
//

bool KURLCompletion::urlCompletion(const MyURL &url, QString *match)
{
	//kdDebug() << "urlCompletion: url = " << url.kurl()->prettyURL() << endl;

	// Use d->cwd as base url in case url is not absolute
	KURL url_cwd = KURL( d->cwd ); 
	
	// Create an URL with the directory to be listed
	KURL *url_dir = new KURL( url_cwd, url.kurl()->url() );

	// Don't try url completion if
	// 1. malformed url
	// 2. protocol that doesn't have listDir()
	// 3. there is no directory (e.g. "ftp://ftp.kd" shouldn't do anything)
	// 4. auto or popup completion mode depending on settings

	bool man_or_info = ( url_dir->protocol() == QString("man")
	                     || url_dir->protocol() == QString("info") );

	if ( url_dir->isMalformed()
	     || !KProtocolInfo::supportsListing( *url_dir )
	     || ( !man_or_info
	          && ( url_dir->directory(false,false).isEmpty()
	               || ( isAutoCompletion()
	                    && !d->url_auto_completion ) ) ) )
		return false;

	url_dir->setFileName(""); // not really nesseccary, but clear the filename anyway...

	// Remove escapes
	QString dir = url_dir->directory( false, false );

	dir = unescape( dir );

	url_dir->setPath( dir );

	// List files if needed
	//
	if ( !isListedURL( CTUrl, url_dir->prettyURL(), url.file() ) )
	{	
		stop();
		clear();
		
		setListedURL( CTUrl, url_dir->prettyURL(), "" );
		
		QValueList<KURL*> url_list;
		url_list.append(url_dir);

		listURLs( url_list, "", false );

		*match = QString::null;
	}
	else if ( !isRunning() ) {
		delete url_dir;
		*match = finished();
	}
	else {
		delete url_dir;
		*match = QString::null;
	}

	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Directory and URL listing
//

/*
 * addMatches
 *
 * Called to add matches to KCompletion
 */
void KURLCompletion::addMatches( QStringList *matches )
{
	QStringList::ConstIterator it = matches->begin();
	QStringList::ConstIterator end = matches->end();

	for ( ; it != end; it++ )
		addItem( d->prepend + (*it));
}

/*
 * slotTimer
 *
 * Keeps calling listBatch() on d->dir_lister until it is done
 * with all directories, then makes completion by calling
 * addMatches() and finished()
 */
void KURLCompletion::slotTimer()
{
	// dir_lister is NULL if stop() has been called
	if ( d->dir_lister ) {

		bool done = d->dir_lister->listBatch();

//		kdDebug() << "listed: " << d->dir_lister->files()->count() << endl;

		if ( done ) {
			addMatches( d->dir_lister->files() );
			finished();

			delete d->dir_lister;
			d->dir_lister = 0L;
		}
		else {
			QTimer::singleShot( 0, this, SLOT(slotTimer()) );
		}
	}
}

/*
 * listDirectories
 *
 * List files starting with 'filter' in the given directories,
 * either using DirLister or listURLs()
 *
 * In either case, addMatches() is called with the listed
 * files, and eventually finished() when the listing is done
 *
 * Returns the match if available, or QString::null if
 * DirLister timed out or using kio
 */
QString KURLCompletion::listDirectories(
		const QStringList &dirs,
		const QString &filter,
		bool only_exe,
		bool only_dir,
		bool no_hidden,
        bool append_slash_to_dir)
{
//	kdDebug() << "Listing (listDirectories): " << dirs.join(",") << endl;
	
	assert( !isRunning() );
	
	if ( !::getenv("KURLCOMPLETION_LOCAL_KIO") ) {
		
		// Don't use KIO
		
		if (!d->dir_lister)
			d->dir_lister = new DirLister;
			
		assert( !d->dir_lister->isRunning() );

	
		if ( isAutoCompletion() )
			// Start with a longer timeout as a compromize to
			// be able to return the match more often
			d->dir_lister->setTimeout(100); // 100 ms
		else
			// More like no timeout for manual completion
			d->dir_lister->setTimeout(3000); // 3 s
			
		
		bool done = d->dir_lister->listDirectories(dirs,
		                                      filter,
		                                      only_exe,
		                                      only_dir,
		                                      no_hidden,
		                                      append_slash_to_dir);
		
		d->dir_lister->setTimeout(20); // 20 ms
		
		QString match = QString::null;
		
		if ( done ) {
			// dir_lister finished before the first timeout
			addMatches( d->dir_lister->files() );
			match = finished();

			delete d->dir_lister;
			d->dir_lister = 0L;
		}
		else {
			// dir_lister timed out, let slotTimer() continue
			// the work...
			QTimer::singleShot( 0, this, SLOT(slotTimer()) );
		}

		return match;
	}
	else {

		// Use KIO
		
		QValueList<KURL*> url_list;
		
		QStringList::ConstIterator it = dirs.begin();

		for ( ; it != dirs.end(); it++ )
			url_list.append( new KURL(*it) );
		
		listURLs( url_list, filter, only_exe, no_hidden );
			// Will call addMatches() and finished()
		
		return QString::null;
	}
}

/*
 * listURLs
 *
 * Use KIO to list the given urls
 *
 * addMatches() is called with the listed files
 * finished() is called when the listing is done
 */
void KURLCompletion::listURLs(
		const QValueList<KURL *> &urls,
		const QString &filter,
		bool only_exe,
		bool no_hidden )
{
	assert( d->list_urls.isEmpty() );
	assert( d->list_job == 0L );

	d->list_urls = urls;
	d->list_urls_filter = filter;
	d->list_urls_only_exe = only_exe;
	d->list_urls_no_hidden = no_hidden;

//	kdDebug() << "Listing URLs: " << urls[0]->prettyURL() << ",..." << endl;
	
	// Start it off by calling slotIOFinished
	//
	// This will start a new list job as long as there
	// are urls in d->list_urls
	//
	slotIOFinished(0L);
}

/*
 * slotEntries
 *
 * Receive files listed by KIO and call addMatches()
 */
void KURLCompletion::slotEntries(KIO::Job*, const KIO::UDSEntryList& entries)
{
	QStringList matches;
	
	KIO::UDSEntryListConstIterator it = entries.begin();
	KIO::UDSEntryListConstIterator end = entries.end();

	QString filter = d->list_urls_filter;
	
	int filter_len = filter.length();

	// Iterate over all files
	//
	for (; it != end; ++it) {
		QString name;
		bool is_exe = false;
		bool is_dir = false;

		KIO::UDSEntry e = *it;
		KIO::UDSEntry::ConstIterator it_2 = e.begin();

		for( ; it_2 != e.end(); it_2++ ) {
			switch ( (*it_2).m_uds ) {
				case KIO::UDS_NAME:
					name = (*it_2).m_str;
					break;
				case KIO::UDS_ACCESS:
					is_exe = ((*it_2).m_long & MODE_EXE) != 0;
					break;
				case KIO::UDS_FILE_TYPE:
					is_dir = ((*it_2).m_long & S_IFDIR) != 0;
					break;
			}
		}

		if ( name[0] == '.' &&
		     ( d->list_urls_no_hidden ||
		        name.length() == 1 ||
		          ( name.length() == 2 && name[1] == '.' ) ) )
			continue;

		if ( d->mode == DirCompletion && !is_dir )
			continue;

		if ( filter_len == 0 || name.left(filter_len) == filter ) {
			if ( is_dir )
				name.append( '/' );

			if ( is_exe || !d->list_urls_only_exe )
				matches.append( name );
		}
	}

	addMatches( &matches );
}

/*
 * slotIOFinished
 *
 * Called when a KIO job is finished.
 *
 * Start a new list job if there are still urls in
 * d->list_urls, otherwise call finished()
 */
void KURLCompletion::slotIOFinished( KIO::Job * job )
{
//	kdDebug() << "slotIOFinished() " << endl;

	assert( job == d->list_job );

	if ( d->list_urls.isEmpty() ) {
		
		d->list_job = 0L;
		
		finished(); // will call KCompletion::makeCompletion()

	}
	else {

		KURL *kurl = d->list_urls.first();

		d->list_urls.remove( kurl );

//		kdDebug() << "Start KIO: " << kurl->prettyURL() << endl;

		d->list_job = KIO::listDir( *kurl, false );

		assert( d->list_job );

		connect( d->list_job,
				SIGNAL(result(KIO::Job*)),
				SLOT(slotIOFinished(KIO::Job*)) );

		connect( d->list_job,
				SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
				SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)) );

		delete kurl;
	}
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////

/*
 * postProcessMatch, postProcessMatches
 *
 * Called by KCompletion before emitting match() and matches()
 *
 * Append '/' to directories for file completion. This is
 * done here to avoid stat()'ing a lot of files
 */
void KURLCompletion::postProcessMatch( QString *match ) const
{
//	kdDebug() << "KURLCompletion::postProcess: " << *match << endl;

	if ( !match->isEmpty() ) {

		// Add '/' to directories in file completion mode
		// unless it has already been done
		if ( d->last_compl_type == CTFile
		       && (*match).at( (*match).length()-1 ) != '/' )
		{
			QString copy;

			if ( (*match).startsWith( QString("file:") ) )
				copy = (*match).mid(5);
			else
				copy = *match;

			expandTilde( copy );
			expandEnv( copy );
			if ( copy[0] != '/' )
				copy.prepend( d->cwd + '/' );

//			kdDebug() << "postProcess: stating " << copy << endl;

			struct stat sbuff;

			QCString file = QFile::encodeName( copy );

			if ( ::stat( (const char*)file, &sbuff ) == 0 ) {
				if ( S_ISDIR ( sbuff.st_mode ) )
					match->append( '/' );
			}
			else {
				kdDebug() << "Could not stat file " << copy << endl;
			}
		}
	}
}

void KURLCompletion::postProcessMatches( QStringList * /*matches*/ ) const
{
	// Maybe '/' should be added to directories here as in
	// postProcessMatch() but it would slow things down
	// when there are a lot of matches...
}

void KURLCompletion::postProcessMatches( KCompletionMatches * /*matches*/ ) const
{
	// Maybe '/' should be added to directories here as in
	// postProcessMatch() but it would slow things down
	// when there are a lot of matches...
}

QString KURLCompletion::replacedPath( const QString& text )
{
	MyURL url( text, d->cwd );
	if ( !url.kurl()->isLocalFile() )
		return text;

	url.filter( d->replace_home, d->replace_env );
	return url.dir() + url.file();
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// Static functions

/*
 * expandEnv
 *
 * Expand environment variables in text. Escaped '$' are ignored.
 * Return true if expansion was made.
 */
static bool expandEnv( QString &text )
{
	// Find all environment variables beginning with '$'
	//
	int pos = 0;

	bool expanded = false;

	while ( (pos = text.find('$', pos)) != -1 ) {
		
		// Skip escaped '$'
		//
		if ( text[pos-1] == '\\' ) {
			pos++;
		}
		// Variable found => expand
		//
		else {
			// Find the end of the variable = next '/' or ' '
			//
			int pos2 = text.find( ' ', pos+1 );
			int pos_tmp = text.find( '/', pos+1 );
			
			if ( pos2 == -1 || (pos_tmp != -1 && pos_tmp < pos2) )
				pos2 = pos_tmp;

			if ( pos2 == -1 )
				pos2 = text.length();

			// Replace if the variable is terminated by '/' or ' '
			// and defined
			//
			if ( pos2 >= 0 ) {
				int len	= pos2 - pos;
				QString key	= text.mid( pos+1, len-1);
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

	return expanded;
}

/*
 * expandTilde
 *
 * Replace "~user" with the users home directory
 * Return true if expansion was made.
 */
static bool expandTilde(QString &text)
{
	if ( text[0] != '~' )
		return false;

	bool expanded = false;

	// Find the end of the user name = next '/' or ' '
	//
	int pos2 = text.find( ' ', 1 );
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
				dir = QFile::decodeName( pw->pw_dir );
				
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
 * unescape
 *
 * Remove escapes and return the result in a new string
 *
 */
static QString unescape(const QString &text)
{
	QString result;

	for (uint pos = 0; pos < text.length(); pos++)
		if ( text[pos] != '\\' )
			result.insert( result.length(), text[pos] );
		
	return result;
}

void KURLCompletion::virtual_hook( int id, void* data )
{ KCompletion::virtual_hook( id, data ); }

#include "kurlcompletion.moc"

