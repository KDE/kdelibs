/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright 1999 Lars Knoll (knoll@kde.org)
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
 * This file includes excerpts from the Document Object Model (DOM)
 * Level 1 Specification (Recommendation)
 * http://www.w3.org/TR/REC-DOM-Level-1/
 * Copyright © World Wide Web Consortium , (Massachusetts Institute of
 * Technology , Institut National de Recherche en Informatique et en
 * Automatique , Keio University ). All Rights Reserved.
 */

#ifndef _DOM_Document_h_
#define _DOM_Document_h_

#include <dom/dom_node.h>
#include <dom/css_stylesheet.h>

class KHTMLView;
class KHTMLPart;

namespace DOM {

class DOMString;
class DocumentType;
class NodeList;
class CDATASection;
class Comment;
class DocumentFragment;
class Text;
class DOMImplementation;
class Element;
class Attr;
class EntityReference;
class ProcessingInstruction;
class DocumentImpl;
class Range;
class NodeIterator;
class TreeWalker;
class NodeFilter;
class DOMImplementationImpl;
class DocumentTypeImpl;
class Event;
class AbstractView;
class CSSStyleDeclaration;
class HTMLElementImpl;
class HTMLFrameElement;
class HTMLElementImpl;
class HTMLIFrameElement;
class HTMLObjectElement;
class HTMLDocument;

/**
 * The \c DOMImplementation interface provides a number of
 * methods for performing operations that are independent of any
 * particular instance of the document object model.
 *
 * DOM Level 2 and newer provide means for creating documents directly,
 * which was not possible with DOM Level 1.
 */
class KHTML_EXPORT DOMImplementation
{
   friend class Document;
public:
    DOMImplementation();
    DOMImplementation(const DOMImplementation &other);

    DOMImplementation & operator = (const DOMImplementation &other);
    ~DOMImplementation();

    /**
     * Test if the DOM implementation implements a specific feature.
     *
     * @param feature The package name of the feature to test. In
     * Level 1, the legal values are "HTML" and "XML"
     * (case-insensitive).
     *
     * @param version This is the version number of the package name
     * to test. In Level 1, this is the string "1.0". If the version
     * is not specified, supporting any version of the feature will
     * cause the method to return \c true .
     *
     * @return \c true if the feature is implemented in
     * the specified version, \c false otherwise.
     *
     */
    bool hasFeature ( const DOMString &feature, const DOMString &version );

    /**
     * Introduced in DOM Level 2
     *
     * Creates an empty DocumentType node. Entity declarations and notations
     * are not made available. Entity reference expansions and default
     * attribute additions do not occur. It is expected that a future version
     * of the DOM will provide a way for populating a DocumentType.
     *
     * HTML-only DOM implementations do not need to implement this method.
     *
     * @param qualifiedName The qualified name of the document type to be
     * created.
     *
     * @param publicId The external subset public identifier.
     *
     * @param systemId The external subset system identifier.
     *
     * @return A new DocumentType node with Node.ownerDocument set to null.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified qualified name contains
     * an illegal character.
     *
     * NAMESPACE_ERR: Raised if the qualifiedName is malformed.
     */
    DocumentType createDocumentType ( const DOMString &qualifiedName,
                                      const DOMString &publicId,
                                      const DOMString &systemId );

    /**
     * Introduced in DOM Level 2
     *
     * Creates an XML Document object of the specified type with its document
     * element. HTML-only DOM implementations do not need to implement this
     * method.
     *
     * @param namespaceURI The namespace URI of the document element to create.
     *
     * @param qualifiedName The qualified name of the document element to be
     * created.
     *
     * @param doctype The type of document to be created or null. When doctype
     * is not null, its Node.ownerDocument attribute is set to the document
     * being created.
     *
     * @return A new Document object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified qualified name contains
     * an illegal character.
     *
     * NAMESPACE_ERR: Raised if the qualifiedName is malformed, if the
     * qualifiedName has a prefix and the namespaceURI is null, or if the
     * qualifiedName has a prefix that is "xml" and the namespaceURI is
     * different from "http://www.w3.org/XML/1998/namespace" [Namespaces].
     *
     * WRONG_DOCUMENT_ERR: Raised if doctype has already been used with a
     * different document or was created from a different implementation.
     */
    Document createDocument ( const DOMString &namespaceURI,
                              const DOMString &qualifiedName,
                              const DocumentType &doctype );

    /**
     * Introduced in DOM Level 3
     * This method makes available a DOMImplementation's specialized
     * interface.
     *
     * @param feature The name of the feature requested (case-insensitive)
     *
     * @return Returns an alternate DOMImplementation which implements
     * the specialized APIs of the specified feature, if any, or null
     * if there is no alternate DOMImplementation object which implements
     * interfaces associated with that feature. Any alternate DOMImplementation
     * returned by this method must delegate to the primary core DOMImplementation
     * and not return results inconsistent with the primary DOMImplementation.
     */
    DOMImplementation getInterface(const DOMString &feature) const;

    /**
     * Introduced in DOM Level 2
     * This method is from the DOMImplementationCSS interface
     *
     * Creates a new CSSStyleSheet.
     *
     * @param title The advisory title. See also the Style Sheet Interfaces
     * section.
     *
     * @param media The comma-separated list of media associated with the
     * new style sheet. See also the Style Sheet Interfaces section.
     *
     * @return A new CSS style sheet.
     *
     * @exception SYNTAX_ERR: Raised if the specified media string value has a syntax error and is unparsable.
     */
    CSSStyleSheet createCSSStyleSheet(const DOMString &title, const DOMString &media);

    /**
     * Introduced in DOM Level 2
     * This method is from the HTMLDOMImplementation interface
     *
     * Creates an HTMLDocument with the minimal tree made of these
     * elements: HTML,HEAD,TITLE and BODY.
     * It extends the core interface which can be used to create an
     * XHTML document by passing the XHTML namespace as the namespace
     * for the root element.
     *
     * @param title The title of the document to be set as the content
     * of the TITLE element, through a child Text node.
     *
     * @return the HTMLdocument
     */
    HTMLDocument createHTMLDocument(const DOMString& title);

    /**
     * @internal
     * not part of the DOM
     */
    DOMImplementationImpl *handle() const;
    bool isNull() const;

protected:
    DOMImplementation(DOMImplementationImpl *i);
    DOMImplementationImpl *impl;
};

/**
 * The \c Document interface represents the entire HTML or
 * XML document. Conceptually, it is the root of the document tree,
 * and provides the primary access to the document's data.
 *
 *  Since elements, text nodes, comments, processing instructions,
 * etc. cannot exist outside the context of a \c Document
 * , the \c Document interface also contains the factory
 * methods needed to create these objects. The \c Node
 * objects created have a \c ownerDocument attribute which
 * associates them with the \c Document within whose
 * context they were created.
 *
 */
class KHTML_EXPORT Document : public Node
{
    friend class ::KHTMLView;
    friend class ::KHTMLPart;
    friend class AbstractView;
    friend class DOMImplementation;
    friend class HTMLFrameElement;
    friend class HTMLIFrameElement;
    friend class HTMLObjectElement;

public:
    Document();
    /**
     * don't create an implementation if false
     * use at own risk
     */
    Document(bool);
    Document(const Document &other);
    Document(const Node &other) : Node()
	     {(*this)=other;}

    Document & operator = (const Node &other);
    Document & operator = (const Document &other);

    ~Document();

    /**
     * The Document Type Declaration (see \c DocumentType
     * ) associated with this document. For HTML documents as well as
     * XML documents without a document type declaration this returns
     * \c null . The DOM Level 1 does not support editing
     * the Document Type Declaration, therefore \c docType
     * cannot be altered in any way.
     *
     */
    DocumentType doctype() const;

    /**
     * The \c DOMImplementation object that handles this
     * document. A DOM application may use objects from multiple
     * implementations.
     *
     */
    DOMImplementation implementation() const;

    /**
     * This is a convenience attribute that allows direct access to
     * the child node that is the root element of the document. For
     * HTML documents, this is the element with the tagName "HTML".
     *
     */
    Element documentElement() const;

    /**
     * Creates an element of the type specified. Note that the
     * instance returned implements the Element interface, so
     * attributes can be specified directly on the returned object.
     *
     * @param tagName The name of the element type to instantiate. For
     * XML, this is case-sensitive. For HTML, the \c tagName
     * parameter may be provided in any case, but it must be
     * mapped to the canonical uppercase form by the DOM
     * implementation.
     *
     * @return A new \c Element object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified name contains an
     * invalid character.
     *
     */
    Element createElement ( const DOMString &tagName );

    /**
     * Introduced in DOM Level 2
     * Creates an element of the given qualified name and namespace URI.
     *
     * @param namespaceURI The namespace URI of the element to create.
     *
     * @param qualifiedName The qualified name of the element type to instantiate.
     *
     * @return A new Element object with the following attributes:
     *
     * @exception INVALID_CHARACTER_ERR Raised if the specified qualified name
     * contains an illegal character.
     *
     * @exception NAMESPACE_ERR Raised if the qualifiedName is malformed, if
     * the qualifiedName has a prefix and the namespaceURI is null, or if the
     * qualifiedName has a prefix that is "xml" and the namespaceURI is
     * different from "http://www.w3.org/XML/1998/namespace"
     */
    Element createElementNS( const DOMString &namespaceURI,
                             const DOMString &qualifiedName );

    /**
     * Creates an empty \c DocumentFragment object.
     *
     * @return A new \c DocumentFragment .
     *
     */
    DocumentFragment createDocumentFragment (  );

    /**
     * Creates a \c Text node given the specified string.
     *
     * @param data The data for the node.
     *
     * @return The new \c Text object.
     *
     */
    Text createTextNode ( const DOMString &data );

    /**
     * Creates a \c Comment node given the specified
     * string.
     *
     * @param data The data for the node.
     *
     * @return The new \c Comment object.
     *
     */
    Comment createComment ( const DOMString &data );

    /**
     * Creates a \c CDATASection node whose value is the
     * specified string.
     *
     * @param data The data for the \c CDATASection
     * contents.
     *
     * @return The new \c CDATASection object.
     *
     * @exception DOMException
     * NOT_SUPPORTED_ERR: Raised if this document is an HTML document.
     *
     */
    CDATASection createCDATASection ( const DOMString &data );

    /**
     * Creates a \c ProcessingInstruction node given the
     * specified name and data strings.
     *
     * @param target The target part of the processing instruction.
     *
     * @param data The data for the node.
     *
     * @return The new \c ProcessingInstruction object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if an invalid character is
     * specified.
     *
     *  NOT_SUPPORTED_ERR: Raised if this document is an HTML
     * document.
     *
     */
    ProcessingInstruction createProcessingInstruction ( const DOMString &target,
                                                        const DOMString &data );

    /**
     * Creates an \c Attr of the given name. Note that the
     * \c Attr instance can then be set on an \c Element
     * using the \c setAttribute method.
     *
     * @param name The name of the attribute.
     *
     * @return A new \c Attr object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified name contains an
     * invalid character.
     *
     */
    Attr createAttribute ( const DOMString &name );

    /**
     * Introduced in DOM Level 2
     * Creates an attribute of the given qualified name and namespace URI.
     * HTML-only DOM implementations do not need to implement this method.
     *
     * @param namespaceURI The namespace URI of the attribute to create.
     *
     * @param qualifiedName The qualified name of the attribute to instantiate.
     *
     * @return A new Attr object with the following attributes:
     * Node.nodeName - qualifiedName
     * Node.namespaceURI - namespaceURI
     * Node.prefix - prefix, extracted from qualifiedName, or null if there is
     * no prefix
     * Node.localName - local name, extracted from qualifiedName
     * Attr.name - qualifiedName
     * Node.nodeValue - the empty string
     *
     * @exception INVALID_CHARACTER_ERR Raised if the specified qualified name
     * contains an illegal character.
     *
     * @exception NAMESPACE_ERR Raised if the qualifiedName is malformed, if
     * the qualifiedName has a prefix and the namespaceURI is null, if the
     * qualifiedName has a prefix that is "xml" and the namespaceURI is
     * different from "http://www.w3.org/XML/1998/namespace", or if the
     * qualifiedName is "xmlns" and the namespaceURI is different from
     * "http://www.w3.org/2000/xmlns/".
     */
    Attr createAttributeNS( const DOMString &namespaceURI,
                            const DOMString &qualifiedName );

    /**
     * Creates an EntityReference object.
     *
     * @param name The name of the entity to reference.
     *
     * @return The new \c EntityReference object.
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified name contains an
     * invalid character.
     *
     *  NOT_SUPPORTED_ERR: Raised if this document is an HTML
     * document.
     *
     */
    EntityReference createEntityReference ( const DOMString &name );

    /**
     * Moved from HTMLDocument in DOM Level 2
     * Returns the Element whose \c id is given by
     * elementId. If no such element exists, returns \c null
     * . Behavior is not defined if more than one element has
     * this \c id .
     *
     * @param elementId The unique \c id value for an
     * element.
     *
     * @return The matching element.
     *
     */
    Element getElementById ( const DOMString &elementId ) const;

    /**
     * No Exceptions.
     *
     * Returns a \c NodeList of all the \c Element 's
     * with a given tag name in the order in which they
     * would be encountered in a preorder traversal of the
     * \c Document tree.
     *
     * @param tagname The name of the tag to match on. The special
     * value "*" matches all tags.
     *
     * @return A new \c NodeList object containing all the
     * matched \c Element s.
     *
     */
    NodeList getElementsByTagName ( const DOMString &tagname );

    /**
     * Introduced in DOM Level 2
     * No Exceptions
     *
     * Returns a NodeList of all the Elements with a given local name and
     * namespace URI in the order in which they are encountered in a preorder
     * traversal of the Document tree.
     *
     * @param namespaceURI The namespace URI of the elements to match on. The
     * special value "*" matches all namespaces.
     *
     * @param localName The local name of the elements to match on. The special
     * value "*" matches all local names.
     *
     * @return A new NodeList object containing all the matched Elements.
     */
    NodeList getElementsByTagNameNS( const DOMString &namespaceURI,
                                     const DOMString &localName );

    /**
     * Introduced in HTML 5.
     * No Exceptions.
     *
     * Returns a \c NodeList of all the \c Element 's
     * with a given class name in the order in which they
     * would be encountered in a preorder traversal of the
     * \c Document tree.
     *
     * @param tagname An unordered set of unique space-separated
     * tokens representing classes.
     *
     * @return A new \c NodeList object containing all the
     * matched \c Element s.
     *
     * @since 4.1
     */
    NodeList getElementsByClassName ( const DOMString &className );

    /**
     * Introduced in DOM Level 2
     *
     * Imports a node from another document to this document. The returned node
     * has no parent; (parentNode is null). The source node is not altered or
     * removed from the original document; this method creates a new copy of
     * the source node.
     *
     * For all nodes, importing a node creates a node object owned by the
     * importing document, with attribute values identical to the source node's
     * nodeName and nodeType, plus the attributes related to namespaces
     * (prefix, localName, and namespaceURI).
     *
     * As in the cloneNode operation on a Node, the source node is not altered.
     * Additional information is copied as appropriate to the nodeType,
     * attempting to mirror the behavior expected if a fragment of XML or HTML
     * source was copied from one document to another, recognizing that the two
     * documents may have different DTDs in the XML case. The following list
     * describes the specifics for each type of node.
     *
     * ATTRIBUTE_NODE
     * The ownerElement attribute is set to null and the specified flag is set
     * to true on the generated Attr. The descendants of the source Attr are
     * recursively imported and the resulting nodes reassembled to form the
     * corresponding subtree. Note that the deep parameter has no effect on
     * Attr nodes; they always carry their children with them when imported.
     *
     * DOCUMENT_FRAGMENT_NODE
     * If the deep option was set to true, the descendants of the source
     * element are recursively imported and the resulting nodes reassembled to
     * form the corresponding subtree. Otherwise, this simply generates an
     * empty DocumentFragment.
     *
     * DOCUMENT_NODE
     * Document nodes cannot be imported.
     *
     * DOCUMENT_TYPE_NODE
     * DocumentType nodes cannot be imported.
     *
     * ELEMENT_NODE
     * Specified attribute nodes of the source element are imported, and the
     * generated Attr nodes are attached to the generated Element. Default
     * attributes are not copied, though if the document being imported into
     * defines default attributes for this element name, those are assigned. If
     * the importNode deep parameter was set to true, the descendants of the
     * source element are recursively imported and the resulting nodes
     * reassembled to form the corresponding subtree.
     *
     * ENTITY_NODE
     * Entity nodes can be imported, however in the current release of the DOM
     * the DocumentType is readonly. Ability to add these imported nodes to a
     * DocumentType will be considered for addition to a future release of the
     * DOM.
     * On import, the publicId, systemId, and notationName attributes are
     * copied. If a deep import is requested, the descendants of the source
     * Entity are recursively imported and the resulting nodes reassembled to
     * form the corresponding subtree.
     *
     * ENTITY_REFERENCE_NODE Only the EntityReference itself is copied, even if
     * a deep import is requested, since the source and destination documents
     * might have defined the entity differently. If the document being
     * imported into provides a definition for this entity name, its value is
     * assigned.
     *
     * NOTATION_NODE
     * Notation nodes can be imported, however in the current release of the
     * DOM the DocumentType is readonly. Ability to add these imported nodes to
     * a DocumentType will be considered for addition to a future release of
     * the DOM.
     * On import, the publicId and systemId attributes are copied.
     * Note that the deep parameter has no effect on Notation nodes since they
     * never have any children.
     *
     * PROCESSING_INSTRUCTION_NODE
     * The imported node copies its target and data values from those of the
     * source node.
     *
     * TEXT_NODE, CDATA_SECTION_NODE, COMMENT_NODE
     * These three types of nodes inheriting from CharacterData copy their data
     * and length attributes from those of the source node.
     *
     * @param importedNode The node to import.
     *
     * @param deep If true, recursively import the subtree under the specified
     * node; if false, import only the node itself, as explained above. This
     * has no effect on Attr, EntityReference, and Notation nodes.
     *
     * @return The imported node that belongs to this Document.
     *
     * @exception DOMException
     * NOT_SUPPORTED_ERR: Raised if the type of node being imported is not
     * supported.
     */
    Node importNode( const Node & importedNode, bool deep );

    /**
     * @internal
     * not part of the DOM
     */
    bool isHTMLDocument() const;

    /**
     * Introduced in DOM Level 2
     * This method is from the DocumentRange interface
     *
     * @return Range
     * The initial state of the Range returned from this method is such that
     * both of its boundary-points are positioned at the beginning of the
     * corresponding Document, before any content. The Range returned can only
     * be used to select content associated with this Document, or with
     * DocumentFragments and Attrs for which this Document is the ownerDocument.
     */
    Range createRange();

    /**
     * Introduced in DOM Level 2
     * This method is from the DocumentTraversal interface
     *
     * Create a new NodeIterator over the subtree rooted at the specified node.
     *
     * @param root The node which will be iterated together with its children.
     * The iterator is initially positioned just before this node. The
     * whatToShow flags and the filter, if any, are not considered when setting
     * this position. The root must not be null.
     *
     * @param whatToShow This flag specifies which node types may appear in the
     * logical view of the tree presented by the iterator. See the description
     * of NodeFilter for the set of possible SHOW_ values. These flags can be
     * combined using OR.
     *
     * @param filter The NodeFilter to be used with this NodeIterator, or null to
     * indicate no filter.
     *
     * @param entityReferenceExpansion The value of this flag determines
     * whether entity reference nodes are expanded.
     *
     * @return NodeIterator The newly created NodeIterator.
     *
     * @exception DOMException
     * NOT_SUPPORTED_ERR: Raised if the specified root is null.
     */
    NodeIterator createNodeIterator(Node root, unsigned long whatToShow,
                                    NodeFilter filter,
                                    bool entityReferenceExpansion);

    /**
     * Introduced in DOM Level 2
     * This method is from the DocumentTraversal interface
     *
     * Create a new TreeWalker over the subtree rooted at the specified node.
     *
     * @param root The node which will serve as the root for the TreeWalker.
     * The whatToShow flags and the NodeFilter are not considered when setting
     * this value; any node type will be accepted as the root. The currentNode
     * of the TreeWalker is initialized to this node, whether or not it is
     * visible. The root functions as a stopping point for traversal methods
     * that look upward in the document structure, such as parentNode and
     * nextNode. The root must not be null.
     *
     * @param whatToShow This flag specifies which node types may appear in the
     * logical view of the tree presented by the tree-walker. See the
     * description of NodeFilter for the set of possible SHOW_ values. These
     * flags can be combined using OR.
     *
     * @param filter The NodeFilter to be used with this TreeWalker, or null to
     * indicate no filter.
     *
     * @param entityReferenceExpansion If this flag is false, the contents of
     * EntityReference nodes are not presented in the logical view.
     *
     * @return The newly created TreeWalker.
     *
     * @exception DOMException
     * NOT_SUPPORTED_ERR: Raised if the specified root is null.
     */
    TreeWalker createTreeWalker(Node root, unsigned long whatToShow,
                                NodeFilter filter,
                                bool entityReferenceExpansion);

    /**
     * Introduced in DOM Level 2
     * This method is from the DocumentEvent interface
     *
     * The createEvent method is used in creating Events when it is either
     * inconvenient or unnecessary for the user to create an Event themselves.
     * In cases where the implementation provided Event is insufficient, users
     * may supply their own Event implementations for use with the
     * dispatchEvent method.
     *
     * @param eventType The eventType parameter specifies the type of Event
     * interface to be created. If the Event interface specified is supported
     * by the implementation this method will return a new Event of the
     * interface type requested. If the Event is to be dispatched via the
     * dispatchEvent method the appropriate event init method must be called
     * after creation in order to initialize the Event's values. As an example,
     * a user wishing to synthesize some kind of UIEvent would call createEvent
     * with the parameter "UIEvents". The initUIEvent method could then be
     * called on the newly created UIEvent to set the specific type of UIEvent
     * to be dispatched and set its context information.
     *
     * @return The newly created EventExceptions
     *
     * @exception DOMException
     * NOT_SUPPORTED_ERR: Raised if the implementation does not support the
     * type of Event interface requested
     */
    Event createEvent(const DOMString &eventType);

    /**
     * Introduced in DOM Level 2
     * This method is from the DocumentView interface
     *
     * The default AbstractView for this Document, or null if none available.
     */
    AbstractView defaultView() const;

    /**
     * Introduced in DOM Level 2
     * This method is from the DocumentStyle interface
     *
     * A list containing all the style sheets explicitly linked into or
     * embedded in a document. For HTML documents, this includes external style
     * sheets, included via the HTML LINK element, and inline STYLE elements.
     * In XML, this includes external style sheets, included via style sheet
     * processing instructions (see [XML-StyleSheet]).
     */
    StyleSheetList styleSheets() const;

    /**
     * CSS3 mechanism for selecting alternate stylesheets using the DOM.
     * Might change without further notice.
     */

    DOMString preferredStylesheetSet();
    DOMString selectedStylesheetSet();
    void setSelectedStylesheetSet(const DOMString& aString);

    /**
     * Adds a new style sheet to the list of style sheets.
     *
     * The new style sheet will be applied after all author and implicit
     * style sheets, but before the user style sheet.
     *
     * Create new style sheets with e. g.
     * \c DOMImplementation::createCSSStyleSheet
     *
     * This is not part of the official DOM.
     *
     * @param sheet style sheet
     * @exception DOMException
     */
    void addStyleSheet(const StyleSheet &sheet);
    
    /**
     * Removes a style sheet to the list of style sheets.
     *
     * Only sheets added by \c addStyleSheet may be removed.
     * 
     * This is not part of the official DOM.
     *
     * @param sheet style sheet to remove
     * @exception DOMException
     * NOT_FOUND_ERR \c sheet is not contained in the list of style sheets or
     * it has not been added by \c addStyleSheet
     */
    void removeStyleSheet(const StyleSheet &sheet);
    
    /**
     * @return The KHTML view widget of this document.
     */
    KHTMLView *view() const;

    /**
     * Introduced in DOM Level 2
     * This method is from the DocumentCSS interface
     *
     * This method is used to retrieve the override style declaration for a
     * specified element and a specified pseudo-element.
     *
     * @param elt The element whose style is to be modified. This parameter
     * cannot be null.
     *
     * @param pseudoElt The pseudo-element or null if none.
     *
     * @return The override style declaration.
     */
    CSSStyleDeclaration getOverrideStyle(const Element &elt,
                                         const DOMString &pseudoElt);

    /**
     * Introduced in DOM Level 3
     * This method is from the DocumentLS interface
     *
     * Indicates whether the method DocumentLS.load() should be synchronous or
     * asynchronous. When the async attribute is set to true the load method
     * returns control to the caller before the document has completed loading.
     * The default value of this attribute is true.
     */
    bool async() const;

    /**
     * Introduced in DOM Level 3
     * This method is from the DocumentLS interface
     *
     * see async
     *
     * @exception DOMException
     * NOT_SUPPORTED_ERR: Raised if the implementation doesn't support the mode
     * the attribute is being set to.
     */
    void setAsync( bool );


    /**
     * Introduced in DOM Level 3
     * This method is from the DocumentLS interface
     *
     * If the document is currently being loaded as a result of the method load
     * being invoked the loading and parsing is immediately aborted. The
     * possibly partial result of parsing the document is discarded and the
     * document is cleared.
     */
    void abort();

    /**
     * Introduced in DOM Level 3
     * This method is from the DocumentLS interface
     *
     * Replaces the content of the document with the result of parsing the
     * given URI. Invoking this method will either block the caller or return
     * to the caller immediately depending on the value of the async attribute.
     * Once the document is fully loaded a "load" event (as defined in
     * [DOM Level 3 Events], except that the Event.targetNode will be the
     * document, not an element) will be dispatched on the document. If an
     * error occurs, an implementation dependent "error" event will be
     * dispatched on the document. If this method is called on a document that
     * is currently loading, the current load is interrupted and the new URI
     * load is initiated.
     *
     * When invoking this method the parameters used in the DOMParser interface
     * are assumed to have their default values with the exception that the
     * parameters "entities", "normalize-characters",
     * "check-character-normalization" are set to "false".
     *
     * The result of a call to this method is the same the result of a call to
     * DOMParser.parseWithContext with an input stream referencing the URI that
     * was passed to this call, the document as the context node, and the
     * action ACTION_REPLACE_CHILDREN.
     *
     * @param uri of type DOMString
     * The URI reference for the XML file to be loaded. If this is a relative
     * URI, the base URI used by the implementation is implementation dependent.
     *
     * @return If async is set to true load returns true if the document load
     * was successfully initiated. If an error occurred when initiating the
     * document load, load returns false.
     * If async is set to false load returns true if the document was
     * successfully loaded and parsed. If an error occurred when either loading
     * or parsing the URI, load returns false.
     */
    void load( const DOMString &uri );

    /**
     * Introduced in DOM Level 3
     * This method is from the DocumentLS interface
     *
     * Replace the content of the document with the result of parsing the input
     * string, this method is always synchronous. This method always parses
     * from a DOMString, which means the data is always UTF-16. All other
     * encoding information is ignored.
     *
     * The parameters used in the DOMParser interface are assumed to have their
     * default values when invoking this method.
     *
     * The result of a call to this method is the same as the result of a call
     * to DOMParser.parseWithContext with an input stream containing the string
     * passed to this call, the document as the context node, and the action
     * ACTION_REPLACE_CHILDREN.
     *
     * @param source A string containing an XML document.
     */
    void loadXML( const DOMString &source );

    /**
     * Introduced in Selectors Level 1.
     *
     * Returns the first (in document order) element matching the given
     * CSS selector @p query.
     *
     * @since 4.5
     */
    Element querySelector(const DOMString& query) const;

    /**
     * Introduced in Selectors Level 1.
     *
     * Returns all (in document order) elements matching the given
     * CSS selector @p query. Note that the returned NodeList is
     * static and not live, and will not be updated when the document
     * changes
     *
     * @since 4.5
     */
    NodeList querySelectorAll(const DOMString& query) const;

    /**
     * not part of the official DOM
     *
     * Documents are read-only by default, but they can be made editable by
     * entering "design mode".
     *
     * @return whether this document is in design mode.
     */
    bool designMode() const;

    /**
     * not part of the official DOM
     *
     * @param enable @p true to enable design mode, @p false to disable.
     * @see designMode
     */
    void setDesignMode(bool enable);

    /**
     * not part of the DOM
     *
     * completes a given URL
     */
    DOMString completeURL(const DOMString& url);

    DOMString toString() const;
    

    /**
     * not part of the DOM
     *
     * javascript editing command support
     */
    bool execCommand(const DOMString &command, bool userInterface, const DOMString &value);
    bool queryCommandEnabled(const DOMString &command);
    bool queryCommandIndeterm(const DOMString &command);
    bool queryCommandState(const DOMString &command);
    bool queryCommandSupported(const DOMString &command);
    DOMString queryCommandValue(const DOMString &command);
    
    /**
     * not part of the DOM
     *
     * Updates the rendered display after one or more changes to
     * the DOM structure
     */
    void updateRendering();

    Document( DocumentImpl *i);
protected:

    friend class Node;
};

class DocumentFragmentImpl;

/**
 * \c DocumentFragment is a "lightweight" or "minimal"
 * \c Document object. It is very common to want to be
 * able to extract a portion of a document's tree or to create a new
 * fragment of a document. Imagine implementing a user command like
 * cut or rearranging a document by moving fragments around. It is
 * desirable to have an object which can hold such fragments and it is
 * quite natural to use a Node for this purpose. While it is true that
 * a \c Document object could fulfil this role, a
 * \c Document object can potentially be a heavyweight object,
 * depending on the underlying implementation. What is really needed
 * for this is a very lightweight object. \c DocumentFragment
 * is such an object.
 *
 *  Furthermore, various operations -- such as inserting nodes as
 * children of another \c Node -- may take
 * \c DocumentFragment objects as arguments; this results in all
 * the child nodes of the \c DocumentFragment being moved
 * to the child list of this node.
 *
 *  The children of a \c DocumentFragment node are zero or
 * more nodes representing the tops of any sub-trees defining the
 * structure of the document. \c DocumentFragment nodes do
 * not need to be well-formed XML documents (although they do need to
 * follow the rules imposed upon well-formed XML parsed entities,
 * which can have multiple top nodes). For example, a
 * \c DocumentFragment might have only one child and that child
 * node could be a \c Text node. Such a structure model
 * represents neither an HTML document nor a well-formed XML document.
 *
 *  When a \c DocumentFragment is inserted into a
 * \c Document (or indeed any other \c Node that may
 * take children) the children of the \c DocumentFragment
 * and not the \c DocumentFragment itself are inserted
 * into the \c Node . This makes the
 * \c DocumentFragment very useful when the user wishes to create
 * nodes that are siblings; the \c DocumentFragment acts
 * as the parent of these nodes so that the user can use the standard
 * methods from the \c Node interface, such as
 * \c insertBefore() and \c appendChild() .
 *
 */
class KHTML_EXPORT DocumentFragment : public Node
{
    friend class Document;
    friend class HTMLElementImpl;
    friend class Range;

public:
    DocumentFragment();
    DocumentFragment(const DocumentFragment &other);
    DocumentFragment(const Node &other) : Node()
         {(*this)=other;}

    DocumentFragment & operator = (const Node &other);
    DocumentFragment & operator = (const DocumentFragment &other);

    ~DocumentFragment();

    /**
     * Introduced in Selectors Level 1.
     *
     * Returns the first (in document order) element in this fragment
     * matching the given CSS selector @p query.
     *
     * @since 4.5
     */
    Element querySelector(const DOMString& query) const;

    /**
     * Introduced in Selectors Level 1.
     *
     * Returns all (in document order) elements in this fragment matching the
     * given CSS selector @p query. Note that the returned NodeList is
     * static and not live, and will not be updated when the document
     * changes
     *
     * @since 4.5
     */
    NodeList querySelectorAll(const DOMString& query) const;
protected:
    DocumentFragment(DocumentFragmentImpl *i);
};

class NamedNodeMap;
class DOMString;

/**
 * Each \c Document has a \c doctype attribute
 * whose value is either \c null or a \c DocumentType
 * object. The \c DocumentType interface in the
 * DOM Level 1 Core provides an interface to the list of entities that
 * are defined for the document, and little else because the effect of
 * namespaces and the various XML scheme efforts on DTD representation
 * are not clearly understood as of this writing.
 *
 *  The DOM Level 1 doesn't support editing \c DocumentType
 * nodes.
 *
 */
class KHTML_EXPORT DocumentType : public Node
{
    friend class Document;
    friend class DOMImplementation;
public:
    DocumentType();
    DocumentType(const DocumentType &other);

    DocumentType(const Node &other) : Node()
         {(*this)=other;}
    DocumentType & operator = (const Node &other);
    DocumentType & operator = (const DocumentType &other);

    ~DocumentType();

    /**
     * The name of DTD; i.e., the name immediately following the
     * \c DOCTYPE keyword.
     *
     */
    DOMString name() const;

    /**
     * A \c NamedNodeMap containing the general entities,
     * both external and internal, declared in the DTD. Duplicates are
     * discarded. For example in: &lt;!DOCTYPE ex SYSTEM "ex.dtd" [
     * &lt;!ENTITY foo "foo"> &lt;!ENTITY bar "bar"> &lt;!ENTITY % baz
     * "baz"> ]> &lt;ex/> the interface provides access to \c foo
     * and \c bar but not \c baz .
     * Every node in this map also implements the \c Entity
     * interface.
     *
     *  The DOM Level 1 does not support editing entities, therefore
     * \c entities cannot be altered in any way.
     *
     */
    NamedNodeMap entities() const;

    /**
     * A \c NamedNodeMap containing the notations declared
     * in the DTD. Duplicates are discarded. Every node in this map
     * also implements the \c Notation interface.
     *
     *  The DOM Level 1 does not support editing notations, therefore
     * \c notations cannot be altered in any way.
     *
     */
    NamedNodeMap notations() const;

    /**
     * Introduced in DOM Level 2
     *
     * The public identifier of the external subset.
     */
    DOMString publicId() const;

    /**
     * Introduced in DOM Level 2
     *
     * The system identifier of the external subset.
     */
    DOMString systemId() const;

    /**
     * Introduced in DOM Level 2
     *
     * The internal subset as a string.
     *
     * Note: The actual content returned depends on how much information is
     * available to the implementation. This may vary depending on various
     * parameters, including the XML processor used to build the document.
     */
    DOMString internalSubset() const;

protected:
    DocumentType(DocumentTypeImpl *impl);
};

} //namespace
#endif
