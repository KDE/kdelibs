/* This file is part of the KDE libraries
    Copyright (C) 2000 David Smith  <dsmith@algonet.se>

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

#ifndef KURLCOMPLETION_H
#define KURLCOMPLETION_H

#include <kcompletion.h>
#include <kio/jobclasses.h>
#include <qstring.h>
#include <qstringlist.h>

class KURL;
class KURLCompletionPrivate;

/**
 * This class does completion of URLs including user directories (~user)
 * and environment variables.  Remote URLs are passed to KIO.
 *
 * @short Completion of a single URL
 * @author David Smith <dsmith@algonet.se>
 * @version $Id$
 */
class KURLCompletion : public KCompletion
{
	Q_OBJECT

public:
	/**
	 * Determines how completion is done
	 * @li ExeCompletion - executables in $PATH or with full path
	 * @li FileCompletion - all files with full path or in dir(), URLs
	 * are listed using KIO
	 */
	enum Mode { ExeCompletion=1, FileCompletion };

	/**
	 * Constructs a KURLCompletion object in FileCompletion mode.
	 */
	KURLCompletion();
	/**
	 * This overloaded constructor allows you to set the Mode to ExeCompletion
	 * or FileCompletion without using @ref setMode. Default is FileCompletion
	 */
	KURLCompletion(Mode);
	/**
	 * Destructs the KURLCompletion object.
	 */
	virtual ~KURLCompletion();

	/**
	 * Finds completions to the given text.
	 *
	 * Remote URLs are listed with KIO. For performance reasons, local files
	 * are listed with KIO only if KURLCOMPLETION_LOCAL_KIO is set.
	 * The completion is done asyncronously if KIO is used.
	 *
	 * Returns the first match for user, environment, and local dir completion
	 * and QString::null for asyncronous completion (KIO).
	 */
	virtual QString makeCompletion(const QString&);

	/**
	 * Sets the current directory (used as base for completion)
	 * Default = $HOME.
	 */
	virtual void setDir(const QString &dir) { m_cwd = dir; };
	
	/**
	 * Returns the current directory.
	 */
	virtual QString dir() const { return m_cwd; };

	/**
	 * Returns true if asyncronous completion is in progress.
	 */
	virtual bool isRunning() const;

	/**
	 * Stops asyncronous completion.
	 */
	virtual void stop();

	/**
	 * Returns the completion mode: exe or file completion (default FileCompletion)
	 */
	virtual Mode mode() const { return m_mode; };

	/**
	 * Changes the completion mode: exe or file completion
	 */
	virtual void setMode( Mode mode ) { m_mode = mode; };

	/**
	 * Returns whether environment variables are completed and
	 * whether they are replaced internally while finding completions.
	 * Default is enabled.
	 */
	virtual bool replaceEnv() const { return m_replace_env; };
	
	/**
	 * Enables/disables completion and replacement (internally) of
	 * environment variables in URLs. Default is enabled.
	 */
	virtual void setReplaceEnv( bool replace ) { m_replace_env = replace; };

	/**
	 * Returns whether ~username is completed and whether ~username
	 * is replaced internally with the user's home directory while
	 * finding completions. Default is enabled.
	 */
	virtual bool replaceHome() { return m_replace_home; };
	
	/**
	 * Enables/disables completion of ~username and replacement
	 * (internally) of ~username with the user's home directory.
	 * Default is enabled.
	 */
	virtual void setReplaceHome( bool replace ) { m_replace_home = replace; };

	/**
	 * Replaces username and/or environment variables, depending on the
	 * current settings and returns the filtered url. Only works with
	 * local files, i.e. returns back the original string for non-local
         * urls.
	 */
	QString replacedPath( const QString& text );

        class MyURL;
        class DirLister;
protected:
	// Called by KCompletion, adds '/' to directories
	void postProcessMatch( QString *match ) const;
	void postProcessMatches( QStringList *matches ) const;

protected slots:
	void slotEntries( KIO::Job *, const KIO::UDSEntryList& );
	void slotIOFinished( KIO::Job * );

private slots:
	void slotTimer();

private:

        bool isAutoCompletion();
	
	bool userCompletion(const MyURL &url, QString *match);
	bool envCompletion(const MyURL &url, QString *match);
	bool exeCompletion(const MyURL &url, QString *match);
	bool fileCompletion(const MyURL &url, QString *match);
	bool urlCompletion(const MyURL &url, QString *match);

	// List a directory using readdir()
	void listDir( const QString& dir,
	              QStringList *matches,
	              const QString& filter,
	              bool only_exe,
	              bool no_hidden );

	// List the next dir in m_dirs
	QString listDirectories(const QStringList &,
	                        const QString &,
	                        bool only_exe = false,
	                        bool no_hidden = false,
	                        bool stat_files = true);

	void listURLs( const QValueList<KURL *> &urls,
		           const QString &filter = QString::null,
	               bool only_exe = false,
	               bool no_hidden = false );
	
	void addMatches( QStringList * );
	QString finished();
	
	void init();

	void setListedURL(int compl_type /* enum ComplType */,
	                  QString dir = QString::null,
	                  QString filter = QString::null,
	                  bool no_hidden = false );

	bool isListedURL( int compl_type /* enum ComplType */,
	                  QString dir = QString::null,
	                  QString filter = QString::null,
	                  bool no_hidden = false );

	QString m_last_path_listed;
	QString m_last_file_listed;
	int m_last_compl_type;
	int m_last_no_hidden; // int m_last_mode;

	QString m_cwd; // "current directory" = base dir for completion // WAS: m_dir
	
	Mode m_mode; // ExeCompletion or FileCompletion
	bool m_replace_env;
	bool m_replace_home;

	KIO::ListJob *m_list_job; // kio job to list directories

	QString m_prepend; // text to prepend to listed items
	QString m_compl_text; // text to pass on to KCompletion

	bool m_list_urls_only_exe; // true = only list executables
	bool m_list_urls_no_hidden; // bool m_running; // flag set when all dirs have been listed
	QString m_list_urls_filter; // QString m_file_filter; // filter for listed files

	QChar m_word_break_char;
	QChar m_quote_char1;
	QChar m_quote_char2;
	QChar m_escape_char;

	KURLCompletionPrivate *d;
};

#endif // KURLCOMPLETION_H
