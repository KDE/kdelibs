/*
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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
 * This file includes excerpts from the Document Object Model (DOM)
 * Level 1 Specification (Recommendation)
 * http://www.w3.org/TR/REC-DOM-Level-1/
 * Copyright � World Wide Web Consortium , (Massachusetts Institute of
 * Technology , Institut National de Recherche en Informatique et en
 * Automatique , Keio University ). All Rights Reserved.
 *
 */
#ifndef _DOM_Node_h_
#define _DOM_Node_h_

#include <qstring.h>
#include <kdemacros.h>

class QRect;

namespace KJS {
    class HTMLDocument;
    class Window;
}
namespace DOM {

class Node;
class DOMString;
class NodeImpl;
class NamedNodeMapImpl;
class EventListener;
class Event;

/**
 * Objects implementing the \c NamedNodeMap interface are
 * used to represent collections of nodes that can be accessed by
 * name. Note that \c NamedNodeMap does not inherit from
 * \c NodeList ; \c NamedNodeMap s are not
 * maintained in any particular order. Objects contained in an object
 * implementing \c NamedNodeMap may also be accessed by an
 * ordinal index, but this is simply to allow convenient enumeration
 * of the contents of a \c NamedNodeMap , and does not
 * imply that the DOM specifies an order to these Nodes.
 *
 */
class NamedNodeMap
{
public:
    NamedNodeMap();
    NamedNodeMap(const NamedNodeMap &other);

    NamedNodeMap & operator = (const NamedNodeMap &other);

    ~NamedNodeMap();

    /**
     * The number of nodes in the map. The range of valid child node
     * indices is 0 to \c length-1 inclusive.
     *
     */
    unsigned long length() const;

    /**
     * Retrieves a node specified by name.
     *
     * @param name Name of a node to retrieve.
     *
     * @return A \c Node (of any type) with the specified
     * name, or \c null if the specified name did not
     * identify any node in the map.
     *
     */
    Node getNamedItem ( const DOMString &name ) const;

    /**
     * Adds a node using its \c nodeName attribute.
     *
     *  As the \c nodeName attribute is used to derive the
     * name which the node must be stored under, multiple nodes of
     * certain types (those that have a "special" string value) cannot
     * be stored as the names would clash. This is seen as preferable
     * to allowing nodes to be aliased.
     *
     * @param arg A node to store in a named node map. The node will
     * later be accessible using the value of the \c nodeName
     * attribute of the node. If a node with that name is
     * already present in the map, it is replaced by the new one.
     *
     * @return If the new \c Node replaces an existing
     * node with the same name the previously existing \c Node
     * is returned, otherwise \c null is returned.
     *
     * @exception DOMException
     * WRONG_DOCUMENT_ERR: Raised if \c arg was created
     * from a different document than the one that created the
     * \c NamedNodeMap .
     *
     *  NO_MODIFICATION_ALLOWED_ERR: Raised if this
     * \c NamedNodeMap is readonly.
     *
     *  INUSE_ATTRIBUTE_ERR: Raised if \c arg is an
     * \c Attr that is already an attribute of another
     * \c Element object. The DOM user must explicitly clone
     * \c Attr nodes to re-use them in other elements.
     *
     */
    Node setNamedItem ( const Node &arg );

    /**
     * Removes a node specified by name. If the removed node is an
     * \c Attr with a default value it is immediately
     * replaced.
     *
     * @param name The name of a node to remove.
     *
     * @return The node removed from the map or \c null if
     * no node with such a name exists.
     *
     * @exception DOMException
     * NOT_FOUND_ERR: Raised if there is no node named \c name
     * in the map.
     *
     */
    Node removeNamedItem ( const DOMString &name );

    /**
     * Returns the \c index th item in the map. If
     * \c index is greater than or equal to the number of nodes
     * in the map, this returns \c null .
     *
     * @param index Index into the map.
     *
     * @return The node at the \c index th position in the
     * \c NamedNodeMap , or \c null if that is
     * not a valid index.
     *
     */
    Node item ( unsigned long index ) const;

    /**
     * Introduced in DOM Level 2
     *
     * Retrieves a node specified by local name and namespace URI. HTML-only
     * DOM implementations do not need to implement this method.
     *
     * @param namespaceURI The namespace URI of the node to retrieve.
     *
     * @param localName The local name of the node to retrieve.
     *
     * @return A Node (of any type) with the specified local name and namespace
     * URI, or null if they do not identify any node in this map.
     */
    Node getNamedItemNS( const DOMString &namespaceURI,
                         const DOMString &localName ) const;

    /**
     * Introduced in DOM Level 2
     *
     * Adds a node using its namespaceURI and localName. If a node with that
     * namespace URI and that local name is already present in this map, it is
     * replaced by the new one.
     * HTML-only DOM implementations do not need to implement this method.
     *
     * @param arg A node to store in this map. The node will later be
     * accessible using the value of its namespaceURI and localName attributes.
     *
     * @return If the new Node replaces an existing node the replaced Node is
     * returned, otherwise null is returned.
     *
     * @exception DOMException
     * WRONG_DOCUMENT_ERR: Raised if arg was created from a different document
     * than the one that created this map.
     *
     * NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly.
     *
     * INUSE_ATTRIBUTE_ERR: Raised if arg is an Attr that is already an
     * attribute of another Element object. The DOM user must explicitly clone
     * Attr nodes to re-use them in other elements.
     */
    Node setNamedItemNS( const Node &arg );

    /**
     * Introduced in DOM Level 2
     *
     * Removes a node specified by local name and namespace URI. A removed
     * attribute may be known to have a default value when this map contains
     * the attributes attached to an element, as returned by the attributes
     * attribute of the Node interface. If so, an attribute immediately appears
     * containing the default value as well as the corresponding namespace URI,
     * local name, and prefix when applicable.
     * HTML-only DOM implementations do not need to implement this method.
     *
     * @param namespaceURI The namespace URI of the node to remove.
     *
     * @param localName The local name of the node to remove.
     *
     * @return The node removed from this map if a node with such a local name
     * and namespace URI exists.
     *
     * @exception DOMException
     * NOT_FOUND_ERR: Raised if there is no node with the specified
     * namespaceURI and localName in this map.
     *
     * NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly.
     */
    Node removeNamedItemNS( const DOMString &namespaceURI,
                            const DOMString &localName );

    /**
     * @internal
     * not part of the DOM
     */
    NamedNodeMapImpl *handle() const { return impl; }
    bool isNull() const { return !impl; }

protected:
    NamedNodeMap( NamedNodeMapImpl *i);
    NamedNodeMapImpl *impl;

    friend class Node;
    friend class DocumentType;
    friend class NodeImpl;
};

class NamedNodeMap;
class NodeList;
class Document;
class DOMString;
class StyleSheet;

class NodeImpl;

/**
 * The \c Node interface is the primary datatype for the
 * entire Document Object Model. It represents a single node in the
 * document tree. While all objects implementing the \c Node
 * interface expose methods for dealing with children, not all
 * objects implementing the \c Node interface may have
 * children. For example, \c Text nodes may not have
 * children, and adding children to such nodes results in a
 * \c DOMException being raised.
 *
 *  The attributes \c nodeName , \c nodeValue
 * and \c attributes are included as a mechanism to get at
 * node information without casting down to the specific derived
 * interface. In cases where there is no obvious mapping of these
 * attributes for a specific \c nodeType (e.g.,
 * \c nodeValue for an Element or \c attributes for a
 * Comment), this returns \c null . Note that the
 * specialized interfaces may contain additional and more convenient
 * mechanisms to get and set the relevant information.
 *
 */
class Node
{
    friend class NamedNodeMap;
    friend class NodeList;
    friend class HTMLCollection;
    friend class StyleSheet;

public:
    Node() : impl(0) {}
    Node(const Node &other);

    /**
     * @internal
     */
    Node( NodeImpl *_impl);

    Node & operator = (const Node &other);

    bool operator == (const Node &other) const;

    bool operator != (const Node &other) const;

    virtual ~Node();
    /**
     * An integer indicating which type of node this is.
     *
     *
     * <p>The values of \c nodeName, \c nodeValue,
     *  and \c attributes vary according to the node type as follows:
     *   <table  border="1">
     *     <tr>
     *       <td></td>
     *       <td>nodeName</td>
     *       <td>nodeValue</td>
     *       <td>attributes</td>
     *     </tr>
     *     <tr>
     *       <td>Element</td>
     *       <td>tagName</td>
     *       <td>null</td>
     *       <td>NamedNodeMap</td>
     *     </tr>
     *     <tr>
     *       <td>Attr</td>
     *       <td>name of attribute</td>
     *       <td>value of attribute</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>Text</td>
     *       <td>#text</td>
     *       <td>content of the text node</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>CDATASection</td>
     *       <td>#cdata-section</td>
     *       <td>content of the CDATA Section</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>EntityReference</td>
     *       <td>name of entity referenced</td>
     *       <td>null</td>
     *       <td>null</td>
     *       </tr>
     *     <tr>
     *       <td>Entity</td>
     *       <td>entity name</td>
     *       <td>null</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>ProcessingInstruction</td>
     *       <td>target</td>
     *       <td>entire content excluding the target</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>Comment</td>
     *       <td>#comment</td>
     *       <td>content of the comment</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>Document</td>
     *       <td>#document</td>
     *       <td>null</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>DocumentType</td>
     *       <td>document type name</td>
     *       <td>null</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>DocumentFragment</td>
     *       <td>#document-fragment</td>
     *       <td>null</td>
     *       <td>null</td>
     *     </tr>
     *     <tr>
     *       <td>Notation</td>
     *       <td>notation name</td>
     *       <td>null</td>
     *       <td>null</td>
     *     </tr>
     *   </table>
     * </p>
     */
    enum NodeType {
        ELEMENT_NODE = 1,
        ATTRIBUTE_NODE = 2,
        TEXT_NODE = 3,
        CDATA_SECTION_NODE = 4,
        ENTITY_REFERENCE_NODE = 5,
        ENTITY_NODE = 6,
        PROCESSING_INSTRUCTION_NODE = 7,
        COMMENT_NODE = 8,
        DOCUMENT_NODE = 9,
        DOCUMENT_TYPE_NODE = 10,
        DOCUMENT_FRAGMENT_NODE = 11,
        NOTATION_NODE = 12
    };

    /**
     * The name of this node, depending on its type; see the table
     * above.
     *
     */
    DOMString nodeName() const;

    /**
     * The value of this node, depending on its type; see the table
     * above.
     *
     * @exception DOMException
     * DOMSTRING_SIZE_ERR: Raised when it would return more characters
     * than fit in a \c DOMString variable on the
     * implementation platform.
     *
     */
    DOMString nodeValue() const;

    /**
     * see nodeValue
     * @exception DOMException
     * NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     *
     */
    void setNodeValue( const DOMString & );

    /**
     * A code representing the type of the underlying object, as
     * defined above.
     *
     */
    unsigned short nodeType() const;

    /**
     * The parent of this node. All nodes, except \c Document
     * , \c DocumentFragment , and \c Attr
     * may have a parent. However, if a node has just been
     * created and not yet added to the tree, or if it has been
     * removed from the tree, this is \c null .
     *
     */
    Node parentNode() const;

    /**
     * A \c NodeList that contains all children of this
     * node. If there are no children, this is a \c NodeList
     * containing no nodes. The content of the returned
     * \c NodeList is &quot;live&quot; in the sense that, for
     * instance, changes to the children of the node object that it
     * was created from are immediately reflected in the nodes
     * returned by the \c NodeList accessors; it is not a
     * static snapshot of the content of the node. This is true for
     * every \c NodeList , including the ones returned by
     * the \c getElementsByTagName method.
     *
     */
    NodeList childNodes() const;

    /**
     * The first child of this node. If there is no such node, this
     * returns \c null .
     *
     */
    Node firstChild() const;

    /**
     * The last child of this node. If there is no such node, this
     * returns \c null .
     *
     */
    Node lastChild() const;

    /**
     * The node immediately preceding this node. If there is no such
     * node, this returns \c null .
     *
     */
    Node previousSibling() const;

    /**
     * The node immediately following this node. If there is no such
     * node, this returns \c null .
     *
     */
    Node nextSibling() const;

    /**
     * A \c NamedNodeMap containing the attributes of this
     * node (if it is an \c Element ) or \c null
     * otherwise.
     *
     */
    NamedNodeMap attributes() const;

    /**
     * The \c Document object associated with this node.
     * This is also the \c Document object used to create
     * new nodes. When this node is a \c Document this is
     * \c null .
     *
     */
    Document ownerDocument() const;

    /**
     * Inserts the node \c newChild before the existing
     * child node \c refChild . If \c refChild
     * is \c null , insert \c newChild at the
     * end of the list of children.
     *
     *  If \c newChild is a \c DocumentFragment
     * object, all of its children are inserted, in the same
     * order, before \c refChild . If the \c newChild
     * is already in the tree, it is first removed.
     *
     * @param newChild The node to insert.
     *
     * @param refChild The reference node, i.e., the node before which
     * the new node must be inserted.
     *
     * @return The node being inserted.
     *
     * @exception DOMException
     * HIERARCHY_REQUEST_ERR: Raised if this node is of a type that
     * does not allow children of the type of the \c newChild
     * node, or if the node to insert is one of this node's
     * ancestors.
     *
     *  WRONG_DOCUMENT_ERR: Raised if \c newChild was
     * created from a different document than the one that created
     * this node.
     *
     *  NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *
     *  NOT_FOUND_ERR: Raised if \c refChild is not a
     * child of this node.
     *
     */
    Node insertBefore ( const Node &newChild, const Node &refChild );

    /**
     * Replaces the child node \c oldChild with
     * \c newChild in the list of children, and returns the
     * \c oldChild node. If the \c newChild is
     * already in the tree, it is first removed.
     *
     * @param newChild The new node to put in the child list.
     *
     * @param oldChild The node being replaced in the list.
     *
     * @return The node replaced.
     *
     * @exception DOMException
     * HIERARCHY_REQUEST_ERR: Raised if this node is of a type that
     * does not allow children of the type of the \c newChild
     * node, or it the node to put in is one of this node's
     * ancestors.
     *
     *  WRONG_DOCUMENT_ERR: Raised if \c newChild was
     * created from a different document than the one that created
     * this node.
     *
     *  NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *
     *  NOT_FOUND_ERR: Raised if \c oldChild is not a
     * child of this node.
     *
     */
    Node replaceChild ( const Node &newChild, const Node &oldChild );

    /**
     * Removes the child node indicated by \c oldChild
     * from the list of children, and returns it.
     *
     * @param oldChild The node being removed.
     *
     * @return The node removed.
     *
     * @exception DOMException
     * NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *
     *  NOT_FOUND_ERR: Raised if \c oldChild is not a
     * child of this node.
     *
     */
    Node removeChild ( const Node &oldChild );

    /**
     * Adds the node \c newChild to the end of the list of
     * children of this node. If the \c newChild is
     * already in the tree, it is first removed.
     *
     * @param newChild The node to add.
     *
     *  If it is a \c DocumentFragment object, the entire
     * contents of the document fragment are moved into the child list
     * of this node
     *
     * @return The node added.
     *
     * @exception DOMException
     * HIERARCHY_REQUEST_ERR: Raised if this node is of a type that
     * does not allow children of the type of the \c newChild
     * node, or if the node to append is one of this node's
     * ancestors.
     *
     *  WRONG_DOCUMENT_ERR: Raised if \c newChild was
     * created from a different document than the one that created
     * this node.
     *
     *  NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *
     */
    Node appendChild ( const Node &newChild );

    /**
     * This is a convenience method to allow easy determination of
     * whether a node has any children.
     *
     * @return \c true if the node has any children,
     * \c false if the node has no children.
     *
     */
    bool hasChildNodes (  );

    /**
     * Returns a duplicate of this node, i.e., serves as a generic
     * copy constructor for nodes. The duplicate node has no parent (
     * \c parentNode returns \c null .).
     *
     *  Cloning an \c Element copies all attributes and
     * their values, including those generated by the XML processor to
     * represent defaulted attributes, but this method does not copy
     * any text it contains unless it is a deep clone, since the text
     * is contained in a child \c Text node. Cloning any
     * other type of node simply returns a copy of this node.
     *
     * @param deep If \c true , recursively clone the
     * subtree under the specified node; if \c false ,
     * clone only the node itself (and its attributes, if it is an
     * \c Element ).
     *
     * @return The duplicate node.
     *
     */
    Node cloneNode ( bool deep );

    /**
     * Modified in DOM Level 2
     *
     * Puts all Text nodes in the full depth of the sub-tree underneath this
     * Node, including attribute nodes, into a "normal" form where only
     * structure (e.g., elements, comments, processing instructions, CDATA
     * sections, and entity references) separates Text nodes, i.e., there are
     * neither adjacent Text nodes nor empty Text nodes. This can be used to
     * ensure that the DOM view of a document is the same as if it were saved
     * and re-loaded, and is useful when operations (such as XPointer
     * [XPointer] lookups) that depend on a particular document tree structure
     * are to be used.
     *
     * Note: In cases where the document contains CDATASections, the normalize
     * operation alone may not be sufficient, since XPointers do not
     * differentiate between Text nodes and CDATASection nodes.
     */
    void normalize (  );

    /**
     * Introduced in DOM Level 2
     *
     * Tests whether the DOM implementation implements a specific feature and
     * that feature is supported by this node.
     *
     * @param feature The name of the feature to test. This is the same name
     * which can be passed to the method hasFeature on DOMImplementation.
     *
     * @param version This is the version number of the feature to test. In
     * Level 2, version 1, this is the string "2.0". If the version is not
     * specified, supporting any version of the feature will cause the method
     * to return true.
     *
     * @return Returns true if the specified feature is supported on this node,
     * false otherwise.
     */
    bool isSupported( const DOMString &feature,
                      const DOMString &version ) const;

    /**
     * Introduced in DOM Level 2
     *
     * The namespace URI of this node, or null if it is unspecified.
     * This is not a computed value that is the result of a namespace lookup
     * based on an examination of the namespace declarations in scope. It is
     * merely the namespace URI given at creation time. For nodes of any type
     * other than ELEMENT_NODE and ATTRIBUTE_NODE and nodes created with a DOM
     * Level 1 method, such as createElement from the Document interface, this
     * is always null.
     *
     * Note: Per the Namespaces in XML Specification [Namespaces] an attribute
     * does not inherit its namespace from the element it is attached to. If an
     * attribute is not explicitly given a namespace, it simply has no
     * namespace.
     */
    DOMString namespaceURI(  ) const;

    /**
     * Introduced in DOM Level 2
     *
     * The namespace prefix of this node, or null if it is unspecified.
     * Note that setting this attribute, when permitted, changes the nodeName
     * attribute, which holds the qualified name, as well as the tagName and
     * name attributes of the Element and Attr interfaces, when applicable.
     * Note also that changing the prefix of an attribute that is known to have
     * a default value, does not make a new attribute with the default value
     * and the original prefix appear, since the namespaceURI and localName do
     * not change.
     * For nodes of any type other than ELEMENT_NODE and ATTRIBUTE_NODE and
     * nodes created with a DOM Level 1 method, such as createElement from the
     * Document interface, this is always null.
     */
    DOMString prefix(  ) const;

    /**
     * see prefix
     *
     * @exception DOMException
     * INVALID_CHARACTER_ERR: Raised if the specified prefix contains an
     * illegal character.
     *
     * NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *
     * NAMESPACE_ERR: Raised if the specified prefix is malformed, if the
     * namespaceURI of this node is null, if the specified prefix is "xml" and
     * the namespaceURI of this node is different from
     * "http://www.w3.org/XML/1998/namespace", if this node is an attribute and
     * the specified prefix is "xmlns" and the namespaceURI of this node is
     * different from "http://www.w3.org/2000/xmlns/", or if this node is an
     * attribute and the qualifiedName of this node is "xmlns" [Namespaces].
     */
    void setPrefix(const DOMString &prefix );

    /**
     * Introduced in DOM Level 2
     *
     * Returns the local part of the qualified name of this node.
     * For nodes of any type other than ELEMENT_NODE and ATTRIBUTE_NODE and
     * nodes created with a DOM Level 1 method, such as createElement from the
     * Document interface, this is always null.
     */
    DOMString localName(  ) const;

    /**
     * Returns whether this node (if it is an element) has any attributes.
     * @return a boolean. True if this node has any attributes, false otherwise.
     *  Introduced in DOM Level 2
     */
    bool hasAttributes (  );

    /**
     * Introduced in DOM Level 2
     * This method is from the EventTarget interface
     *
     * This method allows the registration of event listeners on the event
     * target. If an EventListener is added to an EventTarget while it is
     * processing an event, it will not be triggered by the current actions but
     * may be triggered during a later stage of event flow, such as the
     * bubbling phase.
     *
     * If multiple identical EventListeners are registered on the same
     * EventTarget with the same parameters the duplicate instances are
     * discarded. They do not cause the EventListener to be called twice and
     * since they are discarded they do not need to be removed with the
     * removeEventListener method. Parameters
     *
     * @param type The event type for which the user is registering
     *
     * @param listener The listener parameter takes an interface implemented by
     * the user which contains the methods to be called when the event occurs.
     *
     * @param useCapture If true, useCapture indicates that the user wishes to
     * initiate capture. After initiating capture, all events of the specified
     * type will be dispatched to the registered EventListener before being
     * dispatched to any EventTargets beneath them in the tree. Events which
     * are bubbling upward through the tree will not trigger an EventListener
     * designated to use capture.
     */
    void addEventListener(const DOMString &type,
			  EventListener *listener,
			  const bool useCapture);

    /**
     * Introduced in DOM Level 2
     * This method is from the EventTarget interface
     *
     * This method allows the removal of event listeners from the event target.
     * If an EventListener is removed from an EventTarget while it is
     * processing an event, it will not be triggered by the current actions.
     *
     * EventListeners can never be invoked after being removed.
     *
     * Calling removeEventListener with arguments which do not identify any
     * currently registered EventListener on the EventTarget has no effect.
     *
     * @param type Specifies the event type of the EventListener being removed.
     *
     * @param listener The EventListener parameter indicates the EventListener
     * to be removed.
     *
     * @param useCapture Specifies whether the EventListener being removed was
     * registered as a capturing listener or not. If a listener was registered
     * twice, one with capture and one without, each must be removed
     * separately. Removal of a capturing listener does not affect a
     * non-capturing version of the same listener, and vice versa.
     */

    void removeEventListener(const DOMString &type,
			     EventListener *listener,
			     bool useCapture);

    /**
     * Introduced in DOM Level 2
     * This method is from the EventTarget interface
     *
     * This method allows the dispatch of events into the implementations event
     * model. Events dispatched in this manner will have the same capturing and
     * bubbling behavior as events dispatched directly by the implementation.
     * The target of the event is the EventTarget on which dispatchEvent is
     * called.
     *
     * @param evt Specifies the event type, behavior, and contextual
     * information to be used in processing the event.
     *
     * @return The return value of dispatchEvent indicates whether any of the
     * listeners which handled the event called preventDefault. If
     * preventDefault was called the value is false, else the value is true.
     *
     * @exception EventException
     * UNSPECIFIED_EVENT_TYPE_ERR: Raised if the Event's type was not specified
     * by initializing the event before dispatchEvent was called. Specification
     * of the Event's type as null or an empty string will also trigger this
     * exception.
     */
    bool dispatchEvent(const Event &evt);

    /**
     * @internal
     * not part of the DOM.
     * @returns the element id, in case this is an element, 0 otherwise
     */
    Q_UINT32 elementId() const;

    /**
     * tests if this Node is 0. Useful especially, if casting to a derived
     * class:
     *
     * \code
     * Node n = .....;
     * // try to convert into an Element:
     * Element e = n;
     * if( e.isNull() )
     *   kdDebug(300) << "node isn't an element node" << endl;
     * \endcode
     */
    bool isNull() const { return !impl; }

    /**
     * @internal handle to the implementation object
     */
    NodeImpl *handle() const { return impl; }

    /**
     * @internal returns the index of a node
     */
    unsigned long index() const;
    QString toHTML();
    void applyChanges();
    /**
     * @deprecated without substitution since 3.2
     */
    void getCursor(int offset, int &_x, int &_y, int &height) KDE_DEPRECATED;
    /**
     * not part of the DOM.
     * @returns the exact coordinates and size of this element.
     */
    QRect getRect();

protected:
    NodeImpl *impl;
};


class NodeListImpl;

/**
 * The \c NodeList interface provides the abstraction of
 * an ordered collection of nodes, without defining or constraining
 * how this collection is implemented.
 *
 *  The items in the \c NodeList are accessible via an
 * integral index, starting from 0.
 *
 */
class NodeList
{
    friend class Element;
    friend class Node;
    friend class Document;
    friend class HTMLDocument;
    friend class KJS::HTMLDocument;
    friend class KJS::Window;

public:
    NodeList();
    NodeList(const NodeList &other);

    NodeList & operator = (const NodeList &other);

    ~NodeList();

    /**
     * The number of nodes in the list. The range of valid child node
     * indices is 0 to \c length-1 inclusive.
     *
     */
    unsigned long length() const;

    /**
     * Returns the \c index th item in the collection. If
     * \c index is greater than or equal to the number of
     * nodes in the list, this returns \c null .
     *
     * @param index Index into the collection.
     *
     * @return The node at the \c index th position in the
     * \c NodeList , or \c null if that is not
     * a valid index.
     *
     */
    Node item ( unsigned long index ) const;

    /**
     * @internal
     * not part of the DOM
     */
    NodeListImpl *handle() const { return impl; }
    bool isNull() const { return !impl; }

protected:
    NodeList(const NodeListImpl *i);
    NodeListImpl *impl;
};



/**
 * A DOMTimeStamp represents a number of milliseconds.
 *
 */
typedef unsigned long long DOMTimeStamp;


} //namespace
#endif
