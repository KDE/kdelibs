/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
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

#include "xml/dom_nodeimpl.h"
#include "xml/dom2_traversalimpl.h"

#include <qstringlist.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qdict.h>
#include <qmap.h>

#include <kurl.h>

class QPaintDevice;
class QPaintDeviceMetrics;
class KHTMLView;
class Tokenizer;

namespace khtml {
    class CSSStyleSelector;
    class DocLoader;
    class CSSStyleSelectorList;
}

namespace DOM {

    class AbstractViewImpl;
    class AttrImpl;
    class CDATASectionImpl;
    class CSSStyleSheetImpl;
    class CommentImpl;
    class DocumentFragmentImpl;
    class DocumentImpl;
    class DocumentTypeImpl;
    class ElementImpl;
    class EntityReferenceImpl;
    class EventImpl;
    class EventListener;
    class GenericRONamedNodeMapImpl;
    class HTMLDocumentImpl;
    class HTMLElementImpl;
    class NodeFilterImpl;
    class NodeFilter;
    class NodeIteratorImpl;
    class NodeListImpl;
    class ProcessingInstructionImpl;
    class RangeImpl;
    class RegisteredEventListener;
    class StyleSheetImpl;
    class StyleSheetListImpl;
    class TextImpl;
    class TreeWalkerImpl;

class DOMImplementationImpl : public DomShared
{
public:
    DOMImplementationImpl();
    ~DOMImplementationImpl();

    // DOM methods & attributes for DOMImplementation
    bool hasFeature ( const DOMString &feature, const DOMString &version );
    DocumentTypeImpl *createDocumentType( const DOMString &qualifiedName, const DOMString &publicId,
                                          const DOMString &systemId, int &exceptioncode );
    DocumentImpl *createDocument( const DOMString &namespaceURI, const DOMString &qualifiedName,
                                  const DocumentType &doctype, int &exceptioncode );

    DOMImplementationImpl* getInterface(const DOMString& feature) const;

    // From the DOMImplementationCSS interface
    CSSStyleSheetImpl *createCSSStyleSheet(DOMStringImpl *title, DOMStringImpl *media, int &exceptioncode);

    // From the HTMLDOMImplementation interface
    HTMLDocumentImpl* createHTMLDocument( const DOMString& title);

    // Other methods (not part of DOM)
    DocumentImpl *createDocument( KHTMLView *v = 0 );
    HTMLDocumentImpl *createHTMLDocument( KHTMLView *v = 0 );

    // Returns the static instance of this class - only one instance of this class should
    // ever be present, and is used as a factory method for creating DocumentImpl objects
    static DOMImplementationImpl *instance();

protected:
    static DOMImplementationImpl *m_instance;
};


/**
 * @internal
 */
class DocumentImpl : public QObject, public NodeBaseImpl
{
    Q_OBJECT
public:
    DocumentImpl(DOMImplementationImpl *_implementation, DocumentTypeImpl *_doctype, KHTMLView *v);
    ~DocumentImpl();

    // DOM methods & attributes for Document

    DocumentTypeImpl *doctype() const;
    DOMImplementationImpl *implementation() const;
    ElementImpl *documentElement() const;
    virtual ElementImpl *createElement ( const DOMString &tagName );
    DocumentFragmentImpl *createDocumentFragment ();
    TextImpl *createTextNode ( const DOMString &data );
    CommentImpl *createComment ( const DOMString &data );
    CDATASectionImpl *createCDATASection ( const DOMString &data );
    ProcessingInstructionImpl *createProcessingInstruction ( const DOMString &target, const DOMString &data );
    AttrImpl *createAttribute ( const DOMString &name );
    EntityReferenceImpl *createEntityReference ( const DOMString &name );
    NodeImpl *importNode( NodeImpl *importedNode, bool deep, int &exceptioncode );
    virtual ElementImpl *createElementNS ( const DOMString &_namespaceURI, const DOMString &_qualifiedName );
    AttrImpl *createAttributeNS ( const DOMString &_namespaceURI, const DOMString &_qualifiedName );
    ElementImpl *getElementById ( const DOMString &elementId ) const;

    // DOM methods overridden from  parent classes

    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual DOMString namespaceURI() const;


    // Other methods (not part of DOM)
    virtual bool isDocumentNode() const { return true; }

    virtual bool isHTMLDocument() const { return false; }

    virtual ElementImpl *createHTMLElement ( const DOMString &tagName );


    khtml::CSSStyleSelector *styleSelector() { return m_styleSelector; }
    void updateStyleSelector();

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
                                    NodeFilter &filter, bool entityReferenceExpansion, int &exceptioncode);

    TreeWalkerImpl *createTreeWalker(Node root, unsigned long whatToShow, NodeFilter &filter,
                            bool entityReferenceExpansion);

    virtual void recalcStyle( StyleChange = NoChange );
    static QPtrList<DocumentImpl> * changedDocuments;
    virtual void updateRendering();
    static void updateDocumentsRendering();
    khtml::DocLoader *docLoader() { return m_docLoader; }

    void attach(KHTMLView *w=0);
    virtual void detach();

    // to get visually ordered hebrew and arabic pages right
    void setVisuallyOrdered();

    void setSelection(NodeImpl* s, int sp, NodeImpl* e, int ep);
    void clearSelection();

    void open (  );
    virtual void close (  );
    void write ( const DOMString &text );
    void write ( const QString &text );
    void writeln ( const DOMString &text );
    void finishParsing (  );
    void clear();

    QString URL() const { return m_url; }
    void setURL(QString url) { m_url = url; }

    QString baseURL() const { return m_baseURL.isEmpty() ? m_url : m_baseURL; }
    void setBaseURL(const QString& baseURL) { m_baseURL = baseURL; }

    QString baseTarget() const { return m_baseTarget; }
    void setBaseTarget(const QString& baseTarget) { m_baseTarget = baseTarget; }

    QString completeURL(const QString& url) { return KURL(baseURL(),url).url(); };

    // from cachedObjectClient
    virtual void setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheetStr);
    void setUserStyleSheet(const QString& sheet);
    QString userStyleSheet() const { return m_usersheet; }

    CSSStyleSheetImpl* elementSheet();
    virtual Tokenizer *createTokenizer();
    Tokenizer *tokenizer() { return m_tokenizer; }

    QPaintDeviceMetrics *paintDeviceMetrics() { return m_paintDeviceMetrics; }
    QPaintDevice *paintDevice() const { return m_paintDevice; }
    void setPaintDevice( QPaintDevice *dev );

    enum HTMLMode {
        Html3 = 0,
        Html4 = 1,
        XHtml = 2
    };

    enum ParseMode {
        Unknown,
        Compat,
        Transitional,
        Strict
    };
    virtual void determineParseMode( const QString &str );
    void setParseMode( ParseMode m ) { pMode = m; }
    ParseMode parseMode() const { return pMode; }

    void setHTMLMode( HTMLMode m ) { hMode = m; }
    HTMLMode htmlMode() const { return hMode; }

    void setParsing(bool b) { m_bParsing = b; }
    bool parsing() const { return m_bParsing; }

    void setTextColor( DOMString color ) { m_textColor = color; }
    DOMString textColor() const { return m_textColor; }

    // internal
    NodeImpl *findElement( Id id );

    // overrides NodeImpl
    virtual bool prepareMouseEvent( int x, int y,
                                    int _tx, int _ty,
                                    MouseEvent *ev );

    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short nodeType );
    virtual NodeImpl *cloneNode ( bool deep, int &exceptioncode );

    NodeImpl::Id tagId(DOMStringImpl* _namespaceURI, DOMStringImpl *_name, bool readonly);
    DOMString tagName(NodeImpl::Id _id) const;

    DOMStringImpl* namespaceURI(NodeImpl::Id _id) const;

    StyleSheetListImpl* styleSheets();

    NodeImpl *focusNode() const { return m_focusNode; }
    void setFocusNode(NodeImpl *newFocusNode);

    virtual DocumentImpl* getDocument()
        { return this; }
    bool isDocumentChanged()
	{ return m_docChanged; }
    virtual void setDocumentChanged(bool);
    void attachNodeIterator(NodeIteratorImpl *ni);
    void detachNodeIterator(NodeIteratorImpl *ni);
    void notifyBeforeNodeRemoval(NodeImpl *n);
    AbstractViewImpl *defaultView() const;
    EventImpl *createEvent(const DOMString &eventType, int &exceptioncode);

    // keep track of what types of event listeners are registered, so we don't
    // dispatch events unnecessarily
    enum ListenerType {
        DOMSUBTREEMODIFIED_LISTENER          = 0x01,
        DOMNODEINSERTED_LISTENER             = 0x02,
        DOMNODEREMOVED_LISTENER              = 0x04,
        DOMNODEREMOVEDFROMDOCUMENT_LISTENER  = 0x08,
        DOMNODEINSERTEDINTODOCUMENT_LISTENER = 0x10,
        DOMATTRMODIFIED_LISTENER             = 0x20,
        DOMCHARACTERDATAMODIFIED_LISTENER    = 0x40
    };

    bool hasListenerType(ListenerType listenerType) const { return (m_listenerTypes & listenerType); }
    void addListenerType(ListenerType listenerType) { m_listenerTypes = m_listenerTypes | listenerType; }

    CSSStyleDeclarationImpl *getOverrideStyle(ElementImpl *elt, DOMStringImpl *pseudoElt);

    typedef QMap<QString, ProcessingInstructionImpl*> LocalStyleRefs;
    LocalStyleRefs* localStyleRefs() { return &m_localStyleRefs; }

    virtual void defaultEventHandler(EventImpl *evt);
    virtual void setWindowEventListener(int id, EventListener *listener);
    EventListener *getWindowEventListener(int id);
    virtual void removeWindowEventListener(int id);
    EventListener *createHTMLEventListener(QString code);

    NodeImpl *nextFocusNode(NodeImpl *fromNode);
    NodeImpl *previousFocusNode(NodeImpl *fromNode);

    int nodeAbsIndex(NodeImpl *node);
    NodeImpl *nodeWithAbsIndex(int absIndex);

signals:
    void finishedParsing();

protected:
    khtml::CSSStyleSelector *m_styleSelector;
    KHTMLView *m_view;
    QPtrList<ElementImpl> m_registeredElements;
    QStringList m_state;

    khtml::DocLoader *m_docLoader;
    Tokenizer *m_tokenizer;
    QString m_url;
    QString m_baseURL;
    QString m_baseTarget;

    DocumentTypeImpl *m_doctype;
    DOMImplementationImpl *m_implementation;

    StyleSheetImpl *m_sheet;
    QString m_usersheet;

    CSSStyleSheetImpl *m_elemSheet;

    QPaintDevice *m_paintDevice;
    QPaintDeviceMetrics *m_paintDeviceMetrics;
    ParseMode pMode;
    HTMLMode hMode;

    DOMString m_textColor;

    DOMStringImpl **m_elementNames;
    unsigned short m_elementNameAlloc;
    unsigned short m_elementNameCount;

    NodeImpl *m_focusNode;

    DOMStringImpl** m_namespaceURIs;
    unsigned short m_namespaceURIAlloc;
    unsigned short m_namespaceURICount;

    QPtrList<NodeIteratorImpl> m_nodeIterators;
    AbstractViewImpl *m_defaultView;

    unsigned short m_listenerTypes;
    StyleSheetListImpl* m_styleSheets;
    LocalStyleRefs m_localStyleRefs; // references to inlined style elements
    QPtrList<RegisteredEventListener> m_windowEventListeners;

    bool m_loadingSheet;
    bool visuallyOrdered;
    bool m_bParsing;
    bool m_docChanged;
};

class DocumentFragmentImpl : public NodeBaseImpl
{
public:
    DocumentFragmentImpl(DocumentPtr *doc);
    DocumentFragmentImpl(const DocumentFragmentImpl &other);

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual DOMString namespaceURI() const;
    virtual NodeImpl *cloneNode ( bool deep, int &exceptioncode );

    // Other methods (not part of DOM)
    virtual bool childTypeAllowed( unsigned short type );
};


class DocumentTypeImpl : public NodeImpl
{
public:
    DocumentTypeImpl(DOMImplementationImpl *_implementation, DocumentPtr *doc,
                     const DOMString &qualifiedName, const DOMString &publicId,
                     const DOMString &systemId);
    ~DocumentTypeImpl();

    // DOM methods & attributes for DocumentType
    virtual const DOMString name() const;
    virtual NamedNodeMapImpl *entities() const;
    virtual NamedNodeMapImpl *notations() const;
    virtual DOMString publicId() const;
    virtual DOMString systemId() const;
    virtual DOMString internalSubset() const;

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual DOMString namespaceURI() const;

    // Other methods (not part of DOM)
    void setName(const QString& name);
    virtual void setOwnerDocument(DocumentPtr *doc);
    DOMImplementationImpl *implementation() const;

    GenericRONamedNodeMapImpl *m_entities;
    GenericRONamedNodeMapImpl *m_notations;
    virtual bool childTypeAllowed( unsigned short type );
    virtual NodeImpl *cloneNode ( bool deep, int &exceptioncode );
protected:
    DOMImplementationImpl *m_implementation;

    DOMStringImpl *m_qualifiedName;
    DOMStringImpl *m_publicId;
    DOMStringImpl *m_systemId;
};

}; //namespace
#endif
