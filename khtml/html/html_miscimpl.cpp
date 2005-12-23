/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2005 Maksim Orlovich (maksim@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
// -------------------------------------------------------------------------
#include "html/html_tableimpl.h"
#include "html/html_miscimpl.h"
#include "html/html_formimpl.h"
#include "html/html_documentimpl.h"

#include "misc/htmlhashes.h"
#include "dom/dom_node.h"

using namespace DOM;

#include <kdebug.h>

HTMLBaseFontElementImpl::HTMLBaseFontElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
}

HTMLBaseFontElementImpl::~HTMLBaseFontElementImpl()
{
}

NodeImpl::Id HTMLBaseFontElementImpl::id() const
{
    return ID_BASEFONT;
}

// -------------------------------------------------------------------------

struct CollectionCache: public NodeListImpl::Cache
{
    static Cache* make() { return new CollectionCache; }

    QDict<QValueList<NodeImpl*> > nameCache;

    CollectionCache(): nameCache(127)
    {
        nameCache.setAutoDelete(true);
    }

    virtual void clear(DocumentImpl* doc)
    {
        Cache::clear(doc);
        //qDeletaAll here in Qt4
        nameCache.clear();
    }
};

HTMLCollectionImpl::HTMLCollectionImpl(NodeImpl *_base, int _type):
    NodeListImpl(_base, _type, CollectionCache::make)
{
    type = _type;
}

bool HTMLCollectionImpl::nodeMatches(NodeImpl *current, bool& deep) const
{
    if ( current->nodeType() != Node::ELEMENT_NODE )
    {
        deep = false;
        return false;
    }

    bool check = false;
    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(current);
    switch(type)
    {
    case DOC_IMAGES:
        if(e->id() == ID_IMG)
            check = true;
        break;
    case DOC_FORMS:
        if(e->id() == ID_FORM)
            check = true;
        break;
    case DOC_LAYERS:
        if(e->id() == ID_LAYER || e->id() == ID_ILAYER)
            check = true;
        break;
    case TABLE_TBODIES:
        if(e->id() == ID_TBODY)
            check = true;
        else if(e->id() == ID_TABLE)
            deep = false;
        break;
    case TR_CELLS:
        if(e->id() == ID_TD || e->id() == ID_TH)
            check = true;
        else if(e->id() == ID_TABLE)
            deep = false;
        break;
    case TABLE_ROWS:
    case TSECTION_ROWS:
    case TABLE_BODY_ROWS:
        if(e->id() == ID_TR)
            check = true;
        else if(e->id() == ID_TABLE)
            deep = false;
        else if (type == TABLE_BODY_ROWS) {
            //Avoid going inside the thead, tfoot.
            const HTMLTableElementImpl* table = static_cast<const HTMLTableElementImpl*>(m_refNode);
            if (e == table->tHead() || e == table->tFoot())
                deep = false;
        }
        break;
    case SELECT_OPTIONS:
        if(e->id() == ID_OPTION)
            check = true;
        break;
    case MAP_AREAS:
        if(e->id() == ID_AREA)
            check = true;
        break;
    case DOC_APPLETS:   // all OBJECT and APPLET elements
        if(e->id() == ID_OBJECT || e->id() == ID_APPLET || e->id() == ID_EMBED)
            check = true;
        break;
    case DOC_LINKS:     // all A _and_ AREA elements with a value for href
        if(e->id() == ID_A || e->id() == ID_AREA)
            if(!e->getAttribute(ATTR_HREF).isNull())
                check = true;
        break;
    case DOC_ANCHORS:      // all A elements with a value for name and/or id
        if(e->id() == ID_A) {
            if(e->hasID() || !e->getAttribute(ATTR_NAME).isNull())
                check = true;
        }
        break;
    case DOC_ALL:      // "all" elements
        check = true;
        break;
    case NODE_CHILDREN: // first-level children
        check = true;
        deep = false;
        break;
    default:
        kdDebug( 6030 ) << "Error in HTMLCollection, wrong tagId!" << endl;
    }

    return check;
}

bool HTMLCollectionImpl::checkForNameMatch(NodeImpl *node, const DOMString &name) const
{
    if ( node->nodeType() != Node::ELEMENT_NODE )
        return false;

    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(node);

    //If ID matches, this is definitely a match
    if (e->getAttribute(ATTR_ID) == name)
        return true;

    //Despite what the DOM spec says, neither IE nor Gecko actually
    //care to prefer IDs. Instead, they just match everything
    //that has ID or a name for nodes that have a name.
    //Except for the form elements collection, Gecko always returns
    //just one item. IE is more complex: its namedItem
    //and call notation access return everything that matches,
    //but the subscript notation is sometimes different.
    //For now, we try to match IE, but without the subscript
    //oddness, which I don't understand -- Maks.

    bool checkName;
    switch (e->id())
    {
        case ID_A:
        case ID_APPLET:
        case ID_BUTTON:
        case ID_EMBED:
        case ID_FORM:
        case ID_IMG:
        case ID_INPUT:
        case ID_MAP:
        case ID_META:
        case ID_OBJECT:
        case ID_SELECT:
        case ID_TEXTAREA:
            checkName = true;
            break;
        default:
            checkName = false;
    }

    if (checkName)
       return e->getAttribute(ATTR_NAME) == name;
    else
       return false;
}

NodeImpl *HTMLCollectionImpl::item ( unsigned long index ) const
{
    //Most of the time, we just go in normal document order
    if (type != TABLE_ROWS)
        return NodeListImpl::item(index);

    //For table.rows, we first need to check header, then bodies, then footer.
    //we pack any extra headers/footer with bodies. This matches IE, and 
    //means doing the usual thing with length is right
    //### fix constness throughout instead of loosing the const
    HTMLTableElementImpl* table = const_cast<HTMLTableElementImpl*>(
                            static_cast<const HTMLTableElementImpl*>(m_refNode));

    //Keep track of position for the namedItem code
    unsigned long origIndex = index;

    NodeImpl* found = 0;
    if (table->tHead()) {
        HTMLCollectionImpl headerRows(table->tHead(), TSECTION_ROWS);
        unsigned headerLength = headerRows.length();
        if (index < headerLength)
            found = headerRows.item(index);
        else
            index -= headerLength;
    }

    if (!found) {
        HTMLCollectionImpl bodyRows(table, TABLE_BODY_ROWS);
        unsigned bodyLength = bodyRows.length();
        if (index < bodyLength)
            found = bodyRows.item(index);
        else
            index -= bodyLength;
    }

    if (!found && table->tFoot()) {
        HTMLCollectionImpl footerRows(table->tFoot(), TSECTION_ROWS);
        found = footerRows.item(index);
    }

    m_cache->current.node = found; //namedItem needs this.
    m_cache->position     = origIndex;
    return found;
}

NodeImpl *HTMLCollectionImpl::firstItem() const
{
    return item(0);
}

NodeImpl *HTMLCollectionImpl::nextItem() const
{
    //### this assumes this is called immediately after nextItem --
    //it this sane?
    return item(m_cache->position + 1);
}

NodeImpl *HTMLCollectionImpl::namedItem( const DOMString &name ) const
{
    //Reset the position. The invariant is that nextNamedItem will start looking
    //from the current position.
    firstItem();

    return nextNamedItem(name);
}

NodeImpl *HTMLCollectionImpl::nextNamedItem( const DOMString &name ) const
{
    while (NodeImpl* candidate = m_cache->current.node)
    {
        //Always advance, for next call
        nextItem();
        if (checkForNameMatch(candidate, name))
            return candidate;
    }
    return 0;
}

QValueList<NodeImpl*> HTMLCollectionImpl::namedItems( const DOMString &name ) const
{
    QString key = name.string();

    //We use a work-conserving design for the name cache presently -- only
    //remember stuff about elements we were asked for.
    m_cache->updateNodeListInfo(m_refNode->getDocument());
    CollectionCache* cache = static_cast<CollectionCache*>(m_cache);
    if (QValueList<NodeImpl*>* info = cache->nameCache.find(key)) {
        return *info;
    }
    else {
        QValueList<NodeImpl*>* newInfo = new QValueList<NodeImpl*>;

        NodeImpl* match = namedItem(name);
        while (match) {
            newInfo->append(match);
            match = nextNamedItem(name);
        }

        cache->nameCache.insert(key, newInfo);
        return *newInfo;
    }
}

// -----------------------------------------------------------------------------

HTMLFormCollectionImpl::HTMLFormCollectionImpl(NodeImpl* _base)
    : HTMLCollectionImpl(_base, FORM_ELEMENTS), currentNamePos(0), currentNameImgPos(0)
{}

NodeImpl *HTMLFormCollectionImpl::item( unsigned long index ) const
{
    m_cache->updateNodeListInfo(m_refNode->getDocument());

    unsigned int dist = index;
    unsigned int strt = 0;
    if (m_cache->current.index && m_cache->position <= index)
    {
        dist = index - m_cache->position;
        strt = m_cache->current.index;
    }

    QPtrList<HTMLGenericFormElementImpl>& l = static_cast<HTMLFormElementImpl*>( m_refNode )->formElements;
    for (unsigned i = strt; i < l.count(); i++)
    {
        if (l.at( i )->isEnumeratable())
        {
            if (dist == 0)
            {
                //Found it!
                m_cache->position      = index;
                m_cache->current.index = i;
                return l.at( i );
            }
            else
                --dist;
        }
    }
    return 0;
}

unsigned long HTMLFormCollectionImpl::length() const
{
    m_cache->updateNodeListInfo(m_refNode->getDocument());
    if (!m_cache->hasLength)
    {
        m_cache->length = 0;
        QPtrList<HTMLGenericFormElementImpl> l = static_cast<HTMLFormElementImpl*>( m_refNode )->formElements;
        for ( unsigned i = 0; i < l.count(); i++ )
            if ( l.at( i )->isEnumeratable() )
                ++m_cache->length;
        m_cache->hasLength = true;
    }
    return m_cache->length;
}

NodeImpl *HTMLFormCollectionImpl::namedItem( const DOMString &name ) const
{
    currentNamePos    = 0;
    currentNameImgPos = 0;
    foundInput        = false;
    return nextNamedItem(name);
}

NodeImpl *HTMLFormCollectionImpl::nextNamedItem( const DOMString &name ) const
{
    QPtrList<HTMLGenericFormElementImpl>& l = static_cast<HTMLFormElementImpl*>( m_refNode )->formElements;

    //Go through the list, trying to find the appropriate named form element.
    for ( ; currentNamePos < l.count(); ++currentNamePos )
    {
        HTMLGenericFormElementImpl* el = l.at(currentNamePos);
        if (el->isEnumeratable() &&
             ((el->getAttribute(ATTR_ID)   == name) ||
              (el->getAttribute(ATTR_NAME) == name)))
        {
            ++currentNamePos; //Make next call start after this
            foundInput = true;//No need to look for img
            return el;
        }
    }

    //If we got this far, we may need to start looking through the images,
    //but only if no input tags were matched
    if (foundInput) return 0;

    QPtrList<HTMLImageElementImpl>& il = static_cast<HTMLFormElementImpl*>( m_refNode )->imgElements;
    for ( ; currentNameImgPos < il.count(); ++currentNameImgPos )
    {
        HTMLImageElementImpl* el = il.at(currentNameImgPos);
        if ((el->getAttribute(ATTR_ID)   == name) ||
            (el->getAttribute(ATTR_NAME) == name))
        {
            ++currentNameImgPos; //Make next call start after this
            return el;
        }
    }

    return 0;
}

// -------------------------------------------------------------------------
HTMLMappedNameCollectionImpl::HTMLMappedNameCollectionImpl(NodeImpl* _base, int _type, const DOMString& _name): 
    HTMLCollectionImpl(_base, NodeListImpl::UNCACHEABLE), name(_name) 
{
    type = _type; //We pass uncacheable to collection, but need our own type internally.
}

bool HTMLMappedNameCollectionImpl::nodeMatches(NodeImpl *current, bool& deep) const
{
    if ( current->nodeType() != Node::ELEMENT_NODE )
    {
        deep = false;
        return false;
    }

    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(current);
    
    return matchesName(e, type, name);
}

bool HTMLMappedNameCollectionImpl::matchesName( ElementImpl* el, int type, const DOMString& name )
{
    switch (el->id())
    {
    case ID_IMG:
    case ID_FORM:
        //Under document. these require non-empty name to see the element
        if (type == DOCUMENT_NAMED_ITEMS && el->getAttribute(ATTR_NAME).isNull())
            return false;
        //Otherwise, fallthrough
    case ID_OBJECT:
    case ID_EMBED:
    case ID_APPLET:
    case ID_LAYER:
        if (el->getAttribute(ATTR_NAME) == name || el->getAttribute(ATTR_ID) == name)
            return true;
        else
            return false;
    default:
        return false;
    }
}
