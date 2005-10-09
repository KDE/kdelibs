//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __kbookmarkmanager_h
#define __kbookmarkmanager_h

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qdom.h>
#include <qpair.h>
#include <dcopobject.h>
#include "kbookmark.h"
#include "kbookmarknotifier.h"

class KBookmarkManagerList;

/**
 * This class implements the reading/writing of bookmarks in XML.
 * The bookmarks file is read and written using the XBEL standard
 * (http://pyxml.sourceforge.net/topics/xbel/)
 *
 * A sample file looks like this :
 * \code
 * <xbel>
 *   <bookmark href="http://developer.kde.org"><title>Developer Web Site</title></bookmark>
 *   <folder folded="no">
 *     <title>Title of this folder</title>
 *     <bookmark icon="kde" href="http://www.kde.org"><title>KDE Web Site</title></bookmark>
 *     <folder toolbar="yes">
 *       <title>My own bookmarks</title>
 *       <bookmark href="http://www.koffice.org"><title>KOffice Web Site</title></bookmark>
 *       <separator/>
 *       <bookmark href="http://www.kdevelop.org"><title>KDevelop Web Site</title></bookmark>
 *     </folder>
 *   </folder>
 * </xbel>
 * \endcode
 */
class KIO_EXPORT KBookmarkManager : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
protected:
    /**
     * Creates a bookmark manager with a path to the bookmarks.  By
     * default, it will use the KDE standard dirs to find and create the
     * correct location.  If you are using your own app-specific
     * bookmarks directory, you must instantiate this class with your
     * own path <em>before</em> KBookmarkManager::managerForFile() is ever
     * called.
     *
     * @param bookmarksFile full path to the bookmarks file,
     * Use ~/.kde/share/apps/konqueror/bookmarks.xml for the konqueror bookmarks
     *
     * @param bImportDesktopFiles if true, and if the bookmarksFile
     * doesn't already exist, import bookmarks from desktop files
     */
    KBookmarkManager( const QString & bookmarksFile, bool bImportDesktopFiles = true );

    /**
     * @since 3.2
     */
    KBookmarkManager();

public:
    /**
     * Destructor
     */
    ~KBookmarkManager();

    /**
     * Set the update flag. Defaults to true. TODO - check
     * @param update if true then KBookmarkManager will listen to DCOP update requests.
     */
    void setUpdate( bool update );

    /**
     * Save the bookmarks to the default konqueror XML file on disk.
     * You should use emitChanged() instead of this function, it saves
     * and notifies everyone that the file has changed.
     * @param toolbarCache iff true save a cache of the toolbar folder, too
     * @return true if saving was successful
     */
    bool save( bool toolbarCache = true ) const;

    /**
     * Save the bookmarks to the given XML file on disk.
     * @param filename full path to the desired bookmarks file location
     * @param toolbarCache iff true save a cache of the toolbar folder, too
     * @return true if saving was successful
     */
    bool saveAs( const QString & filename, bool toolbarCache = true ) const;

    /**
     * Update access time stamps for a given url.
     * @param url the viewed url
     * @param emitSignal iff true emit KBookmarkNotifier signal
     * @since 3.2
     * @return true if any metadata was modified (bookmarks file is not saved automatically)
     */
    bool updateAccessMetadata( const QString &url, bool emitSignal = true );

    /*
     * NB. currently *unimplemented*
     *
     * Update favicon url for a given url.
     * @param url the viewed url
     * @param faviconurl the favicion url
     * @emitSignal iff true emit KBookmarkNotifier signal
     * @since 3.3
     */
    void updateFavicon( const QString &url, const QString &faviconurl, bool emitSignal = true );

    /**
     * This will return the path that this manager is using to read
     * the bookmarks.
     * @internal
     * @return the path containing the bookmarks
     */
    QString path() { return m_bookmarksFile; }

    /**
     * This will return the root bookmark.  It is used to iterate
     * through the bookmarks manually.  It is mostly used internally.
     *
     * @return the root (top-level) bookmark
     */
    KBookmarkGroup root() const;

    /**
     * This returns the root of the toolbar menu.
     * In the XML, this is the group with the attribute TOOLBAR=1
     *
     * @return the toolbar group
     */
    KBookmarkGroup toolbar();

    /**
     * @return the bookmark designated by @p address
     * @param address the address belonging to the bookmark you're looking for
     * @param tolerate when true tries to find the most tolerable bookmark position
     * @see KBookmark::address
     */
    KBookmark findByAddress( const QString & address, bool tolerate = false );

    /**
     * Saves the bookmark file and notifies everyone.
     * @param group the parent of all changed bookmarks
     */
    void emitChanged( const KBookmarkGroup & group );

    void emitConfigChanged();

    /**
     * @return true if the NS bookmarks should be dynamically shown
     *         in the toplevel kactionmenu
     * @deprecated
     */
    bool showNSBookmarks() const;

    /**
     * Shows an extra menu for NS bookmarks. Set this to false, if you don't
     * want this.
     */
    void setShowNSBookmarks( bool show );

    /**
     * Set options with which slotEditBookmarks called keditbookmarks
     * this can be used to change the appearance of the keditbookmarks
     * in order to provide a slightly differing outer shell depending
     * on the bookmarks file / app which calls it.
     * @param caption the --caption string, for instance "Konsole"
     * @param browser iff false display no browser specific
     *            menu items in keditbookmarks :: --nobrowser
     * @since 3.2
     */
    void setEditorOptions( const QString& caption, bool browser );

    /**
     * This static function will return an instance of the
     * KBookmarkManager, responsible for the given @p bookmarksFile.
     * If you do not instantiate this class either
     * natively or in a derived class, then it will return an object
     * with the default behaviors.  If you wish to use different
     * behaviors, you <em>must</em> derive your own class and
     * instantiate it before this method is ever called.
     *
     * @param bookmarksFile full path to the bookmarks file,
     * Use ~/.kde/share/apps/konqueror/bookmarks.xml for the konqueror bookmarks
     *
     * @param bImportDesktopFiles if true, and if the bookmarksFile
     * doesn't already exist, import bookmarks from desktop files
     * @return a pointer to an instance of the KBookmarkManager.
     */
    static KBookmarkManager* managerForFile( const QString& bookmarksFile,
                                   bool bImportDesktopFiles = true );


    static KBookmarkManager* createTempManager();

    /**
     * Returns a pointer to the users main bookmark collection.
     * @since 3.2
     */
    static KBookmarkManager* userBookmarksManager();

    /**
     * @internal
     */
    const QDomDocument & internalDocument() const;

    /**
     * Access to bookmark notifier, for emitting signals.
     * We need this object to exist in one instance only, so we could
     * connectDCOP to it by name.
     */
    KBookmarkNotifier& notifier() { return m_notifier; }

    /**
     * @since 3.2
     */
    KBookmarkGroup addBookmarkDialog( const QString & _url, const QString & _title,
                                      const QString & _parentBookmarkAddress = QString::null );

public slots:
    void slotEditBookmarks();
    void slotEditBookmarksAtAddress( const QString& address );

public:
k_dcop:
    /**
     * Reparse the whole bookmarks file and notify about the change
     * (Called by the bookmark editor)
     */
    ASYNC notifyCompleteChange( QString caller );

    /**
     * Emit the changed signal for the group whose address is given
     * @see KBookmark::address()
     * Called by the instance of konqueror that saved the file after
     * a small change (new bookmark or new folder).
     */
    ASYNC notifyChanged( QString groupAddress );

    ASYNC notifyConfigChanged();

signals:
    /**
     * Signals that the group (or any of its children) with the address
     * @p groupAddress (e.g. "/4/5")
     * has been modified by the caller @p caller.
     */
    void changed( const QString & groupAddress, const QString & caller );

protected:
    // consts added to avoid a copy-and-paste of internalDocument
    void parse() const;
    void importDesktopFiles();
    static void convertToXBEL( QDomElement & group );
    static void convertAttribute( QDomElement elem, const QString & oldName, const QString & newName );

private:
    KBookmarkNotifier m_notifier;
    QString m_bookmarksFile;
    mutable QDomDocument m_doc;
    mutable QDomDocument m_toolbarDoc;
    mutable bool m_docIsLoaded;
    bool m_update;
    static KBookmarkManagerList* s_pSelf;
    bool m_showNSBookmarks;

private:
    class KBookmarkManagerPrivate* dptr() const;
};

/**
 * The KBookmarkMenu and KBookmarkBar classes gives the user
 * the ability to either edit bookmarks or add their own.  In the
 * first case, the app may want to open the bookmark in a special way.
 * In the second case, the app <em>must</em> supply the name and the
 * URL for the bookmark.
 *
 * This class gives the app this callback-like ability.
 *
 * If your app does not give the user the ability to add bookmarks and
 * you don't mind using the default bookmark editor to edit your
 * bookmarks, then you don't need to overload this class at all.
 * Rather, just use something like:
 *
 * <CODE>
 * bookmarks = new KBookmarkMenu(new KBookmarkOwner(), ...)
 * </CODE>
 *
 * If you wish to use your own editor or allow the user to add
 * bookmarks, you must overload this class.
 */
class KIO_EXPORT KBookmarkOwner
{
public:
    virtual ~KBookmarkOwner() {}
  /**
   * This function is called if the user selects a bookmark.  It will
   * open up the bookmark in a default fashion unless you override it.
   */
  virtual void openBookmarkURL(const QString& _url);

  /**
   * This function is called whenever the user wants to add the
   * current page to the bookmarks list.  The title will become the
   * "name" of the bookmark.  You must overload this function if you
   * wish to give your users the ability to add bookmarks.
   *
   * @return the title of the current page.
   */
  virtual QString currentTitle() const { return QString::null; }

  /**
   * This function is called whenever the user wants to add the
   * current page to the bookmarks list.  The URL will become the URL
   * of the bookmark.  You must overload this function if you wish to
   * give your users the ability to add bookmarks.
   *
   * @return the URL of the current page.
   */
  virtual QString currentURL() const { return QString::null; }

protected:
  virtual void virtual_hook( int id, void* data );
};

/**
 * @since 3.2
 */
class KIO_EXPORT KExtendedBookmarkOwner : public QObject, virtual public KBookmarkOwner
{
    Q_OBJECT
public:
    typedef QList<QPair<QString,QString> > QStringPairList;
public slots:
    void fillBookmarksList( KExtendedBookmarkOwner::QStringPairList & list ) { emit signalFillBookmarksList( list ); };
signals:
    void signalFillBookmarksList( KExtendedBookmarkOwner::QStringPairList & list );
private:
    class KExtendedBookmarkOwnerPrivate;
    KExtendedBookmarkOwnerPrivate *d;
};

#endif
