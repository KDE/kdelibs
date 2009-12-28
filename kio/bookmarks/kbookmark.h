// -*- c-basic-offset: 4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

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
#ifndef __kbookmark_h
#define __kbookmark_h

#include <kio/kio_export.h>

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtXml/QDomElement>
#include <kurl.h>

class KBookmarkManager;
class KBookmarkGroup;

class KIO_EXPORT KBookmark
{
    friend class KBookmarkGroup;
public:
    enum MetaDataOverwriteMode {
        OverwriteMetaData, DontOverwriteMetaData
    };

    /**
     * KUrl::Bookmark is a QList that contains bookmarks with a few
     * convenience methods.
     * @see KBookmark
     * @see QList
     */
    class KIO_EXPORT List : public QList<KBookmark>
    {
    public:
        List();

        /**
         * Adds this list of bookmark into the given QMimeData.
         *
         * @param mimeData the QMimeData instance used to drag or copy this bookmark
         */
        void populateMimeData( QMimeData* mimeData ) const;

        /**
         * Return true if @p mimeData contains bookmarks
         */
        static bool canDecode( const QMimeData *mimeData );

        /**
         * Return the list of mimeTypes that can be decoded by fromMimeData
         */
        static QStringList mimeDataTypes();

        /**
         * Extract a list of bookmarks from the contents of @p mimeData.
         * Decoding will fail if @p mimeData does not contain any bookmarks.
         * @param mimeData the mime data to extract from; cannot be 0
         * @return the list of bookmarks
         * @note those bookmarks are valid QDomElements, but their parent QDomDocument
         * is already deleted, do not use ownerDocument()
         * @deprecated use fromMimeData(mimeData, doc), to avoid crashes
         */
        static KDE_DEPRECATED KBookmark::List fromMimeData( const QMimeData *mimeData );

        /**
         * Extract a list of bookmarks from the contents of @p mimeData.
         * Decoding will fail if @p mimeData does not contain any bookmarks.
         * @param mimeData the mime data to extract from; cannot be 0
         * @param parentDocument pass an empty QDomDocument here, it will be used as
         * container for the bookmarks. You just need to make sure it stays alive longer
         * (or just as long) as the returned bookmarks.
         * @return the list of bookmarks
         * @since 4.3.2
         */
        static KBookmark::List fromMimeData( const QMimeData *mimeData, QDomDocument& parentDocument );
    };


    /**
     * Constructs a null bookmark, i.e. a bookmark for which isNull() returns true
     * If you want to create a new bookmark use eitehr KBookmarkGroup.addBookmark
     * or if you want an interactive dialog use KBookmarkDialog.
     */
    KBookmark( );

    /**
     * Creates the KBookmark wrapper for @param elem
     * Mostly for internal usage.
     */

    explicit KBookmark( const QDomElement &elem );

    /**
     * Creates a stand alone bookmark. This is fairly expensive since a new QDom Tree is build.
     */
    static KBookmark standaloneBookmark( const QString & text, const KUrl & url, const QString & icon = QString() );

    /**
     * Whether the bookmark is a group or a normal bookmark
     */
    bool isGroup() const;

    /**
     * Whether the bookmark is a separator
     */
    bool isSeparator() const;

    /**
     * @return true if this is a null bookmark. This will never
     * be the case for a real bookmark (in a menu), but it's used
     * for instance as the end condition for KBookmarkGroup::next()
     */
    bool isNull() const;

    /**
     * @return true if bookmark is contained by a QDomDocument,
     * if not it is most likely that it has become separated and
     * is thus invalid and/or has been deleted from the bookmarks.
     */
    bool hasParent() const;

    /**
     * Text shown for the bookmark
     * If bigger than 40, the text is shortened by
     * replacing middle characters with "..." (see KStringHandler::csqueeze)
     */
    QString text() const;
    /**
     * Text shown for the bookmark, not truncated.
     * You should not use this - this is mainly for keditbookmarks.
     */
    QString fullText() const;
    /**
     * Set the text shown for the bookmark.
     *
     * @param fullText the new bookmark title
     */
    void setFullText(const QString &fullText);
    /**
     * URL contained by the bookmark
     */
    KUrl url() const;
    /**
     * Set the URL of the bookmark
     *
     * @param url the new bookmark URL
     */
    void setUrl(const KUrl &url);

    /**
     * @return the pixmap file for this bookmark
     * (i.e. the name of the icon)
     */
    QString icon() const;

    /**
     * Set the icon name of the bookmark
     *
     * @param icon the new icon name for this bookmark
     */
    void setIcon(const QString &icon);

    /**
     * @return Description of the bookmark
     * @since 4.4
     */
    QString description() const;

    /**
     * Set the description of the bookmark
     *
     * @param description
     * @since 4.4
     */
    void setDescription(const QString &description);

    /**
     * @return Mime-Type of this item
     * @since 4.1
     */
    QString mimeType() const;

    /**
     * Set the Mime-Type of this item
     *
     * @param Mime-Type
     * @since 4.1
     */
    void setMimeType(const QString &mimeType);

    /**
     * @return if the bookmark should be shown in the toolbar
     * (used by the filtered toolbar)
     *
     */
    bool showInToolbar() const;

    /**
     * Set whether this bookmark is show in a filterd toolbar
     */
    void setShowInToolbar(bool show);


    /**
     * @return the group containing this bookmark
     */
    KBookmarkGroup parentGroup() const;

    /**
     * Convert this to a group - do this only if
     * isGroup() returns true.
     */
    KBookmarkGroup toGroup() const;

    /**
     * Return the "address" of this bookmark in the whole tree.
     * This is used when telling other processes about a change
     * in a given bookmark. The encoding of the address is "/4/2", for
     * instance, to designate the 2nd child inside the 4th child of the
     * root bookmark.
     */
    QString address() const;

    /**
     * Return the position in the parent, i.e. the last number in the address
     */
    int positionInParent() const;

    // Hard to decide. Good design would imply that each bookmark
    // knows about its manager, so that there can be several managers.
    // But if we say there is only one manager (i.e. set of bookmarks)
    // per application, then KBookmarkManager::self() is much easier.
    //KBookmarkManager * manager() const { return m_manager; }

    /**
     * @internal for KEditBookmarks
     */
    QDomElement internalElement() const;

    /**
     * Updates the bookmarks access metadata
     * Call when a user accesses the bookmark
     */
    void updateAccessMetadata();

    // Utility functions (internal)

    /**
     * @return address of parent
     */
    static QString parentAddress( const QString & address );

    /**
     * @return position in parent (e.g. /4/5/2 -> 2)
     */
    static uint positionInParent( const QString & address );

    /**
     * @return address of previous sibling (e.g. /4/5/2 -> /4/5/1)
     * Returns QString() for a first child
     */
    static QString previousAddress( const QString & address );

    /**
     * @return address of next sibling (e.g. /4/5/2 -> /4/5/3)
     * This doesn't check whether it actually exists
     */
    static QString nextAddress( const QString & address );

    /**
     * @return the common parent of both addresses which
     * has the greatest depth
     */
    static QString commonParent( const QString &A, const QString &B );

    /**
     * @return the metadata container node for a certain matadata owner
     * @since 4.1
     */
    QDomNode metaData(const QString &owner, bool create) const;

    /**
     * Get the value of a specific metadata item (owner = "http://www.kde.org").
     * @param key Name of the metadata item
     * @return Value of the metadata item. QString() is returned in case
     * the specified key does not exist.
     */
    QString metaDataItem( const QString &key ) const;

    /**
     * Change the value of a specific metadata item, or create the given item
     * if it doesn't exist already (owner = "http://www.kde.org").
     * @param key Name of the metadata item to change
     * @param value Value to use for the specified metadata item
     * @param mode Whether to overwrite the item's value if it exists already or not.
     */
    void setMetaDataItem( const QString &key, const QString &value, MetaDataOverwriteMode mode = OverwriteMetaData );

    /**
     * Adds this bookmark into the given QMimeData.
     *
     * WARNING: do not call this method multiple times, use KBookmark::List::populateMimeData instead.
     *
     * @param mimeData the QMimeData instance used to drag or copy this bookmark
     */
    void populateMimeData( QMimeData* mimeData ) const;

    /**
     * Comparison operator
     */
    bool operator==(const KBookmark& rhs) const;

protected:
    QDomElement element;
    // Note: you can't add new member variables here.
    // The KBookmarks are created on the fly, as wrappers
    // around internal QDomElements. Any additional information
    // has to be implemented as an attribute of the QDomElement.

};

#ifdef MAKE_KIO_LIB
KDE_DUMMY_QHASH_FUNCTION(KBookmark)
#endif

/**
 * A group of bookmarks
 */
class KIO_EXPORT KBookmarkGroup : public KBookmark
{
public:
    /**
     * Create an invalid group. This is mostly for use in QValueList,
     * and other places where we need a null group.
     * Also used as a parent for a bookmark that doesn't have one
     * (e.g. Netscape bookmarks)
     */
    KBookmarkGroup();

    /**
     * Create a bookmark group as specified by the given element
     */
    KBookmarkGroup( const QDomElement &elem );

    /**
     * @return true if the bookmark folder is opened in the bookmark editor
     */
    bool isOpen() const;

    /**
     * Return the first child bookmark of this group
     */
    KBookmark first() const;
    /**
     * Return the prevous sibling of a child bookmark of this group
     * @param current has to be one of our child bookmarks.
     */
    KBookmark previous( const KBookmark & current ) const;
    /**
     * Return the next sibling of a child bookmark of this group
     * @param current has to be one of our child bookmarks.
     */
    KBookmark next( const KBookmark & current ) const;

    /**
     * Return the index of a child bookmark, -1 if not found
     */
    int indexOf(const KBookmark& child) const;

    /**
     * Create a new bookmark folder, as the last child of this group
     * @param text for the folder.
     * If you want an dialog use KBookmarkDialog
     */
    KBookmarkGroup createNewFolder( const QString & text );
    /**
     * Create a new bookmark separator
     * Don't forget to use KBookmarkManager::self()->emitChanged( parentBookmark );
     */
    KBookmark createNewSeparator();

    /**
     * Create a new bookmark, as the last child of this group
     * Don't forget to use KBookmarkManager::self()->emitChanged( parentBookmark );
     * @param bm the bookmark to add
     */
    KBookmark addBookmark( const KBookmark &bm );

    /**
     * Create a new bookmark, as the last child of this group
     * Don't forget to use KBookmarkManager::self()->emitChanged( parentBookmark );
     * @param text for the bookmark
     * @param url the URL that the bookmark points to
     * @param icon the name of the icon to associate with the bookmark. A suitable default
     * will be determined from the URL if not specified.
     */
    KBookmark addBookmark( const QString & text, const KUrl & url, const QString & icon = QString() );

    /**
     * Moves @p bookmark after @p after (which should be a child of ours).
     * If after is null, @p bookmark is moved as the first child.
     * Don't forget to use KBookmarkManager::self()->emitChanged( parentBookmark );
     */
    bool moveBookmark( const KBookmark & bookmark, const KBookmark & after);

    KDE_DEPRECATED bool moveItem( const KBookmark & item, const KBookmark & after );

    /**
     * Delete a bookmark - it has to be one of our children !
     * Don't forget to use KBookmarkManager::self()->emitChanged( parentBookmark );
     */
    void deleteBookmark( const KBookmark &bk );

    /**
     * @return true if this is the toolbar group
     */
    bool isToolbarGroup() const;
    /**
     * @internal
     */
    QDomElement findToolbar() const;

    /**
     * @return the list of urls of bookmarks at top level of the group
     */
    QList<KUrl> groupUrlList() const;

protected:
    QDomElement nextKnownTag( const QDomElement &start, bool goNext ) const;

private:

    // Note: you can't add other member variables here, except for caching info.
    // The KBookmarks are created on the fly, as wrappers
    // around internal QDomElements. Any additional information
    // has to be implemented as an attribute of the QDomElement.
};

class KIO_EXPORT KBookmarkGroupTraverser {
protected:
    virtual ~KBookmarkGroupTraverser();
    void traverse(const KBookmarkGroup &);
    virtual void visit(const KBookmark &);
    virtual void visitEnter(const KBookmarkGroup &);
    virtual void visitLeave(const KBookmarkGroup &);
};

#endif
