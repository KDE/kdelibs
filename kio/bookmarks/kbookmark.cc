// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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
#include <qvaluestack.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kstringhandler.h>
#include <kinputdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <assert.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kbookmarkmanager.h>

KBookmarkGroup::KBookmarkGroup()
 : KBookmark( QDomElement() )
{
}

KBookmarkGroup::KBookmarkGroup( QDomElement elem )
 : KBookmark(elem)
{
}

QString KBookmarkGroup::groupAddress() const
{
    if (m_address.isEmpty())
        m_address = address();
    return m_address;
}

bool KBookmarkGroup::isOpen() const
{
    return element.attribute("folded") == "no"; // default is: folded
}

// Returns first element node equal to or after node n
static QDomElement firstElement(QDomNode n)
{
    while(!n.isNull() && !n.isElement())
        n = n.nextSibling();
    return n.toElement();
}

// Returns first element node equal to or before node n
static QDomElement lastElement(QDomNode n)
{
    while(!n.isNull() && !n.isElement())
        n = n.previousSibling();
    return n.toElement();
}

KBookmark KBookmarkGroup::first() const
{
    return KBookmark( nextKnownTag( firstElement(element.firstChild()), true ) );
}

KBookmark KBookmarkGroup::previous( const KBookmark & current ) const
{
    return KBookmark( nextKnownTag( lastElement(current.element.previousSibling()), false ) );
}

KBookmark KBookmarkGroup::next( const KBookmark & current ) const
{
    return KBookmark( nextKnownTag( firstElement(current.element.nextSibling()), true ) );
}

// KDE4: Change QDomElement to QDomNode so that we can get rid of
// firstElement() and lastElement()
QDomElement KBookmarkGroup::nextKnownTag( QDomElement start, bool goNext ) const
{
    static const QString & bookmark = KGlobal::staticQString("bookmark");
    static const QString & folder = KGlobal::staticQString("folder");
    static const QString & separator = KGlobal::staticQString("separator");

    for( QDomNode n = start; !n.isNull(); )
    {
        QDomElement elem = n.toElement();
        QString tag = elem.tagName();
        if (tag == folder || tag == bookmark || tag == separator)
            return elem;
        if (goNext)
            n = n.nextSibling();
        else
            n = n.previousSibling();
    }
    return QDomElement();
}

KBookmarkGroup KBookmarkGroup::createNewFolder( KBookmarkManager* mgr, const QString & text, bool emitSignal )
{
    QString txt( text );
    if ( text.isEmpty() )
    {
        bool ok;
        QString caption = parentGroup().fullText().isEmpty() ?
                      i18n( "Create New Bookmark Folder" ) :
                      i18n( "Create New Bookmark Folder in %1" )
                      .arg( parentGroup().text() );
        txt = KInputDialog::getText( caption, i18n( "New folder:" ),
                      QString::null, &ok );
        if ( !ok )
            return KBookmarkGroup();
    }

    Q_ASSERT(!element.isNull());
    QDomDocument doc = element.ownerDocument();
    QDomElement groupElem = doc.createElement( "folder" );
    element.appendChild( groupElem );
    QDomElement textElem = doc.createElement( "title" );
    groupElem.appendChild( textElem );
    textElem.appendChild( doc.createTextNode( txt ) );

    KBookmarkGroup grp(groupElem);

    if (emitSignal) 
        emit mgr->notifier().createdNewFolder(
                                mgr->path(), grp.fullText(), 
                                grp.address() );

    return grp;

}

KBookmark KBookmarkGroup::createNewSeparator()
{
    Q_ASSERT(!element.isNull());
    QDomDocument doc = element.ownerDocument();
    Q_ASSERT(!doc.isNull());
    QDomElement sepElem = doc.createElement( "separator" );
    element.appendChild( sepElem );
    return KBookmark(sepElem);
}

bool KBookmarkGroup::moveItem( const KBookmark & item, const KBookmark & after )
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

KBookmark KBookmarkGroup::addBookmark( KBookmarkManager* mgr, const KBookmark &bm, bool emitSignal )
{
    element.appendChild( bm.internalElement() );

    if (emitSignal) {
        if ( bm.hasMetaData() ) {
            mgr->notifyCompleteChange( "" );
        } else {
            emit mgr->notifier().addedBookmark(
                                     mgr->path(), bm.url().url(),
                                     bm.fullText(), bm.address(), bm.icon() );
        }
    }

    return bm;
}

KBookmark KBookmarkGroup::addBookmark( KBookmarkManager* mgr, const QString & text, const KURL & url, const QString & icon, bool emitSignal )
{
    //kdDebug(7043) << "KBookmarkGroup::addBookmark " << text << " into " << m_address << endl;
    QDomDocument doc = element.ownerDocument();
    QDomElement elem = doc.createElement( "bookmark" );
    elem.setAttribute( "href", url.url( 0, 106 ) ); // write utf8 URL (106 is mib enum for utf8)
    QString _icon = icon;
    if ( _icon.isEmpty() )
        _icon = KMimeType::iconForURL( url );
    elem.setAttribute( "icon", _icon );

    QDomElement textElem = doc.createElement( "title" );
    elem.appendChild( textElem );
    textElem.appendChild( doc.createTextNode( text ) );

    return addBookmark( mgr, KBookmark( elem ), emitSignal );
}

void KBookmarkGroup::deleteBookmark( KBookmark bk )
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

QValueList<KURL> KBookmarkGroup::groupUrlList() const
{
    QValueList<KURL> urlList;
    for ( KBookmark bm = first(); !bm.isNull(); bm = next(bm) )
    {
        if ( bm.isSeparator() || bm.isGroup() )
           continue;
        urlList << bm.url();
    }
    return urlList;
}

//////

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

    return element.namedItem("title").toElement().text();
}

KURL KBookmark::url() const
{
    return KURL(element.attribute("href"), 106); // Decode it from utf8 (106 is mib enum for utf8)
}

QString KBookmark::icon() const
{
    QString icon = element.attribute("icon");
    if ( icon.isEmpty() )
        // Default icon depends on URL for bookmarks, and is default directory
        // icon for groups.
        if ( isGroup() )
            icon = "bookmark_folder";
        else
            if ( isSeparator() )
                icon = "eraser"; // whatever
            else
                icon = KMimeType::iconForURL( url() );
    return icon;
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
        return ""; // not QString::null !
    else
    {
        // Use keditbookmarks's DEBUG_ADDRESSES flag to debug this code :)
        if (!hasParent())
        {
            Q_ASSERT(hasParent());
            return "ERROR"; // Avoid an infinite loop
        }
        KBookmarkGroup group = parentGroup();
        QString parentAddress = group.address();
        uint counter = 0;
        // Implementation note: we don't use QDomNode's childNode list because we
        // would have to skip "TEXT", which KBookmarkGroup already does for us.
        for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk), ++counter )
        {
            if ( bk.element == element )
                return parentAddress + "/" + QString::number(counter);
        }
        kdWarning() << "KBookmark::address : this can't happen!  " << parentAddress << endl;
        return "ERROR";
    }
}

KBookmark KBookmark::standaloneBookmark( const QString & text, const KURL & url, const QString & icon )
{
    QDomDocument doc("xbel");
    QDomElement elem = doc.createElement("xbel");
    doc.appendChild( elem );
    KBookmarkGroup grp( elem );
    grp.addBookmark( 0L, text, url, icon, false );
    return grp.first();
}

// For some strange reason QString("").left(0) returns QString::null;
// That breaks commonParent()
QString KBookmark::left(const QString & str, uint len)
{
    //kdDebug()<<"********"<<QString("").left(0).isNull()<<endl;
    if(len == 0)
        return QString("");
    else
        return str.left(len);
}

QString KBookmark::commonParent(QString A, QString B)
{
    QString error("ERROR");
    if(A == error || B == error)
        return error;

    A += "/";
    B += "/";

    uint lastCommonSlash = 0;
    uint lastPos = A.length() < B.length() ? A.length() : B.length();
    for(uint i=0; i < lastPos; ++i)
    {
        if(A[i] != B[i])
            return left(A, lastCommonSlash);
        if(A[i] == '/')
            lastCommonSlash = i;
    }
    return left(A, lastCommonSlash);
}

static QDomNode cd_or_create(QDomNode node, QString name)
{
    QDomNode subnode = node.namedItem(name);
    if (subnode.isNull()) 
    {
        subnode = node.ownerDocument().createElement(name);
        node.appendChild(subnode);
    }
    return subnode;
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

// Look for a metadata with owner="http://www.kde.org" or without any owner (for compatibility)
static QDomNode findOrCreateMetadata( QDomNode& parent )
{
    static const char kdeOwner[] = "http://www.kde.org";
    QDomElement metadataElement;
    for ( QDomNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) {
        QDomElement elem = _node.toElement();
        if ( !elem.isNull() && elem.tagName() == "metadata" ) {
            const QString owner = elem.attribute( "owner" );
            if ( owner == kdeOwner )
                return elem;
            if ( owner.isEmpty() )
                metadataElement = elem;
        }
    }
    if ( metadataElement.isNull() ) {
        metadataElement = parent.ownerDocument().createElement( "metadata" );
        parent.appendChild(metadataElement);
    }
    metadataElement.setAttribute( "owner", kdeOwner );
    return metadataElement;
}

bool KBookmark::hasMetaData() const
{
    // ### NOTE: this code creates <info> and <metadata>, despite its name and the const.
    // It doesn't matter much in practice since it's only called for newly-created bookmarks,
    // which will get metadata soon after anyway.
    QDomNode n = cd_or_create( internalElement(), "info" );
    return findOrCreateMetadata( n ).hasChildNodes();
}

void KBookmark::updateAccessMetadata()
{
    kdDebug(7043) << "KBookmark::updateAccessMetadata " << address() << " " << url().prettyURL() << endl;

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

QString KBookmark::metaDataItem( const QString &key ) const
{
    QDomNode infoNode = cd_or_create( internalElement(), "info" );
    infoNode = findOrCreateMetadata( infoNode );
    for ( QDomNode n = infoNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
        if ( !n.isElement() ) {
            continue;
        }
        const QDomElement e = n.toElement();
        if ( e.tagName() == key ) {
            return e.text();
        }
    }
    return QString::null;
}

void KBookmark::setMetaDataItem( const QString &key, const QString &value, MetaDataOverwriteMode mode )
{
    QDomNode infoNode = cd_or_create( internalElement(), "info" );
    infoNode = findOrCreateMetadata( infoNode );

    QDomNode item = cd_or_create( infoNode, key );
    QDomText text = get_or_create_text( item );
    if ( mode == DontOverwriteMetaData && !text.data().isEmpty() ) {
        return;
    }

    text.setData( value );
}

void KBookmarkGroupTraverser::traverse(const KBookmarkGroup &root)
{
    // non-recursive bookmark iterator
    QValueStack<KBookmarkGroup> stack;
    stack.push(root);
    KBookmark bk = stack.top().first();
    for (;;) {
        if (bk.isNull())
        {
            if (stack.isEmpty()) 
                return;
            if (stack.count() > 1)
                visitLeave(stack.top());
            bk = stack.pop();
            bk = stack.top().next(bk);
            if (bk.isNull())
                continue;
        } 

        if (bk.isGroup()) 
        {
            KBookmarkGroup gp = bk.toGroup();
            visitEnter(gp);
            if (!gp.first().isNull()) 
            {
                stack.push(gp);
                bk = gp.first();
                continue;
            }
            // empty group
            visitLeave(gp);
        } 
        else 
            visit(bk);

        bk = stack.top().next(bk);
    }

    // never reached
}

