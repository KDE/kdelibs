/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#ifndef _DOM_DocumentImpl_h_
#define _DOM_DocumentImpl_h_

#include "dom_nodeimpl.h"

#include <qlist.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qdict.h>

class QPaintDevice;
class QPaintDeviceMetrics;
class KHTMLView;
class Tokenizer;

namespace khtml {
    class CSSStyleSelector;
    class DocLoader;
}

namespace DOM {

    class ElementImpl;
    class DocumentFragmentImpl;
    class TextImpl;
    class CDATASectionImpl;
    class CommentImpl;
    class AttrImpl;
    class EntityReferenceImpl;
    class NodeListImpl;
    class StyleSheetListImpl;
    class RangeImpl;
    class NodeIteratorImpl;
    class TreeWalkerImpl;
    class NodeFilterImpl;
    class DocumentTypeImpl;
    class NamedEntityMapImpl;
    class ProcessingInstructionImpl;

    class StyleSheetImpl;
    class StyleSheetListImpl;
    class CSSStyleSheetImpl;

class DOMImplementationImpl : public DomShared
{
public:
    DOMImplementationImpl();
    ~DOMImplementationImpl();

    bool hasFeature ( const DOMString &feature, const DOMString &version );
};


/**
 * @internal
 */
class DocumentImpl : public QObject, public NodeBaseImpl
{
    Q_OBJECT
public:
    DocumentImpl();
    DocumentImpl(KHTMLView *v);
    ~DocumentImpl();

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual bool isDocumentNode() const { return true; }

    virtual bool isHTMLDocument() const { return false; }

    virtual void setStyle(khtml::RenderStyle *style) { m_style = style; }
    virtual khtml::RenderStyle *style() { return m_style; }

    virtual void applyChanges(bool top=true, bool force=true);

    DocumentTypeImpl *doctype() const;

    DOMImplementationImpl *implementation() const;

    ElementImpl *documentElement() const;

    virtual ElementImpl *createElement ( const DOMString &tagName );
    virtual ElementImpl *createElementNS ( const DOMString &_namespaceURI, const DOMString &_qualifiedName );

    DocumentFragmentImpl *createDocumentFragment ();

    TextImpl *createTextNode ( const DOMString &data );

    CommentImpl *createComment ( const DOMString &data );

    CDATASectionImpl *createCDATASection ( const DOMString &data );

    ProcessingInstructionImpl *createProcessingInstruction ( const DOMString &target, const DOMString &data );

    AttrImpl *createAttribute ( const DOMString &name );

    EntityReferenceImpl *createEntityReference ( const DOMString &name );

    NodeListImpl *getElementsByTagName ( const DOMString &tagname );

    virtual StyleSheetListImpl *styleSheets();

    khtml::CSSStyleSelector *styleSelector() { return m_styleSelector; }
    virtual void createSelector();

    // Used to maintain list of all elements in the document
    // that want to save and restore state.
    // Returns the state the element should restored to.
    QString registerElement(ElementImpl *);

    // Used to maintain list of all forms in document
    void removeElement(ElementImpl *);

    // Query all registered elements for their state
    QStringList state();

    // Set the state the document should restore to
    void setRestoreState( const QStringList &s) { m_state = s; }

    KHTMLView *view() const { return m_view; }

    RangeImpl *createRange();

    NodeIteratorImpl *createNodeIterator(NodeImpl *root, unsigned long whatToShow,
                                    NodeFilterImpl *filter, bool entityReferenceExpansion);

// ###    TreeWalker createTreeWalker(Node root, unsigned long whatToShow, NodeFilter filter,
//                                bool entityReferenceExpansion);

    QList<NodeImpl> changedNodes;
    virtual void setChanged(bool b=true);
    virtual void recalcStyle();
    virtual void updateRendering();
    khtml::DocLoader *docLoader() { return m_docLoader; }
    void setReloading();
    virtual void attach(KHTMLView *w);

    // to get visually ordered hebrew and arabic pages right
    void setVisuallyOrdered();

    void setSelection(NodeImpl* s, int sp, NodeImpl* e, int ep);
    void clearSelection();

    virtual void open (  );
    virtual void close (  );
    virtual void write ( const DOMString &text );
    virtual void write ( const QString &text );
    virtual void writeln ( const DOMString &text );
    virtual void finishParsing (  );
    void clear();
    // moved from HTMLDocument in DOM2
    ElementImpl *getElementById ( const DOMString &elementId );
    DOMString URL() const { return url; }
    void setURL(DOMString _url) { url = _url; }

    DOMString baseURL() const;

    // from cachedObjectClient
    virtual void setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet);

    CSSStyleSheetImpl* elementSheet();
    virtual Tokenizer *createTokenizer();

    QPaintDeviceMetrics *paintDeviceMetrics() { return m_paintDeviceMetrics; }
    QPaintDevice *paintDevice() const { return m_paintDevice; }
    void setPaintDevice( QPaintDevice *dev );

    enum ParseMode {
	Unknown,
	Compat,
	Transitional,
	Strict
    };
    void determineParseMode( const QString &str );
    void setParseMode( ParseMode m ) { pMode = m; }
    ParseMode parseMode() const { return pMode; }

    void setTextColor( DOMString color ) { m_textColor = color; }
    DOMString textColor() const { return m_textColor; }
    
signals:
    virtual void finishedParsing();

public slots:
    virtual void slotFinishedParsing();

protected:
    khtml::CSSStyleSelector *m_styleSelector;
    KHTMLView *m_view;
    QList<ElementImpl> m_registeredElements;
    QStringList m_state;

    khtml::RenderStyle *m_style;
    khtml::DocLoader *m_docLoader;
    bool visuallyOrdered;
    Tokenizer *tokenizer;
    DOMString url;
    DocumentTypeImpl *m_doctype;
    DOMImplementationImpl *m_implementation;

    StyleSheetImpl *m_sheet;
    bool m_loadingSheet;

    CSSStyleSheetImpl *m_elemSheet;

    QPaintDevice *m_paintDevice;
    QPaintDeviceMetrics *m_paintDeviceMetrics;
    ParseMode pMode;

    DOMString m_textColor; 
};

class DocumentFragmentImpl : public NodeBaseImpl
{
public:
    DocumentFragmentImpl(DocumentImpl *doc);
    DocumentFragmentImpl(const DocumentFragmentImpl &other);

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;

protected:
    virtual bool childAllowed( NodeImpl *newChild );
};


class DocumentTypeImpl : public NodeImpl
{
public:
    DocumentTypeImpl(DocumentImpl *doc);
    ~DocumentTypeImpl();

    virtual const DOMString name() const;
    virtual NamedNodeMapImpl *entities() const;
    virtual NamedNodeMapImpl *notations() const;
    virtual const DOMString nodeName() const;

    NamedEntityMapImpl *m_entities;
/*    QDict<EntityImpl> m_dict;
    QDict<NotationImpl> m_dict;*/
};

class NamedEntityMapImpl : public NamedNodeMapImpl
{
public:
    NamedEntityMapImpl();
    virtual ~NamedEntityMapImpl();
    virtual unsigned long length() const;
    virtual NodeImpl *getNamedItem ( const DOMString &name ) const;
    virtual NodeImpl *setNamedItem ( const Node &arg );
    virtual NodeImpl *removeNamedItem ( const DOMString &name );
    virtual NodeImpl *item ( unsigned long index ) const;

};

}; //namespace
#endif
