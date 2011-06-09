/* This file is part of the KDE libraries
    Copyright (C) 2000 David Smith  <dsmith@algonet.se>

    This class was inspired by a previous KUrlCompletion by
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KURLCOMPLETION_H
#define KURLCOMPLETION_H

#include <kcompletion.h>
#include <kio/jobclasses.h>
#include <QtCore/QString>

class QStringList;
class KUrl;
class KUrlCompletionPrivate;

/**
 * This class does completion of URLs including user directories (~user)
 * and environment variables.  Remote URLs are passed to KIO.
 *
 * @short Completion of a single URL
 * @author David Smith <dsmith@algonet.se>
 */
class KIO_EXPORT KUrlCompletion : public KCompletion
{
    Q_OBJECT

public:
    /**
     * Determines how completion is done.
     * @li ExeCompletion - executables in $PATH or with full path.
     * @li FileCompletion - all files with full path or in dir(), URLs
     * are listed using KIO.
     * @li DirCompletion - Same as FileCompletion but only returns directories.
     */
    enum Mode { ExeCompletion = 1, FileCompletion, DirCompletion };

    /**
     * Constructs a KUrlCompletion object in FileCompletion mode.
     */
    KUrlCompletion();
    /**
     * This overloaded constructor allows you to set the Mode to ExeCompletion
     * or FileCompletion without using setMode. Default is FileCompletion.
     */
    KUrlCompletion(Mode);
    /**
     * Destructs the KUrlCompletion object.
     */
    virtual ~KUrlCompletion();

    /**
     * Finds completions to the given text.
     *
     * Remote URLs are listed with KIO. For performance reasons, local files
     * are listed with KIO only if KURLCOMPLETION_LOCAL_KIO is set.
     * The completion is done asyncronously if KIO is used.
     *
     * Returns the first match for user, environment, and local dir completion
     * and QString() for asynchronous completion (KIO or threaded).
     *
     * @param text the text to complete
     * @return the first match, or QString() if not found
     */
    virtual QString makeCompletion(const QString& text);

    /**
     * Sets the current directory (used as base for completion).
     * Default = $HOME.
     * @param dir the current directory, either as a path or URL
     */
    virtual void setDir(const QString& dir);

    /**
     * Returns the current directory, as it was given in setDir
     * @return the current directory (path or URL)
     */
    virtual QString dir() const;

    /**
     * Check whether asynchronous completion is in progress.
     * @return true if asynchronous completion is in progress
     */
    virtual bool isRunning() const;

    /**
     * Stops asynchronous completion.
     */
    virtual void stop();

    /**
     * Returns the completion mode: exe or file completion (default FileCompletion).
     * @return the completion mode
     */
    virtual Mode mode() const;

    /**
     * Changes the completion mode: exe or file completion
     * @param mode the new completion mode
     */
    virtual void setMode(Mode mode);

    /**
     * Checks whether environment variables are completed and
     * whether they are replaced internally while finding completions.
     * Default is enabled.
     * @return true if environment vvariables will be replaced
     */
    virtual bool replaceEnv() const;

    /**
     * Enables/disables completion and replacement (internally) of
     * environment variables in URLs. Default is enabled.
     * @param replace true to replace environment variables
     */
    virtual void setReplaceEnv(bool replace);

    /**
     * Returns whether ~username is completed and whether ~username
     * is replaced internally with the user's home directory while
     * finding completions. Default is enabled.
     * @return true to replace tilde with the home directory
     */
    virtual bool replaceHome() const;

    /**
     * Enables/disables completion of ~username and replacement
     * (internally) of ~username with the user's home directory.
     * Default is enabled.
     * @param replace true to replace tilde with the home directory
     */
    virtual void setReplaceHome(bool replace);

    /**
     * Replaces username and/or environment variables, depending on the
     * current settings and returns the filtered url. Only works with
     * local files, i.e. returns back the original string for non-local
     * urls.
     * @param text the text to process
     * @return the path or URL resulting from this operation. If you
         * want to convert it to a KUrl, use KUrl::fromPathOrUrl.
     */
    QString replacedPath(const QString& text) const;

    /**
     * @internal I'll let ossi add a real one to KShell :)
     */
    static QString replacedPath(const QString& text,
                                bool replaceHome, bool replaceEnv = true);

protected:
    // Called by KCompletion, adds '/' to directories
    void postProcessMatch(QString* match) const;
    void postProcessMatches(QStringList* matches) const;
    void postProcessMatches(KCompletionMatches* matches) const;

    virtual void customEvent(QEvent* e);

private:
    KUrlCompletionPrivate* const d;

    Q_PRIVATE_SLOT(d, void _k_slotEntries (KIO::Job*, const KIO::UDSEntryList&))
    Q_PRIVATE_SLOT(d, void _k_slotIOFinished (KJob*))
};

#endif // KURLCOMPLETION_H
