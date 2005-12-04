/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2002-2003 Apple Computer, Inc.
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

#ifndef _DOM_DocumentImpl_h_
#define _DOM_DocumentImpl_h_

#include "xml/dom_elementimpl.h"
#include "xml/dom_textimpl.h"
#include "xml/dom2_traversalimpl.h"
#include "misc/shared.h"
#include "misc/loader.h"

#include <qstringlist.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qintcache.h>
#include <qintdict.h>
#include <qdict.h>
#include <qmap.h>

#include <kurl.h>

class QPaintDevice;
class QTextCodec;
class QPaintDeviceMetrics;
class KHTMLView;

namespace khtml {
    class Tokenizer;
    class CSSStyleSelector;
    class DocLoader;
    class CSSStyleSelectorList;
    class RenderArena;
    class RenderObject;
    class CounterNode;
    class CachedObject;
    class CachedCSSStyleSheet;
}

namespace DOM {

    class AbstractViewImpl;
    class AttrImpl;
    class CDATASectionImpl;
    class CSSStyleSheetImpl;
    class CommentImpl;
    class DocumentFragmentImpl;
    class DocumentImpl;
    class DocumentType;
    class DocumentTypeImpl;
    class ElementImpl;
    class EntityReferenceImpl;
    class EventImpl;
    class EventListener;
    class GenericRONamedNodeMapImpl;
    class HTMLDocumentImpl;
    class HTMLElementImpl;
    class HTMLImageElementImpl;
    class NodeFilter;
    class NodeFilterImpl;
    class NodeIteratorImpl;
    class NodeListImpl;
    class ProcessingInstructionImpl;
    class RangeImpl;
    class RegisteredEventListener;
    class StyleSheetImpl;
    class StyleSheetListImpl;
    class TextImpl;
    class TreeWalkerImpl;

class DOMImplementationImpl : public khtml::Shared<DOMImplementationImpl>
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
 * @internal A cache of element name (or id) to pointer
 * ### KDE4, QHash: better to store values here
 */
class ElementMappingCache
{
public:
    /**
     For each name, we hold a reference count, and a
     pointer. If the item is in the table, which implies
     reference count is > 1, the name is a valid key.
     If the pointer is non-null, it points to the appropriate
     mapping
    */
    struct ItemInfo
    {
        int       ref;
        ElementImpl* nd;
    };

    ElementMappingCache();

    /**
     Add a pointer as just one of candidates, not neccesserily the proper one
    */
    void add(const QString& id, ElementImpl* nd);

    /**
     Set the pointer as the definite mapping; it must have already been added
    */
    void set(const QString& id, ElementImpl* nd);

    /**
     Remove the item; it must have already been added.
    */
    void remove(const QString& id, ElementImpl* nd);

    /**
     Returns true if the item exists
    */
    bool contains(const QString& id);

    /**
     Returns the information for the given ID
    */
    ItemInfo* get(const QString& id);
private:
    QDict<ItemInfo> m_dict;
};

/**
 * @internal
 */
class DocumentImpl : public QObject, private khtml::CachedObjectClient, public NodeBaseImpl
{
    Q_OBJECT
public:
    DocumentImpl(DOMImplementationImpl *_implementation, KHTMLView *v);
    ~DocumentImpl();

    // DOM methods & attributes for Document

    DocumentTypeImpl *doctype() const;

    DOMImplementationImpl *implementation() const;
    ElementImpl *documentElement() const;
    virtual ElementImpl *createElement ( const DOMString &tagName, int* pExceptioncode = 0 );
    virtual AttrImpl *createAttribute( const DOMString &tagName, int* pExceptioncode = 0 );
    DocumentFragmentImpl *createDocumentFragment ();
    TextImpl *createTextNode ( DOMStringImpl* data ) { return new TextImpl( docPtr(), data); }
    TextImpl *createTextNode ( const QString& data )
        { return createTextNode(new DOMStringImpl(data.unicode(), data.length())); }
    CommentImpl *createComment ( DOMStringImpl* data );
    CDATASectionImpl *createCDATASection ( DOMStringImpl* data );
    ProcessingInstructionImpl *createProcessingInstruction ( const DOMString &target, DOMStringImpl* data );
    EntityReferenceImpl *createEntityReference ( const DOMString &name );
    NodeImpl *importNode( NodeImpl *importedNode, bool deep, int &exceptioncode );
    virtual ElementImpl *createElementNS ( const DOMString &_namespaceURI, const DOMString &_qualifiedName,
                                           int* pExceptioncode = 0 );
    virtual AttrImpl *createAttributeNS( const DOMString &_namespaceURI, const DOMString &_qualifiedName,
                                           int* pExceptioncode = 0 );
    ElementImpl *getElementById ( const DOMString &elementId ) const;

    // Actually part of HTMLDocument, but used for giving XML documents a window title as well
    DOMString title() const { return m_title; }
    void setTitle(const DOMString& _title);

    // DOM methods overridden from  parent classes

    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;

    // Other methods (not part of DOM)
    virtual bool isDocumentNode() const { return true; }
    virtual bool isHTMLDocument() const { return false; }

    virtual ElementImpl *createHTMLElement ( const DOMString &tagName );

    khtml::CSSStyleSelector *styleSelector() { return m_styleSelector; }

     /**
     * Updates the pending sheet count and then calls updateStyleSelector.
     */
    void styleSheetLoaded();

    /**
     * This method returns true if all top-level stylesheets have loaded (including
     * any @imports that they may be loading).
     */
    bool haveStylesheetsLoaded() { return m_pendingStylesheets <= 0 || m_ignorePendingStylesheets; }

    /**
     * Increments the number of pending sheets.  The <link> elements
     * invoke this to add themselves to the loading list.
     */
    void addPendingSheet() { m_pendingStylesheets++; }

    /**
     * Returns true if the document has pending stylesheets
     * loading.
     */
    bool hasPendingSheets() const { return m_pendingStylesheets; }

    /**
     * Called when one or more stylesheets in the document may have been added, removed or changed.
     *
     * Creates a new style selector and assign it to this document. This is done by iterating through all nodes in
     * document (or those before <BODY> in a HTML document), searching for stylesheets. Stylesheets can be contained in
     * <LINK>, <STYLE> or <BODY> elements, as well as processing instructions (XML documents only). A list is
     * constructed from these which is used to create the a new style selector which collates all of the stylesheets
     * found and is used to calculate the derived styles for all rendering objects.
     */
    void updateStyleSelector();

    void recalcStyleSelector();

    bool usesDescendantRules() { return m_usesDescendantRules; }
    void setUsesDescendantRules(bool b) { m_usesDescendantRules = b; }

    QString nextState();

    // Query all registered elements for their state
    QStringList docState();
    bool unsubmittedFormChanges();
    void registerMaintainsState(NodeImpl* e) { m_maintainsState.append(e); }
    void deregisterMaintainsState(NodeImpl* e) { m_maintainsState.removeRef(e); }

    // Set the state the document should restore to
    void setRestoreState( const QStringList &s) { m_state = s; }

    KHTMLView *view() const { return m_view; }
    KHTMLPart* part() const;

    RangeImpl *createRange();

    NodeIteratorImpl *createNodeIterator(NodeImpl *root, unsigned long whatToShow,
                                    NodeFilter &filter, bool entityReferenceExpansion, int &exceptioncode);

    TreeWalkerImpl *createTreeWalker(NodeImpl *root, unsigned long whatToShow, NodeFilterImpl *filter,
                            bool entityReferenceExpansion, int &exceptioncode);

    virtual void recalcStyle( StyleChange = NoChange );
    static QPtrList<DocumentImpl> * changedDocuments;
    virtual void updateRendering();
    void updateLayout();
    static void updateDocumentsRendering();
    khtml::DocLoader *docLoader() { return m_docLoader; }

    virtual void attach();
    virtual void detach();

    khtml::RenderArena* renderArena() { return m_renderArena; }

    // to get visually ordered hebrew and arabic pages right
    void setVisuallyOrdered();
    // to get URL decoding right
    void setDecoderCodec(const QTextCodec *codec);

    void setSelection(NodeImpl* s, int sp, NodeImpl* e, int ep);
    void clearSelection();

    void open ( bool clearEventListeners = true );
    virtual void close (  );
    void write ( const DOMString &text );
    void write ( const QString &text );
    void writeln ( const DOMString &text );
    void finishParsing (  );

    KURL URL() const { return m_url; }
    void setURL(const QString& url) { m_url = url; }

    KURL baseURL() const { return m_baseURL.isEmpty() ? m_url : m_baseURL; }
    void setBaseURL(const KURL& baseURL) { m_baseURL = baseURL; }

    QString baseTarget() const { return m_baseTarget; }
    void setBaseTarget(const QString& baseTarget) { m_baseTarget = baseTarget; }

    QString completeURL(const QString& url) const { return KURL(baseURL(),url,m_decoderMibEnum).url(); };
    DOMString canonURL(const DOMString& url) const { return url.isEmpty() ? url : completeURL(url.string()); }

    void setUserStyleSheet(const QString& sheet);
    QString userStyleSheet() const { return m_usersheet; }
    void setPrintStyleSheet(const QString& sheet) { m_printSheet = sheet; }
    QString printStyleSheet() const { return m_printSheet; }

    CSSStyleSheetImpl* elementSheet();
    virtual khtml::Tokenizer *createTokenizer();
    khtml::Tokenizer *tokenizer() { return m_tokenizer; }

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

    bool inCompatMode() const { return pMode == Compat; }
    bool inTransitionalMode() const { return pMode == Transitional; }
    bool inStrictMode() const { return pMode == Strict; }

    //void setHTMLMode( HTMLMode m ) { hMode = m; }
    HTMLMode htmlMode() const { return hMode; }

    void setParsing(bool b) { m_bParsing = b; }
    bool parsing() const { return m_bParsing; }

    void setTextColor( QColor color ) { m_textColor = color; }
    QColor textColor() const { return m_textColor; }

    void setDesignMode(bool b);
    bool designMode() const;

    // internal
    bool prepareMouseEvent( bool readonly, int x, int y, MouseEvent *ev );

    virtual bool childAllowed( NodeImpl *newChild );
    virtual bool childTypeAllowed( unsigned short nodeType );
    virtual NodeImpl *cloneNode ( bool deep );

    NodeImpl::Id getId( NodeImpl::IdType _type, DOMStringImpl* _nsURI, DOMStringImpl *_localName,
                        DOMStringImpl *_prefix, bool readonly, bool lookupHTML, int *pExceptioncode = 0);
    NodeImpl::Id getId( NodeImpl::IdType _type, DOMStringImpl *_nodeName, bool readonly, bool lookupHTML,
                                      int *pExceptioncode = 0);
    DOMString getName( NodeImpl::IdType _type, NodeImpl::Id _id ) const;

    StyleSheetListImpl* styleSheets() { return m_styleSheets; };

    DOMString preferredStylesheetSet() const { return m_preferredStylesheetSet; }
    DOMString selectedStylesheetSet() const;
    void setSelectedStylesheetSet(const DOMString&);
    void setPreferredStylesheetSet(const DOMString& s) { m_preferredStylesheetSet = s; }

    void addStyleSheet(StyleSheetImpl *, int *exceptioncode = 0);
    void removeStyleSheet(StyleSheetImpl *, int *exceptioncode = 0);

    QStringList availableStyleSheets() const { return m_availableSheets; }

    NodeImpl* hoverNode() const { return m_hoverNode; }
    void setHoverNode(NodeImpl *newHoverNode);
    NodeImpl *focusNode() const { return m_focusNode; }
    void setFocusNode(NodeImpl *newFocusNode);
    // Updates for :target (CSS3 selector).
    void setCSSTarget(NodeImpl* n);
    NodeImpl* getCSSTarget() { return m_cssTarget; }

    bool isDocumentChanged()	{ return m_docChanged; }
    virtual void setDocumentChanged(bool = true);
    void attachNodeIterator(NodeIteratorImpl *ni);
    void detachNodeIterator(NodeIteratorImpl *ni);
    void notifyBeforeNodeRemoval(NodeImpl *n);
    AbstractViewImpl *defaultView() const { return m_defaultView; }
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

    bool async() const { return m_async; }
    void setAsync(bool b) { m_async = b; }
    void abort();
    void load(const DOMString &uri);
    void loadXML(const DOMString &source);
    // from cachedObjectClient
    void setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet);
    void error(int err, const QString &text);

    typedef QMap<QString, ProcessingInstructionImpl*> LocalStyleRefs;
    LocalStyleRefs* localStyleRefs() { return &m_localStyleRefs; }

    virtual void defaultEventHandler(EventImpl *evt);
    virtual void setHTMLWindowEventListener(int id, EventListener *listener);
    EventListener *getHTMLWindowEventListener(int id);
    EventListener *createHTMLEventListener(const QString& code, const QString& name, NodeImpl* node);

    void addWindowEventListener(int id, EventListener *listener, const bool useCapture);
    void removeWindowEventListener(int id, EventListener *listener, bool useCapture);
    bool hasWindowEventListener(int id);

    EventListener *createHTMLEventListener(QString code);

    /**
     * Searches through the document, starting from fromNode, for the next selectable element that comes after fromNode.
     * The order followed is as specified in section 17.11.1 of the HTML4 spec, which is elements with tab indexes
     * first (from lowest to highest), and then elements without tab indexes (in document order).
     *
     * @param fromNode The node from which to start searching. The node after this will be focused. May be null.
     *
     * @return The focus node that comes after fromNode
     *
     * See http://www.w3.org/TR/html4/interact/forms.html#h-17.11.1
     */
    NodeImpl *nextFocusNode(NodeImpl *fromNode);

    /**
     * Searches through the document, starting from fromNode, for the previous selectable element (that comes _before_)
     * fromNode. The order followed is as specified in section 17.11.1 of the HTML4 spec, which is elements with tab
     * indexes first (from lowest to highest), and then elements without tab indexes (in document order).
     *
     * @param fromNode The node from which to start searching. The node before this will be focused. May be null.
     *
     * @return The focus node that comes before fromNode
     *
     * See http://www.w3.org/TR/html4/interact/forms.html#h-17.11.1
     */
    NodeImpl *previousFocusNode(NodeImpl *fromNode);

    ElementImpl* findAccessKeyElement(QChar c);

    int nodeAbsIndex(NodeImpl *node);
    NodeImpl *nodeWithAbsIndex(int absIndex);

    /**
     * Handles a HTTP header equivalent set by a meta tag using <meta http-equiv="..." content="...">. This is called
     * when a meta tag is encountered during document parsing, and also when a script dynamically changes or adds a meta
     * tag. This enables scripts to use meta tags to perform refreshes and set expiry dates in addition to them being
     * specified in a HTML file.
     *
     * @param equiv The http header name (value of the meta tag's "equiv" attribute)
     * @param content The header value (value of the meta tag's "content" attribute)
     */
    void processHttpEquiv(const DOMString &equiv, const DOMString &content);

    void dispatchImageLoadEventSoon(HTMLImageElementImpl *);
    void dispatchImageLoadEventsNow();
    void removeImage(HTMLImageElementImpl *);
    virtual void timerEvent(QTimerEvent *);

    // Returns the owning element in the parent document.
    // Returns 0 if this is the top level document.
    ElementImpl *ownerElement() const;

    DOMString domain() const;
    void setDomain( const DOMString &newDomain ); // not part of the DOM

    bool isURLAllowed(const QString& url) const;

    HTMLElementImpl* body();

    DOMString toString() const;

    void incDOMTreeVersion() { ++m_domtree_version; }
    unsigned int domTreeVersion() const { return m_domtree_version; }

    QDict<khtml::CounterNode>* counters(const khtml::RenderObject* o) { return m_counterDict[(void*)o]; }
    void setCounters(const khtml::RenderObject* o, QDict<khtml::CounterNode> *dict) { m_counterDict.insert((void*)o, dict);}
    void removeCounters(const khtml::RenderObject* o) { m_counterDict.remove((void*)o); }


    ElementMappingCache& underDocNamedCache() {
        return m_underDocNamedCache;
    }

    NodeListImpl::Cache* acquireCachedNodeListInfo(NodeListImpl::CacheFactory* fact,
                                                   NodeImpl* base, int type);
    void                 releaseCachedNodeListInfo(NodeListImpl::Cache* cache);

    ElementMappingCache& getElementByIdCache() const {
        return m_getElementByIdCache;
    }

signals:
    void finishedParsing();

protected:
    khtml::CSSStyleSelector *m_styleSelector;
    KHTMLView *m_view;
    QStringList m_state;

    khtml::DocLoader *m_docLoader;
    khtml::Tokenizer *m_tokenizer;
    KURL m_url;
    KURL m_baseURL;
    QString m_baseTarget;

    DocumentTypeImpl *m_doctype;
    DOMImplementationImpl *m_implementation;

    QString m_usersheet;
    QString m_printSheet;
    QStringList m_availableSheets;

    // Track the number of currently loading top-level stylesheets.  Sheets
    // loaded using the @import directive are not included in this count.
    // We use this count of pending sheets to detect when we can begin attaching
    // elements.
    int m_pendingStylesheets;
    bool m_ignorePendingStylesheets;

    CSSStyleSheetImpl *m_elemSheet;

    QPaintDevice *m_paintDevice;
    QPaintDeviceMetrics *m_paintDeviceMetrics;
    ParseMode pMode;
    HTMLMode hMode;

    QColor m_textColor;
    NodeImpl *m_hoverNode;
    NodeImpl *m_focusNode;
    NodeImpl *m_cssTarget;

    unsigned int m_domtree_version;

    struct IdNameMapping {
        IdNameMapping(unsigned short _start)
            : idStart(_start), count(0) {}
        ~IdNameMapping() {
            QIntDictIterator<DOM::DOMStringImpl> it(names);
            for (; it.current() ; ++it)
                it.current()->deref();
        }
        unsigned short idStart;
        unsigned short count;
        QIntDict<DOM::DOMStringImpl> names;
        QDict<void> ids;
    };

    IdNameMapping *m_attrMap;
    IdNameMapping *m_elementMap;
    IdNameMapping *m_namespaceMap;

    QPtrList<NodeIteratorImpl> m_nodeIterators;
    AbstractViewImpl *m_defaultView;

    unsigned short m_listenerTypes;
    StyleSheetListImpl* m_styleSheets;
    StyleSheetListImpl *m_addedStyleSheets; // programmatically added style sheets
    LocalStyleRefs m_localStyleRefs; // references to inlined style elements
    QPtrList<RegisteredEventListener> m_windowEventListeners;
    QPtrList<NodeImpl> m_maintainsState;

    // ### evaluate for placement in RenderStyle
    QPtrDict<QDict<khtml::CounterNode> > m_counterDict;

    bool visuallyOrdered;
    bool m_bParsing;
    bool m_docChanged;
    bool m_styleSelectorDirty;
    bool m_inStyleRecalc;
    bool m_usesDescendantRules;
    bool m_async;
    bool m_hadLoadError;
    bool m_docLoading;
    bool m_inSyncLoad;

    DOMString m_title;
    DOMString m_preferredStylesheetSet;
    khtml::CachedCSSStyleSheet *m_loadingXMLDoc;

    int m_decoderMibEnum;

    //Forms, images, etc., must be quickly accessible via document.name.
    ElementMappingCache m_underDocNamedCache;

    //Cache for nodelists and collections.
    QIntDict<NodeListImpl::Cache> m_nodeListCache;

    QPtrList<HTMLImageElementImpl> m_imageLoadEventDispatchSoonList;
    QPtrList<HTMLImageElementImpl> m_imageLoadEventDispatchingList;
    int m_imageLoadEventTimer;

    //Cache for getElementById
    mutable ElementMappingCache m_getElementByIdCache;

    khtml::RenderArena* m_renderArena;
private:
    mutable DOMString m_domain;
};

class DocumentFragmentImpl : public NodeBaseImpl
{
public:
    DocumentFragmentImpl(DocumentPtr *doc);
    DocumentFragmentImpl(const DocumentFragmentImpl &other);

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)
    virtual bool childTypeAllowed( unsigned short type );

    virtual DOMString toString() const;
};


class DocumentTypeImpl : public NodeImpl
{
public:
    DocumentTypeImpl(DOMImplementationImpl *_implementation, DocumentPtr *doc,
                     const DOMString &qualifiedName, const DOMString &publicId,
                     const DOMString &systemId);
    ~DocumentTypeImpl();

    // DOM methods & attributes for DocumentType
    NamedNodeMapImpl *entities() const;
    NamedNodeMapImpl *notations() const;

    DOMString name() const { return m_qualifiedName; }
    DOMString publicId() const { return m_publicId; }
    DOMString systemId() const { return m_systemId; }
    DOMString internalSubset() const { return m_subset; }

    // DOM methods overridden from  parent classes
    virtual DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual bool childTypeAllowed( unsigned short type );
    virtual NodeImpl *cloneNode ( bool deep );

    // Other methods (not part of DOM)
    void setName(const DOMString& n) { m_qualifiedName = n; }
    void setPublicId(const DOMString& publicId) { m_publicId = publicId; }
    void setSystemId(const DOMString& systemId) { m_systemId = systemId; }
    DOMImplementationImpl *implementation() const { return m_implementation; }
    void copyFrom(const DocumentTypeImpl&);

    virtual DOMString toString() const;

protected:
    DOMImplementationImpl *m_implementation;
    mutable NamedNodeMapImpl* m_entities;
    mutable NamedNodeMapImpl* m_notations;

    DOMString m_qualifiedName;
    DOMString m_publicId;
    DOMString m_systemId;
    DOMString m_subset;
};

} //namespace
#endif
