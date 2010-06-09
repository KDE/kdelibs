// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>
   Copyright (C) 2008 Norbert Frese <nf2@scheinwelt.at>

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

#include "kbookmark.h"
#include <QStack>
#include <kdebug.h>
#include <kmimetype.h>
#include <kstringhandler.h>
#include <kglobal.h>
#include <klocale.h>
#include <assert.h>
#include <kbookmarkmanager.h>

#include <qdatetime.h>
#include <qmimedata.h>

#define METADATA_KDE_OWNER "http://www.kde.org"
#define METADATA_FREEDESKTOP_OWNER "http://freedesktop.org"
#define METADATA_MIME_OWNER "http://www.freedesktop.org/standards/shared-mime-info"

////// utility functions

static QDomNode cd(QDomNode node, const QString &name, bool create)
{
    QDomNode subnode = node.namedItem(name);
    if (create && subnode.isNull())
    {
        subnode = node.ownerDocument().createElement(name);
        node.appendChild(subnode);
    }
    return subnode;
}

static QDomNode cd_or_create(QDomNode node, const QString &name)
{
    return cd(node, name, true);
}

static QDomText get_or_create_text(QDomNode node)
{
    QDomNode subnode = node.firstChild();
    if (subnode.isNull())
    {
        subnode = node.ownerDocument().createTextNode("");
        node.appendChild(subnode);
    }
    return subnode.toText();
}

static QDomNode findMetadata(const QString & forOwner, QDomNode& parent, bool create)
{
    bool forOwnerIsKDE = forOwner == METADATA_KDE_OWNER;

    QDomElement metadataElement;
    for ( QDomNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) {
        QDomElement elem = _node.toElement();
        if ( !elem.isNull() && elem.tagName() == "metadata" ) {
            const QString owner = elem.attribute( "owner" );
            if ( owner == forOwner )
                return elem;
            if ( owner.isEmpty() && forOwnerIsKDE )
                metadataElement = elem;
        }
    }
    if ( create && metadataElement.isNull() ) {
        metadataElement = parent.ownerDocument().createElement( "metadata" );
        parent.appendChild(metadataElement);
        metadataElement.setAttribute( "owner", forOwner );

    } else if (!metadataElement.isNull() && forOwnerIsKDE) {
        // i'm not sure if this is good, we shouln't take over foreign metatdata
        metadataElement.setAttribute( "owner", METADATA_KDE_OWNER );
    }
    return metadataElement;
}

//////

KBookmarkGroup::KBookmarkGroup()
 : KBookmark( QDomElement() )
{
}

KBookmarkGroup::KBookmarkGroup( const QDomElement &elem )
 : KBookmark(elem)
{
}

bool KBookmarkGroup::isOpen() const
{
    return element.attribute("folded") == "no"; // default is: folded
}

KBookmark KBookmarkGroup::first() const
{
    return KBookmark( nextKnownTag( element.firstChildElement(), true ) );
}

KBookmark KBookmarkGroup::previous( const KBookmark & current ) const
{
    return KBookmark( nextKnownTag( current.element.previousSiblingElement(), false ) );
}

KBookmark KBookmarkGroup::next( const KBookmark & current ) const
{
    return KBookmark( nextKnownTag( current.element.nextSiblingElement(), true ) );
}

int KBookmarkGroup::indexOf(const KBookmark& child) const
{
    uint counter = 0;
    for ( KBookmark bk = first(); !bk.isNull(); bk = next(bk), ++counter ) {
        if ( bk.element == child.element )
            return counter;
    }
    return -1;
}

QDomElement KBookmarkGroup::nextKnownTag( const QDomElement &start, bool goNext ) const
{
    static const QString & bookmark = KGlobal::staticQString("bookmark");
    static const QString & folder = KGlobal::staticQString("folder");
    static const QString & separator = KGlobal::staticQString("separator");

    for( QDomElement elem = start; !elem.isNull(); )
    {
        QString tag = elem.tagName();
        if (tag == folder || tag == bookmark || tag == separator)
            return elem;
        if (goNext)
            elem = elem.nextSiblingElement();
        else
            elem = elem.previousSiblingElement();
    }
    return QDomElement();
}

KBookmarkGroup KBookmarkGroup::createNewFolder( const QString & text )
{
    if (isNull())
        return KBookmarkGroup();
    QDomDocument doc = element.ownerDocument();
    QDomElement groupElem = doc.createElement( "folder" );
    element.appendChild( groupElem );
    QDomElement textElem = doc.createElement( "title" );
    groupElem.appendChild( textElem );
    textElem.appendChild( doc.createTextNode( text ) );
    return KBookmarkGroup(groupElem);

}

KBookmark KBookmarkGroup::createNewSeparator()
{
    if (isNull())
        return KBookmark();
    QDomDocument doc = element.ownerDocument();
    Q_ASSERT(!doc.isNull());
    QDomElement sepElem = doc.createElement( "separator" );
    element.appendChild( sepElem );
    return KBookmark(sepElem);
}

bool KBookmarkGroup::moveItem( const KBookmark & bookmark, const KBookmark & after )
{
    return moveBookmark(bookmark, after);
}

bool KBookmarkGroup::moveBookmark( const KBookmark & item, const KBookmark & after )
{
    QDomNode n;
    if ( !after.isNull() )
        n = element.insertAfter( item.element, after.element );
    else // first child
    {
        if ( element.firstChild().isNull() ) // Empty element -> set as real first child
            n = element.insertBefore( item.element, QDomElement() );

        // we have to skip everything up to the first valid child
        QDomElement firstChild = nextKnownTag(element.firstChild().toElement(), true);
        if ( !firstChild.isNull() )
            n = element.insertBefore( item.element, firstChild );
        else
        {
            // No real first child -> append after the <title> etc.
            n = element.appendChild( item.element );
        }
    }
    return (!n.isNull());
}

KBookmark KBookmarkGroup::addBookmark( const KBookmark &bm )
{
    element.appendChild( bm.internalElement() );
    return bm;
}

KBookmark KBookmarkGroup::addBookmark( const QString & text, const KUrl & url, const QString & icon )
{
    if (isNull())
        return KBookmark();
    QDomDocument doc = element.ownerDocument();
    QDomElement elem = doc.createElement( "bookmark" );
    elem.setAttribute( "href", url.url() ); // gives us utf8

    QDomElement textElem = doc.createElement( "title" );
    elem.appendChild( textElem );
    textElem.appendChild( doc.createTextNode( text ) );

    KBookmark newBookmark =  addBookmark( KBookmark( elem ) );

    // as icons are moved to metadata, we have to use the KBookmark API for this
    newBookmark.setIcon(icon.isEmpty() ? KMimeType::iconNameForUrl( url ) : icon );
    return newBookmark;
}

void KBookmarkGroup::deleteBookmark( const KBookmark &bk )
{
    element.removeChild( bk.element );
}

bool KBookmarkGroup::isToolbarGroup() const
{
    return ( element.attribute("toolbar") == "yes" );
}

QDomElement KBookmarkGroup::findToolbar() const
{
    if ( element.attribute("toolbar") == "yes" )
        return element;
    for (QDomNode n = element.firstChild(); !n.isNull() ; n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        // Search among the "folder" children only
        if ( e.tagName() == "folder" )
        {
            if ( e.attribute("toolbar") == "yes" )
                return e;
            else
            {
                QDomElement result = KBookmarkGroup(e).findToolbar();
                if (!result.isNull())
                    return result;
            }
        }
    }
    return QDomElement();
}

QList<KUrl> KBookmarkGroup::groupUrlList() const
{
    QList<KUrl> urlList;
    for ( KBookmark bm = first(); !bm.isNull(); bm = next(bm) )
    {
        if ( bm.isSeparator() || bm.isGroup() )
           continue;
        urlList << bm.url();
    }
    return urlList;
}

//////

KBookmark::KBookmark()
{
}

KBookmark::KBookmark( const QDomElement &elem ) : element(elem)
{
}

bool KBookmark::isGroup() const
{
    QString tag = element.tagName();
    return ( tag == "folder"
             || tag == "xbel" ); // don't forget the toplevel group
}

bool KBookmark::isSeparator() const
{
    return (element.tagName() == "separator");
}

bool KBookmark::isNull() const
{
    return element.isNull();
}

bool KBookmark::hasParent() const
{
    QDomElement parent = element.parentNode().toElement();
    return !parent.isNull();
}

QString KBookmark::text() const
{
    return KStringHandler::csqueeze( fullText() );
}

QString KBookmark::fullText() const
{
    if (isSeparator())
        return i18n("--- separator ---");

    QString text = element.namedItem("title").toElement().text();
    text.replace('\n', ' '); // #140673
    return text;
}

void KBookmark::setFullText(const QString &fullText)
{
    QDomNode titleNode = element.namedItem("title");
    if (titleNode.isNull()) {
        titleNode = element.ownerDocument().createElement("title");
        element.appendChild(titleNode);
    }

    if (titleNode.firstChild().isNull()) {
        QDomText domtext = titleNode.ownerDocument().createTextNode("");
        titleNode.appendChild(domtext);
    }

    QDomText domtext = titleNode.firstChild().toText();
    domtext.setData(fullText);
}

KUrl KBookmark::url() const
{
    return KUrl(element.attribute("href").toAscii()); // Decodes it from utf8
}

void KBookmark::setUrl(const KUrl &url)
{
    element.setAttribute("href", url.url());
}

QString KBookmark::icon() const
{
    QDomNode metaDataNode = metaData(METADATA_FREEDESKTOP_OWNER, false);
    QDomElement iconElement = cd(metaDataNode, "bookmark:icon", false).toElement();

    QString icon = iconElement.attribute("name");

    // migration code
    if (icon.isEmpty())
      icon = element.attribute("icon");
    if (icon == "www") // common icon for kde3 bookmarks
        return "internet-web-browser";
    // end migration code

    if (icon == "bookmark_folder") {
        return "folder-bookmarks";
    }
    if (icon.isEmpty()) {
        // Default icon depends on URL for bookmarks, and is default directory
        // icon for groups.
        if (isGroup()) {
            icon = "folder-bookmarks";
        }
        else {
            if (isSeparator()) {
                icon = "edit-clear"; // whatever
            } else {
                // get icon from mimeType
                QString _mimeType = mimeType();
                if (!_mimeType.isEmpty()) {
                    KMimeType::Ptr mime = KMimeType::mimeType(_mimeType, KMimeType::ResolveAliases);
                    if (mime) {
                        return mime->iconName();
                    }
                }
                // get icon from URL
                icon = KMimeType::iconNameForUrl(url());
            }
        }
    }
    return icon;
}

void KBookmark::setIcon(const QString &icon)
{
    QDomNode metaDataNode = metaData(METADATA_FREEDESKTOP_OWNER, true);
    QDomElement iconElement = cd_or_create(metaDataNode, "bookmark:icon").toElement();
    iconElement.setAttribute ( "name", icon );

    // migration code
    if(!element.attribute("icon").isEmpty())
        element.removeAttribute("icon");
}

QString KBookmark::description() const
{
    if (isSeparator())
        return QString();

    QString description = element.namedItem("desc").toElement().text();
    description.replace('\n', ' '); // #140673
    return description;
}

void KBookmark::setDescription(const QString &description)
{
    QDomNode descNode = element.namedItem("desc");
    if (descNode.isNull()) {
        descNode = element.ownerDocument().createElement("desc");
        element.appendChild(descNode);
    }

    if (descNode.firstChild().isNull()) {
        QDomText domtext = descNode.ownerDocument().createTextNode(QString());
        descNode.appendChild(domtext);
    }

    QDomText domtext = descNode.firstChild().toText();
    domtext.setData(description);
}

QString KBookmark::mimeType() const
{
    QDomNode metaDataNode = metaData(METADATA_MIME_OWNER, false);
    QDomElement mimeTypeElement = cd(metaDataNode, "mime:mime-type", false).toElement();
    return mimeTypeElement.attribute("type");
}

void KBookmark::setMimeType(const QString &mimeType)
{
    QDomNode metaDataNode = metaData(METADATA_MIME_OWNER, true);
    QDomElement iconElement = cd_or_create(metaDataNode, "mime:mime-type").toElement();
    iconElement.setAttribute ( "type", mimeType );
}

bool KBookmark::showInToolbar() const
{
    if(element.hasAttribute("showintoolbar"))
    {
        bool show = element.attribute("showintoolbar") == "yes";
        const_cast<QDomElement *>(&element)->removeAttribute("showintoolbar");
        const_cast<KBookmark *>(this)->setShowInToolbar(show);
    }
    return  metaDataItem("showintoolbar") == "yes";
}


void KBookmark::setShowInToolbar(bool show)
{
    setMetaDataItem("showintoolbar", show ? "yes" : "no");
}

KBookmarkGroup KBookmark::parentGroup() const
{
    return KBookmarkGroup( element.parentNode().toElement() );
}

KBookmarkGroup KBookmark::toGroup() const
{
    Q_ASSERT( isGroup() );
    return KBookmarkGroup(element);
}

QString KBookmark::address() const
{
    if ( element.tagName() == "xbel" )
        return ""; // not QString() !
    else
    {
        // Use keditbookmarks's DEBUG_ADDRESSES flag to debug this code :)
        if (element.parentNode().isNull())
        {
            Q_ASSERT(false);
            return "ERROR"; // Avoid an infinite loop
        }
        KBookmarkGroup group = parentGroup();
        QString parentAddress = group.address();
        int pos = group.indexOf(*this);
        Q_ASSERT(pos != -1);
        return parentAddress + '/' + QString::number(pos);
    }
}

int KBookmark::positionInParent() const
{
    return parentGroup().indexOf(*this);
}

QDomElement KBookmark::internalElement() const
{
    return element;
}

KBookmark KBookmark::standaloneBookmark( const QString & text, const KUrl & url, const QString & icon )
{
    QDomDocument doc("xbel");
    QDomElement elem = doc.createElement("xbel");
    doc.appendChild( elem );
    KBookmarkGroup grp( elem );
    grp.addBookmark( text, url, icon );
    return grp.first();
}


QString KBookmark::commonParent(const QString &first, const QString &second)
{
    QString A = first;
    QString B = second;
    QString error("ERROR");
    if(A == error || B == error)
        return error;

    A += '/';
    B += '/';

    uint lastCommonSlash = 0;
    uint lastPos = A.length() < B.length() ? A.length() : B.length();
    for(uint i=0; i < lastPos; ++i)
    {
        if(A[i] != B[i])
            return A.left(lastCommonSlash);
        if(A[i] == '/')
            lastCommonSlash = i;
    }
    return A.left(lastCommonSlash);
}

void KBookmark::updateAccessMetadata()
{
    kDebug(7043) << "KBookmark::updateAccessMetadata " << address() << " " << url().prettyUrl();

    const uint timet = QDateTime::currentDateTime().toTime_t();
    setMetaDataItem( "time_added", QString::number( timet ), DontOverwriteMetaData );
    setMetaDataItem( "time_visited", QString::number( timet ) );

    QString countStr = metaDataItem( "visit_count" ); // TODO use spec'ed name
    bool ok;
    int currentCount = countStr.toInt(&ok);
    if (!ok)
        currentCount = 0;
    currentCount++;
    setMetaDataItem( "visit_count", QString::number( currentCount ) );

    // TODO - for 4.0 - time_modified
}

QString KBookmark::parentAddress( const QString & address )
{
    return address.left( address.lastIndexOf(QLatin1Char('/')) );
}

uint KBookmark::positionInParent( const QString & address )
{
    return address.mid( address.lastIndexOf(QLatin1Char('/')) + 1 ).toInt();
}

QString KBookmark::previousAddress( const QString & address )
{
    uint pp = positionInParent(address);
    return pp>0
        ? parentAddress(address) + QLatin1Char('/') + QString::number(pp-1)
        : QString();
}

QString KBookmark::nextAddress( const QString & address )
{
    return parentAddress(address) + QLatin1Char('/') +
        QString::number(positionInParent(address)+1);
}

QDomNode KBookmark::metaData(const QString &owner, bool create) const
{
    QDomNode infoNode = cd( internalElement(), "info", create);
    if (infoNode.isNull()) return QDomNode();
    return findMetadata(owner, infoNode , create);
}

QString KBookmark::metaDataItem( const QString &key ) const
{
    QDomNode metaDataNode = metaData(METADATA_KDE_OWNER, false);
    for ( QDomElement e = metaDataNode.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() )
    {
        if ( e.tagName() == key ) {
            return e.text();
        }
    }
    return QString();
}

void KBookmark::setMetaDataItem( const QString &key, const QString &value, MetaDataOverwriteMode mode )
{
    QDomNode metaDataNode = metaData(METADATA_KDE_OWNER, true);
    QDomNode item = cd_or_create( metaDataNode, key );
    QDomText text = get_or_create_text( item );
    if ( mode == DontOverwriteMetaData && !text.data().isEmpty() ) {
        return;
    }

    text.setData( value );
}


bool KBookmark::operator==(const KBookmark& rhs) const
{
    return element == rhs.element;
}

////

KBookmarkGroupTraverser::~KBookmarkGroupTraverser()
{
}

void KBookmarkGroupTraverser::traverse(const KBookmarkGroup &root)
{
    QStack<KBookmarkGroup> stack;
    stack.push(root);
    KBookmark bk = root.first();
    for(;;) {
        if(bk.isNull()) {
            if(stack.count() == 1) // only root is on the stack
                return;
            if(stack.count() > 0) {
                visitLeave(stack.top());
                bk = stack.pop();
            }
            bk = stack.top().next(bk);
        } else if(bk.isGroup()) {
            KBookmarkGroup gp = bk.toGroup();
            visitEnter(gp);
            bk = gp.first();
            stack.push(gp);
        } else {
            visit(bk);
            bk = stack.top().next(bk);
        }
    }
}

void KBookmarkGroupTraverser::visit(const KBookmark &)
{
}

void KBookmarkGroupTraverser::visitEnter(const KBookmarkGroup &)
{
}

void KBookmarkGroupTraverser::visitLeave(const KBookmarkGroup &)
{
}

void KBookmark::populateMimeData( QMimeData* mimeData ) const
{
    KBookmark::List bookmarkList;
    bookmarkList.append( *this );
    bookmarkList.populateMimeData( mimeData );
}

KBookmark::List::List() : QList<KBookmark>()
{
}

void KBookmark::List::populateMimeData( QMimeData* mimeData ) const
{
    KUrl::List urls;

    QDomDocument doc( "xbel" );
    QDomElement elem = doc.createElement( "xbel" );
    doc.appendChild( elem );

    for ( const_iterator it = begin(), end = this->end() ; it != end ; ++it ) {
        urls.append( (*it).url() );
        elem.appendChild( (*it).internalElement().cloneNode( true /* deep */ ) );
    }

    // This sets text/uri-list and text/plain into the mimedata
    urls.populateMimeData( mimeData, KUrl::MetaDataMap() );

    mimeData->setData( "application/x-xbel", doc.toByteArray() );
}

bool KBookmark::List::canDecode( const QMimeData *mimeData )
{
    return mimeData->hasFormat( "application/x-xbel" )  || KUrl::List::canDecode(mimeData);
}

QStringList KBookmark::List::mimeDataTypes()
{
    return QStringList()<<("application/x-xbel")<<KUrl::List::mimeDataTypes();
}

KBookmark::List KBookmark::List::fromMimeData( const QMimeData *mimeData )
{
    QDomDocument doc;
    kWarning(7043) << "Deprecated method called, with wrong lifetime of QDomDocument, will probably crash";
    return fromMimeData(mimeData, doc);
}

KBookmark::List KBookmark::List::fromMimeData( const QMimeData *mimeData, QDomDocument& doc )
{
    KBookmark::List bookmarks;
    QByteArray payload = mimeData->data( "application/x-xbel" );
    if ( !payload.isEmpty() ) {
        doc.setContent( payload );
        QDomElement elem = doc.documentElement();
        const QDomNodeList children = elem.childNodes();
        for ( int childno = 0; childno < children.count(); childno++)
        {
            bookmarks.append( KBookmark( children.item(childno).toElement() ));
        }
        return bookmarks;
    }
    const KUrl::List urls = KUrl::List::fromMimeData( mimeData );
    if ( !urls.isEmpty() )
    {
        KUrl::List::ConstIterator uit = urls.begin();
        KUrl::List::ConstIterator uEnd = urls.end();
        for ( ; uit != uEnd ; ++uit )
        {
            //kDebug(7043) << "url=" << (*uit);
            bookmarks.append( KBookmark::standaloneBookmark(
                                  (*uit).prettyUrl(), (*uit) ));
        }
    }
    return bookmarks;
}

